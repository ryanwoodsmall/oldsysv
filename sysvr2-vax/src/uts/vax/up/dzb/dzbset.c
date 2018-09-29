/*	dzbset.c 6.1 of 8/22/83
	@(#)dzbset.c	6.1
 */


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/dz.kmc.h>





main(argc,argv)
char *argv[];
{

	if (argc < 3 || argc > 6) {
		fprintf(stderr,"Incorrect number of arguments\n");
		fprintf(stderr,"Usage: dzbset   kmcname dzbmdno ...\n");
		exit(1);
	}
	for ( argc -= 2; argc > 0; argc--) {
		setdev(argv[1], argv[argc+1]);
		printf("dzbset: dzb unit %d and %s associated\n",(ushort)(mdevno(argv[argc+1]))>>3,argv[1]);
	}
	exit(0);
}

mdevno(filep)
char *filep;
{
	struct stat *bp;
	struct stat buf;

	bp = &buf;
	if (stat(filep,bp)) {
		perror("Stat failed");
		exit(1);
	}
	if (( bp->st_mode&0170000) != 020000) {
		fprintf(stderr,"%s",filep);
		perror("not a character special file");
		exit(1);
	}
	return(bp->st_rdev&0377);
}


setdev(kmcfile,ttyfile)
char *kmcfile;
char *ttyfile;
{
	int fd;
	struct dzk dzk;

	if ((fd=open(ttyfile,FNDELAY)) < 0) {
		perror("Open failed");
		exit(1);
	}
	dzk.dzb_mdno = (ushort)(mdevno(ttyfile))>>3;
	dzk.kmc_mdno = mdevno(kmcfile);
	if (ioctl(fd,DZBATTACH,&dzk) < 0) {
		perror("DZBATTACH ioctl failed");
		exit(1);
	}
}
