/* @(#)hu.c	6.3 */
/*
 * TU78 tape driver
 * Handles one TM78 controller, up to 4 TU78 slave transports
 * minor device classes:
 * bits 0,1: slave select
 * bit 2 off: rewind on close; on: position after first TM
 * bit 3 off: 1600 bpi; on: 6250 bpi
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/buf.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/elog.h"
#include "sys/iobuf.h"
#include "sys/mba.h"

struct	device
{
	int	hucsr, hudtc, hufm, humr1;
	int	huas, hufc, hudt, huds;
	int	husn, humr2, humr3, hundc;
	int	hundt[4];
	int	huia, huid;
};

#define	NUNIT	4
struct	iotime	hustat[NUNIT];
struct	iobuf	hutab = tabinit(HT0,0);
struct	buf	chubuf;

#define	INF	1000000

char	huopenf[NUNIT];
int	hueot[NUNIT];
short	husndt[NUNIT];
daddr_t	hublkno[NUNIT];
daddr_t	hunxrec[NUNIT];

extern struct mba *hu_addr;
#define	mbactl	(hu_addr->ireg.regs)
#define	mbadev	(hu_addr->ereg)

#define	GO	01
#define	REW	06
#define	SENSE	010
#define	SFORW	020
#define	SREV	022
#define	SFFILE	024
#define	SRFILE	026
#define	ERASE	034
#define	CLFILE	040
#define	CLFGCR	042
#define	SFLEOT	046
#define	WCOM	060
#define	WGCR	062
#define	RCOM	070
#define	XSNS	072
#define ONL	0x2000
#define	PE	0x0800
#define	BOT	0x0400
#define EOT	0x0200
#define FPT	0x0100

#define	OXCLD	01
#define	OWRIT	02
#define	OLEOT	04
#define	OFATAL	0200

#define	CDONE	01
#define	CTM	02
#define	CEOT	04
#define	CLEOT	05
#define	CREW	07
#define	CFPT	010
#define	CNOC	015
#define	CLOREC	020
#define	CSHREC	021
#define	CRETRY	022
#define	CTMFLTB	032
#define	CMBFLT	034
#define	CKEYFLT	077

#define	TMCLR	0x4000

#define	SIO	1
#define	SSFOR	2
#define	SSREV	3
#define SCOM	5
#define SOK	6


huopen(dev, flag)
{
	register unit, ds;

	unit = dev&03;
	if (unit >= NUNIT || huopenf[unit]) {
		u.u_error = ENXIO;
		return;
	}
	hutab.io_addr = (physadr)&mbadev[0];
	hutab.io_mba = (physadr)&mbactl;
	hutab.io_nreg = NDEVREG;
	huopenf[unit] = OXCLD;
	hublkno[unit] = 0;
	hunxrec[unit] = INF;
	ds = hucommand(unit, SENSE);
	if ((ds&ONL)==0 || ((flag&FWRITE) && (ds&FPT))) {
		u.u_error = ENXIO;
		huopenf[unit] = 0;
		return;
	}
	if (flag&FAPPEND) {
		hucommand(unit, SFFILE);
		hucommand(unit, SREV);
		ds = hucommand(unit, SENSE);
	}
	if (!(ds&EOT))
		hueot[unit] = 0;
}

huclose(dev, flag)
{
	register unit;

	unit = dev&03;
	if (flag&FWRITE)
		hucommand(unit, (dev&010) ? CLFGCR : CLFILE);
	if (dev&04) {
		if (!(flag&FWRITE))
			if (hublkno[unit] <= hunxrec[unit])
				hucommand(unit, SFLEOT);
	} else
		hucommand(unit, REW);
	huopenf[unit] = 0;
}

hucommand(unit, com)
{
	register struct buf *bp;

	bp = &chubuf;
	spl5();
	while(bp->b_flags&B_BUSY) {
		bp->b_flags |= B_WANTED;
		sleep(bp, PRIBIO);
	}
	spl0();
	bp->b_dev = unit;
	bp->b_resid = com;
	bp->b_blkno = 0;
	bp->b_flags = B_BUSY|B_READ;
	hustrategy(bp);
	iowait(bp);
	if (bp->b_flags&B_WANTED)
		wakeup(bp);
	bp->b_flags = 0;
	return(bp->b_resid);
}

hustrategy(bp)
register struct buf *bp;
{
	register daddr_t *p;
	register unit;

	if (bp != &chubuf) {
		unit = bp->b_dev&03;
		p = &hunxrec[unit];
		if (bp->b_blkno > *p) {
			bp->b_flags |= B_ERROR;
			bp->b_error = ENXIO;
			iodone(bp);
			return;
		}
		if (bp->b_blkno == *p && bp->b_flags&B_READ) {
			clrbuf(bp);
			bp->b_resid = bp->b_bcount;
			iodone(bp);
			return;
		}
		if ((bp->b_flags&B_READ)==0)
			*p = bp->b_blkno + 1;
		bp->b_start = lbolt;
		hustat[unit].io_bcnt += btoc(bp->b_bcount);
	}
	bp->av_forw = NULL;
	spl5();
	if (hutab.b_actf == NULL)
		hutab.b_actf = bp;
	else
		hutab.b_actl->av_forw = bp;
	hutab.b_actl = bp;
	if (hutab.b_active==0)
		hustart();
	spl0();
}

hustart()
{
	register struct buf *bp;
	register unit, com;
	register struct device *rp;
	daddr_t blkno;

	rp = (struct device *)&mbadev[0];
    loop:
	if ((bp = hutab.b_actf) == NULL)
		return;
	unit = bp->b_dev&03;
	blkno = hublkno[unit];
	if (bp == &chubuf) {
		hustat[unit].ios.io_misc++;
		hutab.b_active = SCOM;
		rp->hundt[unit] = bp->b_resid|GO;
		return;
	}
	if ((huopenf[unit]&OFATAL) || bp->b_blkno > hunxrec[unit])
		goto abort;
	if (hutab.io_start == 0)
		hutab.io_start = lbolt;
	if (blkno == bp->b_blkno) {
		if (hueot[unit] > 8)
			goto abort;
		if (bp->b_flags&B_READ)
			com = RCOM|GO;
		else {
			if (hueot[unit] && blkno > hueot[unit]) {
				bp->b_error = ENOSPC;
				goto abort;
			}
			if (bp->b_dev&010)
				com = WGCR|GO;
			else
				com = WCOM|GO;
		}
		hutab.b_active = SIO;
		rp->hufm = (1<<2) | unit;
		rp->hufc = bp->b_bcount;
		hustat[unit].io_cnt++;
		blkacty |= (1<<HT0);
		mbasetup(bp, hu_addr);
		if (pwr_act >= 0)
			rp->hucsr = com;
	} else {
		hustat[unit].ios.io_misc++;
		com = blkno - bp->b_blkno;
		if (blkno < bp->b_blkno) {
			com = -com;
			hutab.b_active = SSFOR;
			if (com > 254)
				com = 254;
			com = (com<<8)|SFORW|GO;
		} else {
			hutab.b_active = SSREV;
			if (com > 254)
				com = 254;
			com = (com<<8)|SREV|GO;
		}
		husndt[unit] = com;
		rp->hundt[unit] = com;
	}
	return;
    abort:
	bp->b_flags |= B_ERROR;
    next:
	hutab.b_actf = bp->av_forw;
	iodone(bp);
	goto loop;
}

huintr(dev)
{
	register struct buf *bp;
	register int unit, state;
	register struct device *rp;
	int	ci, cf;
	extern huprint();

	if (mbactl.csr&0xffffff00)
		mbasrv(&mbactl);
	rp = (struct device *)&mbadev[0];
	if ((bp = hutab.b_actf)==NULL) {
		rp->huid = TMCLR;
		mbactl.sr = mbactl.sr;
		rp->huas = rp->huas;
		return;
	}
	blkacty &= ~(1<<HT0);
	unit = bp->b_dev&03;
	if ((mbactl.sr&MBATTN) && (unit != ((rp->hundc>>8)&03))) {
		rp->huas = rp->huas;
		return;
	}
	state = hutab.b_active;
	hutab.b_active = 0;
	if (state == SIO) {
		switch(rp->hudtc&0x3f) {
		case CTM:
			mbactl.bcr = -bp->b_bcount;
			hunxrec[unit] = bp->b_blkno;
			goto done;
		case CDONE:
			if (mbactl.sr & MBADTABT)
				goto retry;
		done:
			hublkno[unit]++;
			if (hutab.io_erec)
				logberr(&hutab,bp->b_flags&B_ERROR);
			hutab.b_errcnt = 0;
			hutab.b_actf = bp->av_forw;
			bp->b_resid = (-mbactl.bcr) & 0xffff;
			iodone(bp);
			hustat[unit].io_resp += lbolt - bp->b_start;
			hustat[unit].io_act += lbolt - hutab.io_start;
			hutab.io_start = 0;
			break;
		case CEOT:
			hueot[unit]++;
			goto done;
		case CRETRY:
		case CFPT:
		retry:
			hustat[unit].ios.io_misc++;
			hutab.io_stp = &hustat[unit].ios;
			fmtberr(&hutab,0);
			if (++hutab.b_errcnt > 10) {
				bp->b_flags |= B_ERROR;
				goto done;
			}
			if (hutab.b_errcnt > 4)
				prcom(huprint, bp, rp->hudtc, mbactl.sr);
			break;
		case CSHREC:
		case CLOREC:
			if (!(bp->b_flags&B_PHYS))
				bp->b_flags |= B_ERROR;
			goto done;
		case CTMFLTB:
		case CMBFLT:
		case CKEYFLT:
			rp->huid = TMCLR;
		default:
			huopenf[unit] |= OFATAL;
			prcom(huprint, bp, rp->hudtc, mbactl.sr);
			hutab.io_stp = &hustat[unit].ios;
			fmtberr(&hutab, 0);
			bp->b_flags |= B_ERROR;
			goto done;
		}
	} else {
		ci = (husndt[unit]>>8)&0377;
		cf = (rp->hundt[unit]>>8)&0377;
		switch(rp->hundc&0x3f) {
		case CEOT:
			hueot[unit]++;
			goto ndone;
		case CTMFLTB:
		case CMBFLT:
		case CKEYFLT:
			rp->huid = TMCLR;
			if (state == SCOM)
				goto ndone;
		default:
			huopenf[unit] = OFATAL;
			hutab.io_stp = &hustat[unit].ios;
			fmtberr(&hutab, 0);
		case CDONE:
		case CFPT:
		case CLEOT:
		case CREW:
		ndone:
			switch(state) {
			case SCOM:
				if (hutab.io_erec)
					logberr(&hutab, bp->b_flags&B_ERROR);
				hutab.b_errcnt = 0;
				hutab.b_actf = bp->av_forw;
				bp->b_resid = rp->huds;
				iodone(bp);
				break;
			case SSFOR:
				hublkno[unit] += ci;
				break;
			case SSREV:
				hublkno[unit] -= ci;
				break;
			}
			break;
		case CTM:
		case CNOC:
			switch(state) {
			case SSFOR:
				hublkno[unit] += (ci-cf) + 1;
				hunxrec[unit] = hublkno[unit] - 1;
				break;
			case SSREV:
				hublkno[unit] -= (ci-cf) + 1;
				hunxrec[unit] = hublkno[unit];
				break;
			}
			break;
		}
	}
	rp->huas = rp->huas;
	mbactl.sr = mbactl.sr;
	hustart();
}

huread(dev)
{
	huphys(dev);
	physio(hustrategy, 0, dev, B_READ);
}

huwrite(dev)
{
	huphys(dev);
	physio(hustrategy, 0, dev, B_WRITE);
}

huphys(dev)
{
	register unit;
	daddr_t a;

	unit = dev&03;
	a = u.u_offset >> BSHIFT;
	hublkno[unit] = a;
	hunxrec[unit] = a+1;
}

huprint(dev, str)
char *str;
{
	printf("%s on TU78 drive %d\n", str, dev&3);
}
