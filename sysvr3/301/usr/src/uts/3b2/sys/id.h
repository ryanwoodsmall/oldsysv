/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/id.h	10.1"
/* 3B2 Integral Winchester Disk Definitions */

#define IDNULL		0
#define IDSET		1
#define IDFRSTBLK	0

#define	IDIDLE		0x01
#define	IDSEEK0		0x02
#define	IDSEEK1		0x04
#define	IDXFER		0x08
#define IDBUSY		0x10
#define	IDWAITING		0x20

#define IDSEEKMSK (IDRDYCHG|IDDRVNRDY|IDSEEKERR|IDEQUIPT|IDSEEKEND)

/*  Function return states  */
#define	IDPASS	0
#define	IDFAIL	1

/*  Number of disk operation retries  */
#define	IDRETRY	16
#define	IDRESEEK	3

/*  Interrupt definitions to idldcmd  */
#define	IDINTON		1
#define	IDINTOFF	0

/* special block numbers */
#define IDPDBLKNO	0x00
#define IDVTOCBLK	0x01

/* Minor number macros */
#define	iddn(x)		((x>>4)&1)	/* extract drive number */
#define	idmkmin(dn)	((dn)<<4)	/* make minor from drive number */
#define	idslice(x)	(x&0x0f)	/* extract drive partition no. */
#define	idnodev(x)	((x>>5)&1)	/* test for physical r/w */
#define idspecdev(x)	((x>>6)&1)	/* test for special device */

/* save contents of buffer header */
struct idsave {
	caddr_t	b_addr;
	daddr_t b_blkno;
	unsigned int b_bcount;
} ;

/* disk address structures */
struct partaddr {
	unsigned char pcnh;	/* physical cylinder number high */
	unsigned char pcnl;	/* physical cylinder number low */
	unsigned char phn;	/* physical head number */
	unsigned char psn;	/* physical sector number */
} ;

union diskaddr {
	unsigned long full;	/* full physical disk address */
	struct partaddr part;	/* allows referencing of partial addresses */
} ;

/* defect maps */
#define IDDEFSIZ	2048
#define IDDEFCNT	256

struct defect {
	union diskaddr bad;
	union diskaddr good;
} ;

struct defstruct	{
	struct defect map[IDDEFCNT];
};


/* Manufacturer's Defect Table */
struct mfgdef	{
	unsigned short length;
	unsigned short indexbytes;
	unsigned short cylinder;
	unsigned short head;
};

/* seek parameter structure */
struct idseekstruct {
	unsigned char pcnh;	/* physical cylinder number high */
	unsigned char pcnl;	/* physical cylinder number low */
};

#define IDSEEKCNT	2

/* transfer parameter structure */
struct idxferstruct {
	unsigned char	phn;	/* physical head number */
	unsigned char	lcnh;	/* logical cylinder number high */
	unsigned char	lcnl;	/* logical cylinder number low */
	unsigned char	lhn;	/* logical head number */
	unsigned char	lsn;	/* logical sector number */
	unsigned char	scnt;	/* sector count */
	unsigned int	bcnt;	/* byte count */
	unsigned char	necop;	/* disk controller command opcode */
	unsigned char	dmacop;	/* dmac command opcode */
	unsigned int	b_addr; /* memory address */
	unsigned int	unitno; /* drive number */
};

#define IDXFERCNT 	6

/*  controller initialization - specify parameter structure  */
struct idspecparam {
	unsigned char	mode;	/* operational mode of controller */
	unsigned char	dtlh;	/* sector data length high byte */
	unsigned char	dtll;	/* sector data length low byte */
	unsigned char	etn;	/* end track number:last track on cyl */
	unsigned char	esn;	/* end sector number: last sector on track */
	unsigned char	gpl2;	/* gap length 2 */
	unsigned char	rwch;	/* reduced write current start cyl: high byte */
	unsigned char	rwcl;	/* reduced write current start cyl: low byte */
} ;

#define IDSPECCNT	8

#define IDNOPARAMCNT	0

/*    drive status bytes    */
struct idstatstruct {
	unsigned int 	state;		/*  internal per-drive driver state */
	unsigned char	open;		/*  flag marking first open */
	unsigned char 	equipped;	/*  drive equipped flag */
	unsigned char 	drvtype;	/*  drive equipped flag */
	unsigned char	statreg;	/*  controller state register (STR)  */
	unsigned char	estbyte;	/*  error status byte (EST)  */
	unsigned char	istbyte;	/*  interrupt status byte (IST)  */
	unsigned char	ustbyte;	/*  unit status byte (UST)  */
	unsigned char	phn;		/*  physical head number */
	unsigned char	lcnh;		/* logical cylinder number high */
	unsigned char	lcnl; 		/* logical cylinder number low */
	unsigned char	lhn;		/* logical head number */
	unsigned char	lsn;		/* logical sector number */
	unsigned char	scnt;		/* sector count */
	int		retries;	/* number of retries left */
	unsigned char	reseeks;	/* number of reseeks left */
};

/* open states and special not-a-device partition definitions */
#define IDADDEV		0x02
#define IDMAXHD		0x08
#define IDNODEV		0x20
#define IDNOUNIT	0x40
#define IDNOTOPEN	0x00
#define IDISOPEN	0x01
#define IDOPENING	0x02

struct iddev {
	unsigned char fifo;
	unsigned char statcmd;
};

#define	OIDISK		0x0004A000L
#define	IDISK		((struct iddev *)OIDISK)


/*   Disk Controller Status Register Layout

	Bit #	  Definition
	 7	Controller Busy
	 6	Command End High 
	 5	Command End Low 
	 4	Sense Interrupt Status Request
	 3	Reset Request
	 2	ID Error
	 1	Not Coincident
	 0	Data Request

*/

#define	IDCBUSY		0x80		/* controller busy */
#define	IDCMDINP	0x00		/* command in progress */
#define	IDCMDABT	0x20		/* abnormal command termination */
#define	IDCMDNRT	0x40		/* normal command termination */
#define	IDCMDINV	0x60		/* command invalid */
#define	IDSINTRQ	0x10		/* sense interrupt request status */
#define	IDRESETRQ	0x08		/* reset request */
#define	IDERROR		0x04		/* ID error */
#define	IDNCOIN		0x02		/* not coincident */
#define	IDDATARQ	0x01		/* data request */
#define	IDENDMASK	0x60		/* mask for command end bits */


/*   Disk Error Status Byte (EST) Layout

	Bit #	  Definition
	 7	End of Cylinder
	 6	Overrun
	 5	Data Error
	 4	Equiptment Check
	 3	Not Ready
	 2	Not Data
	 1	Not Writable
	 0	Missing Data Mark

*/

#define	IDENDCYL	0x80		/* end of cylinder */
#define	IDDMAOVR	0x40		/* DMA overrun */
#define	IDDATAERR	0x20		/* data error */
#define	IDEQUIPTC	0x10		/* equiptment check */
#define	IDDRVNRDY	0x08		/* drive not ready */
#define	IDNODATA	0x04		/* not data */
#define	IDWTPROT	0x02		/* not writable */
#define	IDMISSMRK	0x01		/* missing data mark */


/*   Disk Interrupt Status (IST) Byte Layout

	Bit #	  Definition
	 7	Seek End
	 6	Ready Change
	 5	Seek Error
	 4	Equiptment Check
	 3	Not Ready
	 2	Unit Address Most Significant
	 1	Unit Address
	 0	Unit Address Least Significant

*/

#define	IDSEEKEND	0x80		/* seek end */
#define	IDRDYCHG	0x40		/* ready change */
#define	IDSEEKERR	0x20		/* seek error */
#define	IDEQUIPT	0x10		/* equiptment check */
#define	IDDRVNRDY	0x08		/* drive not ready */
#define	IDUNITADD	0x07		/* unit address mask */

/*   Disk Unit Status (UST) Byte Layout

	Bit #	  Definition
	 4	Drive Selected
	 3	Seek Complete
	 2	Track 0
	 1	Ready
	 0	Write Fault

*/

#define	IDDRIVESL	0x10		/* drive selected */
#define	IDSEEKCMP	0x08		/* seek complete */
#define	IDTRACK0	0x04		/* track 0 */
#define IDREADY		0x02		/* ready */
#define	IDWTFAULT	0x01		/* write fault */

/*   Auxiliary Disk Commands   */

#define	IDRESET		0x01		/* reset the disk controller */
#define	IDCLFIFO	0x02		/* clear the data FIFO */
#define	IDMASKSRQ	0x04		/* mask the SRQ interrupt */
#define	IDCLCMNDEND	0x08		/* clear command end bits */

/* Drive IDs */

#define IDRV0		0x00
#define IDRV1		0x01

/*   Disk Command Definitions   */

#define	IDRECAL		0x50	/* position the R/W heads to cylinder 0 */
#define	IDSEEK		0x60	/* position the R/W heads to the specified cylinder */
#define	IDWTFORMAT	0x70	/* write a track format to the disk */
#define	IDVERID		0x80	/* compare sector ID fields */
#define	IDREADID	0x90	/* read a sector ID field   */
#define	IDREAD		0xb0	/* read a sector data field */
#define IDCHECK		0xc0	/* check CRC of a specified sector */
#define	IDSCAN		0xd0	/* find a sector with specified data */
#define	IDVERDT		0xe0	/* compare sector data fields */
#define	IDWRITE		0xf0	/* write sector data fields */
#define	IDSPECIFY	0x20	/* sets the operation mode and format */
#define	IDSENSEINT	0x10	/* checks status information */
#define	IDSENSEUS	0x30	/* checks the disk drive status */
#define	IDDETERROR	0x40	/* checks the error pattern */

/*   Disk Command Modifier Definitions   */

#define	IDBUFFERED	0x08
#define	IDSKEWED	0x08

#define	VALIDINFO	0xFEEDBEEF	/* disk info is valid non-vtoc */
#define	VALIDVTOC	0xFEEDF00D	/* disk info is valid vtoc */

/*   Parameter values for an ST-412 Disk Drive   */

#define IDNDRV		2		/*  number of drives in system  */
#define	IDNUM_CYLINDER	306		/* number of disk cylinders */
#define	IDNUM_HEADS	4		/* number of disk heads 4 */
#define	IDRED_WRITE	128		/* reduce write cylinder */
#define	IDWT_PRECOM	64		/* write precomp cylinder */
#define	IDNUMSECT	18		/* number of sectors per track */
#define	IDBYTESCT	512		/* number of bytes per sector */

/*   Track ID Format Specification   */

struct	ididfmat	{
		unsigned char LCNH;	/* logical cylinder number high byte */
		unsigned char LCNL;	/* logical cylinder number low byte */
		unsigned char LHN;	/* logical head number */
		unsigned char LSN;	/* logical sector number */
		};

struct	idtrkfmat	{
		struct ididfmat idtrkimage[IDNUMSECT];
		};

/*   GAP Definitions for FORMAT   */

/* #define	IDGAP3	15 */
#define IDGAP1_SIZE 16
#define IDSYNC_SIZE 13
#define IDPAD_SIZE 3
#define	IDDPAD	0
#define IDIRG_SIZE 15
#define IDIDCRC_SIZE 2

#ifdef ECC
#define IDGAP4_SIZE 104
#define IDCHKSM_SIZE 4
#else
#define IDGAP4_SIZE 140
#define IDCHKSM_SIZE 2
#endif

/* raw sector format for hard disk */

struct idsect {
	char idsync[IDSYNC_SIZE];
	char idam;
	char lcnh;
	char lcnl;
	char lhn;
	char lsn;
	char idcrc[IDIDCRC_SIZE];
	char idpad[IDPAD_SIZE];
	char dsync[IDSYNC_SIZE];
	char dam;
	char dm;
	char data[IDBYTESCT];
	char chksm[IDCHKSM_SIZE];
	char dpad[IDPAD_SIZE];
	char irgap[IDIRG_SIZE];
};

/* raw track format for integral disk */

struct idtrk {
	char gap1[IDGAP1_SIZE];
	struct idsect idsect[IDNUMSECT];
	char gap4[IDGAP4_SIZE];
};

union iddskadr {			/* disk address */
	unsigned long all;
	struct part {
		unsigned char pcnh;
		unsigned char pcnl;
		unsigned char phn;
		unsigned char psn;
	} part;
};

/* first disk sector info */

struct iddeftab {
	union iddskadr bad;
	union iddskadr good;
};

/*
 *  The following sector 0 structure should not be used by any other programs
 *  other than those used for conversion from 1.0 to later releases.
 */
#define IDDEFTAB 61

struct idsector0 {
	unsigned long driveid;
	unsigned long reserved;
	unsigned long cyls;
	unsigned long tracks;
	unsigned long sectors;
	unsigned long bytes;
	struct iddeftab iddeftab[IDDEFTAB];
};
