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
 * National Cartridge Tape Drive
 * Dump routine
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"

struct device {
	short	status;
	short	clrinhib;
	short	setinhib;
	short	fill;
	short	ack;
};

#define cmdfirst	status
#define cmdlast	setinhib

struct tc_iocb {
	char	tc_opcode;
	char	tc_count;
	short	tc_fill;
	caddr_t tc_buffer;
	char	tc_status;
	char	tc_resid;
	short	tc_tapestat;
	short	tc_errcount;
	short	tc_overrun;
	caddr_t	tc_chain;
	char	tc_endstat;
	char	tc_fill2;
} td_iocb;
char td_err;

static short stdat[] = {
	0x2040, 0x810, 0x204, 0xfe01, 0xfbfd, 0xeff7, 0xbfdf
};

#define DMPBLK	32
int td_buf[DMPBLK];

#define TCADDR ((struct device *)0xd00200)

#define BUSY	0
#define ERROR	1
#define DONE	2
#define IDLE	3

#define START	0x31
#define WCOM	0x1c
#define REWIND	0x3e
#define ARMERASE 0xcc
#define WRITEMK 0x28
#define SENSE	0x22
#define SELFTEST 0xaa
#define DISPLAY	0x0e

#define WRLOCK	0x10
#define OFFLINE	0x20
#define NOTAPE	0x40

tdopen()
{
	register ds;

	if (td_cmd(SELFTEST) != 0)
		goto fail;
	if (td_cmd(SENSE) != 0)
		goto fail;
	ds = td_iocb.tc_tapestat;
	if (ds & (OFFLINE|NOTAPE|WRLOCK))
		goto fail;
	td_cmd(REWIND);
	td_cmd(ARMERASE);
	return 0;

fail:
	return 1;
}

tdclose()
{
	td_cmd(WRITEMK);
	td_cmd(REWIND);
}

td_cmd(com)
{
	register int i = com;
	register struct tc_iocb *io = &td_iocb;
	char *addr;

	switch (i) {
	default:
		tddoit(i, 0, 0, 0);
		break;
	case SENSE:
		io->tc_status = 0;
		io->tc_tapestat = 0;
		io->tc_errcount = 0;
		addr = &io->tc_status;
		tddoit(i, 0, 2, &addr);
		break;
	case SELFTEST:
		i = TCADDR->setinhib;
		tddoit(SELFTEST, 0x80, 7, stdat);
		break;
	}
	i = TCADDR->clrinhib;
	return td_err;
}

tcdump()
{
	extern int physmem;
	register struct tc_iocb *io = &td_iocb;
	char *addr;
	register base, i;
	int blkno;

	printf("\nTape Dump\n");
	printf("%d memory blocks\n%d blocks per record\n", ptod(physmem), DMPBLK);
	if (tdopen() != 0)
		goto out;
	for (blkno=0, base=0; base < ptod(physmem); base += DMPBLK) {
		tddoit(DISPLAY, blkno++ & 07, 0, 0);
		for (i=0; i<DMPBLK; i++)
			td_buf[i] = (base+i)<<9;
		io->tc_opcode = WCOM;
		io->tc_buffer = (caddr_t)td_buf;
		io->tc_count = DMPBLK;	/* set page count */
		io->tc_resid = DMPBLK;	/* here too in case sick port */
		io->tc_status = 0;	/* clear controller status */
		io->tc_tapestat = 0;	/* clear tape status */
		io->tc_errcount = 0;	/* and error counter */
		io->tc_endstat = 0;		/* clear ending status */
		io->tc_chain = 0;		/* set no chaining */
		addr = (char *) io;
		tddoit(START, 0, 2, &addr);		/* start IO */
		if (td_err)
			goto out;
	}
	tdclose();
out:
	if (td_err)
		printf("dump failed\n");
	else
		printf("dump completed successfully\n");
}

tddoit(opcode, opdata, argcount, argptr)
char	opcode;			/* opcode in first byte */
char	opdata;			/* char of data in second byte */
register int	argcount;	/* number of remaining words of data */
register short	*argptr;	/* pointer to word list */
{
	register short	firstarg;	/* first word to write */
	register struct device *rp;
	register status;

#ifdef DEBUG
	printf("tcdo %x", opcode);
#endif
	rp = TCADDR;
again:
	while ((rp->status & 3) == BUSY)
		;
	firstarg = ((opcode & 0xff) | ((opdata & 0xff) << 8));
	if (argcount <= 0) {
		rp->cmdlast = firstarg;
	} else {
		rp->cmdfirst = firstarg;
		while (--argcount > 0) rp->cmdfirst = *argptr++;
		rp->cmdlast = *argptr;
	}
wait:
	status = rp->status & 3;
	if (status == BUSY || status == IDLE)
		goto wait;
	rp->ack = 0;
#ifdef DEBUG
	printf(" status %d\n", status);
#endif
	if (status != DONE) {
		if (td_err++ < 7)
			goto again;
		else {
			printf("cmd %x failed\n", opcode);
			return;
		}
	} else
		td_err = 0;
}
