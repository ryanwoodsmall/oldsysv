/* @(#)rp.c	1.1 */
/*
 * RP11/RP03 disk driver
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

struct device
{
	int	rpds, rper, rpcs, rpwc;
	int	rpba, rpca, rpda;
};

struct device *rp_addr;

struct {
	daddr_t	nblocks;
	int	cyloff;
} rp_sizes[8];

struct	iotime	rpstat[8];
struct	iobuf	rptab =	tabinit(RP0,0);
struct	buf	rrpbuf;


#define	NSECT	10
#define	NTRAC	20

#define	GO	01
#define	RESET	0
#define	WCOM	02
#define	RCOM	04
#define	HSEEK	014

#define	IENABLE	0100
#define	READY	0200

#define	SUFU	01000
#define	SUSU	02000
#define	SUSI	04000
#define	HNF	010000

#define SUWP	0400

/*
 * Use av_back to save track+sector,
 * b_resid for cylinder.
 */

#define	trksec	av_back
#define	cylin	b_resid


rpopen(dev)
{
	rptab.io_addr = (physadr)rp_addr;
	rptab.io_nreg = NDEVREG;
	if ((rptab.b_flags&B_ONCE)==0) {
		rptab.b_flags |= B_ONCE;
		rptab.io_s1 = ubmalloc(0-1);
	}
}

rpclose(dev)
{
}

rpstrategy(bp)
register struct buf *bp;
{
	register struct buf *dp;
	register struct buf *ap;
	daddr_t last;
	int	unit;
	int	co;

	unit = minor(bp->b_dev)&07;
	last = rp_sizes[unit].nblocks;
	co = rp_sizes[unit].cyloff;
	if (bp->b_blkno >= last) {
		if (bp->b_blkno == last && bp->b_flags&B_READ)
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
	unit = (bp->b_dev&070)>>3;
	rpstat[unit].io_cnt++;
	rpstat[unit].io_bcnt += btoc(bp->b_bcount);
	bp->cylin = bp->b_blkno/(NSECT*NTRAC) + co;
	co = bp->b_blkno%(NSECT*NTRAC);
	*((int *)(&bp->trksec)) = ((co/NSECT)<<8) + (co%NSECT);
	spl5();
	if ((ap = rptab.b_actf) == NULL) {
		rptab.b_actf = bp;
		rptab.b_actl = bp;
	} else {
		for (ap = rptab.b_actl; dp = ap->av_forw; ap = dp) {
			if (ap->cylin <= bp->cylin
			 && bp->cylin <  dp->cylin
			 || ap->cylin >= bp->cylin
			 && bp->cylin >  dp->cylin) 
				break;
		}
		ap->av_forw = bp;
		bp->av_forw = dp;
	}
	if (rptab.b_active == 0)
		rpstart();
	spl0();
}

rpstart()
{
	register struct buf *bp;
	register struct device *rp;
	register com;
	paddr_t addr;

	if ((bp = rptab.b_actf) == NULL)
		return;
	rptab.b_active++;
	rp = rp_addr;
	addr = ubmaddr(bp, rptab.io_s1);
	rp->rpda = (int)bp->trksec;
	rp->rpca = bp->cylin;
	rp->rpba = loword(addr);
	rp->rpwc = -(bp->b_bcount>>1);
	com = ((bp->b_dev&070)<<5)|((hiword(addr)&03)<<4)|IENABLE|GO;
	if (bp->b_flags & B_READ)
		com |= RCOM;
	else
		com |= WCOM;
	blkacty |= (1<<RP0);
	rptab.io_start = lbolt;
	if (pwr_act >= 0)
		rp->rpcs = com;
}

rpintr()
{
	register struct buf *bp;
	register int ctr;
	register struct device *rp;
	int unit, status;
	extern rpprint();

	rp = rp_addr;
	if (rptab.b_active == 0) {
		logstray(rp);
		return;
	}
	blkacty &= ~(1<<RP0);
	bp = rptab.b_actf;
	rptab.b_active = 0;
	unit = minor(bp->b_dev);
	if (rp->rpcs < 0) {		/* error bit */
		rptab.io_stp = &rpstat[unit>>3].ios;
		fmtberr(&rptab,rp_sizes[unit&07].cyloff);
		if (rptab.b_errcnt > 2)
			prcom(rpprint, bp, rp->rper, rp->rpds);
		status = rp->rpds;
		if (rp->rpds & (SUFU|SUSI|HNF)) {
			rp->rpcs = ((bp->b_dev&070)<<5)|HSEEK|GO;
			ctr = 0;
			while ((rp->rpds&SUSU) && --ctr);
		}
		rp->rpcs = RESET|GO;
		ctr = 0;
		while ((rp->rpcs&READY) == 0 && --ctr);
		if (++rptab.b_errcnt <= 10 && (status&SUWP) == 0) {
			rpstart();
			return;
		}
		bp->b_flags |= B_ERROR;
	}
	if (rptab.io_erec)
		logberr(&rptab,bp->b_flags&B_ERROR);
	rptab.b_errcnt = 0;
	rptab.b_actf = bp->av_forw;
	bp->b_resid = (-rp->rpwc)<<1;
	unit >>= 3;
	rpstat[unit].io_resp += lbolt - bp->b_start;
	rpstat[unit].io_act += lbolt - rptab.io_start;
	iodone(bp);
	if (bp=rptab.b_actf) while(bp->av_forw) bp = bp->av_forw;
	rptab.b_actl = bp;
	rpstart();
}

rpread(dev)
{
	if (physck(rp_sizes[dev&07].nblocks, B_READ))
		physio(rpstrategy, &rrpbuf, dev, B_READ);
}

rpwrite(dev)
{
	if (physck(rp_sizes[dev&07].nblocks, B_WRITE))
		physio(rpstrategy, &rrpbuf, dev, B_WRITE);
}

rpclr()
{
	rptab.b_active = 0;
	rpstart();
}

rpprint(dev, str)
char *str;
{
	printf("%s on RP03 drive %d, slice %d\n", str, (dev>>3)&7, dev&7);
}
