/* @(#)hp.c	6.3 */
/*
 * RP04/RP06 disk driver
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/buf.h"
#include "sys/elog.h"
#include "sys/iobuf.h"
#include "sys/systm.h"
#include "sys/mba.h"

struct device
{
	int	hpcs1, hpds, hper1, hpmr;
	int	hpas, hpda, hpdt, hpla;
	int	hpsn, hpof, hpdc, hpcc;
	int	hper2, hper3, hpec1, hpec2;
};

extern struct mba *hp_addr;
#define	mbactl	(hp_addr->ireg.regs)
#define	mbadev	(hp_addr->ereg)
#define	mbamap	(hp_addr->map)

struct	size
{
	daddr_t	nblocks;
	int	cyloff;
} hp_sizes[8];

struct	iotime	hpstat[8];
struct	iobuf	hptab = tabinit(HP0,0);
#define	sa(X)	tabinit(HP0,&hpstat[X].ios)
struct	iobuf	hputab[8] = {
	sa(0), sa(1), sa(2), sa(3), sa(4), sa(5), sa(6), sa(7)
};

#define	NSECT	22
#define	NTRAC	19
#define	SDIST	3
#define	RDIST	7
#define	GDIST	1

#define	GO	01
#define	RECAL	06
#define	DCLR	010
#define	RELEASE	012
#define	PRESET	020
#define	SEARCH	030
#define	WCOM	060
#define	RCOM	070

#define	ERR	040000	/* ds - Error */
#define	MOL	010000	/* ds - Medium online */
#define	VV	0100	/* ds - volume valid */
#define	WLE	04000	/* er1 - Write lock error */
#define	DCK	0100000	/* er1 - Data check */
#define	FMT22	010000
#define	NED	0x40000

#define	wtime	b_flags
#define	WOK	0
#define	WABORT	1
#define	WNED	2
#define	WMOL	4
#define	WERR	30

#define	acts	io_s1
#define	qcnt	io_s2

#define	trksec	av_back
#define	cylin	b_resid

hpopen(dev)
{
	register struct iobuf *dp;

	if ((hptab.b_flags&B_ONCE)==0) {
		hptab.b_flags |= B_ONCE;
		hptab.io_mba = (physadr)&mbactl;
		hptimer();
	}
	dp = &hputab[dev>>3];
	dp->io_addr = (physadr)&mbadev[dev>>3];
	dp->io_mba = (physadr)&mbactl;
	dp->io_nreg = NDEVREG;
}

hpclose(dev)
{
}

hpstrategy(bp)
register struct buf *bp;
{
	register struct iobuf *dp;
	register struct buf *ap;
	daddr_t	last;
	register unit;
	int	co;

	unit = minor(bp->b_dev);
	last = hp_sizes[unit&07].nblocks;
	co = hp_sizes[unit&07].cyloff;
	unit >>= 3;
	dp = &hputab[unit];
	if (bp->b_blkno < 0 || bp->b_blkno >= last) {
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
	bp->cylin = bp->b_blkno/(NSECT*NTRAC) + co;
	co = bp->b_blkno%(NSECT*NTRAC);
	*((int *)(&bp->trksec)) = ((co/NSECT)<<8)+(co%NSECT);
	hpstat[unit].io_cnt++;
	hpstat[unit].io_bcnt += btoc(bp->b_bcount);
	spl6();
	dp->qcnt++;
	if (dp->b_actf == NULL) {
		dp->b_actf = bp;
		dp->b_actl = bp;
		dp->acts = (int)bp;
	} else {
		register struct buf *cp;

		if (((int)hpstat[unit].io_cnt&07) == 0)
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
			hputab[unit].b_actl = bp;
	}
	if (hputab[unit].b_active == 0) {
		hpustart(unit);
		if (hptab.b_active == 0 && hptab.b_actf != NULL)
			hpstart();
	}
	spl0();
}

hpustart(unit)
{
	register struct buf *bp;
	register struct iobuf *dp;
	register struct device *rp;
	int	sn, dsn;

	dp = &hputab[unit];
	rp = (struct device *)&mbadev[unit];
	rp->hpas = 1<<unit;
	bp = dp->b_actf;
	if ((rp->hpds&MOL) == 0) {
		if (dp->wtime!=WOK)
			return;
		if (bp == NULL)
			dp->wtime = WOK;
		else if (mbactl.sr&NED)
			dp->wtime = WNED;
		else if (!pwr_act)
			dp->wtime = WMOL;
		else
			dp->wtime = WERR;
		return;
	} else if (rp->hpds&ERR) {
		dp->b_dev = makedev(HP0, (unit<<3));
		fmtberr(dp,(bp==NULL)?0:hp_sizes[minor(bp->b_dev)&07].cyloff);
		rp->hpcs1 = DCLR|GO;
		if (rp->hpds&ERR || ++dp->b_errcnt > 16) {
			printf("hard err on RP04/5/6 drive %d %o %o %o %o\n",
			unit, rp->hpds, rp->hper1, rp->hper2, rp->hper3);
			dp->wtime = WERR;
			rp->hpas = 1<<unit;
			return;
		}
	}
	dp->wtime = WOK;
	if ((rp->hpds&VV) == 0) {
		rp->hpcs1 = PRESET|GO;
		rp->hpof = FMT22;
	}
	if (bp == NULL) {
		rp->hpcs1 = RELEASE|GO;
		return;
	}
	if ((dp->b_errcnt&07) == 05) {
		dp->b_errcnt++;
		rp->hpcs1 = RECAL|GO;
		return;
	}
	if (dp->b_active == 0)
		dp->io_start = lbolt;
	dp->b_active++;
	sn = (int)bp->trksec&0377;
	dsn = (sn<<2) - ((rp->hpla&0xffff)>>4) - GDIST;
	if (dsn<=0) dsn += NSECT*4;
	if (((bp->cylin - rp->hpcc) || dsn > RDIST*4) &&
	    dp->b_active<3) {
		rp->hpdc = bp->cylin;
		sn -= SDIST;
		if (sn<0) sn += NSECT;
		rp->hpda = sn;
	/* sector search, track ignored, lookahead SDIST */
		rp->hpcs1 = SEARCH|GO;
		if (dp->b_active == 1) {
			hpstat[unit].ios.io_misc++;
		}
	} else {
		dp->b_forw = NULL;
		if (hptab.b_actf == NULL)
			hptab.b_actf = (struct buf *)dp;
		else
			hptab.b_actl->b_forw = (struct buf *)dp;
		hptab.b_actl = (struct buf *)dp;
	}
}

hpstart()
{
	register struct buf *bp;
	register struct iobuf *dp;
	register struct device *rp;
	register unit;

    loop:
	if ((dp = (struct iobuf *)hptab.b_actf) == NULL)
		return;
	if ((bp = dp->b_actf) == NULL) {
		hptab.b_actf = dp->b_forw;
		goto loop;
	}
	unit = minor(bp->b_dev)>>3;
	rp = (struct device *)&mbadev[unit];
	hptab.b_active++;
	rp->hpdc = bp->cylin;
	rp->hpda = (int)bp->trksec;
	blkacty |= (1<<HP0);
	mbasetup(bp, hp_addr);
	if (bp->b_flags & B_READ)
		unit = RCOM | GO;
	else
		unit = WCOM | GO;
	if (pwr_act >= 0)
		rp->hpcs1 = unit;
}

hpintr(dev)
{
	register struct buf *bp;
	register unit;
	register struct device *rp;
	register struct iobuf *dp;
	register as;
	int	mbastat;

	if (mbactl.csr&0xffffff00)
		mbasrv(&mbactl);
	if (hptab.b_active) {	/* data transfer underway */
		blkacty &= ~(1<<HP0);
		dp = (struct iobuf *)hptab.b_actf;
		bp = dp->b_actf;
		unit = minor(bp->b_dev);
		rp = (struct device *)&mbadev[unit>>3];
		as = rp->hpas&0377;
		mbastat = mbactl.sr;
		if (mbastat & MBADTABT) {
			if (++dp->b_errcnt>16)
				bp->b_flags |= B_ERROR;
			else if (rp->hper1&WLE) {
				bp->b_flags |= B_ERROR;
				hpprint(bp->b_dev, "\nWrite lock error");
			} else {
				hptab.b_active = 0;
			}
			fmtberr(dp,hp_sizes[unit&07].cyloff);
			if (dp->b_errcnt > 2)
				prcom(hpprint, bp, rp->hper1&0xfff, mbastat);
			mbactl.sr = -1;
			if ((rp->hper1&0xffff) == DCK) {
				if (hpecc(rp, bp))
					return;
			}
			rp->hpcs1 = DCLR|GO;
		}
		unit >>= 3;
		if (hptab.b_active) {
			if (dp->io_erec)
				logberr(dp,bp->b_flags&B_ERROR);
			hptab.b_active = 0;
			dp->b_active = 0;
			dp->b_errcnt = 0;
			dp->b_actf = bp->av_forw;
			bp->b_resid = -(mbactl.bcr) & 0xffff;
			dp->qcnt--;
			if (bp == (struct buf *)dp->acts)
				dp->acts = (int)dp->b_actf;
			rp->hpcs1 = RELEASE|GO;
			hpstat[unit].io_resp += lbolt - bp->b_start;
			hpstat[unit].io_act += lbolt - dp->io_start;
			iodone(bp);
		}
		hptab.b_actf = dp->b_forw;
		if (dp->b_actf)
			hpustart(unit);
		as &= ~(1<<unit);
	} else {
		as = ((struct device *)&mbadev[0])->hpas&0377;
	}
	for (unit=0; as; unit++)
		if (as&(1<<unit)) {
			as &= ~(1<<unit);
			hpustart(unit);
		}
	mbactl.sr = -1;
	if (hptab.b_actf != NULL)
		hpstart();
}

hpread(dev)
{
	if (physck(hp_sizes[dev&07].nblocks, B_READ))
		physio(hpstrategy, 0, dev, B_READ);
}

hpwrite(dev)
{
	if (physck(hp_sizes[dev&07].nblocks, B_WRITE))
		physio(hpstrategy, 0, dev, B_WRITE);
}

hpecc(rp, bp)
register struct device *rp;
register struct buf *bp;
{
	register i;
	register b, n, map, mix;
	register char * cp;
	register mask;
	extern short piget();

	mask = rp->hpec2&0xffff;
	if (mask == 0) {
		rp->hpof = FMT22;
		return(0);
	}
	i = (rp->hpec1&0xffff) - 1;
	n = i&017;
	i = (i&~017)>>3;
	b = (((mbactl.bcr&0xffff) +
		(bp->b_bcount) - 1)>>9)&0177;
	map = b;
	mix = i + ((int)paddr(bp)&0x1ff);
	i += b<<BSHIFT;
	if (i < bp->b_bcount) {
		cp = (char *)((mbamap[map+(mix>>9)]&0x1fffff)<<9)+(mix&0x1ff);
		piput(cp,piget(cp)^(mask<<n));
	}
	mix += 2;
	i += 2;
	if (i < bp->b_bcount) {
		cp = (char *)((mbamap[map+(mix>>9)]&0x1fffff)<<9)+(mix&0x1ff);
		piput(cp,piget(cp)^(mask>>(16-n)));
	}
	hptab.b_active++;
	if (mbactl.bcr) {
		i = (int)bp->trksec;
		i = NSECT*(i>>8) + (i&0377) + b + 1;
		if (i >= NSECT*NTRAC) {
			i -= NSECT*NTRAC;
			rp->hpdc = bp->cylin + 1;
		} else
			rp->hpdc = bp->cylin;
		rp->hpda = ((i/NSECT)<<8) + (i%NSECT);
		rp->hpcs1 = DCLR|GO;
		mbactl.sr = -1;
		mbactl.var = ((b+1)<<9)|((int)paddr(bp)&0x1ff);
		if (pwr_act >= 0)
			rp->hpcs1 = RCOM|GO;
		return(1);
	} else
		return(0);
}

hptimer()
{
	register unit;
	register struct iobuf *dp;
	register struct buf *bp;

	for (unit=0; unit < 8; unit++) {
		dp = &hputab[unit];
		if (dp->wtime == WOK)
			continue;
		hpustart(unit);
		if (dp->wtime == WOK)
			continue;
		if (dp->wtime == WABORT) {
			if (dp->io_erec)
				logberr(dp, B_ERROR);
			while (bp = dp->b_actf) {
				bp->b_flags |= B_ERROR;
				dp->b_actf = bp->av_forw;
				iodone(bp);
			}
			dp->b_active = 0;
			dp->b_errcnt = 0;
			dp->qcnt = 0;
			printf("RP04/5/6 drive %d not available\n", unit);
		}
		dp->wtime--;
	}
	if (hptab.b_active == 0 && hptab.b_actf != NULL)
		hpstart();
	timeout(hptimer, 0, 15*HZ);
}

hpclr()
{
	register unit;

	hptab.b_active = 0;
	hptab.b_actf = NULL;
	for (unit = 0; unit < 8; unit++)
		hpustart(unit);
	hpstart();
}

hpprint(dev, str)
char *str;
{
	printf("%s on RP04/5/6 drive %d, slice %d\n", str, (dev>>3)&7, dev&7);
}
