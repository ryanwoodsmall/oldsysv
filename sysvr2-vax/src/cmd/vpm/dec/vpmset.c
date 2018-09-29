/*	vpmset.c 1.11 of 11/16/82
	@(#)vpmset.c	1.11
 */



#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "sys/csi.h"
#include "sys/vpmt.h"
#include <sys/file.h>
#include "sys/csikmc.h"

static char Sccsid[] = "@(#)vpmset.c	1.11";
main(argc, argv)
char *argv[];
{
	extern int optind;
	int c, options, errflg, pcdopts;
	int dflg, sflg, bflg, mflg;

	errflg=0;
	options = 0;
	dflg=0;
	bflg=0;
	sflg=0;
	mflg=0;
	pcdopts =0;
	while ((c = getopt (argc, argv, "bdsm")) != EOF)
		switch (c) {
		case 'b':
			bflg++;
			options = 01;
			break;
		case 'd':
			dflg++;
			break;
		case 's':
			sflg++;
			break;
		case 'm':
			mflg++;
			pcdopts = DSRMODE;
			break;
		case '?':
			errflg++;
			break;
		}
	if (((argc - optind) < 1 || (argc - optind) > 3) && !errflg) {
		printf("Incorrect number of arguments\n");
		errflg++;
	}
	if (errflg ||
		(bflg && dflg) ||
		(bflg && sflg) ||
		(dflg && sflg)||
		(mflg && dflg)||
		(mflg && sflg)) {
		fprintf(stderr, "usage: vpmset [-bdms] tdev pdev [lineno]\n");
		exit(1);
	}
	if (sflg) {
		csistat(argv[optind]);
		exit(0);
	}
	if (dflg) {
		unsetdev(argv[optind]);
		exit(0);
	}
	setdev(argv[optind],mdevno(argv[optind+1])|((atoi(argv[optind+2])<<5)&0340),pcdopts,options);
	exit(0);
}
mdevno(file)
char *file;
{
	struct stat *bp;
	struct stat buf;

	bp = &buf;
	if (stat(file, bp)) {
		perror("Stat failed");
		exit(1);
	}
	if ((bp->st_mode & 0170000) != 0020000) {
		fprintf(stderr,"%s ", file);
		perror("not character special file");
		exit(1);
	}
	return(bp->st_rdev&0377);
}
setdev(file, mdno, pcdopts, options)
char *file;
{
	int fd;

	if ((fd = open(file, FNDELAY)) <0) {
		perror("Open failed");
		exit(1);
	}
	if (ioctl(fd, VPMSDEV, mdno) < 0) {
		perror("VPMSDEV ioctl failed");
		exit(1);
	}
	if(ioctl(fd,VPMOPTS,options) < 0) {
		perror("VPMOPTS ioctl failed\n");
		exit(1);
	}
	if (ioctl(fd,VPMPCDOPTS,pcdopts)< 0){
		perror("VPMPCDOPTS ioctl failed\n");
		exit(1);
		
	}
}
unsetdev(file)
char *file;
{
	int fd;

	if ((fd=open(file,FNDELAY)) < 0) {
		perror("Open failed\n");
		exit(1);
	}
	if(ioctl(fd,VPMDETACH,0) <0) {
		perror("VPMDETACH ioctl failed\n");
		exit(1);
	}
}
csistat(file)
char *file;
{
	int fd, idev;

	if ((fd=open(file,FNDELAY)) < 0) {
		perror("Open failed\n");
		exit(1);
	}
	if (ioctl(fd,VPMSTAT,&idev) < 0) {
		perror("VPMSTAT ioctl failed\n");
		exit(1);
	}
	printf("Common synchronous interface index for %s = %d\n",file,idev);
}
