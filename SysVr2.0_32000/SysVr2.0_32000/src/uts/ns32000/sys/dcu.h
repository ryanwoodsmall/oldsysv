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
 * @(#)dcu.h	3.5	7/14/83
 * @(#)Copyright (C) 1983 by National Semiconductor Corp.
 */

/* Definitions for Mesa DCU (disk controller) */

/* status bits in immediate status byte (DCUA_STATUS) */
#define DS_CHAN1	0x03		/* channel 1 status code */
#define DS_CHAN0	0x0c		/* channel 0 status code */
#define DS_AUXERR	0x10		/* auxillery error (illegal command) */
#define DS_SELFTEST	0x20		/* DCU executing self test */
#define DS_ERINT	0x40		/* error status in ERINT byte */
#define DS_READY	0x80		/* command port is ready */

/* channel status codes (in DCUIS_CHAN0 and DCUIS_CHAN1 fields) */
#define DS_BUSY	0		/* channel is executing a command */
#define DS_ERROR	1		/* command completed with error */
#define DS_DONE	2		/* command completed successfully */
#define DS_IDLE	3		/* channel is idle */

#define	DSTMASK	3		/* mask to get channel status code */
#define	DSTSH0	2		/* shift to position channel 0 code */
#define	DSTSH1	0		/* shift to position channel 1 code */
#define	STATUS0(data)	(((data)>>DSTSH0)&DSTMASK) /* get DS_CHAN0 */
#define	STATUS1(data)	(((data)>>DSTSH1)&DSTMASK) /* get DS_CHAN1 */


/* command codes (written into DCUA_CHAN0 or DCUA_CHAN1 bytes) */
#define DC_SELFTEST	0x16		/* self test */
#define	DC_START	0x18		/* start port for channel command */
#define DC_SENSE	0x84		/* sense I/O */
#define DC_SETBURST	0x74		/* set burst/interleave */
#define DC_ASSDRIVE	0x20		/* assign drive */
#define DC_ASSCYLINDER 0x2e		/* assign cylinder */
#define DC_ASSBUFFER	0x42		/* assign buffer address */
#define	DC_WRITE	0xe6		/* disk write */
#define DC_READ	0xde		/* disk read */
#define DC_FORMAT	0xbc		/* format write */
#define	DC_SPECIFIC	0xe8		/* drive specific (seek, rezero) */
#define DC_DIAGREAD	0x8a		/* diagnostic memory read */
#define DC_DIAGWRITE	0xb2		/* diagnostic memory write */

/* drive specific codes (used with DC_SPECIFIC command) */
#define DC_ENABLE	0x41		/* write protect/enable a drive */
#define DC_REZERO	0x04		/* seek to cylinder 0, write protect */
#define DC_SEEK	0x03		/* seek to cylinder */


/* Macros to define data for various commands */
#define	DD_DRIVE(drive)	((drive)<<13)
					/* data for DCU_ASSDRIVE */
#define	DD_SEEK		(DC_SEEK<<8)
					/* data for DC_SPECIFIC */
#define	DD_ENABLE(bit)	((DC_ENABLE<<8)|((bit)<<7))
					/* data for DC_SPECIFIC */
#define	DD_DHS(drive,head,sector) (((drive)<<13)|((head)<<8)|(sector))
					/* data for DCU_READ and DCU_WRITE */



/* Structure of a IOCB block for channel IO (started with DC_START) */

struct dcu_iocb {
/* The following is not used by the controller, but is here for convenience. */
/* ALSO, it makes all of the offsets for the data correctly word aligned. */
	unsigned short	dc_skip;	/* number of sectors skipped */

/* The following is the actual data used by the controller */
	unsigned char	dc_opcode;	/* DCU command opcode */
	unsigned char	dc_res1;	/* reserved */
	unsigned char	dc_drivepar;	/* drive parameter out */
	unsigned char	dc_count;	/* count of sectors to transfer */
	unsigned short	dc_res2;	/* reserved */
	unsigned short	dc_cylinder;	/* cylinder for transfer */
	unsigned short	dc_drhdsec;	/* drive, head, and sector */
	unsigned long	dc_buffer;	/* address of buffer table */
	unsigned long	dc_sense;	/* sense bytes 0, 1, 2, and 3 */
	unsigned long	dc_ecc;	/* ecc bytes 0, 1, 2, and 3 */
	unsigned long	dc_chain;	/* address of next IOCB in chain */
	unsigned char	dc_status;	/* ending status byte */
	unsigned char	dc_resid;	/* number of sectors not transfered */
};

#define	dc_drivecmmd dc_sectors	/* drive specific data */
#define	IOCHAIN	(0x80000000)		/* bit in dc_chain to validate it */


/* bit fields inside of the ending status byte of the IOCB (dc_status) */
#define	DCUES_DRIVCHECK	0x08		/* drive check (in sense bytes 1-3) */
#define	DCUES_CTRLCHECK	0x10		/* controller check (in ctrl fault) */
#define	DCUES_BUSCHECK	0x20		/* bus check (in DCUSEN_BUSFAULT) */
#define	DCUES_CHAIN	0x40		/* command chain acknowledge */
#define	DCUES_DONE	0x80		/* done with all IOCBs */



/* Sense bytes. Read by the DC_SENSE immediate commmand, or automatically
 * stored in the dc_sense field of the IOCB block if command chaining and
 * and error occurs.
 */

#define BAD_ERR	0xff0f0f
#define CTL_FAULT 0xf0
#define ECC_ERR	0x40
