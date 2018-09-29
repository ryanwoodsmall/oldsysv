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
# ifndef NOSCCS
static char *sccsid = "@(#)tcusaio.c	1.11	10/10/83";
static char *cpyrid = "@(#)Copyright (C) 1983 by National Semiconductor Corp.";
# endif

/* Quickie Mesa Tape Channel IO module */
#undef DEBUG

#include "tcu.h"

#define	PAGESIZE	512		/* page size */
#define	PAGEOFFSET	511		/* maximum offset into a page */
#define	MAXPAGES	127		/* maximum pages to transfer */
#define BADR(n) ((unsigned char *)(n))	/* byte address recast */
#define	WADR(n)	((unsigned short *)(n))	/* word address recase */
#define TCUA_STATUS	BADR(0xd00208)  /* immediate status (read only) */
#define TCUA_ACK	BADR(0xd00208)  /* interrupt acknowledge location */
#define	TCUA_CLRINHIBIT	BADR(0xd00202)	/* clear inhibit of tape unit */
#define	TCUA_SETINHIBIT	BADR(0xd00204)	/* set inhibit of tape unit */
#define	TCUA_CMMDFIRST	WADR(0xd00200)	/* first command word location */
#define	TCUA_CMMDLAST	WADR(0xd00204)	/* last command word location */
#define	NOERROR		0		/* no error occurred */
#define	ARGERROR	1		/* bad arguments */
#define	CMMDERROR	2		/* illegal command sequence */
#define	SENSEERROR	3		/* failed to read sense bytes */
#define	EOFERROR	4		/* end of file error (file mark) */
#define	EOTERROR	5		/* end of tape error */
#define	TCUERROR	6		/* other errors (see sense bytes) */

char	tcuinitf;			/* nonzero if tape is initialized */
char	tcuwritef;			/* nonzero if ready for writing */
struct	tcu_iocb	tcuiocb;	/* iocb data */
struct	tcu_sense	tcusense;	/* sense data for error handling */
char	tcuerrstat;			/* immediate status on error */
char	tcuerrcmmd;			/* command which caused error */
long	tcerror = NOERROR;		/* error value for dotapeio */
long	tcupt[MAXPAGES];		/* storage for page table */


/* The following data is words 2 through 8 of the selftest command */
static	short	stdat[] = {
	0x2040, 0x0810, 0x0204, 0xfe01, 0xfbfd, 0xeff7, 0xbfdf
};

/* Subroutine to read/write the Mesa tape.  Returns the number of sectors
 * successfully transferred, and sets tcerror to one of the error codes
 * such as NOERROR, ARGERROR, etc.
 * Parameters:
 *	writeflag/	Zero to read, nonzero to write
 *	count/		number of pages to transfer
 *	address/	memory address for transfer (must be on page boundary)
 */
dotapeio(writeflag, count, address)
{
	register struct	tcu_iocb *io;
	register long	pg, curcount, addr, remaining;
	long	opcode;

#ifdef DEBUG
	printf("dotapeio: writeflag=%d, count=%d, address=%x\n",
		writeflag, count, address);
#endif
	tcerror = inittcunit();
	if (tcerror) goto done;
	if (((address & PAGEOFFSET) != 0) || (count <= 0)) {
		tcerror = ARGERROR;
		goto done;
	}
	opcode = writeflag ? TCUC_WRITE : TCUC_READ;
	if (opcode == TCUC_WRITE) {
		if (!tcuwritef) tcerror = tcucommand(TCUC_ARMERASE);
		if (tcerror) goto done;
	}
	io = &tcuiocb;
	remaining = count;
	while (remaining > 0) {
		curcount = remaining;
		if (curcount > MAXPAGES) curcount = MAXPAGES;
		addr = address;
		for (pg = 0; pg < curcount; pg++) {
			tcupt[pg] = addr;
			addr += PAGESIZE;
		}
		io->tcuio_opcode = opcode;
		io->tcuio_buffer = (int)tcupt;
		io->tcuio_count = curcount;
		io->tcuio_sense.tcusen_resid = curcount;
		io->tcuio_sense.tcusen_status = 0;
		io->tcuio_sense.tcusen_tapestat = 0;
		io->tcuio_chain = 0;
		tcerror = tcucommand(TCUC_START);
		remaining -= curcount;
		if (tcerror) {
			remaining += io->tcuio_sense.tcusen_resid;
			goto done;
		}
		address += curcount * PAGESIZE;
	}
#ifdef DEBUG
	printf("dotapeio: return %d\n", count-remaining);
#endif
done:	return (count-remaining);
}

/* Routines to do particular functions to the tape. */
doretension()
{
	register long	error;

	error = inittcunit();
	if (!error) error = tcucommand(TCUC_RETENSION);
	return(error);
}



dorewind()
{
	register long	error;

	error = inittcunit();
	if (!error) error = tcucommand(TCUC_REWIND);
	return(error);
}


doskipfile()
{
	register long	error;

	error = inittcunit();
	if (!error) error = tcucommand(TCUC_SEARCHMARK);
	return(error);
}



dowritemark()
{
	register long	error;

	error = inittcunit();
	if (error) goto done;
	if (!tcuwritef) error = tcucommand(TCUC_ARMERASE);
	if (error) goto done;
	error = tcucommand(TCUC_WRITEMARK);

done:	return(error);
}

/* Initialize the unit of the TCD.  This performs a selftest, and rewinds
 * the tape.
 */
inittcunit()
{
	register long	error;

	if (tcuinitf) return(NOERROR);
	error = tcucommand(TCUC_SELFTEST);
	if (error) goto done;
	error = tcucommand(TCUC_SENSE);
	if (error) goto done;
	error = tcucommand(TCUC_REWIND);
	if (error) goto done;
	tcuinitf = 1;

done:	return(error);
}

/* Execute an operation on the tape, and wait for it to complete.  Returns
 * whether or not an error occurred, and if so, what kind of error it was.
 */
tcucommand(opcode)
{
	register char	immstat;	/* immediate status of the port */
	register long	status;		/* channel status from sense bytes */
	register long	tapestat;	/* tape status from sense bytes */
	static	long	dummy;		/* to force optimizer references */
	long	addr;			/* address */

#ifdef DEBUG
	printf("tcucommand: %x\n", opcode);
#endif
again:	dummy += *TCUA_CLRINHIBIT;
	switch (opcode) {
		case TCUC_START:
			if ((tcuiocb.tcuio_opcode == TCUC_READ)
				!= (tcuwritef == 0))
					return(CMMDERROR);
			addr = (int) &tcuiocb;
			tcudoio(TCUC_START, 0, 2, &addr);
			break;

		case TCUC_REWIND:
		case TCUC_RETENSION:
		case TCUC_SEARCHMARK:
			tcuwritef = 0;
			tcudoio(opcode, 0, 0, 0);
			break;

		case TCUC_WRITEMARK:
			if (!tcuwritef) return(CMMDERROR);
			tcudoio(TCUC_WRITEMARK, 0, 0, 0);
			break;

		case TCUC_ARMERASE:
			if (tcuwritef) return(CMMDERROR);
			tcuwritef = 1;
			tcudoio(TCUC_ARMERASE, 0, 0, 0);
			break;

		case TCUC_SENSE:
			addr = (int) &tcusense;
			tcudoio(TCUC_SENSE, 0, 2, &addr);
			break;

		case TCUC_SELFTEST:
			/* The controller requires that the tape unit
			 * be inhibited before doing a selftest command.
			 */
			dummy += *TCUA_SETINHIBIT;
			tcuwritef = 0;
			tcudoio(TCUC_SELFTEST, 0x80, 7, stdat);
			break;

		default: 
			return(ARGERROR);
	}
	immstat = tcuwait();
	if (STATUS(immstat) == TCUSTS_DONE) return (NOERROR);	/* successful */
	tcuerrstat = immstat;		/* save status for caller to see */
	tcuerrcmmd = opcode;
	if (opcode == TCUC_START) tcusense = tcuiocb.tcuio_sense;
	if ((opcode != TCUC_START) || (immstat & TCUIS_STSAVAIL)) {
		addr = (int) &tcusense;
		tcudoio(TCUC_SENSE, 0, 2, &addr);
		if (STATUS(tcuwait()) != TCUSTS_DONE) return(SENSEERROR);
	}
	status = tcusense.tcusen_status;
	tapestat = tcusense.tcusen_tapestat;
	if (immstat & (TCUIS_SEQERR|TCUIS_MIFCHK)) goto lose;
	/*
	 * The wonderful tape controller reads ahead, and can cause a file mark
	 * error AFTER a read has completed, thus making the next command give
	 * an "exception during command" error, even for things like rewind !!
	 * So we have to ignore such errors and retry the command.
	 */
	if (immstat & TCUIS_CTRLCHK) {
		if (CTLFAULTCODE(status) != TCUCF_EXCCMMD) goto lose;
		if ((tapestat & ~TCUSB_WRITELOCK) !=
			(TCUSB_FILEMARK | TCUSB_ANY0)) goto lose;
		goto again;
	}
	if (tapestat & TCUSB_RESET) {
		tcuinitf = 0;			/* drive was reset */
		goto lose;
	}
	if (tapestat & (TCUSB_OFFLINE|TCUSB_NOTAPE|TCUSB_ILLCMMD)) goto lose;
	if (tapestat & TCUSB_FILEMARK) {
		if (opcode == TCUC_SEARCHMARK) return (NOERROR);
		return(EOFERROR);		/* tape mark */
	}
	if (tapestat & TCUSB_NODATA) {
		tcuwritef = 1;
		return (EOTERROR);		/* end of tape */
	}

lose:	return (TCUERROR);			/* all other errors */
}

/* Routine to write a command to the port.  Commands are written 16 bits at
 * a time, using one address for all but the last word, and another address
 * for the final word.  Arguments:
 *   opcode/	A byte of opcode determining the immediate command.
 *   opdata/	A byte of data written in the same word as the opcode.
 *   argcount/	The number of additional 16 bit words to be written.
 *   argptr/	The list of words to write.
 *
 */
tcudoio(opcode, opdata, argcount, argptr)
	char	opcode;			/* opcode in first byte */
	char	opdata;			/* char of data in second byte */
	register int	argcount;	/* number of remaining words of data */
	register short	*argptr;	/* pointer to word list */
{
	register short	firstarg;	/* first word to write */

	firstarg = ((opcode & 0xff) | ((opdata & 0xff) << 8));
	if (argcount <= 0) {
		*TCUA_CMMDLAST = firstarg;
		return;
	}
	*TCUA_CMMDFIRST = firstarg;
	while (--argcount > 0) {
		tcudummy();		/* make optimizer do references */
		*TCUA_CMMDFIRST = *argptr++;
	}
	*TCUA_CMMDLAST = *argptr;
	return(NOERROR);
}



/* Wait until the tcu is done with the current command, and return the
 * final status.  The port is acknowledged.
 */
tcuwait()
{
	register char	stat;		/* status */
	register char	code;		/* state code */

	while (1) {
		stat = *TCUA_STATUS;
		code = STATUS(stat);
		if (code != TCUSTS_BUSY) break;
		tcudummy();
	};
	if (code != TCUSTS_IDLE) *TCUA_ACK = '\0';
	return(stat);
}


/* Subroutine called to make optimizer do device register accesses */
tcudummy() {}
