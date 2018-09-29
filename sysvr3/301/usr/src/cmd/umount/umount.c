/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)umount:umount.c	1.19"
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <mnttab.h>
#include <unistd.h>
#include <time.h>

#define EQ(a,b)		(!strcmp(a,b))
#define LOCAL(x)	(!((x) & 2))
#define REMOTE(x)	((x) & 2)
#define ISDIR(x)	((x & S_IFMT) == S_IFDIR)

#define MAXFIELD	4
#define MINFIELD	2

#define MNTTAB		"/etc/mnttab"
#define FSTAB		"/etc/fstab"
#define SEM_FILE	"/etc/.mnt.lock"

static int   distflag = 0;

static char fsbuf[BUFSIZ];
static char *fieldv[MAXFIELD];

extern char *getcwd();
extern char *strtok();

extern int errno;

main(argc, argv)
int  argc;
char **argv;
{
	char	*usage = "usage: umount [-d] special\n";
	register char	*special;
	int 	 equal_flag = 0;
	int	 num_left = 0;
	int	 rec, temprec;
	int	 field;
	int	 pr_warn = 1;

	dev_t	 r_rdev;
	dev_t	 r_dev;
	ino_t	 r_ino;

	char	 buf[32];

	struct	 stat sbuf;
	struct	 mnttab *mtab;
	struct	 mnttab *end_of_mtab;
	register struct mnttab *mp;

	/*
	 *	Process arguments...
	 */

	switch (argc) {
		case 3:
			if (strcmp(argv[1], "-d") == 0)
				distflag = 1;
			else {
				fprintf(stderr, "%s", usage);
				exit(2);
			}
			/*
			 *	Fall through to set special...
			 */
		case 2:
			special = argv[argc - 1];
			break;
		default:
			fprintf(stderr, "%s", usage);
			exit(2);
	}

	if (geteuid() != 0) {
		fprintf(stderr, "umount: not super user\n");
		exit(2);
	}

	/*
	 *	Lock a temporary file to prevent many unmounts at once.
	 *	This is done to ensure integrity of the MNTTAB.
	 */

	if ((temprec = creat(SEM_FILE, 0600)) == -1 ||
	     lockf(temprec, F_LOCK, 0L) < 0) {
		fprintf(stderr, "mount: warning: cannot lock temp file <%s>\n", SEM_FILE);
	}

	/*
	 *	If the argument provided is a directory
	 *	(or the stat fails because a link has been broken,
	 *	then look for the special device name from the
	 *	/etc/fstab file.
	 */

	if ((stat(special, &sbuf) == 0 && ISDIR(sbuf.st_mode))
	|| (stat(special, &sbuf) < 0 && errno == ENOLINK)) {
		if (parse(special)) {
			special = fieldv[0];
			if (distflag)
				printf("umount -d %s\n", special);
		}
	}

	if (distflag) {
		if (rumount(special) < 0) {
			rpterr(special);
			/*
			 *	If the rumount failed due to a link
			 *	being down, continue and remove the
			 *	remote resource from MNTTAB.
			 */
			if (errno != ECOMM) {
				exit(2);
			}
		}
	} else {
		if (umount(special) < 0) {
			rpterr(special);
			if (errno != EINVAL) {
				exit(2);
			}
			pr_warn = 0;
		}
	}

	if (!distflag) {

		/*
		 *	get the device number of special so
		 *	it can be removed from MNTTAB.
		 */

		field = 2;
		if (stat(special, &sbuf) < 0) {
			r_rdev = -1;
		} else {
			if ((sbuf.st_mode & S_IFMT) == S_IFBLK) {
				r_rdev = sbuf.st_rdev;
			} else {
				r_ino = sbuf.st_ino;
				r_dev = sbuf.st_dev;
				field = 1;
			}
		}

		/*
		 *	Generate a full path name if necessary.
		 */

		if (*special != '/') {
			if (getcwd(buf,sizeof(buf)) == NULL)
				fprintf(stderr,"umount: warning: cannot get current directory\n");
			strncat(buf,"/",sizeof(buf)-strlen(buf));
			strncat(buf,special,sizeof(buf)-strlen(buf));
			if (buf[sizeof(buf) - 1] != '\0')
				buf[sizeof(buf) -1] = '\0';
		}
	}

	if ((rec = open(MNTTAB, O_RDONLY)) < 0) {
		fprintf(stderr, "umount: warning: cannot open %s\n", MNTTAB);
		exit(0);
	}

	if (fstat(rec, &sbuf) < 0) {
		fprintf(stderr, "umount: warning: cannot stat %s\n", MNTTAB);
		exit(0);
	}

	if ((mtab = (struct mnttab *)malloc(sbuf.st_size)) == 0) {
		fprintf(stderr, "umount: warning: cannot allocate space for %s\n", MNTTAB);
		exit(0);
	}

	if (read(rec, mtab, sbuf.st_size) != sbuf.st_size) {
		fprintf(stderr, "umount: warning: read error on %s\n", MNTTAB);
		exit(0);
	}

	end_of_mtab = (struct mnttab *)((char *)mtab + sbuf.st_size);

	for (mp = mtab; mp < end_of_mtab; mp++) {
		if (distflag)
			equal_flag = EQ(special, mp->mt_dev) &&
				     REMOTE(mp->mt_ro_flg);
		else {
			if (field == 2) {
				if (stat(mp->mt_dev, &sbuf) < 0)
					equal_flag = EQ(special, mp->mt_dev) &&
					     	     LOCAL(mp->mt_ro_flg);
				else
					equal_flag = (sbuf.st_rdev == r_rdev) &&
					     	     LOCAL(mp->mt_ro_flg);
			} else {
				if (stat(mp->mt_filsys, &sbuf) < 0)
					equal_flag = EQ(special, mp->mt_filsys) &&
					     	     LOCAL(mp->mt_ro_flg);
				else
					equal_flag = (sbuf.st_ino == r_ino) &&
						     (sbuf.st_dev == r_dev) &&
					     	     LOCAL(mp->mt_ro_flg);
			}
		}
		if (equal_flag) {
			rec = creat(MNTTAB, 0644);
			if ((mp - mtab) > 0)
				write(rec, mtab, (mp - mtab) * sizeof (struct mnttab));
			mp ++;
			num_left = end_of_mtab - mp;
			if (num_left > 0)
				write(rec, mp, num_left * sizeof (struct mnttab));
			exit(0);
		}
	}

	if (pr_warn)
		fprintf(stderr, "warning: %s was not in mount table\n", special);
	exit(0);
}

static
rpterr(s)
char *s;
{
	switch(errno){
	case EPERM:
		fprintf(stderr,"umount: not super user\n");
		break;
	case ENXIO:
		fprintf(stderr,"umount: %s no device\n",s);
		break;
	case ENOENT:
		fprintf(stderr,"umount: %s no such file or directory\n",s);
		break;
	case EINVAL:
		fprintf(stderr,"umount: %s not mounted\n",s);
		break;
	case EBUSY:
		fprintf(stderr,"umount: %s busy\n",s);
		break;
	case ENOTBLK:
		fprintf(stderr,"umount: %s block device required\n",s);
		break;
	case ECOMM:
		fprintf(stderr,"umount: warning: broken link detected\n");
		break;
	default:
		perror("umount");
		fprintf(stderr,"umount: cannot unmount %s\n",s);
	}
}


/*
 *	parse() takes the given directory and returns the
 *	fstab entry for that directory.  fieldv[0] will
 *	contain the entry for the special file associated with
 *	the directory.
 */

static
parse(dir)
char	*dir;
{

	FILE	*fd;
	int	i, found = 0;
	int	num_fields;
	char	c;

	if ((fd = fopen(FSTAB,"r")) == NULL)
		return(0);

	while (fgets(fsbuf,sizeof(fsbuf),fd)) {

		for (i = 0; i < MAXFIELD; i++)
			fieldv[i] = NULL;

		num_fields = get_fields(fsbuf);
		if (num_fields < MINFIELD || fieldv[0][0] == '#' )
			continue;

		if (strcmp(dir,fieldv[1]) == 0) {
			found = 1;	
			break;
		}
	}

	/*
	 *	If the third field is the "flags" field,
	 *	then set the specified flags.  Since the
	 *	third field is expected to point to the
	 *	fstype, set the third field to the fourth.
	 */

	if (found) {
		if (num_fields > 2 && fieldv[2][0] == '-') {
			i = 1;
			while (c = fieldv[2][i]) {
				if (c == 'd')
					distflag = 1;
				i ++;
			}
		}
	}

	fclose(fd);
	return(found);
}

static
get_fields(s)
char	*s;
{
	int fieldc = 0;

	if ((fieldv[fieldc] = strtok(s, " \t\n")) == NULL)
		return(fieldc);

	fieldc ++;
	while (fieldc < MAXFIELD
	   && (fieldv[fieldc] = strtok(NULL, " \t\n")) != NULL)
		fieldc ++;

	return(fieldc);
}
