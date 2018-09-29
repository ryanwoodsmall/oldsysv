/* @(#)rf.c	1.1 */
/*
 * RF disk driver
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

struct device {
	int	rfcs, rfwc, rfba, rfda;
	int	rfdae;
};

struct	iotime	rfstat[8];
struct	iobuf	rftab =	tabinit(RF0,0);
struct	buf	rrfbuf;


#define	NRFBLK	1024
#define	RFSHIFT	10

struct device *rf_addr;
int	rf_cnt;

#define	CTLCLR	0400
#define	IENABLE	0100
#define	WCOM	03
#define	RCOM	05
#define	WLO	02000

rfopen(dev)
{
	if (dev >= rf_cnt)
		u.u_error = ENXIO;
	rftab.io_addr = (physadr)rf_addr;
	rftab.io_nreg = NDEVREG;
	if ((rftab.b_flags&B_ONCE)==0) {
		rftab.b_flags |= B_ONCE;
		rftab.io_s1 = ubmalloc(0-1);
	}
}

rfclose(dev)
{
}

rfstrategy(bp)
register struct buf *bp;
{
	register unit;
	daddr_t limit;

	limit = NRFBLK*(minor(bp->b_dev)+1);
	if (bp->b_blkno >= limit) {
		if (bp->b_blkno == limit && bp->b_flags&B_READ)
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
	unit = bp->b_blkno>>RFSHIFT;
	rfstat[unit].io_cnt++;
	rfstat[unit].io_bcnt += btoc(bp->b_bcount);
	spl5();
	if (rftab.b_actf == NULL)
		rftab.b_actf = bp;
	else
		rftab.b_actl->av_forw = bp;
	rftab.b_actl = bp;
	if (rftab.b_active==0)
		rfstart();
	spl0();
}

rfstart()
{
	register struct buf *bp;
	register struct device *rp;
	register com;
	paddr_t addr;

	if ((bp = rftab.b_actf) == NULL)
		return;
	rftab.b_active++;
	rp = rf_addr;
	rp->rfwc = -(bp->b_bcount>>1);
	addr = ubmaddr(bp, rftab.io_s1);
	rp->rfba = loword(addr);
	rp->rfda = (int)(bp->b_blkno<<8);
	rp->rfdae = (int)(bp->b_blkno>>8);
	rftab.io_start = lbolt;
	blkacty |= (1<<RF0);
	com = bp->b_flags&B_READ ? RCOM : WCOM;
	rp->rfcs = com | IENABLE | ((hiword(addr)&03)<<4);
}

rfintr()
{
	register struct buf *bp;
	register struct device *rp;
	register unit;
	register status;
	extern rfprint();

	rp = rf_addr;
	if (rftab.b_active == 0) {
		logstray(rp);
		return;
	}
	blkacty &= ~(1<<RF0);
	bp = rftab.b_actf;
	rftab.b_active = 0;
	unit = bp->b_blkno>>RFSHIFT;
	if (rp->rfcs < 0) {		/* error bit */
		status = rp->rfcs;
		rftab.io_stp = &rfstat[unit].ios;
		fmtberr(&rftab,0);
		if (rftab.b_errcnt > 2)
			prcom(rfprint, bp, rp->rfcs, rp->rfdae);
		rp->rfcs = CTLCLR;
		if (++rftab.b_errcnt <= 10 && (status&WLO) == 0) {
			rfstart();
			return;
		}
		bp->b_flags |= B_ERROR;
	}
	if (rftab.io_erec)
		logberr(&rftab,bp->b_flags&B_ERROR);
	rftab.b_errcnt = 0;
	rftab.b_actf = bp->av_forw;
	bp->b_resid = (-rp->rfwc)<<1;
	rfstat[unit].io_resp += lbolt - bp->b_start;
	rfstat[unit].io_act += lbolt - rftab.io_start;
	iodone(bp);
	rfstart();
}

rfread(dev)
{

	if (physck((daddr_t)(NRFBLK*((dev&07)+1)), B_READ))
		physio(rfstrategy, &rrfbuf, dev, B_READ);
}

rfwrite(dev)
{

	if (physck((daddr_t)(NRFBLK*((dev&07)+1)), B_WRITE))
		physio(rfstrategy, &rrfbuf, dev, B_WRITE);
}

rfprint(dev, str)
char *str;
{
	printf("%s on RF11 drive %d\n", str, dev);
}
