/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/if.h	10.2"
/* This file contains the command and status information needed 
   to initialize structure definitions for the 3B2 product 
   floppy disk subsystem utilizing the Western Digital 2797
   floppy controller.  */

struct ifdev {
	unsigned char statcmd;
	unsigned char track;
	unsigned char sector;
	unsigned char data;
} ;

struct ifformat {
	char iftrack;
	char ifsector;
	char ifside;
	caddr_t	data;
	int size;
	unsigned char retval;
} ;

struct ifccmd {			/* structure to track disk requests */
	char trknum;		/* track number */
	char sectnum;   	/* sector number */
	char c_opc;		/* command opcode */
	char c_bkcnt;		/* request block count */
	daddr_t b_blkno; 	/* disk block number */
	paddr_t baddr; 		/* physical data buffer address */
	unsigned int bcnt;	/* request byte count */
} ;


#define	OIFLOPY		0x0004D000L
#define	IFLOPY		((struct ifdev *)OIFLOPY)

#define IFNTRAC 	2	/* Number of tracks per cylinder */
#define IFNHD		2	/* Number of heads per cylinder */	
#define IFNDRV		0	/* Number of integral floppy disk drives */
#define IFPTN 		0x8	/* Fake partition for full floppy access */
#define IFNODEV		0x10	/* Non-partition - allows full disk access */
#define IFPDBLKNO	1422	/* Physical description sector number */

/* ioctl commands */
#define FIOC		('F'<<8)
#define IFFORMAT	(FIOC|1)
#define IFBCHECK	(FIOC|2)
#define IFCONFIRM	(FIOC|3)

#define IFIDLEF 	0	/* Defines used for semaphore communication */
#define IFWAITF 	1	/* between the integral winchester and */
#define IFBUSYF 	2	/* the integral floppy to avoid I/O bus */
#define IFFMAT0 	4	/*    contention    */
#define IFFMAT1 	8	
#define IFDMACNT	511	/* dma byte count for floppy transfer */

#define IFRESTORE 	1	/* Used for interrupt cases */
#define IFSEEKATT	2
#define IFXFER 		3
#define IFNONACTIVE 	0

#define IFMAXSEEK 5	/* Maximum retries on various requests */
#define IFMAXXFER 15
#define IFMAXLSTD 1024


/* The command opcodes ARE dependent on drive configuration.
   The following commands are set for a TANDON floppy drive
   with stepping rates of 6 mS and head load upon spin up. */

/* Controller Command Opcodes */

#define IFREST 		0x00	/* Recal */
#define IFSEEK 		0x10	/* Seek without verify-with verify is 14 */	
#define IFSTEP		0x20	/* STEP RATES 0=6 ms, 1=12ms, 2=20ms, 3=30ms */
#define IFSTEPI		0x40	/* Step in */
#define IFSTEPO		0x60	/* Step out */
#define IFRDS 		0x80	/* Read Sector */
#define IFWTS		0xA0	/* Write Sector */
#define IFRDADD		0xC0	/* Read Address */
#define IFRDTRK		0xE0	/* Read Track */
#define IFWRTRK		0xF0	/* Write Track */
#define IFFRCINT  	0xD0 	/* Force Interrupt Dependent upon int type */

/* Disk Command Modifiers */

#define IFSTEPRATE	0x00	/* 0=6ms,1=12ms,2=20ms,3=30ms */
#define IFVERIFY	0x04	/* Verify Track Address */
#define IFLDHEAD	0x08	/* Load Disk Head */
#define IFUPTRACK	0x10	/* Update Track Register */
#define IFF8DAM		0x01	/* Deleted Data Address Mark */
#define IFUPDSSO1	0x02	/* Update Side Select */
#define IFMSDELAY	0x04	/* 15ms Delay for Head Settling */
#define IFSLENGRP1	0x08	/* Sector Length */
#define IFMRECORD	0x10 	/* Multi Record Field */

/* Command Status for REST SEEK STEP */

#define IFBUSY		0x01	/* Controller Busy Status */
#define IFINDPUL	0x02	/* Index Mark Detected form Drive */
#define IFTRK00		0x04	/* Head is at Track 00 */
#define IFCRCERR	0x08	/* CRC Error */
#define IFSKERR		0x10	/* Desired Track was not Verified */
#define IFHDLOAD	0x20	/* Head Loaded */
#define IFWRPT		0x40	/* Write Protect */
#define IFNRDY		0x80	/* Drive Not Ready */

/* Status Information for Read / Write Sector Track Commands */

#define IFBUSY		0x01	/* Controller Busy */
#define IFDRQ		0x02	/* Synonymous with Data Request Output */
#define IFLSTDATA 	0x04	/* No Data Response from CPU Within ONE Byte Time */
#define IFCRCERR	0x08	/* Error in Data Field */
#define IFRECNF		0x10	/* Track Sector Side Were Not Found */
#define IFRECT		0x20	/* Zero On WRITE Indicates Record Type on Read */
#define IFWRPT		0x40	/* Indicates Write Protection */
#define IFNRDY		0x80	/* Drive not Ready */

#define	IFIDSIZE	6	/*  size of id field */
#define IFTRACKS 	160	/*  total number of tracks  */
#define	IFTRKSIDE	80	/*  number of tracks per side  */
#define	IFNUMSECT	9	/*  number of sectors per track  */
#define	IFBYTESCT	512	/*  number of bytes per sector  */


/*    IBM Standard Dual-Density Format Specification - 
           (512 bytes/sector  9 sectors/track)
*/

#define IFGAP4a_SIZE	80		/* preamble field sizes */
#define IFPRSYNC_SIZE	12
#define IFIDXMRK_SIZE	4
#define IFGAP1_SIZE	52

#define IFSYNC1_SIZE	12		/*  sector field sizes  */
#define IFIDAMRK_SIZE	4
#define IFGAP2_SIZE	22
#define IFSYNC2_SIZE	12
#define IFDMRK_SIZE	4
#define	IFGAP3_SIZE	84

#define	IFGAP4_SIZE	316		/*  postamble field size  */

/*    Layout of Track Preamble Field    */

struct	ifpreamfmat	{
	unsigned char GAP4a[IFGAP4a_SIZE];
	unsigned char PRSYNC[IFPRSYNC_SIZE];
	unsigned char INDEX_MARK[IFIDXMRK_SIZE];
	unsigned char GAP1[IFGAP1_SIZE];
} ;

/*    Layout of Sector Field    */

struct	ifsectfmat	{
	unsigned char SYNC1[IFSYNC1_SIZE];
	unsigned char IDADD_MARK[IFIDAMRK_SIZE];
	unsigned char TRACK;
	unsigned char SIDE;
	unsigned char SECTOR;
	unsigned char SECTLEN;
	unsigned char CRC1;
	unsigned char GAP2[IFGAP2_SIZE];
	unsigned char SYNC2[IFSYNC2_SIZE];
	unsigned char DATA_MARK[IFDMRK_SIZE];
	unsigned char DATA[IFBYTESCT];
	unsigned char CRC2;
	unsigned char GAP3[IFGAP3_SIZE];
} ;

/*    Layout of Postamble Field    */

struct	ifpostfmat	{
	unsigned char GAP4b[IFGAP4_SIZE];
} ;

/*    Layout of a Complete Track    */

struct	iftrkfmat	{
	struct ifpreamfmat dskpream;
	struct ifsectfmat dsksct[IFNUMSECT];
	struct ifpostfmat dskpost;
} ;
