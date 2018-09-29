/*	dmkset.c 1.2 of 8/31/82
	@(#)dmkset.c	1.2
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include "sys/dmk.h"

static char Sccsid[] = "@(#)dmkset.c	1.2";

main(argc, argv)
char *argv[];
	
{
	extern 	int	optind;
	int 	c,dflg,errflg;
	
	dflg= 0;
	errflg = 0;

	while (( c = getopt (argc, argv, "d")) != EOF)
		switch (c) {
			case 'd':
				dflg++;
				break;
			
			case '?':
				errflg++;
				break;
				
			}
	if (( argc - optind) < 1 ||
	(argc - optind) > 2 && !errflg) {
		printf( " Incorrect number of arguments\n");
				errflg++;
	}
	
	if (errflg){
		fprintf( stderr, "usage: dmkset [-d] dmkname kmcname \n");
		exit(1);
	}
	
	if (dflg){
		unsetdev(argv[optind]);
		exit (0);
	}
	
	setdev(argv[optind],mdevno(argv[optind+1]));
	exit(0);
	
}
mdevno(file)
char *file;
{
	struct stat *bp;
	struct stat buf;

	bp = &buf;
	if (stat(file,bp)){
		perror("dmkset: Stat failed");
		exit(1);
	}
	if ((bp->st_mode & 0170000) != 0020000) {
		fprintf(stderr,"dmkset: %s ",file);
		perror("not character special file");
		exit(1);
	}
	return(bp->st_rdev&0377);
}
setdev(file,mdno)
char *file;
{
	int fd;
	if (( fd = open(file, FNDELAY)) <0) {
		perror("dmkset: Open failed");
		exit(1);
	}
	if (ioctl (fd,DMKATTACH,mdno) <0) {
		perror ("dmkset: DMKATTACH ioctl failed");
		exit(1);
	}
}
unsetdev(file)
char *file;
{
	int fd;
	
	if ((fd = open(file, FNDELAY)) <0) {
		perror("dmkset: Open failed\n");
		exit(1);
	}
	if (ioctl(fd,DMKDETACH,0) <0) {
		perror("dmkset: DMKDETACH ioctl failed\n");
		exit(1);
	}
}
