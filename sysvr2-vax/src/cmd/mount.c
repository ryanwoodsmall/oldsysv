/*	@(#)mount.c	1.11	*/
#include <sys/param.h>
#ifndef RT
#include <sys/types.h>
#include <sys/sysmacros.h>
#endif
#include <sys/filsys.h>
#include <sys/errno.h>
#include <signal.h>
#include <stdio.h>
#include <mnttab.h>

extern char *getcwd();

char	*flg[] = {
	"read/write",
	"read only"
	};

#define MNTTAB "/etc/mnttab"
#define BADPATH 1
#define TRUNCAT 2

extern int errno;

struct mnttab mtab[NMOUNT], *mp;
struct filsys super;
main(argc,argv)
char **argv;
{
	int rec, p, dev, roflag = 0, stat = 0;
	register char *np;
	char	*ctime();
	char buf[128];


	rec = open(MNTTAB,0);
	if(rec < 0) {
		fprintf(stderr, "mount: cannot open %s!\n",MNTTAB);
		exit(2);
	}
	read(rec, mtab, sizeof mtab);
	if(argc == 1) {
		for(mp = mtab; mp < &mtab[NMOUNT]; mp++) {
			if(mp->mt_dev[0]) {
				printf("%.14s on ", mp->mt_filsys);
				if(mp->mt_dev[0] == '/')
					printf("%s ", mp->mt_dev);
				else
					printf("/dev/dsk/%s ", mp->mt_dev);
				printf("%s on %s",
				flg[mp->mt_ro_flg], ctime(&mp->mt_time));
			}
		}
		exit(0);
	}
		/* check for proper arguments */

	signal(SIGHUP, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	if(argc == 2) {
		fprintf(stderr, "usage: mount [special directory] [-r]\n");
		exit(2);
	}
	if(*argv[2] != '/') {
		fprintf(stderr, "usage: preceed argument with / such as : /%s\n",argv[2]);
		exit(2);
	}
	if((dev = open(argv[1],0)) <1) {
		fprintf(stderr, "mount: cannot open %s\n", argv[1]);
		exit(2);
	}
	lseek(dev, (long)SUPERBOFF, 0);
	if(read(dev, &super, sizeof(super)) < 0) {
		fprintf(stderr, "mount: read error on %s\n", argv[1]);
		exit(2);
	}
	close(dev);

	if(argc > 3) {
		if(strcmp(argv[3], "-r")) {
			fprintf(stderr, "%s: bad option\n", argv[3]);
			exit(2);
		}
		roflag = 1;
	}
	np = argv[1];	/* strip trailing '/'s if any */
	while(*np++);
	np--;
	while(*--np == '/') *np = '\0';

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
			if((np = getcwd(buf,128)) == NULL)
				stat |= BADPATH;
			strncat(np,"/",126);
			strncat(np,argv[1],127-strlen(argv[1]));
			if(strlen(np) > 31){
				np[31] = '\0';
				stat |= TRUNCAT;
			}
		}
		
	}

		/* tell system to mount device */

	if(mount(argv[1], argv[2], roflag)) {
		rpterr(argv[1], argv[2]);
		exit(2);
	}
	if(strncmp(strrchr(argv[2], '/') + 1, super.s_fname, 5))
		fprintf(stderr,"mount: warning! <%.6s> mounted as <%.6s>\n",
			super.s_fname, argv[2]);

		/* report inconsistancies in MNTTAB */

	if(stat & BADPATH)
		fprintf(stderr,"mount: cannot get current directory\n");
	if(stat & TRUNCAT)
	   fprintf(stderr,"mount: warning! pathname truncated in  %s '%s'\n",
								MNTTAB, np);
	for(mp = mtab; mp < &mtab[NMOUNT]; mp++) {
		if(!strncmp(mp->mt_dev,np,31) 
		    || !strncmp(mp->mt_filsys,argv[2],31)) {
			fprintf(stderr,"mount: warning!\t");
			if(mp->mt_dev[0] != '/') fprintf(stderr,"/dev/dsk/");
			fprintf(stderr,"%s %s already in %s\n",
				mp->mt_dev,mp->mt_filsys,MNTTAB);
		}
	}

		/* put it in the mount table */

	argv[1] = np;
	for(mp = mtab; mp < &mtab[NMOUNT]; mp++) {
		if(mp->mt_dev[0] == 0) {
			for(np = mp->mt_dev; np < &mp->mt_dev[32];)
				if((*np++ = *argv[1]++ ) == 0) argv[1]--;
			for(np = mp->mt_filsys; np < &mp->mt_filsys[32];)
				if((*np++ = *argv[2]++) == 0) argv[2]--;
			time(&mp->mt_time);
			mp->mt_ro_flg = roflag; 
			mp = &mtab[NMOUNT];
			while((--mp)->mt_dev[0] == 0);
			rec = creat(MNTTAB,0644);
			write(rec, mtab, (mp-mtab+1)*sizeof mtab[0]);
			exit(0);
		}
	}
}

rpterr(bs, mp)
register char *bs, *mp;
{
	switch(errno){
	case EPERM:
		fprintf(stderr,"mount: not super user\n");
		break;
	case ENXIO:
		fprintf(stderr,"mount: %s no such device\n",bs);
		break;
	case ENOTDIR:
		fprintf(stderr,"mount: %s not a directory\n",mp);
		break;
	case ENOENT:
		fprintf(stderr,"mount: %s no such file or directory\n",mp);
		break;
	case EINVAL:
		fprintf(stderr,"mount: %s not a valid file system\n",bs);
		break;
	case EBUSY:
		fprintf(stderr,"mount: %s or %s busy\n",bs,mp);
		break;
	case ENOTBLK:
		fprintf(stderr,"mount: %s not a block device\n",bs);
		break;
	case EROFS:
		fprintf(stderr,"mount: %s write-protected\n",bs);
		break;
	case ENOSPC:
		fprintf(stderr,"mount: %s needs checking\n",bs);
		break;
	default:
		fprintf(stderr, "mount: errno %d, cannot mount %s\n",
			errno, bs);
	}
}
