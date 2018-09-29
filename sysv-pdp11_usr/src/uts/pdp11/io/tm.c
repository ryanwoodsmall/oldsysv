/* @(#)tm.c	1.1 */
/*
 * TM tape driver
 * minor device classes:
 * bits 0,1: slave select
 * bit 2 off: rewind on close; on: position after first TM
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/buf.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/systm.h"
#include "sys/elog.h"
#include "sys/iobuf.h"

struct device
{
	int tmer, tmcs, tmbc, tmba;
	int tmdb, tmrd;
};

#define NUNIT	4
struct	iotime	tmstat[NUNIT];
struct	iobuf	tmtab =	tabinit(TM0,0);
struct	buf	rtmbuf;

#define INF	1000000L

char	t_openf[NUNIT];
daddr_t	t_blkno[NUNIT], t_nxrec[NUNIT];

struct device *tm_addr;

#define	RCOM	03
#define	WCOM	05
#define	WEOF	07
#define	SFORW	011
#define	SREV	013
#define	WIRG	015
#define	REW	017
#define FUNCTION	016
#define REWIND	016
#define	DENS	060000		/* 9-channel */
#define	IENABLE	0100
#define GSD	010000
#define	HARD	0102200	/* ILC, EOT, NXM */
#define	EOF	0040000
#define	RLE	01000

#define	SIO	1
#define	SSFOR	2
#define	SSREV	3
#define SCOM	4

tmopen(dev, flag)
{
	register unit;

	unit = dev&03;
	if (t_openf[unit]) {
		u.u_error = ENXIO;
		return;
	}
	if ((tmtab.b_flags&B_ONCE)==0) {
		tmtab.b_flags |= B_ONCE;
		tmtab.io_s1 = ubmalloc(0-1);
	}
	tmtab.io_addr = (physadr)tm_addr;
	tmtab.io_nreg = NDEVREG;
	t_openf[unit]++;
	t_blkno[unit] = 0;
	t_nxrec[unit] = INF;
	if (flag&FAPPEND) {
		tcommand(unit, SFORW);
		tcommand(unit, SREV);
	}
}

tmclose(dev, flag)
{
	register unit;

	unit = dev&03;
	if (flag&FWRITE) {
		tcommand(unit, WEOF);
		tcommand(unit, WEOF);
	}
	if (dev&04) {
		if (flag&FWRITE)
			tcommand(unit, SREV); else
			if (t_blkno[unit] <= t_nxrec[unit])
				tcommand(unit, SFORW);
	} else
		tcommand(unit, REW);
	t_openf[unit] = 0;
}

tcommand(unit, com)
{
	register struct device *rp;

	rp = tm_addr;
	spl5();
	while(tmtab.b_active)
		sleep(&tmtab, PRIBIO);
	rp->tmcs = unit<<8;
	tmtab.b_active = SCOM;
	rp->tmbc = 0;
	rp->tmcs = IENABLE|DENS|com | (unit<<8);
	spl0();
}

tmstrategy(bp)
register struct buf *bp;
{
	register unit;
	register daddr_t *p;

	unit = bp->b_dev & 03;
	p = &t_nxrec[unit];
	if (bp->b_blkno > *p) {
		bp->b_flags |= B_ERROR;
		bp->b_error = ENXIO;
		iodone(bp);
		return;
	}
	if (bp->b_blkno == *p && bp->b_flags&B_READ) {
		clear(paddr(bp), BSIZE);
		bp->b_resid = BSIZE;
		iodone(bp);
		return;
	}
	if ((bp->b_flags&B_READ) == NULL)
		*p = bp->b_blkno + 1;
	bp->b_start = lbolt;
	tmstat[unit].io_cnt++;
	tmstat[unit].io_bcnt += btoc(bp->b_bcount);
	bp->av_forw = NULL;
	spl5();
	if (tmtab.b_actf == NULL)
		tmtab.b_actf = bp;
	else
		tmtab.b_actl->av_forw = bp;
	tmtab.b_actl = bp;
	if (tmtab.b_active == NULL)
		tmstart();
	spl0();
}

tmstart()
{
	register struct buf *bp;
	register com;
	register struct device *rp;
	int unit;
	daddr_t blkno;
	paddr_t addr;

	rp = tm_addr;
    loop:
	if ((bp = tmtab.b_actf) == 0) {
		tmtab.b_active = 0;
		wakeup(&tmtab);
		return;
	}
	unit = bp->b_dev&03;
	com = (unit<<8);
	rp->tmcs = com;	/* give unit select time */
	blkno = t_blkno[unit];
	if (t_openf[unit] < 0 || bp->b_blkno > t_nxrec[unit]) {
		bp->b_flags |= B_ERROR;
		tmtab.b_actf = bp->av_forw;
		iodone(bp);
		goto loop;
	}
	if (tmtab.io_start == 0)
		tmtab.io_start = lbolt;
	addr = ubmaddr(bp, tmtab.io_s1);
	com |= ((hiword(addr) & 03) << 4) | IENABLE|DENS;
	if (blkno != bp->b_blkno) {
		tmstat[unit].ios.io_misc++;
		if (bp->b_blkno > blkno) {
			com |= SFORW;
			tmtab.b_active = SSFOR;
			rp->tmbc = blkno - bp->b_blkno;
		} else {
			com |= SREV;
			tmtab.b_active = SSREV;
			rp->tmbc = bp->b_blkno - blkno;
		}
		rp->tmcs = com;
		return;
	}
	tmtab.b_active = SIO;
	rp->tmbc = -bp->b_bcount;
	rp->tmba = loword(addr);
	blkacty |= (1<<TM0);
	rp->tmcs = com | ((bp->b_flags&B_READ)? RCOM:
	    ((tmtab.b_errcnt)? WIRG: WCOM));
}

tmintr()
{
	register struct buf *bp;
	register unit;
	register struct device *rp;
	int	state;

	rp = tm_addr;
	if ((state = tmtab.b_active) == NULL) {
		if((rp->tmcs&FUNCTION) != REWIND)
			logstray(rp);
		return;
	}
	blkacty &= ~(1<<TM0);
	if (state != SCOM) {
	bp = tmtab.b_actf;
	unit = bp->b_dev&03;
	if (rp->tmcs < 0) {		/* error bit */
		while(rp->tmrd & GSD) ; /* wait for gap shutdown */
		if ((rp->tmer&(HARD|EOF)) == 0 && state==SIO) {
			if (bp != &rtmbuf || (rp->tmer&RLE) == 0) {
				tmtab.io_stp = &tmstat[unit];
				fmtberr(&tmtab,0);
				if (++tmtab.b_errcnt < 10) {
					t_blkno[unit]++;
					tmstart();
					return;
				} else
					bp->b_flags |= B_ERROR;
			}
		} else
		if (rp->tmer&EOF) {
			if (state==SIO) {
				rp->tmbc = -bp->b_bcount;
				t_nxrec[unit] = bp->b_blkno;
			} else {
				t_nxrec[unit] = bp->b_blkno+(state==SSREV?
					-rp->tmbc:rp->tmbc-1);
				t_blkno[unit] = bp->b_blkno+(state==SSREV?
					-rp->tmbc:rp->tmbc);
				tmstart();
				return;
			}
		} else {
			tmtab.io_stp = &tmstat[unit].ios;
			fmtberr(&tmtab,0);
			bp->b_flags |= B_ERROR;
			if (bp != &rtmbuf)
				t_openf[unit] = -1;
		}
	}
	if(tmtab.io_erec)
		logberr(&tmtab,bp->b_flags&B_ERROR);
	if (state == SIO) {
		tmtab.b_errcnt = 0;
		t_blkno[unit]++;
		tmtab.b_actf = bp->av_forw;
		bp->b_resid = -rp->tmbc;
		tmstat[unit].io_resp += lbolt - bp->b_start;
		tmstat[unit].io_act += lbolt - tmtab.io_start;
		tmtab.io_start = 0;
		iodone(bp);
	} else
		t_blkno[unit] = bp->b_blkno;
	}
	tmstart();
}

tmread(dev)
{
	tmphys(dev);
	physio(tmstrategy, &rtmbuf, dev, B_READ);
}

tmwrite(dev)
{
	tmphys(dev);
	physio(tmstrategy, &rtmbuf, dev, B_WRITE);
}

tmphys(dev)
{
	register unit;
	daddr_t a;

	unit = dev&03;
	a = u.u_offset >> BSHIFT;
	t_blkno[unit] = a;
	t_nxrec[unit] = ++a;
}

tmprint(dev, str)
char *str;
{
	printf("%s on TM11 drive %d\n", str, dev&03);
}
