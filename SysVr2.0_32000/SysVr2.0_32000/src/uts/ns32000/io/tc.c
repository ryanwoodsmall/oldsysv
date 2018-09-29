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
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/errno.h"
#include "sys/buf.h"
#include "sys/elog.h"
#include "sys/iobuf.h"
#include "sys/systm.h"
#include "sys/mtio.h"
#include "sys/file.h"
#include "sys/page.h"
#include "sys/mmu.h"

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
} tc_iocb;

static short stdat[] = {
	0x2040, 0x810, 0x204, 0xfe01, 0xfbfd, 0xeff7, 0xbfdf
};

int tapebuf[MAXBLK];

struct iotime	tcstat;
struct iobuf	tctab = tabinit(TC0,&tcstat.ios);
struct buf	ctcbuf;

char	t_openf;
char t_flags;

#define DIDINIT	1
#define ATEOF	2
#define WROTE	4

#define INF	1000000

extern struct device *tc_addr[];

#define BUSY	0
#define ERROR	1
#define DONE	2
#define IDLE	3
#define SEQERR	4
#define STSAVAIL 8
#define CTRLCHK	0x10
#define MIFCHK	0x20

#define START	0x31
#define WCOM	0x1c
#define RCOM	0x3b
#define REWIND	0x3e
#define ARMERASE 0xcc
#define ERASE	0x25
#define WRITEMK 0x28
#define FINDMK	0x19
#define RETEN	0x13
#define SENSE	0x22
#define SELFTEST 0xaa

#define	FILEMARK 1
#define ENDOFTAPE 8
#define WRLOCK	0x10
#define OFFLINE	0x20
#define NOTAPE	0x40
#define ANY0	0x80

#define RESET	0x100
#define RETRYS	0x1000
#define ILLCMMD	0x4000

#define SEQUENCE 2
#define EXCCMMD	6

#define SIO	1
#define SBAD	2
#define SCOM	3
#define SOK	4
#define SEOF	5
#define SREDO	6
#define SRESET  7

tcopen(dev, flag)
{
	register ds;

	if (t_openf) {
		u.u_error = ENXIO;
		return;
	}
	t_openf++;
	if ((t_flags&DIDINIT) == 0) {
		tctab.io_addr = (physadr)&tc_iocb.tc_status;
		tctab.io_nreg = 4;
		icuenable(0x10);
		if (tcommand(SELFTEST) != 0)
			goto fail;
	}
	if (tcommand(SENSE) != 0)
		goto fail;
	ds = tc_iocb.tc_tapestat;
	if ((ds&(OFFLINE|NOTAPE)) || ((flag&FWRITE) && (ds&WRLOCK)))
		goto fail;
	if ((t_flags&DIDINIT) == 0) {
		tcommand(REWIND);
		t_flags |= DIDINIT;
	}
	if (flag&FAPPEND)
		if (tcommand(FINDMK) != 0)
			goto fail;
	return;

fail:
	u.u_error = EIO;
	t_openf = 0;
}

tcclose(dev, flag)
{
	if (flag & FWRITE) {
		if ((t_flags&WROTE) == 0)
			tcommand(ARMERASE);
		tcommand(WRITEMK);
	}
	if (dev&04) {
		if ((flag&FWRITE) == 0) {
			if ((t_flags&ATEOF) == 0)
				tcommand(FINDMK);
		}
	} else
		tcommand(REWIND);
	t_openf = 0;
	t_flags &= DIDINIT;
}

tcommand(com)
{
	register struct buf *bp;

	bp = &ctcbuf;
	spl5();
	while (bp->b_flags&B_BUSY) {
		bp->b_flags |= B_WANTED;
		sleep(bp, PRIBIO+1);
	}
	spl0();
	bp->b_resid = com;
	bp->b_flags = B_BUSY;
	tcstrategy(bp);
	iowait(bp);
	if (bp->b_flags&B_WANTED)
		wakeup(bp);
	bp->b_flags = 0;
	return(bp->b_resid);
}

tcstrategy(bp)
register struct buf *bp;
{
	if (bp != &ctcbuf) {
		if ((bp->b_flags&B_PHYS) == 0) {
			if ((bp->b_flags&B_READ) && t_flags&WROTE) {
				clrbuf(bp);
				bp->b_resid = bp->b_bcount;
				iodone(bp);
				return;
			}
		}
		if ((bp->b_flags&B_READ) == 0 && (t_flags&WROTE) == 0) {
			tcommand(ARMERASE);
			t_flags |= WROTE;
		}
		bp->b_start = lbolt;
		tcstat.io_bcnt += btod(bp->b_bcount);
	}
	bp->av_forw = NULL;
	spl5();
	if (tctab.b_actf == NULL)
		tctab.b_actf = bp;
	else
		tctab.b_actl->av_forw = bp;
	tctab.b_actl = bp;
	if (tctab.b_active == 0)
		tcstart();
	spl0();
}

tcstart()
{
	register struct buf *bp;
	register struct tc_iocb *io = &tc_iocb;
	register int *pt;
	int i, blkcnt;
	char *addr;

loop:
	if ((bp = tctab.b_actf) == NULL)
		return;
	if (bp == &ctcbuf) {
		tcstat.ios.io_misc++;
		tctab.b_active = SCOM;
		i = bp->b_resid;
		switch (i) {
		default:
			tcdoio(i, 0, 0, 0);
			break;
		case SENSE:
			io->tc_status = 0;
			io->tc_tapestat = 0;
			io->tc_errcount = 0;
			addr = &io->tc_status;
			tcdoio(i, 0, 2, &addr);
			break;
		case SELFTEST:
			i = tc_addr[0]->setinhib;
			tcdoio(SELFTEST, 0x80, 7, stdat);
			break;
		}
		return;
	}
	if (t_openf < 0)
		goto abort;
	if ((bp->b_flags&B_READ) && (t_flags&ATEOF)) {
		bp->b_resid = bp->b_bcount;
		goto eof;
	}
	if (tctab.io_start == 0)
		tctab.io_start = lbolt;
	tctab.b_active = SIO;
	tcstat.io_cnt++;
	blkacty |= (1<<TC0);
	blkcnt = btod(bp->b_bcount);
	if (blkcnt > MAXBLK || blkcnt < 0)
		panic("bad bcount");
	if (bp->b_flags & B_PHYS) {
		physmap(bp, blkcnt, tapebuf);
	} else {
		pt = (int *)&kpte2[svtoc(paddr(bp))];
		for (i = 0; i < blkcnt; i++)
			tapebuf[i] = (*pt++) & PFNMASK;
	}
	if (bp->b_flags & B_READ)
		io->tc_opcode = RCOM;
	else
		io->tc_opcode = WCOM;
	io->tc_buffer = (caddr_t)tapebuf;
	io->tc_count = blkcnt;	/* set page count */
	io->tc_resid = blkcnt;	/* here too in case sick port */
	io->tc_status = 0;	/* clear controller status */
	io->tc_tapestat = 0;	/* clear tape status */
	io->tc_errcount = 0;	/* and error counter */
	io->tc_endstat = 0;		/* clear ending status */
	io->tc_chain = 0;		/* set no chaining */
	addr = (char *) io;
	tcdoio(START, 0, 2, &addr);		/* start IO */
	return;
abort:
	bp->b_flags |= B_ERROR;
eof:
	tctab.b_actf = bp->av_forw;
	iodone(bp);
	goto loop;
}

tcintr(dev)
{
	register struct buf *bp;
	register struct device *rp;
	char status, code, state;
	extern tcprint();

	rp = tc_addr[0];
	status = rp->status;
	code = status&3;
	if (code == BUSY) return;
	bp = tctab.b_actf;
	if (code == IDLE) {
		if (bp == NULL)
			return;
		panic("tcidle");
	}
	rp->ack = 0;
	state = rp->clrinhib;
	if (bp == NULL) {
		logstray(rp);
		return;
	}

	state = tctab.b_active;
	tctab.b_active = 0;
	if (code == ERROR) {
		state = tcerror(bp, status);
#ifdef TCDEBUG
		tcmon(1, 0x40+state);
#endif
		switch (state) {
		case SOK:
			bp->b_resid = 0;
			break;
		case SREDO:
			if (++tctab.b_errcnt < 8) {
				tcstart();
				return;
			}
			break;
		case SRESET:
			t_openf = -1;
			t_flags &= ~DIDINIT;
			break;
		case SEOF:
			bp->b_resid = dtob(tc_iocb.tc_resid);
			if (bp->b_resid == 0) /* stupid controller! */
				bp->b_resid = bp->b_bcount;
			t_flags |= ATEOF;
			state = SOK;
			break;
		}
		if (state != SOK) {
			fmtberr(&tctab, 0);
			bp->b_flags |= B_ERROR;
		}
	} else 
		bp->b_resid = 0;
	if (tctab.io_erec)
		logberr(&tctab, bp->b_flags&B_ERROR);
	tctab.b_errcnt = 0;
	tctab.b_actf = bp->av_forw;
	iodone(bp);
	if (bp != &ctcbuf) {
		tcstat.io_resp += lbolt - bp->b_start;
		tcstat.io_act += lbolt - tctab.io_start;
		tctab.io_start = 0;
	}
	tcstart();
}

tcdoio(opcode, opdata, argcount, argptr)
char	opcode;			/* opcode in first byte */
char	opdata;			/* char of data in second byte */
register int	argcount;	/* number of remaining words of data */
register short	*argptr;	/* pointer to word list */
{
	register short	firstarg;	/* first word to write */
	register struct device *rp;

#ifdef TCDEBUG
	tcmon(2, 1, opcode);
#endif
	rp = tc_addr[0];
	firstarg = ((opcode & 0xff) | ((opdata & 0xff) << 8));
	if (argcount <= 0) {
		rp->cmdlast = firstarg;
		return;
	}
	rp->cmdfirst = firstarg;
	while (--argcount > 0) rp->cmdfirst = *argptr++;
	rp->cmdlast = *argptr;
}

tcread(dev)
dev_t dev;
{
	if (aln_chk())
		fak_phys(tcstrategy, dev, B_READ, 1024);
	else
		physio(tcstrategy, 0, dev, B_READ);
}


tcwrite(dev)
dev_t dev;
{
	if (aln_chk())
		fak_phys(tcstrategy, dev, B_WRITE, 1024);
	else
		physio(tcstrategy, 0, dev, B_WRITE);
}

tcprint(dev, str)
char *str;
{
	printf("%s on TCU\n", str);
}

/*
 * look at the error cause.
 * return SOK if no real error
 *        SREDO if need to retry
 *	  SEOF if hit tape mark
 *	  SRESET if want to give up totally
 * else   SBAD
 */

tcerror(bp, immstat)
register struct buf *bp;
{
	register status, tapestat;
	register struct tc_iocb *io = &tc_iocb;

	if (immstat & STSAVAIL) {	/* get status */
		char * addr;

		addr = &io->tc_status;
		tcdoio(SENSE, 0, 2, &addr);
		tcwait();
	} else if (bp == &ctcbuf && bp->b_resid == ARMERASE)
		return(SOK);
	status = io->tc_status;
	tapestat = io->tc_tapestat;
#ifdef TCDEBUG
	tcmon(4, 2, immstat, status, tapestat);
#endif
	if (immstat & SEQERR)
		return(SBAD);
	if (immstat & MIFCHK)
		return(SRESET);
	if (immstat & CTRLCHK) {
		switch(status & 0x1f) {
		case SEQUENCE:
			return(SRESET);
		case EXCCMMD:
			if ((tapestat & ~(WRLOCK|ANY0)) == FILEMARK) {
				if (bp == &ctcbuf && bp->b_resid == FINDMK)
					return(SOK);
				return ((bp->b_flags&B_READ) ? SEOF : SREDO);
			}
		default:
			return (SBAD);
		}
	}
	if (tapestat & (RESET|OFFLINE|NOTAPE|ILLCMMD)) {
		binval(bp->b_dev);
		return (tapestat&RESET ? SRESET : SBAD);
	}
	if ((tapestat&(WRLOCK|ENDOFTAPE)) && (bp->b_flags&B_READ)==0)
		return(SBAD);
	if (tapestat & FILEMARK) {
		if (bp == &ctcbuf && bp->b_resid == FINDMK)
			return (SOK);
		return ((bp->b_flags&B_READ) ? SEOF : SBAD);
	}
	if (tapestat == RETRYS)
		return(SOK);
	return(SBAD);
}

tcwait()
{
	register struct device *rp = tc_addr[0];
	char code, status;
	int count;

	for (count=0; count<10000; count++) {
		status = rp->status;
		code = status&3;
		if (code != BUSY)
			goto out;
	}
	panic("tc wait");
out:
	if (code != IDLE)
		rp->ack = 0;
	return;
}

tcioctl(dev, cmd, addr, flag)
dev_t dev;
caddr_t addr;
{
	register int	func;		/* function code to execute */
	register int	count;		/* repeat counter */
	struct	mtop	mtop;		/* block for reading user data */

	switch (cmd) {
	case MTIOCTOP:	/* tape operation */
		if (copyin((caddr_t)addr, (caddr_t)&mtop, sizeof(mtop))) {
			u.u_error = EFAULT;
			return;
		}
		count = 1;
		switch(mtop.mt_op) {
		case MTREW:
		case MTOFFL:
			func = REWIND;
			break;
		case MTNOP:
			return;

		/* The following functions are non-standard */

		case MTRETEN:			/* retension tape */
			func = RETEN;
			break;
		case MTERASE:			/* erase whole tape */
			tcommand(REWIND);
			tcommand(ARMERASE);
			t_flags |= WROTE;
			func = ERASE;
			break;
		default:
			u.u_error = ENXIO;
			return;
		}
		if (tcommand(func)) {
				u.u_error = EIO;
				return;
		}
		return;

	default:
		u.u_error = ENXIO;
	}
}

#ifdef TCDEBUG
int tc_mp;
char tc_mon[512];
tcmon(n, a1, a2, a3, a4)
char a1, a2, a3, a4;
{
	register i;
	register int *ap = (int *)&a1;

	for (i=0; i<n; i++) {
		tc_mon[tc_mp++] = (char)*ap++;
		if (tc_mp >= 512)
			tc_mp = 0;
	}
}
#endif
