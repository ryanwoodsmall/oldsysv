/* @(#)hs.c	1.1 */
/*
 * RS03/04 disk driver
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

struct device {
	int	hscs1, hswc, hsba, hsda;
	int	hscs2, hsds, hser, hsas;
	int	hsla, hsdb, hsmr, hsdt;
	int	hsbae, hscs3;
};

struct	iotime	hsstat[8];
struct	iobuf	hstab =	tabinit(HS0,0);
struct	buf	rhsbuf;

struct device *hs_addr;
int	hs_cnt;


#define	GO	01
#define	RCLR	010
#define	WCOM	060
#define	RCOM	070
#define	IE	0100

#define	MOL	010000	/* ds - Medium OnLine */
#define	TRE	040000	/* cs1 - transfer error */
#define	CTLCLR	040	/* cs2 - init */

#define	wtime	io_s2
#define	WOK	0
#define	WABORT	1
#define	WMOL	4
#define	WERR	30
#define	HSTIMEOUT	15*HZ

#define	rh70	(cputype == 70)

hsopen(dev)
{
	hstab.io_addr = (physadr)hs_addr;
	hstab.io_nreg = rh70?NDEVREG:NDEVREG-2;
}

hsclose(dev)
{
}

hsstrategy(bp)
register struct buf *bp;
{
	register unit;
	daddr_t	mblks;

	if (bp->b_dev&010)
		mblks = 2048; /* RS04 */
	else	mblks = 1024; /* RS03 */
	if (bp->b_blkno >= mblks) {
		if (bp->b_blkno == mblks && bp->b_flags&B_READ)
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
	hsstat[unit].io_cnt++;
	hsstat[unit].io_bcnt += btoc(bp->b_bcount);
	spl5();
	if (hstab.b_actf==0)
		hstab.b_actf = bp; else
		hstab.b_actl->av_forw = bp;
	hstab.b_actl = bp;
	if (hstab.b_active==0)
		hsstart();
	spl0();
}

hsstart()
{
	register struct buf *bp;
	register struct device *rp;
	register addr;
	extern hstimer();

	if ((bp = hstab.b_actf) == 0)
		return;
	hstab.b_active++;
	rp = hs_addr;
	addr = bp->b_blkno;
	addr <<= 1;
	if ((bp->b_dev&010) == 0)
		addr <<= 1;
	rp->hscs2 = bp->b_dev & 07;
	if ((rp->hsds&MOL) == 0) {
		if (hstab.wtime)
			return;
		if (!pwr_act)
			hstab.wtime = WMOL;
		else
			hstab.wtime = WERR;
		if ((hstab.b_flags&B_TIME) == 0) {
			hstab.b_flags |= B_TIME;
			timeout(hstimer, 0, HSTIMEOUT);
		}
		return;
	}
	hstab.wtime = WOK;
	hstab.io_start = lbolt;
	if (rh70)
		rp->hsbae = hiword(bp->b_paddr);
	rp->hsda = addr;
	rp->hsba = loword(bp->b_paddr);
	rp->hswc = -(bp->b_bcount>>1);
	addr = ((hiword(bp->b_paddr)&3)<<8)|IE|GO;
	if (bp->b_flags & B_READ)
		addr |= RCOM;
	else
		addr |= WCOM;
	blkacty |= (1<<HS0);
	if (pwr_act >= 0)
		rp->hscs1 = addr;
}

hsintr()
{
	register struct buf *bp;
	register struct device *rp;
	register unit;
	extern hsprint();

	rp = hs_addr;
	if (hstab.b_active == 0) {
		logstray(rp);
		return;
	}
	blkacty &= ~(1<<HS0);
	bp = hstab.b_actf;
	hstab.b_active = 0;
	unit = bp->b_dev & 07;
	if (rp->hscs1 & TRE) {	/* error bit */
		hstab.io_stp = &hsstat[unit].ios;
		fmtberr(&hstab,0);
		if (hstab.b_errcnt > 2)
			prcom(hsprint, bp, rp->hscs2, 0);
		rp->hscs1 = RCLR|GO;
		if (++hstab.b_errcnt <= 10) {
			hsstart();
			return;
		}
		bp->b_flags |= B_ERROR;
	}
	if (hstab.io_erec)
		logberr(&hstab,bp->b_flags&B_ERROR);
	hstab.b_errcnt = 0;
	hstab.b_actf = bp->av_forw;
	bp->b_resid = (-rp->hswc)<<1;
	hsstat[unit].io_resp += lbolt - bp->b_start;
	hsstat[unit].io_act += lbolt - hstab.io_start;
	iodone(bp);
	hsstart();
}

hsread(dev)
{
	if (physck((daddr_t)((dev&010) ? 2048 : 1024), B_READ))
		physio(hsstrategy, &rhsbuf, dev, B_READ);
}

hswrite(dev)
{
	if (physck((daddr_t)((dev&010) ? 2048 : 1024), B_WRITE))
		physio(hsstrategy, &rhsbuf, dev, B_WRITE);
}

hstimer()
{
	register struct buf *bp;

	hstab.b_flags &= ~B_TIME;
	if (hstab.wtime == WOK)
		return;
	hsstart();
	if (hstab.wtime == WOK)
		return;
	if (hstab.wtime == WABORT) {
		if (hstab.io_erec)
			logberr(&hstab, B_ERROR);
		while (bp = hstab.b_actf) {
			bp->b_flags |= B_ERROR;
			hstab.b_actf = bp->av_forw;
			iodone(bp);
		}
		hstab.b_active = 0;
		hstab.b_errcnt = 0;
		printf("RS03/4 not available\n");
	}
	hstab.wtime--;
	if (hstab.wtime == WOK)
		return;
	hstab.b_flags |= B_TIME;
	timeout(hstimer, 0, HSTIMEOUT);
}

hsclr()
{
	register struct device *rp;

	rp = hs_addr;
	rp->hscs2 = CTLCLR;
	hstab.b_active = 0;
	blkacty &= ~(1<<HS0);
	hsstart();
}

hsprint(dev, str)
char *str;
{
	printf("%s on RS03/4 drive %d\n", str, dev&7);
}
