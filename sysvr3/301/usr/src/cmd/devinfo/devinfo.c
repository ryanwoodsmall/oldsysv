/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)devinfo:devinfo.c	1.2"
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/vtoc.h>
#include <sys/id.h>
 
#define BLKSIZE 512
#define DEFECTSZ 64
#define DEVICE	"/dev/rdsk/"
#define BACKUP	6
#define BOOT	7
#define FIOCTL	1
#define DRERR	2
#define FILERR	2
#define OPENERR	2
#define WRITERR 2

union datasect {
	struct idsector0 oldsect0;
	struct pdsector newsect0;
	char buf[BLKSIZE];
} ;
	

struct io_arg args;
extern int errno;

main(argc, argv)
int argc;
char **argv;
{
	union datasect sect0;
	extern char optarg;
	extern int optind, opterr;
	int errflg, iflg, pflg, fd, i, c;
	char *device;
	iflg = 0;
	pflg = 0;
	errflg = 0;

	if (argv[1][0] != '-') {
		fprintf(stderr, "Usage: devinfo -p device -i device \n");
		exit(2);
	}
	else {
		while ((c = getopt(argc, argv, "i:p:")) != EOF) {
			switch (c) {
			case 'i':
				iflg++;
				device = argv[2];
				break;
			case 'p':
				pflg++;
				device = argv[2];
				break;
			case '?':
				errflg++;
				break;
			}
			if (errflg) {
				fprintf(stderr, "Usage: devinfo -p device -i device \n");
				exit(2);
			}
		}
	}
	if ((fd = open(device,O_RDONLY)) < 0)  
		exit(OPENERR);

	/* ZERO SECTOR 0 DATA STRUCTURE */

	for(i=0; i < BLKSIZE; i++) 
		sect0.buf[i] = 0xff;

	/* READ SECTOR 0 */

	args.sectst = 0x00;
	args.datasz = BLKSIZE;
	args.memaddr = (unsigned long)&sect0;

	if((ioctl(fd,V_PREAD,&args)) == -1) {
		exit(FIOCTL);
	}
	if(args.retval == V_BADREAD)	{
		exit(DRERR);
	}
	if(iflg)
		devinfo(&sect0, fd, device);
	if(pflg)
		partinfo(&sect0, fd, device);
	exit(0);
}
partinfo(osect0, ofd, device)
union datasect *osect0;
char *device;
int ofd;
{
	int i;
	unsigned int startblock, noblocks, flag, tag, major, slice;
	struct vtoc vtdata;
	struct stat statbuf;

	major = 0;
	slice = 0;
	startblock = 0;
	noblocks = 0;

	if(osect0->newsect0.pdinfo.sanity != VALID_PD) {
		close(ofd);
		exit(255);
	}

	/* READ VTOC OF DEVICE*/
	args.sectst = osect0->newsect0.pdinfo.logicalst +1;
	args.datasz = sizeof(struct vtoc);
	args.memaddr = (unsigned long)&vtdata;
	if((ioctl(ofd,V_PREAD,&args)) == -1)
		exit(FIOCTL);
	if(args.retval == V_BADREAD) 
		exit(DRERR);
	close(ofd);
	if(vtdata.v_sanity != VTOC_SANE)
		exit(DRERR);
	i = stat(device, &statbuf);
	if(i < 0)
		exit(DRERR);
	major = (statbuf.st_rdev >> 4) & 0xf;
	slice = statbuf.st_rdev & 0xf;
	startblock = vtdata.v_part[slice].p_start;
	noblocks = vtdata.v_part[slice].p_size;
	flag = vtdata.v_part[slice].p_flag;
	tag = vtdata.v_part[slice].p_tag;
	printf("%s	%0x	%0x	%d	%d	%x	%x\n", device, major, slice, startblock, noblocks, flag, tag);
}

devinfo(osect0, ofd, device)
union datasect *osect0;
char *device;
int ofd;
{
	int i;
	unsigned int nopartitions, sectorcyl, version, driveid, bytes;
	struct vtoc vtdata;
	struct stat statbuf;

	nopartitions = 0;
	sectorcyl = 0;
	bytes = 0;
	version = 0;

	if(osect0->newsect0.pdinfo.sanity != VALID_PD) {
		close(ofd);
		exit(255);
	}
	sectorcyl = osect0->newsect0.pdinfo.tracks * osect0->newsect0.pdinfo.sectors;
	bytes = osect0->newsect0.pdinfo.bytes;
	driveid = osect0->newsect0.pdinfo.driveid;
	version = osect0->newsect0.pdinfo.version;

	/* READ VTOC OF DEVICE*/
	args.sectst = osect0->newsect0.pdinfo.logicalst +1;
	args.datasz = sizeof(struct vtoc);
	args.memaddr = (unsigned long)&vtdata;
	if((ioctl(ofd,V_PREAD,&args)) == -1)
		exit(FIOCTL);
	if(args.retval == V_BADREAD)
		exit(DRERR);
	close(ofd);
	if(vtdata.v_sanity != VTOC_SANE)
		exit(DRERR);
	for(i=0;i<V_NUMPAR;i++)	{
		if(vtdata.v_part[i].p_size != 0x00)
			nopartitions++;
	}
	printf("%s	%0x	%0x	%d	%d	%d\n", device, version, driveid, sectorcyl, bytes, nopartitions);
}
