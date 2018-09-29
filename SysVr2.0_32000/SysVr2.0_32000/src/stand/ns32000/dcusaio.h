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


/*
 *     DCU definitions for dcutest.c
 */


/* Error codes returned by dcusaio routines */
#define	NOERROR		0		/* code for no errors (must be zero) */
#define	ARGUMENTERROR	-1		/* bad calling arguments error */
#define	DCUERROR	-2		/* non-io DCU errors */
#define	SENSEERROR	-3		/* error with sense bytes which are 0 */
#define	TYPEERROR	-4		/* unknown disk type */

#define	MAXUNITS	8		/* number of units */

struct iocb {				/* IOCB structure */
	unsigned short	dcuio_crock;	/* needed for proper alignment */
	unsigned char	dcuio_opcode;	/* DCU command opcode */
	unsigned char	dcuio_res1;	/* reserved */
	unsigned char	dcuio_drivepar;	/* drive parameter out */
	unsigned char	dcuio_count;	/* count of sectors to transfer */
	unsigned short	dcuio_res2;	/* reserved */
	unsigned short	dcuio_cylinder;	/* cylinder for transfer */
	unsigned short	dcuio_drhdsec;	/* drive, head, and sector */
	unsigned long	dcuio_buffer;	/* address of buffer table */
	unsigned long	dcuio_sense;	/* sense bytes 0, 1, 2, and 3 */
	unsigned long	dcuio_ecc;	/* ecc bytes 0, 1, 2, and 3 */
	unsigned long	dcuio_chain;	/* address of next IOCB in chain */
	unsigned char	dcuio_status;	/* ending status byte */
	unsigned char	dcuio_resid;	/* number of sectors not transfered */
};


/* bit fields inside of the ending status byte of the IOCB (dcuio_status) */
#define	DCUES_DRIVCHECK	0x08		/* drive check (in sense bytes 1-3) */
#define	DCUES_CTRLCHECK	0x10		/* controller check (in ctrl fault) */
#define	DCUES_BUSCHECK	0x20		/* bus check (in DCUSEN_BUSFAULT) */
#define	DCUES_CHAIN	0x40		/* command chain acknowledge */
#define	DCUES_DONE	0x80		/* done with all IOCBs */


/* Sense bytes. Read by the DCUC_SENSE immediate commmand, or automatically
 * stored in the dcuio_sense field of the IOCB block if command chaining and
 * and error occurs.
 */
#define DCUSB_SEQERR	0x00000001	/* sequence error */
#define DCUSB_BUSFAULT	0x0000000e	/* bus fault error code */
#define DCUSB_CTLFAULT	0x000000f0	/* control fault error code */
#define DCUSB_NOTREADY	0x00000100	/* device not ready */
#define DCUSB_BUSERROR	0x00000200	/* control bus error */
#define DCUSB_ILLCMMD	0x00000400	/* illegal command */
#define DCUSB_ILLPARAM	0x00000800	/* illegal parameter */
#define DCUSB_SENSE2	0x00001000	/* sense byte 2 nonzero */
#define DCUSB_SENSE3	0x00002000	/* sense byte 3 nonzero */
#define DCUSB_BUSY	0x00004000	/* busy executing */
#define DCUSB_NORMAL	0x00008000	/* normal completion */
#define DCUSB_SEEKERR	0x00010000	/* seek error */
#define DCUSB_RWFAULT	0x00020000	/* read/write fault */
#define DCUSB_POWER	0x00040000	/* power fault */
#define DCUSB_VIOLATION	0x00080000	/* read/write permit violation */
#define DCUSB_SPEEDERR	0x00100000	/* speed error */
#define DCUSB_REJECT	0x00200000	/* command reject */
#define DCUSB_OTHERERR	0x00400000	/* other errors */
#define DCUSB_VENDORERR 0x00800000	/* vendor unique errors */
#define DCUSB_INITSTATE 0x01000000	/* initial state */
#define DCUSB_RDYTRANS	0x02000000	/* ready transition */
#define DCUSB_THISPORT	0x04000000	/* reserved to this port */
#define DCUSB_RELEASED	0x08000000	/* forced release */
#define DCUSB_ALTPORT	0x10000000	/* reserved to alternate port */
#define DCUSB_ATTMOD	0x20000000	/* attribute table modified */
#define DCUSB_WRITELOCK	0x40000000	/* within write-protected area */
#define DCUSB_VENDATTN	0x80000000	/* vendor unique attentions */


/* macros to extract fields from the sense bytes */
#define	BUSFAULTCODE(data)	(((data) & DCUSB_BUSFAULT) >> 1)
#define	CTLFAULTCODE(data)	(((data) & DCUSB_CTLFAULT) >> 4)


/* bus fault definitions (in DCUSB_BUSFAULT field of the sense bytes) */
#define	DCUBF_NOERROR	0		/* no error */
#define	DCUBF_DRIVETIME	1		/* drive acknowledge time-out */
#define	DCUBF_MESAERROR	2		/* Mesa interface error detected */
#define	DCUBF_PARITY	3		/* cbus input parity error */
#define	DCUBF_INDEXTIME	4		/* index/sector pulse time-out */
#define	DCUBF_WORDTIME	5		/* word flag time-out */
#define	DCUBF_DMATIME	6		/* DMA transfer time-out */
#define	DCUBF_CBUSBUSY	7		/* cbus busy */

/* control fault codes (in dcus_ctrlfault field of the sense bytes) */
#define	DCUCF_NOERROR	0		/* no error */
#define	DCUCF_OCCUPIED	1		/* drive occupied */
#define	DCUCF_NOSECTOR	2		/* sector not found */
#define	DCUCF_ECCHEADER	3		/* ecc error in header field */
#define	DCUCF_ECCDATA	4		/* ecc error in data field */
#define	DCUCF_ILLPARAM	5		/* illegal dcu parameter */
#define	DCUCF_ILLCMMD	6		/* illegal dcu command */
#define	DCUCF_REJECT	7		/* reject dcu command */
#define	DCUCF_WRITELOCK	8		/* write protection error */
#define	DCUCF_MEMERROR	9		/* memory read error */
#define	DCUCF_SYNCERROR	10		/* sync error in data field */
#define	DCUCF_DRIVEERR	11		/* drive fault */
#define	DCUCF_NODRIVES	12		/* no available drive */
#define	DCUCF_DRIVEADDR	13		/* incorrect drive address */
					/* codes 14-15 are undefined */

struct disktab *unittable[MAXUNITS];	/* table of disk information */
struct iocb	erroriocb;		/* iocb at last disk error */
int	noretries;			/* nonzero if retries disabled */
