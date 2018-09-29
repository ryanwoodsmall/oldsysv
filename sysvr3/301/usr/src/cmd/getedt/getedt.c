/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)getedt:getedt.c	1.3"
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/vtoc.h>
#include <sys/id.h>
#include <sys/hdelog.h>
#include <sys/hdeioctl.h>
#include <sys/stat.h>
#include "../bbh/hdecmds.h"
#include "../bbh/edio.h"
#include <stdio.h>
#include <fcntl.h>

extern char *malloc(), *realloc();
extern int errno;
extern char *sys_errlist[];
dev_t edisopen = -1;
long edsecsz;
daddr_t edpdsno;
int edisfix;
daddr_t edenddad;

struct eddata edpdsec;
struct eddata eddmap;		/* machine form of defect map */
struct eddata eddm;		/* standard form of defect map */
struct eddata edhdel;
struct eddata edvtoc;

static struct hdearg edarg, edzarg;

static int hdelfdes = -1;
int edcnt;
dev_t *edtable;
struct hdeedd *ededdp;

int edcnt;
char * cmdname;

#define BLKSIZE 512
#define	FLOPPY	0
#define DISK	1
#define	TAPE	2

union datasect {
	struct idsector0 oldsect0;
	char buf[BLKSIZE];
} ;

struct io_arg args;
extern int errno;

main(argc, argv)
int argc;
char **argv;
{
	extern char optarg;
	extern int optind, opterr;
	int errflg, dflg, fflg, oflg, tflg, fd, i, c, devtype;
	char *device; 
	FILE *fp;

	cmdname = argv[0];
	dflg = 0;
	fflg = 0;
	oflg = 0;
	tflg = 0;

	errflg = 0;
	if(argv[1][0] != '-') {
		fprintf(stderr,"Usage: getedt -d -f -o device -t \n");
			exit(2);
	}
	while ((c = getopt(argc, argv, "dfo:t")) != EOF)
		switch (c) {
		case 'd':
			dflg++;
			devtype = DISK;
			break;
		case 'f':
			fflg++;
			devtype = FLOPPY;
			break;
		case 'o':
			oflg++;
			device = argv[2];
			break;
		case 't':
			tflg++;
			devtype = TAPE;
			break;
		case '?':
			errflg++;
			break;
		}
		if (errflg) {
			fprintf(stderr, "Usage: getedt -d -f -o device -t \n");
			exit(2);
		}
	if(dflg || fflg || tflg)
		geteqtbl(devtype);
	if(oflg)
		checkdisk(device);
}
	
geteqtbl(devicetype)
int devicetype;
{
	int i;
	unsigned int diskcnt, flopcnt, tapecnt;
	
	diskcnt = 0;
	flopcnt = 0;
	tapecnt = 0;
	
	edinit();
	for (i = 0; i < edcnt; i++) {
		switch(ededdp[i].hdetype) {
			case EQD_ID:
				diskcnt++;
				break;
			case EQD_IF:
				flopcnt++;
				break;
			case EQD_TAPE:
				tapecnt++;
				break;
			default:
				fprintf(stderr,"\ngetedt: unknown device type: %d\n",ededdp[i].hdetype);
				exit(1);
		}
	}
	if(devicetype == DISK)
		printf("%d\n", diskcnt);
	else if(devicetype == FLOPPY)
		printf("%d\n", flopcnt);
		else
			printf("%d\n", tapecnt);
}

checkdisk(device)
char * device;
{
 
	union datasect sect0;
	int i, fd;

	/* Open device for read of cylinder 0, sector 0 */

		if ((fd = open(device,O_RDONLY)) < 0)  {
			fprintf(stderr, "getedt:  Cannot open device %s \n", device);
			exit(2);
		}

	/* ZERO SECTOR 0 DATA STRUCTURE */

		for(i=0; i < BLKSIZE; i++) {
			sect0.buf[i] = 0xff;
		}

		args.sectst = 0x00;
		args.datasz = BLKSIZE;
		args.memaddr = (unsigned long)&sect0;

		if((ioctl(fd,V_PREAD,&args)) == -1) {
			fprintf(stderr, "getedt:  ioctl failed, errno = %d\n", errno);
			exit(1);
		}
		if(args.retval == V_BADREAD)	{
			printf("getedt:  Cannot read sector 0 of device %s\n", device);
			exit(2);
		}
	/* RETURN 255 IF SANITY OF DEVICE IS 0XFEEDBEEF  ELSE RETURN 0
	*/
		if(sect0.oldsect0.reserved == 0xfeedbeef) 
			exit(255);
		else if(sect0.oldsect0.reserved == VALID_PD) 
			exit(0);
			else
				exit(2);
}

/* This file contains the interface subroutines for accessing
 * the hdelog driver and, indirectly through that driver,
 * for accessing the equipped disks for physical device I/O.
 * These interface subroutines maintain a discipline of
 * using at most one equipped disk at a time.
 */

edinit()
{
	register int i;

	if ((hdelfdes = open("/dev/hdelog", O_RDWR)) == -1) {
		fprintf(stderr, "%s: open of /dev/hdelog failed: %s\n",
			cmdname,
			sys_errlist[errno]);
		exit(ERREXIT);
	}
	edarg = edzarg;
	if (ioctl(hdelfdes, HDEGEDCT, &edarg) < 0) {
		fprintf(stderr, "%s: cannot get equipped disk count\n", cmdname);
		exit(ERREXIT);
	}
	edcnt = edarg.hdersize;
	if (edcnt <= 0) {
		fprintf (stderr, "%s: no equipped disks?\n", cmdname);
		exit(ERREXIT);
	}
	if (!(edtable = (dev_t *) malloc(edcnt * (sizeof *edtable)))) {
		fprintf(stderr, "%s: malloc of edtable failed\n", cmdname);
		exit(ERREXIT);
	}
	if (!(ededdp = (struct hdeedd *) malloc(edcnt * (sizeof *ededdp)))) {
		fprintf(stderr, "%s: malloc of ededd failed\n", cmdname);
		exit(ERREXIT);
	}
	edarg = edzarg;
	edarg.hdebody.hdegeqdt.hdeeqdct = edcnt;
	edarg.hdebody.hdegeqdt.hdeudadr = ededdp;
	if (ioctl(hdelfdes, HDEGEQDT, &edarg) < 0) {
		fprintf(stderr, "%s: cannot get equipped disk table\n", cmdname);
		exit(ERREXIT);
	}
	for (i = 0; i < edcnt; i++) edtable[i] = ededdp[i].hdeedev;
}
