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
 * @(#)tcu.h	3.2	7/14/83
 * @(#)Copyright (C) 1983 by National Semiconductor Corp.
 */

/*	tcu.h	definitions for Mesa 16000 TCD (tape controller device)	*/

/* status bits in immediate status byte (TCUA_STATUS) */
#define	TCUIS_CHAN	0x03		/* channel status code */
#define	TCUIS_SEQERR	0x04		/* command sequence error */
#define	TCUIS_STSAVAIL	0x08		/* drive status available */
#define	TCUIS_CTRLCHK	0x10		/* controller check */
#define	TCUIS_MIFCHK	0x20		/* mesa interface check */
#define	TCUIS_ERRINT	0x40		/* non-maskable interrupt (unused) */
#define	TCUIS_SELFTEST	0x80		/* TCU executing self test */

/* channel status codes (in TCUIS_CHAN field) */
#define TCUSTS_BUSY	0		/* channel is executing a command */
#define TCUSTS_ERROR	1		/* command completed with error */
#define TCUSTS_DONE	2		/* command completed successfully */
#define TCUSTS_IDLE	3		/* channel is idle */

#define	STATUS(data)	((data)&3)	/* macro to get TCUIS_CHAN */


/* command codes (written into TCUA_CMMDFIRST or stored into tcuio_opcode) */
#define	TCUC_START	0x31		/* start channel command */
#define	TCUC_WRITE	0x1c		/* tape write */
#define	TCUC_READ	0x3b		/* tape read */
#define	TCUC_REWIND	0x3e		/* tape rewind */
#define	TCUC_ARMERASE	0xcc		/* arm erase */
#define	TCUC_TAPEERASE	0x25		/* erase whole tape */
#define	TCUC_WRITEMARK	0x28		/* write file mark */
#define	TCUC_SEARCHMARK	0x19		/* search for file mark */
#define	TCUC_RETENSION	0x13		/* tape retension */
#define	TCUC_SENSE	0x22		/* sense I/O */
#define	TCUC_DISPLAY	0x0e		/* change display indicator */
#define	TCUC_SELFTEST	0xaa		/* perform diagnostics */
#define	TCUC_DIAGREAD	0x06		/* mesa interface read */
#define	TCUC_DIAGWRITE	0x12		/* mesa interface write */
#define	TCUC_DUMPSTATE	0x95		/* dump hardware state */


/* Sense bytes returned by controller in IOCB or from TCUC_SENSE command */

struct	tcu_sense {
	unsigned char	tcusen_status;	/* error status bits */
	unsigned char	tcusen_resid;	/* residual page count */
	unsigned short	tcusen_tapestat; /* status bytes 0 and 1 from drive */
	unsigned short	tcusen_errcount; /* tape data block error count */
	unsigned short	tcusen_overrun;	/* tape data block overrun count */
};


/* Structure of a IOCB block for channel IO (started with TCUC_START) */

struct	tcu_iocb {
	unsigned char	tcuio_opcode;	/* command opcode */
	unsigned char	tcuio_count;	/* count of pages to transfer */
	unsigned short	tcuio_res1;	/* reserved */
	unsigned long	tcuio_buffer;	/* address of transfer buffer */
	struct tcu_sense tcuio_sense;	/* sense block on error */
	unsigned long	tcuio_chain;	/* next iocb address in chain */
	unsigned char	tcuio_endstat;	/* ending status */
	unsigned char	tcuio_res2;	/* reserved */
};


#define	IOCHAIN	(0x80000000)		/* bit in tcuio_chain to validate it */



/* bit definitions in the ending status byte of the IOCB (tcuio_endstat) */
#define	TCUES_CHAN	0x03		/* channel status (like TCUIS_CHAN) */
#define	TCUES_AVAIL	0x08		/* drive status is available */
#define	TCUES_CTRLCHK	0x10		/* controller check */
#define	TCUES_MIFCHK	0x20		/* mesa interface check */
#define	TCUES_CHAIN	0x40		/* command chain acknowledge */
#define	TCUES_DONE	0x80		/* iocb completion */


/* bit definitions in status bytes 0 and 1 of the drive (tcuio_tapestat) */
#define	TCUSB_FILEMARK	0x0001		/* file mark detected */
#define	TCUSB_BADBLOCK	0x0002		/* bad block detected */
#define	TCUSB_DATAERROR	0x0004		/* unrecoverable data error */
#define	TCUSB_ENDOFTAPE	0x0008		/* end of tape */
#define	TCUSB_WRITELOCK	0x0010		/* write-protected tape */
#define	TCUSB_OFFLINE	0x0020		/* drive not online */
#define	TCUSB_NOTAPE	0x0040		/* cartridge not in place */
#define	TCUSB_ANY0	0x0080		/* some of above errors are set */

#define	TCUSB_RESET	0x0100		/* reset occured */
#define	TCUSB_RES1	0x0600		/* unused */
#define	TCUSB_STARTTAPE	0x0800		/* start of tape */
#define	TCUSB_RETRYS	0x1000		/* eight or more retries */
#define	TCUSB_NODATA	0x2000		/* no data detected */
#define	TCUSB_ILLCMMD	0x4000		/* illegal drive command */
#define	TCUSB_ANY1	0x8000		/* some of above errors are set */


/* macros to extract fields from the tcuio_status field of the iocb */
#define	TCUSB_CTLFAULT	0x1f		/* controller fault code field */
#define	TCUSB_MIFFAULT	0xe0		/* mesa interface fault code field */
#define	CTLFAULTCODE(data)	((data) & TCUSB_CTLFAULT)
#define	MIFFAULTCODE(data)	(((data) & TCUSB_MIFFAULT) >> 5)


/* controller fault codes (from TCUSB_CTLFAULT field) */
#define	TCUCF_NOERROR	0		/* no error */
#define	TCUCF_NOCMMD	1		/* command not found */
#define	TCUCF_SEQERR	2		/* improper command sequence */
#define	TCUCF_PGIOCB	3		/* page violation in iocb */
#define	TCUCF_PGTABLE	4		/* page violation in buffer table */
#define	TCUCF_PGBLOCK	5		/* page violation in sense/dump block */
#define	TCUCF_EXCCMMD	6		/* command not allowed in exception */
#define	TCUCF_CTRLFLT	7		/* fault in controller */
#define	TCUCF_FIFO	8		/* fifi has too many bytes */
#define	TCUCF_MIFERR	9		/* data compare during mif read test */
#define	TCUCF_CMDINHIB	17		/* tape action is inhibited */
#define	TCUCF_STSHUNG	18		/* getsts timeout (drive reset) */
#define	TCUCF_DATAINHIB	19		/* tape inhibited in data transfer */
#define	TCUCF_EXCREAD	20		/* exception during status read */
#define	TCUCF_CMDPAR	21		/* parity error in command to drive */
#define	TCUCF_STSPAR	22		/* parity error in status read */
#define	TCUCF_DATAPAR	23		/* parity error in data transfer */
#define	TCUCF_CMDTIME	24		/* timeout issuing command to drive */
#define	TCUCF_POSTIME	25		/* timeout rewind, retension, erase */
#define	TCUCF_SRCTIME	26		/* timeout during search */
#define	TCUCF_BLKTIME	27		/* timeout waiting for next block */
#define	TCUCF_DATATIME	28		/* timeout during block transfer */

/* mesa interface error codes (from TCUSB_MIFFAULT field) */
#define	TCUMF_NOERROR	0		/* no error */
#define	TCUMF_MIFIOCB	1		/* iocb parameter parity error */
#define	TCUMF_MIFTABLE	2		/* page table entry parity error */
#define	TCUMF_MIFCHAIN	3		/* next chain address parity error */
#define	TCUMF_MIFREAD	4		/* data read parity error */
#define	TCUMF_MIFDATA	5		/* data timeout */
#define	TCUMF_MIFCMMD	6		/* command timeout */
#define	TCUMF_CLOSE	7		/* iocb close-up timeout */
