/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
/*	@(#)umount.c	1.3	*/
#include <stdio.h>
#include <sys/types.h>
#include <mnttab.h>
#include <sys/errno.h>
#define EQ(a,b) (!strcmp(a,b))

extern int errno;
struct mnttab mtab[NMOUNT], *mp;

main(argc, argv)
char **argv;
{
	register char *np;
	int rec;

	sync();
	rec = open("/etc/mnttab",0);
	if(rec < 0) {
		fprintf(stderr, "umount: cannot open /etc/mnttab\n");
		exit(2);
	}
	read(rec, mtab, sizeof mtab);
	if(argc != 2) {
		fprintf(stderr, "usage: umount device\n");
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
	if ((np - argv[1] > 3 && strncmp(np-4,"/dsk/",5) == 0) ||
	    (np - argv[1] == 3 && strncmp(np-3,"dsk/",4) == 0))
		np -= 3;
	if(*np == '/') np++;
	argv[1] = np;
	for (mp = mtab; mp < &mtab[NMOUNT]; mp++) {
		if(EQ(argv[1], mp->mt_dev)) {
			mp->mt_dev[0] = '\0';
			time(&mp->mt_time);
			mp = &mtab[NMOUNT];
			while ((--mp)->mt_dev[0] == '\0');
			rec = creat("/etc/mnttab", 0644);
			write(rec, mtab, (mp-mtab+1)*sizeof mtab[0]);
			exit(0);
		}
	}
	fprintf(stderr, "warning: /dev/%s was not in mount table\n", argv[1]);
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
