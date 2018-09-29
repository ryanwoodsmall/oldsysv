/* @(#)gdhp.c	1.1 */
#include <sys/param.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/buf.h>
#include <sys/elog.h>
#include <sys/iobuf.h>
#include <sys/errno.h>
#include <sys/systm.h>
#include <sys/gdisk.h>

#define	SDIST	3
#define	RDIST	7
#define	GDIST	1

ghpstrategy(bp)
register struct buf *bp;
{
	register struct iobuf *dp;
	register struct buf *ap;
	register pos, ctl, slice;
	int	co;

	slice = gdslice(bp->b_dev);
	pos = gdpos(bp->b_dev);
	co = gdloc(pos).sizep[slice].cyloff;
	dp = &gdutab[pos];
	bp->cylin = bp->b_blkno/(gdloc(pos).sec*gdloc(pos).trk) + co;
	co = bp->b_blkno%(gdloc(pos).sec*gdloc(pos).trk);
	*((int *)(&bp->trksec)) = ((co/gdloc(pos).sec)<<8)+(co%gdloc(pos).sec);
	ctl = gdctl(bp->b_dev);
	spl5();
	dp->qcnt++;
	if (dp->b_actf == NULL) {
		dp->b_actf = bp;
		dp->b_actl = bp;
		dp->acts = (int)bp;
	} else {
		register struct buf *cp;
		if (((int)gdstat[pos].io_cnt&07) == 0)
			dp->acts = (int)dp->b_actl;
		for (ap = (struct buf *)dp->acts; cp = ap->av_forw; ap = cp) {
			int s1, s2;

			if ((s1 = ap->cylin - bp->cylin)<0) s1 = -s1;
			if ((s2 = ap->cylin - cp->cylin)<0) s2 = -s2;
			if (s1 < s2)
				break;
		}
		ap->av_forw = bp;
		if ((bp->av_forw = cp) == NULL)
			gdutab[pos].b_actl = bp;
	}
	if (gdutab[pos].b_active == 0) {
		ghpustart(pos);
		if (gdtab[ctl].b_active == 0 && gdtab[ctl].b_actf != NULL)
			gdstart(ctl);
	}
	spl0();
}

ghpustart(pos)
{
	register struct buf *bp;
	register struct iobuf *dp;
	register struct device *rp;
	int	ctl, drive;
	int	sn, dsn;

	dp = &gdutab[pos];
	drive = gdptod(pos);
	ctl = gdptoc(pos);
	rp = (struct device *)dp->io_addr;
	lobyte(rp->cs2) = drive;
	lobyte(rp->cs1) = IE;	/* seize if dual port, set IE */
	bp = dp->b_actf;
	if (rp->ds&ERR) {
		fmtberr(dp,(bp==NULL)?0:gdloc(pos).sizep[gdslice(bp->b_dev)].cyloff);
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
	if (!(rp->of & FMT22))
		rp->of = FMT22;
	if (bp == NULL) {
		lobyte(rp->cs1) = IE|RELEASE|GO;
		return;
	}
	if ((dp->b_errcnt&07) == 05) {
		dp->b_errcnt++;
		lobyte(rp->cs1) = IE|RECAL|GO;
		return;
	}
	if (dp->b_active == 0)
		dp->io_start = lbolt;
	dp->b_active++;
	dsn = sn = (int)bp->trksec&0377;
	dsn = (dsn<<2) - ((rp->la&0xfff0)>>4) - GDIST;
	if (dsn<=0) dsn += gdloc(pos).sec*4;
	if (((bp->cylin - rp->dc) || dsn > RDIST*4) &&
	    dp->b_active<3) {
		rp->dc = bp->cylin;
		sn -= SDIST;
		if (sn<0) sn += gdloc(pos).sec;
		rp->da = sn;
	/* sector search, track ignored, lookahead SDIST */
		lobyte(rp->cs1) = IE|SEARCH|GO;
		if (dp->b_active == 1) {
			gdstat[pos].ios.io_misc++;
		}
	} else {
		dp->b_forw = NULL;
		if (gdtab[ctl].b_actf == NULL)
			gdtab[ctl].b_actf = (struct buf *)dp;
		else
			gdtab[ctl].b_actl->b_forw = (struct buf *)dp;
		gdtab[ctl].b_actl = (struct buf *)dp;
	}
}

ghpintr(pos, bp, dp, rp)
register struct buf *bp;
register struct iobuf *dp;
register struct device *rp;
{
	if (++dp->b_errcnt>16)
		bp->b_flags |= B_ERROR;
	else if (rp->er1&WLE) {
		bp->b_flags |= B_ERROR;
		gdprint(bp->b_dev, "\nWrite lock error");
	} else
		gdtab[gdptoc(pos)].b_active = 0;
	fmtberr(dp, gdloc(pos).sizep[gdslice(bp->b_dev)].cyloff);
	if (dp->b_errcnt > 2)
		prcom(gdprint, bp, rp->er1, rp->ds);
	if (rp->er1 == DCK)
		if (ghpecc(rp, bp, pos)) return(1);
	rp->cs1 = TRE|IE|DCLR|GO;
	return(0);
}

ghpecc(rp, bp, pos)
register struct device *rp;
register struct buf *bp;
{
	register i;
	register b, n;

	if (rp->ec2 == 0) {
		rp->of = FMT22;
		return(0);
	}
	i = rp->ec1 - 1;
	n = i&017;
	i = (i&~017)>>3;
	b = ((rp->wc + (bp->b_bcount>>1) - 1)>>8)&0377;
	i += b<<BSHIFT;
	if (i < bp->b_bcount)
		biput(bp, i, biget(bp, i)^(rp->ec2<<n));
	i += 2;
	if (i < bp->b_bcount)
		biput(bp, i, biget(bp, i)^(rp->ec2>>(16-n)));
	gdtab[gdptoc(pos)].b_active++;
	if (rp->wc) {
		i = (int)bp->trksec;
		i = gdloc(pos).sec*(i>>8) + (i&0377) + b + 1;
		if (i >= gdloc(pos).sec*gdloc(pos).trk) {
			i -= gdloc(pos).sec*gdloc(pos).trk;
			rp->dc = bp->cylin + 1;
		} else
			rp->dc = bp->cylin;
		rp->da = ((i/gdloc(pos).sec)<<8) + (i%gdloc(pos).sec);
		i = (rp->cs1&~RDY)|IE|GO;
		rp->cs1 = TRE|IE|DCLR|GO;
		if (pwr_act >= 0)
			rp->cs1 = i;
		return(1);
	} else
		return(0);
}
