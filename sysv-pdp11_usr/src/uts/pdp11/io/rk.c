/* @(#)rk.c	1.1 */
/*
 * RK disk driver
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/buf.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/systm.h"
#include "sys/elog.h"
#include "sys/iobuf.h"

#define	NRKBLK	4872

#define	RESET	0
#define	GO	01
#define	WCOM	02
#define	RCOM	04
#define	DRESET	014
#define	IENABLE	0100
#define	DRY	0200
#define	WLO	020000
#define	CTLRDY	0200

struct device {
	int	rkds, rker, rkcs, rkwc;
	int	rkba, rkda;
};

struct	iotime	rkstat[8];
struct	iobuf	rktab =	tabinit(RK0,0);
struct	buf	rrkbuf;

struct device *rk_addr;
int	rk_cnt;

rkopen(dev)
{
	if (dev >= rk_cnt)
		u.u_error = ENXIO;
	rktab.io_addr = (physadr)rk_addr;
	rktab.io_nreg = NDEVREG;
	if ((rktab.b_flags&B_ONCE)==0) {
		rktab.b_flags |= B_ONCE;
		rktab.io_s1 = ubmalloc(0-1);
	}
}

rkclose(dev)
{
}

rkstrategy(bp)
register struct buf *bp;
{
	register unit;

	if (bp->b_blkno >= NRKBLK) {
		if (bp->b_blkno == NRKBLK && bp->b_flags&B_READ)
			bp->b_resid = bp->b_bcount;
		else {
			bp->b_flags |= B_ERROR;
			bp->b_error = ENXIO;
		}
		iodone(bp);
		return;
	}
	bp->av_forw = NULL;
	bp->b_start = lbolt;
	unit = bp->b_dev & 07;
	rkstat[unit].io_cnt++;
	rkstat[unit].io_bcnt += btoc(bp->b_bcount);
	spl5();
	if (rktab.b_actf == NULL)
		rktab.b_actf = bp;
	else
		rktab.b_actl->av_forw = bp;
	rktab.b_actl = bp;
	if (rktab.b_active==0)
		rkstart();
	spl0();
}

rkaddr(bp)
register struct buf *bp;
{
	register int b, d;

	b = bp->b_blkno;
	d = minor(bp->b_dev);
	return(d<<13 | (b/12)<<4 | b%12);
}

rkstart()
{
	register struct buf *bp;
	register struct device *rp;
	paddr_t addr;

	if ((bp = rktab.b_actf) == NULL)
		return;
	rktab.b_active++;
	rp = rk_addr;
	rp->rkwc = -(bp->b_bcount>>1);
	addr = ubmaddr(bp, rktab.io_s1);
	rp->rkba = loword(addr);
	rp->rkda = rkaddr(bp);
	blkacty |= (1<<RK0);
	rktab.io_start = lbolt;
	rp->rkcs = (bp->b_flags&B_READ?RCOM:WCOM) | IENABLE |
		GO | ((hiword(addr)&03)<<4);
}

rkintr()
{
	register struct buf *bp;
	register struct device *rp;
	register unit;
	register status;
	extern rkprint();

	rp = rk_addr;
	if (rktab.b_active == 0) {
		logstray(rp);
		return;
	}
	blkacty &= ~(1<<RK0);
	bp = rktab.b_actf;
	rktab.b_active = 0;
	unit = bp->b_dev & 07;
	if (rp->rkcs < 0) {		/* error bit */
		status = rp->rker;
		rktab.io_stp = &rkstat[unit].ios;
		fmtberr(&rktab,0);
		if (rktab.b_errcnt > 2)
			prcom(rkprint, bp, rp->rker, rp->rkds);
		rp->rkcs = RESET|GO;
		while((rp->rkcs&CTLRDY) == 0) ;
		if (++rktab.b_errcnt <= 10 && (status&WLO) == 0) {
			rkstart();
			return;
		}
		bp->b_flags |= B_ERROR;
	}
	if (rktab.io_erec)
		logberr(&rktab,bp->b_flags&B_ERROR);
	rktab.b_errcnt = 0;
	rktab.b_actf = bp->av_forw;
	bp->b_resid = (-rp->rkwc)<<1;
	rkstat[unit].io_resp += lbolt - bp->b_start;
	rkstat[unit].io_act += lbolt - rktab.io_start;
	iodone(bp);
	rkstart();
}

rkread(dev)
{
	if (physck((daddr_t)NRKBLK, B_READ))
		physio(rkstrategy, &rrkbuf, dev, B_READ);
}

rkwrite(dev)
{
	if (physck((daddr_t)NRKBLK, B_WRITE))
		physio(rkstrategy, &rrkbuf, dev, B_WRITE);
}

rkclr()
{
	rktab.b_active = 0;
	rkstart();
}

rkprint(dev, str)
char *str;
{
	printf("%s on RK05 drive %d\n", str, dev);
}
