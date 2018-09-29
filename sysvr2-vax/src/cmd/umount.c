/*	@(#)umount.c	1.4	*/
#include <stdio.h>
#include <sys/types.h>
#include <mnttab.h>
#include <sys/errno.h>
#define EQ(a,b) (!strcmp(a,b))
#define MNTTAB "/etc/mnttab"

extern char *getcwd();

extern int errno;
struct mnttab mtab[NMOUNT], *mp;

main(argc, argv)
char **argv;
{
	register char *np;
	char buf[128];
	int rec;

	sync();
	rec = open(MNTTAB,0);
	if(rec < 0) {
		fprintf(stderr, "umount: cannot open MNTTAB\n");
		exit(2);
	}
	read(rec, mtab, sizeof mtab);
	if(argc != 2) {
		fprintf(stderr, "usage: umount special\n");
		exit(2);
	}

	if (umount(argv[1]) < 0) {
		rpterr(argv[1]);
		exit(2);
	}

	np = argv[1];
	while(*np++);
	np--;
	while(*--np == '/') *np = '\0';
	while(np > argv[1] && *--np != '/');
		/* most times things are mounted on special files in /dev/dsk.
		   Ifso, we can strip "/dev/dsk/" off the device name in the 
		   mount table. */
	if(strncmp(argv[1],"/dev/dsk/",9) == 0) np = &argv[1][9];

		/*  Otherwise use the complete file string */
	else {
		/* if the full path name was passed in, we have it made */
		if(*argv[1] == '/') np = argv[1];

			/* Otherwise, generate the full path name */
		else{
			if((np = getcwd(buf,128)) == NULL){
				fprintf(stderr,"umount: cannot get ");
				fprintf(stderr,"current directory\n");
				exit(2);
			}
			strncat(np,"/",126);
			strncat(np,argv[1],127-strlen(argv[1]));
			if(strlen(np) > 31) np[31] = '\0';
		}
		
	}
	for (mp = mtab; mp < &mtab[NMOUNT]; mp++) {
		if(EQ(np, mp->mt_dev)) {
			mp->mt_dev[0] = '\0';
			mp->mt_filsys[0] = '\0';
			time(&mp->mt_time);
			mp = &mtab[NMOUNT];
			while ((--mp)->mt_dev[0] == '\0');
			rec = creat(MNTTAB, 0644);
			write(rec, mtab, (mp-mtab+1)*sizeof mtab[0]);
			exit(0);
		}
	}
	fprintf(stderr, "warning: %s was not in mount table\n", argv[1]);
	exit(2);
}
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
	default:
		fprintf(stderr, "umount: errno %d, cannot unmount %s\n",
			errno, s);
	}
}
