/* @(#)ts.c	6.1 */
/*
 * TS11 tape driver
 * Handles one TS11. Minor device classes:
 * bit 2 - off: rewind on close; on: position after first TM.
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"

#include "sys/buf.h"
#include "sys/dir.h"
#include "sys/elog.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/iobuf.h"
#include "sys/signal.h"
#include "sys/systm.h"
#include "sys/user.h"
#include "sys/uba.h"


struct	device {
	ushort	tsdb;
	ushort	tssr;
};

extern struct	device	*ts_addr;

int tsdebug = 0;
#define NTSREG	8
struct	iotime	tsstat;
struct	iobuf	tstab = tabinit(TM0, &tsstat.ios);
struct	buf	ctsbuf;
struct	buf	rtsbuf;
ushort	tsuba;
long	ts_iouba;
char	ts_flags;

#define	INF	100000

char	ts_openf;
int	ts_eot;
daddr_t	ts_blkno;
daddr_t	ts_nxrec;	/* next record */

/* status message */
struct	sts {
	ushort	s_sts;
	ushort	len;
	ushort	rbpcr;	/* residual frame count register */
	ushort	xs0;	/* extended status registers (0-3) */
	ushort	xs1;
	ushort	xs2;
	ushort	xs3;
	ushort tssr;	/* keep a copy of tssr for error logging */
};

/* Error codes in stat 0 */
#define	TMK	0100000
#define	RLS	040000
#define	RLL	010000
#define	WLE	04000
#define	ONL	0100
#define EOT	01

/* command message */
struct cmd {
	ushort	cmd;
	ushort	loba;
	ushort	hiba;
	ushort	size;
};

/* command packet header word */
#define	ACK	0100000
#define	CVC	040000
#define	IE	0200

/* commands, also in command packet header word */
#define	SETCHR	04
#define	GSTAT	017
	/* data transfer */
#define	READ	01
#define	REREAD	01001
#define	WRITE	05
#define	REWRITE	01005
#define	WTM	011
	/* positioning */
#define	SFORW	010
#define SKIPF	01010
#define	SREV	0410
#define	REW	02010
#define NOREW	04
#define NOP	0
#define TCC	0xe	/* Termination Class Code mask */

/* characteristics data */
struct charac {
	ushort	loba;
	ushort	hiba;
	ushort	size;
	ushort	mode;
};

/* All the packets, collected */
struct tsmesg {
	struct	cmd ts_cmd;
	struct	sts ts_sts;
	struct	charac ts_char;
} ts;

/* Bits in status register tssr */
#define	SC	0100000
#define	SSR	0200
#define	OFL	0100
#define	NBA	02000

/* states */
#define	SIO	1
#define	SSFOR	2
#define	SSREV	3
#define SRETRY	4
#define SCOM	5
#define SOK	6

#define H_WRITTEN 1

tsopen(dev, flag)
{
	static struct tsmesg *ubaddr = NULL;
	register struct device *rp;

	if (tsdebug)
		printf("In tsopen: dev = %x\n", dev);
	if (ts_openf) {
		u.u_error = ENXIO;
		return;
	}
	rp = ts_addr;
	tstab.io_addr = (physadr) &ts.ts_sts;
	tstab.io_nreg = NTSREG;
	if ( (tstab.b_flags&B_ONCE)==0) {
		tstab.io_s1 = ubmalloc( ctob(64), 1);
		tstab.b_flags |= B_ONCE;
	}
	if (ubaddr==0 || rp->tssr&(OFL|NBA) || (rp->tssr&SSR)==0) {
		long i = 0;
		rp->tssr = 0;
		while ((rp->tssr & SSR)==0) {
			if (++i > 1000000) {
				u.u_error = ENXIO;
				return;
			}
		}
	}
	if (rp->tssr&OFL) {
		u.u_error = ENXIO;
		return;
	}
	if (rp->tssr&NBA) {
		ctsbuf.b_un.b_addr = (caddr_t) &ts;
		ctsbuf.b_bcount = sizeof(ts);
		if (ubaddr == 0)
			ubaddr = (struct tsmesg *)ubmdata((caddr_t) &ts);
		tsuba = (ushort) ( (long) &ubaddr->ts_cmd + 
				(( (long) &ubaddr->ts_cmd >> 16) &03));
		ts.ts_char.loba = (int)&ubaddr->ts_sts;
		ts.ts_char.hiba = (ushort)((long)&ubaddr->ts_sts>>16) & 03;
		ts.ts_char.size = sizeof(ts.ts_sts);
		ts.ts_char.mode = 0x80;	/* Stop on 2 tape marks */
		ts.ts_cmd.cmd = ACK + 04;	/* write characteristics */
		ts.ts_cmd.loba = (int)&ubaddr->ts_char;
		ts.ts_cmd.hiba = (ushort)((long)&ubaddr->ts_char >> 16) & 03;
		ts.ts_cmd.size = sizeof(ts.ts_sts);
		rp->tsdb = tsuba;
	}
	ts_blkno = 0;
	ts_nxrec = INF;
	ts_flags = 0;
	if (u.u_error==0)
		ts_openf++;
}

tsclose(dev, flag)
{
	if (tsdebug)
		printf("In tsclose: dev = %x, flag = %x\n", dev, flag);
	if (flag == FWRITE || ((flag&FWRITE) && (ts_flags&H_WRITTEN))) {
		tscommand(WTM);
		tscommand(WTM);
	}
	if (dev & NOREW) {
		if (flag == FWRITE || ((flag&FWRITE) && (ts_flags&H_WRITTEN)))
			tscommand(SREV);
		else {
			if (ts_blkno <= ts_nxrec)
				tscommand(SKIPF);
		}
	} else
		tscommand(REW);
	ts_openf = 0;
}

tscommand(com)
{
	register struct buf *bp;

	if (tsdebug)
		printf("In tscommand: com = %x\n", com);
	bp = &ctsbuf;
	spl5();
	while(bp->b_flags&B_BUSY) {
		bp->b_flags |= B_WANTED;
		sleep((caddr_t)bp, PRIBIO);
	}
	spl0();
	bp->b_resid = com;
	bp->b_blkno = 0;
	bp->b_flags = B_BUSY|B_READ;
	tsstrategy(bp);
	iowait(bp);
	if(bp->b_flags&B_WANTED)
		wakeup((caddr_t)bp);
	bp->b_flags = 0;
	return(bp->b_resid);
}

tsstrategy(bp)
register struct buf *bp;
{
	register daddr_t *p;

	if (tsdebug)
		printf("In tsstrategy\n");
	if(bp != &ctsbuf) {
		p = &ts_nxrec;
		if(bp->b_blkno > *p) {
			bp->b_flags |= B_ERROR;
			bp->b_error = ENXIO;
			iodone(bp);
			return;
		}
		if(bp->b_blkno == *p && bp->b_flags&B_READ) {
			clrbuf(bp);
			bp->b_resid = bp->b_bcount;
			iodone(bp);
			return;
		}
		if ((bp->b_flags&B_READ)==0) {
			*p = bp->b_blkno + 1;
			ts_flags |= H_WRITTEN;
		}
		bp->b_start = lbolt;
		tsstat.io_bcnt += btoc(bp->b_bcount);
	}
	bp->av_forw = NULL;
	spl5();
	if (tstab.b_actf == NULL)
		tstab.b_actf = bp;
	else
		tstab.b_actl->av_forw = bp;
	tstab.b_actl = bp;
	if (tstab.b_active==0)
		tsstart();
	spl0();
}

tsstart()
{
	register struct buf *bp;
	register struct cmd *pp;
	register daddr_t blkno;

	if (tsdebug)
		printf("In tsstart\n");
	pp = &ts.ts_cmd;
    loop:
	if ((bp = tstab.b_actf) == NULL)
		return;
	blkno = ts_blkno;
	if (ts_openf < 0 || bp->b_blkno > ts_nxrec)
		goto abort;
	if (bp == &ctsbuf) {
		if (tsdebug)
			printf("command = %x\n", bp->b_resid);
		tstab.b_active = SCOM;
		tsstat.ios.io_misc++;
		pp->loba = 1;		/* count always 1 */
		pp->cmd = ACK|CVC|IE|bp->b_resid;
	} else if (blkno == bp->b_blkno) {
		if (ts_eot > 8)
			goto abort;
		if ((bp->b_flags & B_READ) == 0 && ts_eot && blkno > ts_eot) {
			bp->b_error = ENOSPC;
			goto abort;
		}
		tstab.b_active = SIO;
		ts_iouba = ubmaddr( bp, tstab.io_s1);
		pp->loba = (ushort)ts_iouba;
		pp->hiba = (ushort)(ts_iouba >> 16) & 03;
		pp->size = bp->b_bcount;
		tsstat.io_cnt++;
		if (tstab.io_start == 0)
			tstab.io_start = lbolt;
		blkacty |= (1<<TM0);
		if(bp->b_flags & B_READ)
			pp->cmd = ACK|CVC|IE|READ;
		else
			pp->cmd = ACK|CVC|IE|WRITE;
	} else {
		tsstat.ios.io_misc++;
		if (blkno < bp->b_blkno) {
			tstab.b_active = SSFOR;
			pp->loba = bp->b_blkno - blkno;
			pp->cmd = ACK|CVC|IE|SFORW;
		} else {
			tstab.b_active = SSREV;
			pp->loba = blkno - bp->b_blkno;
			pp->cmd = ACK|CVC|IE|SREV;
		}
	}
	ts_addr->tsdb = tsuba;
	return;

    abort:
	bp->b_flags |= B_ERROR;

    next:
	tstab.b_active = 0;
	tstab.b_actf = bp->av_forw;
	iodone(bp);
	goto loop;
}

tsintr()
{
	register struct buf *bp;
	register struct device *rp;
	register err, errclass, state;
	extern tsprint();

	if ((bp = tstab.b_actf)==NULL)
		return;
	rp = ts_addr;
	if (tsdebug) {
		printf("In tsintr: tssr = %x\n", rp->tssr);
		printf("ts_blkno = %d, ts_nxrec = %d\n", ts_blkno, ts_nxrec);
	}
	state = tstab.b_active;
	blkacty &= ~(1<<TM0);
	tstab.b_active = 0;
	err = (rp->tssr & SC) ? (rp->tssr & TCC) : 0;
	errclass = 0;
	switch (err) {
	case 014:		/* unrecoverable */
	case 016:		/* fatal */
	case 002:		/* attention (shouldn't happen) */
	case 012:		/* "recoverable", but shouldn't happen */
		errclass = 2;
		break;

	case 0:			/* all OK */
		break;

	case 004:		/* status alert */
		if (tsdebug)
			printf("   xs0 = %x\n", ts.ts_sts.xs0);
		if (ts.ts_sts.xs0 & TMK) {		/* tape mark */
			ts.ts_sts.rbpcr = bp->b_bcount;
			ts_nxrec = bp->b_blkno;
			break;
		}
		if ((ts.ts_sts.xs0&RLL) && bp==&rtsbuf)	{
			ts.ts_sts.rbpcr = 0;  /* setup for correct resid set below */
			break;
		}
		if ((ts.ts_sts.xs0&RLS) && bp==&rtsbuf)	/* short record */
			break;
		errclass = 1;
		break;

	case 010:		/* recoverable, tape moved */
		if (tsdebug)
			printf("retry %d: %x %x %x %x %x %x %x %x\n", tstab.b_errcnt, rp->tssr,
				  ts.ts_sts.s_sts, ts.ts_sts.len, ts.ts_sts.rbpcr,
				  ts.ts_sts.xs0, ts.ts_sts.xs1, ts.ts_sts.xs2, ts.ts_sts.xs3);
		if (state==SIO && ++tstab.b_errcnt < 10) {
			ts.ts_cmd.cmd |= 01000;	/* redo bit */
			tstab.b_active = SIO;
			rp->tsdb = tsuba;
			tsstat.ios.io_misc++;
			ts.ts_sts.tssr = ts_addr->tssr;
			fmtberr(&tstab, 0);
			return;
		}
		errclass = 1;
		break;

	case 006:		/* Function reject */
		if (state==SIO && ts.ts_sts.xs0 & WLE)
			printf("Tape needs a ring\n");
		if ((ts.ts_sts.xs0&ONL) == 0)		/* tape offline */
			printf("Tape offline\n");
		errclass = 2;
	}
	if (errclass) {
		ts.ts_sts.tssr = ts_addr->tssr;
		fmtberr(&tstab, 0);
		if (tstab.b_errcnt > 4)
			prcom(tsprint, bp, rp->tssr, ts.ts_sts.xs0);
		if (tsdebug)
			printf("tp: %x %x %x %x %x %x %x %x\n", rp->tssr,
			  ts.ts_sts.s_sts, ts.ts_sts.len, ts.ts_sts.rbpcr,
			  ts.ts_sts.xs0, ts.ts_sts.xs1, ts.ts_sts.xs2, ts.ts_sts.xs3);
	}
	switch(state) {
	case SIO:
		ts_blkno++;
	case SCOM:
		if (ts.ts_sts.xs0 & EOT)
			ts_eot++;
		else
			ts_eot = 0;
		if (tstab.io_erec)
			logberr(&tstab, bp->b_flags & B_ERROR);
		tstab.b_errcnt = 0;
		tstab.b_actf = bp->av_forw;
		bp->b_resid = ts.ts_sts.rbpcr;
		iodone(bp);
		if (bp != &ctsbuf) {
			tsstat.io_resp += lbolt - bp->b_start;
			tsstat.io_act += lbolt - tstab.io_start;
			tstab.io_start = 0;
		}
		break;

	case SSFOR:
	case SSREV:
		if (ts.ts_sts.xs0 & TMK) {
			if (state == SSREV) {
				ts_blkno = bp->b_blkno -
					ts.ts_sts.rbpcr;
				ts_nxrec = ts_blkno;
			} else {
				ts_blkno = bp->b_blkno +
					ts.ts_sts.rbpcr;
				ts_nxrec = ts_blkno -1;
			}
		} else
			ts_blkno = bp->b_blkno;
		break;

	default:
		prcom(tsprint, bp, rp->tssr, ts.ts_sts.xs0);
		errclass = 2;
		break;
	}
	if (errclass > 1) {
		while (bp = tstab.b_actf) {
			bp->b_flags |= B_ERROR;
			iodone(bp);
			tstab.b_actf = bp->av_forw;
		}
	}
	tsstart();
}

tsread(dev)
{
	if (tsdebug)
		printf("In tsread\n");
	tsphys(dev);
	physio(tsstrategy, &rtsbuf, dev, B_READ);
}

tswrite(dev)
{
	if (tsdebug)
		printf("In tswrite\n");
	tsphys(dev);
	physio(tsstrategy, &rtsbuf, dev, B_WRITE);
}

tsphys(dev)
{
	register unit;
	register daddr_t a;

	if (tsdebug)
		printf("In tsphys: dev = %x\n", dev);
	a = u.u_offset >> 9;
	ts_blkno = a;
	ts_nxrec = a+1;
}

tsprint( dev, str)
char *str;
{
      printf( "%s on TS11 drive %d\n", str, dev&077);
}
