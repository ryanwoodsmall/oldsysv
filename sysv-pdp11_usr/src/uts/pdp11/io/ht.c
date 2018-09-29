/* @(#)ht.c	1.1 */
/*
 * TJU16, TWU16 driver
 * Handles one TM02 controller, up to 4 TU16 slave transports
 * minor device classes:
 * bits 0,1: slave select
 * bit 2 off: rewind on close; on: position after first TM
 * bit 3 off: 800 bpi; on: 1600 bpi
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

struct	device
{
	int	htcs1, htwc, htba, htfc;
	int	htcs2, htds, hter, htas;
	int	htck, htdb, htmr, htdt;
	int	htsn, httc, htbae, htcs3;
};

#define	NUNIT	4
struct	iotime	htstat[NUNIT];
struct	iobuf	httab = tabinit(HT0,0);
struct	buf	chtbuf;

#define	INF	1000000

char	h_openf[NUNIT];
int	h_den[NUNIT];
int	h_eot[NUNIT];
daddr_t	h_blkno[NUNIT];
daddr_t	h_nxrec[NUNIT];

struct device *ht_addr;

#define	GO	01
#define	NOP	0
#define	WEOF	026
#define	SFORW	030
#define	SREV	032
#define	ERASE	024
#define	REW	06
#define	DCLR	010
#define	WCOM	060
#define	RCOM	070
#define P800	01300		/* 800 + pdp11 mode */
#define	P1600	02300		/* 1600 + pdp11 mode */
#define	IENABLE	0100
#define	RDY	0200
#define	TM	04
#define	DRY	0200
#define EOT	02000
#define CS	02000
#define COR	0100000
#define PES	040
#define WRL	04000
#define MOL	010000
#define ERR	040000
#define FCE	01000
#define	TRE	040000
#define HARD	064027	/* UNS|OPI|NEF|FMT|RMR|ILR|ILF */

#define	SIO	1
#define	SSFOR	2
#define	SSREV	3
#define SRETRY	4
#define SCOM	5
#define SOK	6

#define rh70	(cputype == 70)

htopen(dev, flag)
{
	register unit, ds;

	unit = dev&03;
	if (unit >= NUNIT || h_openf[unit]) {
		u.u_error = ENXIO;
		return;
	}
	httab.io_addr = (physadr)ht_addr;
	httab.io_nreg = rh70?NDEVREG:NDEVREG-2;
	h_openf[unit]++;
	h_den[unit] = (dev&010 ? P1600 : P800)|unit;
	h_blkno[unit] = 0;
	h_nxrec[unit] = INF;
	ds = hcommand(unit, NOP);
	if ((ds&MOL)==0 || ((flag&FWRITE) && (ds&WRL))) {
		u.u_error = ENXIO;
		h_openf[unit] = 0;
		return;
	}
	if (flag&FAPPEND) {
		hcommand(unit, SFORW);
		hcommand(unit, SREV);
	}
}

htclose(dev, flag)
{
	register unit;

	unit = dev&03;
	if (flag&FWRITE) {
		hcommand(unit, WEOF);
		hcommand(unit, WEOF);
	}
	if (dev&04) {
		if (flag&FWRITE)
			hcommand(unit, SREV); else
			{
				hcommand(unit, NOP);
				if (h_blkno[unit] <= h_nxrec[unit])
					hcommand(unit, SFORW);
			}
	} else
		hcommand(unit, REW);
	h_openf[unit] = 0;
}

hcommand(unit, com)
{
	register struct buf *bp;

	bp = &chtbuf;
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
	htstrategy(bp);
	iowait(bp);
	if (bp->b_flags&B_WANTED)
		wakeup(bp);
	bp->b_flags = 0;
	return(bp->b_resid);
}

htstrategy(bp)
register struct buf *bp;
{
	register daddr_t *p;
	register unit;

	if (bp != &chtbuf) {
		unit = bp->b_dev&03;
		p = &h_nxrec[unit];
		if (bp->b_blkno > *p) {
			bp->b_flags |= B_ERROR;
			bp->b_error = ENXIO;
			iodone(bp);
			return;
		}
		if (bp->b_blkno == *p && bp->b_flags&B_READ) {
			clear(paddr(bp), BSIZE);
			bp->b_resid = bp->b_bcount;
			iodone(bp);
			return;
		}
		if ((bp->b_flags&B_READ)==0)
			*p = bp->b_blkno + 1;
		bp->b_start = lbolt;
		htstat[unit].io_bcnt += btoc(bp->b_bcount);
	}
	bp->av_forw = NULL;
	spl5();
	if (httab.b_actf == NULL)
		httab.b_actf = bp;
	else
		httab.b_actl->av_forw = bp;
	httab.b_actl = bp;
	if (httab.b_active==0)
		htstart();
	spl0();
}

htstart()
{
	register struct buf *bp;
	register unit;
	register struct device *rp;
	daddr_t blkno;
	int	com;

	rp = ht_addr;
    loop:
	if ((bp = httab.b_actf) == NULL)
		return;
	unit = bp->b_dev&03;
	rp->htcs2 = 0;
	rp->htas = 1<<0;
	if ((rp->httc&03777) != h_den[unit])
		rp->httc = h_den[unit];
	if ((rp->htds&(MOL|DRY)) != (MOL|DRY))
		goto abort;
	blkno = h_blkno[unit];
	if (bp == &chtbuf) {
		if (bp->b_resid==NOP) {
			bp->b_resid = rp->htds;
			goto next;
		}
		htstat[unit].ios.io_misc++;
		httab.b_active = SCOM;
		rp->htfc = 0;
		rp->htcs1 = bp->b_resid|IENABLE|GO;
		return;
	}
	if (h_openf[unit] < 0 || bp->b_blkno > h_nxrec[unit])
		goto abort;
	if (httab.io_start == 0)
		httab.io_start = lbolt;
	if (blkno == bp->b_blkno) {
		if (h_eot[unit] > 8)
			goto abort;
		if ((bp->b_flags&B_READ)==0 && h_eot[unit] && blkno > h_eot[unit]) {
			bp->b_error = ENOSPC;
			goto abort;
		}
		httab.b_active = SIO;
		rp->htba = loword(bp->b_paddr);
		if (rh70)
			rp->htbae = hiword(bp->b_paddr);
		rp->htfc = -bp->b_bcount;
		rp->htwc = -(bp->b_bcount>>1);
		com = ((hiword(bp->b_paddr)&3) << 8) | IENABLE | GO;
		htstat[unit].io_cnt++;
		blkacty |= (1<<HT0);
		if (bp->b_flags & B_READ)
			com |= RCOM;
		else
			com |= WCOM;
		if (pwr_act >= 0)
			rp->htcs1 = com;
	} else {
		htstat[unit].ios.io_misc++;
		if (blkno < bp->b_blkno) {
			httab.b_active = SSFOR;
			rp->htfc = blkno - bp->b_blkno;
			rp->htcs1 = SFORW|IENABLE|GO;
		} else {
			httab.b_active = SSREV;
			rp->htfc = bp->b_blkno - blkno;
			rp->htcs1 = SREV|IENABLE|GO;
		}
	}
	return;
    abort:
	bp->b_flags |= B_ERROR;
    next:
	httab.b_actf = bp->av_forw;
	iodone(bp);
	goto loop;
}

htintr(dev)
{
	register struct buf *bp;
	register int unit;
	register struct device *rp;
	int	err, state;
	extern htprint();

	rp = ht_addr;
	if ((bp = httab.b_actf)==NULL) {
		logstray(rp);
		return;
	}
	blkacty &= ~(1<<HT0);
	unit = bp->b_dev&03;
	state = httab.b_active;
	httab.b_active = 0;
	if (rp->htcs1&TRE) {
		err = rp->hter;
		if (rp->htcs2&077400 || err&HARD)
			state = 0;
		if (bp->b_flags&B_PHYS)
			err &= ~FCE;
		if ((bp->b_flags&B_READ) && (rp->htds&PES))
			err &= ~(CS|COR);
		if ((rp->htds&MOL)==0)
			h_openf[unit] = -1;
		else if (rp->htds&TM) {
			rp->htwc = -(bp->b_bcount>>1);
			h_nxrec[unit] = bp->b_blkno;
			state = SOK;
		}
		else if (state && err == 0)
			state = SOK;
		if (state != SOK) {
			httab.io_stp = &htstat[unit].ios;
			fmtberr(&httab,0);
		}
		if (httab.b_errcnt > 4)
			prcom(htprint, bp, rp->hter, rp->htcs2);
		rp->htcs1 = TRE|DCLR|GO;
		if (state==SIO && ++httab.b_errcnt < 10) {
			httab.b_active = SRETRY;
			h_blkno[unit]++;
			rp->htfc = -1;
			htstat[unit].ios.io_misc++;
			rp->htcs1 = SREV|IENABLE|GO;
			return;
		}
		if (state!=SOK) {
			bp->b_flags |= B_ERROR;
			state = SIO;
		}
	} else if (rp->htds & ERR) {	/* SC */
		if ((rp->htds & TM) == 0) {
			httab.io_stp = &htstat[unit].ios;
			fmtberr(&httab,0);
		}
		rp->htcs1 = DCLR|GO;
	}
	switch(state) {
	case SIO:
	case SOK:
		h_blkno[unit]++;
	case SCOM:
		if (rp->htds&EOT)
			h_eot[unit]++;
		else
			h_eot[unit] = 0;
		if (httab.io_erec)
			logberr(&httab,bp->b_flags&B_ERROR);
		httab.b_errcnt = 0;
		httab.b_actf = bp->av_forw;
		iodone(bp);
		bp->b_resid = (-rp->htwc)<<1;
		if (bp != &chtbuf) {
			htstat[unit].io_resp += lbolt - bp->b_start;
			htstat[unit].io_act += lbolt - httab.io_start;
			httab.io_start = 0;
		}
		break;
	case SRETRY:
		if ((bp->b_flags&B_READ)==0) {
			htstat[unit].ios.io_misc++;
			httab.b_active = SSFOR;
			rp->htcs1 = ERASE|IENABLE|GO;
			return;
		}
	case SSFOR:
	case SSREV:
		if (rp->htds & TM) {
			if (state == SSREV) {
				h_blkno[unit] = bp->b_blkno - (rp->htfc&0xffff);
				h_nxrec[unit] = h_blkno[unit];
			} else {
				h_blkno[unit] = bp->b_blkno + (rp->htfc&0xffff);
				h_nxrec[unit] = h_blkno[unit] - 1;
			}
		} else
			h_blkno[unit] = bp->b_blkno;
		break;
	default:
		return;
	}
	htstart();
}

htread(dev)
{
	htphys(dev);
	physio(htstrategy, 0, dev, B_READ);
}

htwrite(dev)
{
	htphys(dev);
	physio(htstrategy, 0, dev, B_WRITE);
}

htphys(dev)
{
	register unit;
	daddr_t a;

	unit = dev&03;
	a = u.u_offset >> BSHIFT;
	h_blkno[unit] = a;
	h_nxrec[unit] = a+1;
}

htprint(dev, str)
char *str;
{
	printf("%s on TU16 drive %d\n", str, dev&03);
}
