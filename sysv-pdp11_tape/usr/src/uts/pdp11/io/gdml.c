/* @(#)gdml.c	1.1 */
#include <sys/param.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/buf.h>
#include <sys/elog.h>
#include <sys/iobuf.h>
#include <sys/errno.h>
#include <sys/systm.h>
#include <sys/gdisk.h>

#define	MLAT	1<<10	/* mr - array type ? 64K : 16K */

struct d_sizes ml_sizes[8];

gmlopen(dev)
{
	register struct device *rp;

	rp = (struct device *)gdutab[gdpos(dev)].io_addr;
	rp->cs2 = gddrive(dev);
	ml_sizes[0].nblocks = (daddr_t)((rp->mr>>11)&037) *
		((rp->mr & MLAT) ? 2048 : 512) - 1;
}

gmlstrategy(bp)
register struct buf *bp;
{
	register struct iobuf *dp;
	register pos, ctl;

	pos = gdpos(bp->b_dev);
	dp = &gdutab[pos];
	bp->cylin = 0;
	*((int *)(&bp->trksec)) = bp->b_blkno;
	ctl = gdctl(bp->b_dev);
	spl5();
	dp->qcnt++;
	if (dp->b_actf == NULL)
		dp->b_actf = bp;
	else
		dp->b_actl->av_forw = bp;
	dp->b_actl = bp;
	if (gdutab[pos].b_active == 0) {
		gmlustart(pos);
		if (gdtab[ctl].b_active == 0 && gdtab[ctl].b_actf != NULL)
			gdstart(ctl);
	}
	spl0();
}

gmlustart(pos)
{
	register struct buf *bp;
	register struct iobuf *dp;
	register struct device *rp;
	int	ctl, drive;

	dp = &gdutab[pos];
	drive = gdptod(pos);
	ctl = gdptoc(pos);
	rp = (struct device *)dp->io_addr;
	lobyte(rp->cs2) = drive;
	lobyte(rp->cs1) = IE;
	bp = dp->b_actf;
	if (rp->ds&ERR) {
		fmtberr(dp, 0);
		lobyte(rp->cs1) = IE|DCLR|GO;
		if (rp->ds&ERR || ++dp->b_errcnt > 16) {
			gdprint(bp ? bp->b_dev : dp->b_dev, "Hard error");
			printf("ds %o, er %o %o %o\n",
				rp->ds, rp->er1, rp->er2, rp->er3);
			rp->as = 1<<drive;
			if (dp->wtime!=WOK)
				return;
			if (bp == NULL)
				dp->wtime = WOK;
			else if (!(rp->ds&MOL))
				dp->wtime = WMOL;
			else
				dp->wtime = WERR;
			return;
		}
	} else if (!(rp->ds&MOL)) {
		if (dp->wtime!=WOK)
			return;
		if (bp == NULL)
			dp->wtime = WOK;
		else if (rp->cs2&NED)
			dp->wtime = WNED;
		else if (!pwr_act)
			dp->wtime = WMOL;
		else
			dp->wtime = WERR;
		return;
	}
	dp->wtime = WOK;
	if (!(rp->ds & VV)) {
		if ((rp->dt & DTYPE) != gdtype[pos]) {
			if (gdtype[pos])
				gdprint(bp ? bp->b_dev : dp->b_dev,
					"Drive type change");
			gdtype[pos] = 0;
		} else
			lobyte(rp->cs1) = IE|PRESET|GO;
	}
	if (bp == NULL)
		return;
	if (dp->b_active == 0)
		dp->io_start = lbolt;
	dp->b_active++;
	dp->b_forw = NULL;
	if (gdtab[ctl].b_actf == NULL)
		gdtab[ctl].b_actf = (struct buf *)dp;
	else
		gdtab[ctl].b_actl->b_forw = (struct buf *)dp;
	gdtab[ctl].b_actl = (struct buf *)dp;
}

gmlintr(pos, bp, dp, rp)
register struct buf *bp;
register struct iobuf *dp;
register struct device *rp;
{
	extern gdprint();

	if (++dp->b_errcnt>16)
		bp->b_flags |= B_ERROR;
	else
		gdtab[gdptoc(pos)].b_active = 0;
	fmtberr(dp, 0);
	if (dp->b_errcnt > 2)
		prcom(gdprint, bp, rp->er1, rp->ds);
	rp->cs1 = TRE|IE|DCLR|GO;
	return(0);
}
