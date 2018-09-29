/* @(#)hp.c	1.1 */
/*
 * RJP04, RWP04 driver
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

struct device
{
	int	hpcs1, hpwc, hpba, hpda;	
	int	hpcs2, hpds, hper1, hpas;	
	int	hpla, hpdb, hpmr, hpdt;	
	int	hpsn, hpof, hpdc, hpcc;	
	int	hper2, hper3, hpec1, hpec2;	
	int	hpbae, hpcs3;	
};

struct device *hp_addr;

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
#define	SC	0100000	/* cs1 - Special condition */
#define	TRE	040000	/* cs1 - transfer error */
#define	DVA	04000	/* cs1 - drive available */
#define	RDY	0200	/* cs1 - Ready */
#define	IE	0100	/* cs1 - interrupt enable */
#define	NED	010000	/* cs2 - non-existent device */
#define	CTLCLR	040	/* cs2 - init */
#define	WLE	04000	/* er1 - Write lock error */
#define	DCK	0100000	/* er1 - Data check */
#define	FMT22	010000	/* of - 16 bit /word format */
#define	ECI	04000	/* of - ecc inhibit */

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

#define	rh70	(cputype == 70)

hpopen(dev)
{
	register struct iobuf *dp;

	if ((hptab.b_flags&B_ONCE)==0) {
		hptab.b_flags |= B_ONCE;
		hptimer();
	}
	dp = &hputab[dev>>3];
	dp->io_addr = (physadr)hp_addr;
	dp->io_nreg = rh70?NDEVREG:NDEVREG-2;
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
	int	unit;
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
	rp = hp_addr;
	lobyte(rp->hpcs2) = unit;
	lobyte(rp->hpcs1) = IE;
	rp->hpas = 1<<unit;
	bp = dp->b_actf;
	if ((rp->hpds&MOL) == 0) {
		if (dp->wtime!=WOK)
			return;
		if (bp == NULL)
			dp->wtime = WOK;
		else if (rp->hpcs2&NED)
			dp->wtime = WNED;
		else if (!pwr_act)
			dp->wtime = WMOL;
		else
			dp->wtime = WERR;
		return;
	} else if (rp->hpds&ERR) {
		fmtberr(dp,(bp==NULL)?0:hp_sizes[minor(bp->b_dev)&07].cyloff);
		lobyte(rp->hpcs1) = IE|DCLR|GO;
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
		lobyte(rp->hpcs1) = IE|PRESET|GO;
		rp->hpof = FMT22;
	}
	if (bp == NULL) {
		lobyte(rp->hpcs1) = IE|RELEASE|GO;
		return;
	}
	if ((dp->b_errcnt&07) == 05) {
		dp->b_errcnt++;
		lobyte(rp->hpcs1) = IE|RECAL|GO;
		return;
	}
	if (dp->b_active == 0)
		dp->io_start = lbolt;
	dp->b_active++;
	sn = (int)bp->trksec&0377;
	dsn = (sn<<2) - (rp->hpla>>4) - GDIST;
	if (dsn<=0) dsn += NSECT*4;
	if (((bp->cylin - rp->hpcc) || dsn > RDIST*4) &&
	    dp->b_active<3) {
		rp->hpdc = bp->cylin;
		sn -= SDIST;
		if (sn<0) sn += NSECT;
		rp->hpda = sn;
	/* sector search, track ignored, lookahead SDIST */
		lobyte(rp->hpcs1) = IE|SEARCH|GO;
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
	int unit;

    loop:
	if ((dp = (struct iobuf *)hptab.b_actf) == NULL)
		return;
	if ((bp = dp->b_actf) == NULL) {
		hptab.b_actf = dp->b_forw;
		goto loop;
	}
	rp = hp_addr;
	unit = minor(bp->b_dev)>>3;
	rp->hpcs2 = unit;
	hptab.b_active++;
	rp->hpdc = bp->cylin;
	rp->hpda = (int)bp->trksec;
	rp->hpba = loword(bp->b_paddr);
	if (rh70)
		rp->hpbae = hiword(bp->b_paddr);
	rp->hpwc = -(bp->b_bcount>>1);
	blkacty |= (1<<HP0);
	if (bp->b_flags & B_READ)
		unit = ((hiword(bp->b_paddr)&3)<<8)|IE|RCOM|GO;
	else
		unit = ((hiword(bp->b_paddr)&3)<<8)|IE|WCOM|GO;
	if (pwr_act >= 0)
		rp->hpcs1 = unit;
}

hpintr()
{
	register struct buf *bp;
	register unit;
	register struct device *rp;
	struct iobuf *dp;
	int	as, cs1;

	rp = hp_addr;
	as = rp->hpas&0377;
	cs1 = rp->hpcs1;
	if (hptab.b_active) {	/* data transfer underway */
		blkacty &= ~(1<<HP0);
		dp = (struct iobuf *)hptab.b_actf;
		bp = dp->b_actf;
		unit = minor(bp->b_dev);
		lobyte(rp->hpcs2) = unit>>3;
		if (cs1 & TRE) {		/* error bit */
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
				prcom(hpprint, bp, rp->hper1, rp->hpcs2);
			if (rp->hper1 == DCK) {
				if (hpecc(rp, bp))
					return;
			}
			rp->hpcs1 = TRE|IE|DCLR|GO;
		}
		unit >>= 3;
		if (hptab.b_active) {
			if (dp->io_erec)
				logberr(dp,bp->b_flags&B_ERROR);
			hptab.b_active = 0;
			dp->b_active = 0;
			dp->b_errcnt = 0;
			dp->b_actf = bp->av_forw;
			bp->b_resid = (-rp->hpwc)<<1;
			dp->qcnt--;
			if (bp == (struct buf *)dp->acts)
				dp->acts = (int)dp->b_actf;
			rp->hpcs1 = IE|RELEASE|GO;
			hpstat[unit].io_resp += lbolt - bp->b_start;
			hpstat[unit].io_act += lbolt - dp->io_start;
			iodone(bp);
		}
		hptab.b_actf = dp->b_forw;
		if (dp->b_actf)
			hpustart(unit);
		as &= ~(1<<unit);
	} else {
		if (as == 0)
			rp->hpcs1 = IE;
		hibyte(rp->hpcs1) = TRE>>8;
	}
	for (unit=0; as; unit++)
		if (as&(1<<unit)) {
			as &= ~(1<<unit);
			hpustart(unit);
		}
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
	int	b, n;

	if (rp->hpec2 == 0) {
		rp->hpof = FMT22;
		return(0);
	}
	i = rp->hpec1 - 1;
	n = i&017;
	i = (i&~017)>>3;
	b = ((rp->hpwc + (bp->b_bcount>>1) - 1)>>8)&0377;
	i += b<<BSHIFT;
	if ( i < bp->b_bcount)
		biput(bp, i, biget(bp, i)^(rp->hpec2<<n));
	i += 2;
	if ( i < bp->b_bcount)
		biput(bp, i, biget(bp, i)^(rp->hpec2>>(16-n)));
	hptab.b_active++;
	if (rp->hpwc) {
		i = (int)bp->trksec;
		i = NSECT*(i>>8) + (i&0377) + b + 1;
		if (i >= NSECT*NTRAC) {
			i -= NSECT*NTRAC;
			rp->hpdc = bp->cylin + 1;
		} else
			rp->hpdc = bp->cylin;
		rp->hpda = ((i/NSECT)<<8) + (i%NSECT);
		i = (rp->hpcs1&~RDY)|IE|GO;
		rp->hpcs1 = TRE|IE|DCLR|GO;
		if (pwr_act >= 0)
			rp->hpcs1 = i;
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

	hp_addr->hpcs2 = CTLCLR;
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
