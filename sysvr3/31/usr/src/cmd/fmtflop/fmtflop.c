/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)fmtflop:fmtflop.c	1.10"
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/lock.h>
#include <sys/types.h>
#include <sys/if.h>
#include <sys/vtoc.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/diskette.h>
#include <signal.h>

#define PASS 0
#define FAIL 1

#define IFID 		0
#define IFVERSION 	1

#define ON	1
#define OFF	0

char intrlv[IFNUMSECT] = {1,4,7,2,5,8,3,6,9};
struct io_arg args;
struct iftrkfmat dsktrack[2];
struct ifformat ifformat;

struct fmtstruct fmtstruct ;



main(argc,argv)
unsigned argc;
char **argv;
{
	int i;
	int cursect;
	struct stat buffer;
	int structnum = 0;
	int c;
	extern char *optarg;
	extern int optind;
	int argdev = 0;
	int errflag = 0;
	int optflag = 0;
	int iffdes;
	char *ifdevice;

	
	while ((c = getopt(argc,argv,"v")) != EOF) 
	switch(c){
		case 'v':
			optflag++;
			break;
		case '?':
			fprintf(stderr,"Usage: fmtflop [-v] special\n");
			exit(2);
	}

	if(argc - optind != 1){
		fprintf(stderr,"Usage: fmtflop [-v] special\n");
		exit(2);
	}

	ifdevice = argv[optind];

	if(stat(ifdevice,&buffer) == -1){
		fprintf(stderr,"fmtflop: device entry not found\n");
		exit(16);
	}
	if(major(buffer.st_rdev) != 17){
		iffdes = open(ifdevice,O_FORMAT);
		if(iffdes < 0){
			fprintf(stderr,"fmtflop: open failed on device: %s\n",ifdevice);
			exit(16);
		}
		if(optflag){
			fmtstruct.mode = VERIFY ;
			fmtstruct.passcnt = OFF;
		}
		else{
			fmtstruct.mode = OFF;
			fmtstruct.passcnt = OFF;
		}
		if(ioctl(iffdes,FORMAT,&fmtstruct)<0){
			if(errno==ENOTTY){
				fprintf(stderr,"fmtflop: Device must be a character device\n");
				exit(16);
			}
			else {
				fprintf(stderr,"fmtflop: error returned is %d, \n",errno);
				exit(16);
			}
		}
		close(iffdes) ;
		exit (0) ;
	}



	iffdes = open(ifdevice, O_RDWR);
	if(iffdes < 0){
		fprintf(stderr,"fmtflop: open failed on device: %s\n",ifdevice);
		exit(16);
	}

	/* lock process in memory to prevent swap */
	if(plock(PROCLOCK)<0){
		fprintf(stderr,"fmtflop: process lock failed\n");
		exit(16);
	}

	for(i=0;i<=IFGAP4a_SIZE;i++)
		dsktrack[structnum].dskpream.GAP4a[i]=0x4e;

	for(i=0;i<=IFPRSYNC_SIZE;i++)
		dsktrack[structnum].dskpream.PRSYNC[i] = 0x00;

	dsktrack[structnum].dskpream.INDEX_MARK[0] = 0xf6;
	dsktrack[structnum].dskpream.INDEX_MARK[1] = 0xf6;
	dsktrack[structnum].dskpream.INDEX_MARK[2] = 0xf6;
	dsktrack[structnum].dskpream.INDEX_MARK[3] = 0xfc;

	for(i=0;i<=IFGAP1_SIZE;i++)
		dsktrack[structnum].dskpream.GAP1[i]=0x4e;
	
	for(cursect=0;cursect<IFNUMSECT;cursect++){
	
		/* INIT ID FIELD */
		for(i=0;i<=IFSYNC1_SIZE;i++)
			dsktrack[structnum].dsksct[cursect].SYNC1[i]=0x00;
		dsktrack[structnum].dsksct[cursect].IDADD_MARK[0]=0xf5;
		dsktrack[structnum].dsksct[cursect].IDADD_MARK[1]=0xf5;
		dsktrack[structnum].dsksct[cursect].IDADD_MARK[2]=0xf5;
		dsktrack[structnum].dsksct[cursect].IDADD_MARK[3]=0xfe;
		dsktrack[structnum].dsksct[cursect].TRACK=0x00;
		dsktrack[structnum].dsksct[cursect].SIDE=0x00;
		dsktrack[structnum].dsksct[cursect].SECTOR=intrlv[cursect];
		dsktrack[structnum].dsksct[cursect].SECTLEN=0x02;
		dsktrack[structnum].dsksct[cursect].CRC1=0xf7;
		
		for(i=0;i<=IFGAP2_SIZE;i++)
			dsktrack[structnum].dsksct[cursect].GAP2[i]=0x4e;
		for(i=0;i<=IFSYNC2_SIZE;i++)
			dsktrack[structnum].dsksct[cursect].SYNC2[i]=0x00;
		dsktrack[structnum].dsksct[cursect].DATA_MARK[0]=0xf5;
		dsktrack[structnum].dsksct[cursect].DATA_MARK[1]=0xf5;
		dsktrack[structnum].dsksct[cursect].DATA_MARK[2]=0xf5;
		dsktrack[structnum].dsksct[cursect].DATA_MARK[3]=0xfb;
		for(i=0;i<=IFBYTESCT;i++)
			dsktrack[structnum].dsksct[cursect].DATA[i]=0xe5;
		dsktrack[structnum].dsksct[cursect].CRC2=0xf7;
		for(i=0;i<=IFGAP3_SIZE;i++)
			dsktrack[structnum].dsksct[cursect].GAP3[i]=0x4e;
	}
		/* INIT POSTAMBLE FIELD */
	for(i=0;i<=IFGAP4_SIZE;i++)
		dsktrack[structnum].dskpost.GAP4b[i]=0x4e;

	fmatdsk(structnum, iffdes);
	if(optflag){
		verify(iffdes);
	}

	/* Write the pdsector and defect map on innermost cylinder */

	writepd(iffdes);
	close(iffdes);
}
fmatdsk(structnum, iffdes)
int structnum;
int iffdes;
{
	int trkcnt=0;
	char ifside;

	while(trkcnt<IFTRKSIDE){
		ifside=0;
		fmatupdate(trkcnt,ifside,structnum, iffdes);
		ifside=1;
		fmatupdate(trkcnt,ifside,structnum, iffdes);
		trkcnt++;
	}
}
fmatupdate(trkcnt,ifside,structnum, iffdes)
int trkcnt;
char ifside;
int structnum;
int iffdes;
{
	short cursect;
	extern int errno;

	for(cursect=0;cursect<IFNUMSECT;cursect++){
		dsktrack[structnum].dsksct[cursect].TRACK=trkcnt;
		dsktrack[structnum].dsksct[cursect].SIDE=ifside;
	}
	if(ioctl(iffdes,IFFORMAT,&dsktrack[structnum])<0){
		if(errno==ENOTTY){
			fprintf(stderr,"fmtflop: Device must be a character device\n");
			plock(UNLOCK);
			exit(16);
		}
		else {
			fprintf(stderr,"Error returned is %d, \n",errno);
			fprintf(stderr,"fmtflop: format failure on track %d, side %d, sector %d\n", trkcnt, ifside, cursect);
			plock(UNLOCK);
			exit(16);
		}
	}
}
verify(iffdes)
{
	extern int errno;

	ifformat.iftrack = 0;
	ifformat.data = (caddr_t) &dsktrack[0];


	while(ifformat.iftrack<IFTRKSIDE){
		ifformat.ifside = 0;
		if(ioctl(iffdes,IFCONFIRM,&ifformat)<0) {
			fprintf(stderr,"Error returned is %d, \n",errno);
			fprintf(stderr,"fmtflop: failure on verifying track number %d, on side %d\n", ifformat.iftrack, ifformat.ifside);
			plock(UNLOCK);
			exit(16);
		}
		ifformat.ifside = 1;
		if(ioctl(iffdes,IFCONFIRM,&ifformat)<0) {
			fprintf(stderr,"Error returned is %d, \n",errno);
			fprintf(stderr,"fmtflop: failure on verifying track number %d, on side %d\n", ifformat.iftrack, ifformat.ifside);
			plock(UNLOCK);
			exit(16);
		}
		ifformat.iftrack++;
	}
}

writepd(iffdes)
{
	struct pdsector bufsect;
	unsigned int bufdef[128];
	int i;
	

	/* Setup pdsector buffer */
	bufsect.pdinfo.driveid = IFID;
	bufsect.pdinfo.sanity = VALID_PD;
	bufsect.pdinfo.version = IFVERSION;
	for(i=0; i<12; i++)
		bufsect.pdinfo.serial[i] = 0x00;
	bufsect.pdinfo.cyls = IFTRKSIDE;
	bufsect.pdinfo.tracks = IFNHD;
	bufsect.pdinfo.sectors = IFNUMSECT;
	bufsect.pdinfo.bytes = IFBYTESCT;
	bufsect.pdinfo.logicalst = 0;
	bufsect.pdinfo.errlogst = IFPDBLKNO + (IFNUMSECT * IFNHD) -1;
	bufsect.pdinfo.errlogsz = IFBYTESCT;
	bufsect.pdinfo.mfgst =0xffffffff;
	bufsect.pdinfo.mfgsz = 0xffffffff;
	bufsect.pdinfo.defectst = IFPDBLKNO + 1;
	bufsect.pdinfo.defectsz = IFBYTESCT;
	bufsect.pdinfo.relno = 1;
	bufsect.pdinfo.relst = bufsect.pdinfo.defectst + 1;
	bufsect.pdinfo.relsz = (bufsect.pdinfo.tracks * bufsect.pdinfo.sectors) - 3;
	bufsect.pdinfo.relnext = bufsect.pdinfo.relst;
	for (i=0;i<10;i++)
		bufsect.reserved[i] = 0x00;
	for (i=0;i<97;i++)
		bufsect.devsp[i] = 0x00;

	/* Setup defect map */
	for (i=0;i<128;i++)	{
		bufdef[i] = 0xffffffff;
	}

	/* Write pdsector */
	args.memaddr = (unsigned long)&bufsect;
	if((ioctl(iffdes,V_PDWRITE,&args))<0)	{
		fprintf(stderr,"fmtflop: Bad pdsector write. Ioctl failed, errno = %d\n",errno);
		exit(1);
	}
	
	/* Write defect map */
	args.sectst = bufsect.pdinfo.defectst;
	args.datasz = IFBYTESCT;
	args.memaddr = (unsigned long)bufdef;
	if((ioctl(iffdes,V_PWRITE,&args))<0)	{
		fprintf(stderr,"fmtflop: Bad defect map write. Ioctl failed, errno = %d\n",errno);
		exit(1);
	}
}
