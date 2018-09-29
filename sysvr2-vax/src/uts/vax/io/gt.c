/*	@(#)gt.c	6.4	*/
/*
 * TM03/TM78 tape driver
 * Handles two tape controllers on one massbus adapter
 * with two slave transports on each controller.
 * minor device classes:
 * bit 0: slave select
 * bit 1: controller select
 * bit 2 off: rewind on close; on: position after first TM
 * If TM78:
 * bit 3 off: 1600 bpi; on: 6250 bpi
 * If TM03:
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
#include "sys/mba.h"

struct	device {
	int	cs1, ds1, er1, mr1;
	int	as, fc, dt, ds2;
	int	sn, mr2, mr3, ndc;
	int	ndt[4];
	int	ia, id;
};
#define	dtc	ds1	/* TM78 */
#define	fm	er1	/* TM78 */
#define	ds78	ds2	/* TM78 */
#define	ds03	ds1	/* TM03 */
#define	tc	mr2	/* TM03 */

#define	gtslave(X)	(X&01)
#define	gtdrive(X)	((X>>1)&01)
#define	gtpos(X)	(X&03)
#define	NOREW	04
#define	DENS	010
#define	GTTM03	050
#define	GTTM78	0100

#define	recno	io_s1

#define	NUNIT	4
struct	iotime	gtstat[NUNIT];
struct	iobuf	gttab = tabinit(HT0,0);
struct	buf	cgtbuf;

#define	INF	1000000

char	gtopenf[NUNIT];
int	gteot[NUNIT];
short	gttype[NUNIT];
short	gtden[NUNIT];
short	gtsndt[NUNIT];
daddr_t	gtblkno[NUNIT];
daddr_t	gtnxrec[NUNIT];

extern struct mba *gt_addr;
#define	mbactl	(gt_addr->ireg.regs)
#define	mbadev	(gt_addr->ereg)

#define	GO	01
#define	NOP	0
#define	REW	06
#define	DCLR	010
#define	SENSE	010
#define	WEOF	026
#define	SFORW	030
#define	SREV	032
#define	SFORW78	020
#define	SREV78	022
#define	SFFILE	024
#define	SRFILE	026
#define	ERASE	024
#define	CLFILE	040
#define	CLFGCR	042
#define	SFLEOT	046
#define	WCOM	060
#define	WGCR	062
#define	RCOM	070
#define	XSNS	072

#define	P800	01700
#define	P1600	02300
#define	TM	04
#define	CS	02000
#define	COR	0100000
#define	PES	040
#define	ERR	040000
#define	FCE	01000
#define	HARD	064027
#define	MOL	010000
#define ONL	0x2000
#define	PE	0x0800
#define	BOT	0x0400
#define	EOT03	02000
#define EOT78	0x0200
#define	WRL	04000
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
#define	SRETRY	4
#define SCOM	5
#define SOK	6


gtopen(dev, flag)
{
	register unit, ds;
	register tm03;

	unit = dev&03;
	if (unit >= NUNIT || gtopenf[unit]) {
		u.u_error = ENXIO;
		return;
	}
	gttab.io_addr = (physadr)&mbadev[0];
	gttab.io_mba = (physadr)&mbactl;
	gttab.io_nreg = NDEVREG;
	gtopenf[unit] = OXCLD;
	gtden[unit] = (dev&DENS ? P1600 : P800) | gtslave(unit);
	gtblkno[unit] = 0;
	gtnxrec[unit] = INF;
	ds = gtcommand(unit, NOP);
	tm03 = (gttype[unit] == GTTM03);
	if (tm03) {
		if ((ds&MOL)==0 || ((flag&FWRITE) && (ds&WRL))) {
			u.u_error = ENXIO;
			gtopenf[unit] = 0;
			return;
		}
	} else {
		if ((ds&ONL)==0 || ((flag&FWRITE) && (ds&FPT))) {
			u.u_error = ENXIO;
			gtopenf[unit] = 0;
			return;
		}
	}
	if (flag&FAPPEND) {
		gtcommand(unit, tm03?SFORW:SFFILE);
		gtcommand(unit, tm03?SREV:SREV78);
		if (!tm03)
			ds = gtcommand(unit, SENSE);
	}
	if ((!tm03) && (!(ds&EOT78)))
		gteot[unit] = 0;
}

gtclose(dev, flag)
{
	register unit;
	register tm03;

	unit = gtpos(dev);
	tm03 = (gttype[unit] == GTTM03);
	if (flag&FWRITE) {
		if (tm03) {
			gtcommand(unit, WEOF);
			gtcommand(unit, WEOF);
		} else
			gtcommand(unit, (dev&DENS) ? CLFGCR : CLFILE);
	}
	if (dev&NOREW) {
		if (flag&FWRITE) {
			if (tm03)
				gtcommand(unit, SREV);
		} else {
			gtcommand(unit, NOP);
			if (gtblkno[unit] <= gtnxrec[unit])
				gtcommand(unit, tm03 ? SFORW : SFLEOT);
		}
	} else
		gtcommand(unit, REW);
	gtopenf[unit] = 0;
}

gtcommand(unit, com)
{
	register struct buf *bp;

	bp = &cgtbuf;
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
	gtstrategy(bp);
	iowait(bp);
	if (bp->b_flags&B_WANTED)
		wakeup(bp);
	bp->b_flags = 0;
	return(bp->b_resid);
}

gtstrategy(bp)
register struct buf *bp;
{
	register daddr_t *p;
	register unit;
	register rec;

	if (bp != &cgtbuf) {
		unit = gtpos(bp->b_dev);
		if (bp->b_flags&B_PHYS)
			rec = bp->b_blkno;
		else
			rec = FsPTOL(bp->b_dev, bp->b_blkno);
		p = &gtnxrec[unit];
		if (rec > *p) {
			bp->b_flags |= B_ERROR;
			bp->b_error = ENXIO;
			iodone(bp);
			return;
		}
		if (rec == *p && bp->b_flags&B_READ) {
			clrbuf(bp);
			bp->b_resid = bp->b_bcount;
			iodone(bp);
			return;
		}
		if ((bp->b_flags&B_READ)==0)
			*p = rec + 1;
		bp->b_start = lbolt;
		gtstat[unit].io_bcnt += btoc(bp->b_bcount);
	}
	bp->av_forw = NULL;
	spl5();
	if (gttab.b_actf == NULL)
		gttab.b_actf = bp;
	else
		gttab.b_actl->av_forw = bp;
	gttab.b_actl = bp;
	if (gttab.b_active==0)
		gtstart();
	spl0();
}

gtstart()
{
	register struct buf *bp;
	register unit, com;
	register struct device *rp;
	daddr_t blkno;
	register tm03;

    loop:
	if ((bp = gttab.b_actf) == NULL)
		return;
	unit = gtpos(bp->b_dev);
	rp = (struct device *)&mbadev[gtdrive(bp->b_dev)];
	if (gttype[unit] == 0) {
		gttype[unit] = rp->dt & 0770;
		if (gttype[unit] == GTTM03)
			tm03 = 1;
		else if (gttype[unit] == GTTM78)
			tm03 = 0;
		else {
			printf("unknown tape type %o\n", rp->dt);
			gttype[unit] = 0;
			bp->b_resid = 0;
			goto next;
		}
	} else
		tm03 = (gttype[unit] == GTTM03);
	if (tm03)
		if ((rp->tc&03777) != gtden[unit])
			rp->tc = gtden[unit];
	blkno = gtblkno[unit];
	if (bp->b_flags&B_PHYS)
		gttab.recno = bp->b_blkno;
	else
		gttab.recno = FsPTOL(bp->b_dev, bp->b_blkno);
	if (bp == &cgtbuf) {
		if (bp->b_resid == NOP) {
			if (tm03) {
				bp->b_resid = rp->ds03 & 0xffff;
				goto next;
			} else
				bp->b_resid = SENSE;
		}
		gtstat[unit].ios.io_misc++;
		gttab.b_active = SCOM;
		if (tm03) {
			rp->fc = 0;
			rp->cs1 = bp->b_resid|GO;
		} else
			rp->ndt[gtslave(unit)] = bp->b_resid|GO;
		return;
	}
	if ((gtopenf[unit]&OFATAL) || gttab.recno > gtnxrec[unit])
		goto abort;
	if (gttab.io_start == 0)
		gttab.io_start = lbolt;
	if (blkno == gttab.recno) {
		if (gteot[unit] > 8)
			goto abort;
		if (bp->b_flags&B_READ)
			com = RCOM|GO;
		else {
			if (gteot[unit] && blkno > gteot[unit]) {
				bp->b_error = ENOSPC;
				goto abort;
			}
			if ((!tm03) && (bp->b_dev&DENS))
				com = WGCR|GO;
			else
				com = WCOM|GO;
		}
		gttab.b_active = SIO;
		if (tm03) {
			rp->fc = -bp->b_bcount;
		} else {
			rp->fm = (1<<2) | gtslave(unit);
			rp->fc = bp->b_bcount;
		}
		gtstat[unit].io_cnt++;
		blkacty |= (1<<HT0);
		mbasetup(bp, gt_addr);
		if (pwr_act >= 0)
			rp->cs1 = com;
	} else {
		gtstat[unit].ios.io_misc++;
		com = blkno - gttab.recno;
		if (blkno < gttab.recno) {
			gttab.b_active = SSFOR;
			if (tm03) {
				rp->fc = com;
				rp->cs1 = SFORW|GO;
			} else {
				com = -com;
				if (com > 254)
					com = 254;
				com = (com<<8)|SFORW78|GO;
			}
		} else {
			gttab.b_active = SSREV;
			if (tm03) {
				rp->fc = -com;
				rp->cs1 = SREV|GO;
			} else {
				if (com > 254)
					com = 254;
				com = (com<<8)|SREV78|GO;
			}
		}
		if (!tm03) {
			gtsndt[unit] = com;
			rp->ndt[gtslave(unit)] = com;
		}
	}
	return;
    abort:
	bp->b_flags |= B_ERROR;
    next:
	gttab.b_actf = bp->av_forw;
	iodone(bp);
	gttab.io_start = 0;
	goto loop;
}

gtintr(dev)
{
	register struct buf *bp;
	register struct device *rp;
	register i;

	if (mbactl.csr&0xffffff00)
		mbasrv(&mbactl);
	rp = (struct device *)&mbadev[0];
	if ((bp = gttab.b_actf)==NULL) {
		for (i=0; i<8; i++) if (rp->as & (1<<i)) {
			rp = (struct device *)&mbadev[i];
			if ((rp->dt&0x1ff)==GTTM78) {
				if ((rp->ndc&0x3f)!=CREW)
					rp->id = TMCLR;
			}
			rp->as = 1<<i;
		}
		mbactl.sr = mbactl.sr;
		return;
	}
	for (i=0; i<8; i++) {
		if (i == gtdrive(bp->b_dev))
			continue;
		if (rp->as & (1<<i)) {
			rp->as = 1<<i;	/* clear stray rewind intrs */
			return;
		}
	}
	blkacty &= ~(1<<HT0);
	if (gttype[gtpos(bp->b_dev)] == GTTM03)
		gt03intr(bp);
	else
		gt78intr(bp);
}

gt03intr(bp)
register struct buf *bp;
{
	register int unit, state;
	register struct device *rp;
	int	err;
	int	mbastat;
	extern gtprint();

	unit = gtpos(bp->b_dev);
	rp = (struct device *)&mbadev[gtdrive(bp->b_dev)];
	state = gttab.b_active;
	gttab.b_active = 0;
	gttab.io_addr = (physadr)rp;
	gttab.io_nreg = NDEVREG-8;
	mbastat = mbactl.sr;
	if (mbastat & MBADTABT) {
		err = rp->er1 & 0xffff;
		if (err == 0)
			err = 0x10000;
		if (err&HARD)
			state = 0;
		if (bp->b_flags&B_PHYS)
			err &= ~FCE;
		if ((bp->b_flags&B_READ) && (rp->ds03&PES))
			err &= ~(CS|COR);
		if ((rp->ds03&MOL)==0)
			gtopenf[unit] |= OFATAL;
		else if (rp->ds03&TM) {
			mbactl.bcr = -bp->b_bcount;
			gtnxrec[unit] = gttab.recno;
			state = SOK;
		}
		else if (state && err == 0)
			state = SOK;
		if (state != SOK) {
			gttab.io_stp = &gtstat[unit].ios;
			fmtberr(&gttab, 0);
		}
		if (gttab.b_errcnt > 4)
			prcom(gtprint, bp, rp->er1&0xffff, mbastat);
		rp->as = 1<<gtdrive(bp->b_dev);
		mbactl.sr = mbactl.sr;
		rp->cs1 = DCLR|GO;
		if (state==SIO && ++gttab.b_errcnt < 10) {
			gttab.b_active = SRETRY;
			gtblkno[unit]++;
			rp->fc = -1;
			gtstat[unit].ios.io_misc++;
			rp->cs1 = SREV|GO;
			return;
		}
		if (state!=SOK) {
			bp->b_flags |= B_ERROR;
			state = SIO;
		}
	} else if (rp->ds03 & ERR) {
		if ((rp->ds03 & TM) == 0) {
			gttab.io_stp = &gtstat[unit].ios;
			fmtberr(&gttab, 0);
		}
		rp->cs1 = DCLR|GO;
	}
	rp->as = 1<<(gtdrive(bp->b_dev));
	mbactl.sr = mbactl.sr;
	switch(state) {
	case SIO:
	case SOK:
		gtblkno[unit]++;
	case SCOM:
		if (rp->ds03&EOT03)
			gteot[unit]++;
		else
			gteot[unit] = 0;
		if (gttab.io_erec)
			logberr(&gttab,bp->b_flags&B_ERROR);
		gttab.b_errcnt = 0;
		gttab.b_actf = bp->av_forw;
		bp->b_resid = (-mbactl.bcr) & 0xffff;
		iodone(bp);
		if (bp != &cgtbuf) {
			gtstat[unit].io_resp += lbolt - bp->b_start;
			gtstat[unit].io_act += lbolt - gttab.io_start;
			gttab.io_start = 0;
		}
		break;
	case SRETRY:
		if ((bp->b_flags&B_READ)==0) {
			gtstat[unit].ios.io_misc++;
			gttab.b_active = SSFOR;
			rp->cs1 = ERASE|GO;
			return;
		}
	case SSFOR:
	case SSREV:
		if (rp->ds03 & TM) {
			if (state == SSREV) {
				gtblkno[unit] = gttab.recno - (rp->fc&0xffff);
				gtnxrec[unit] = gtblkno[unit];
			} else {
				gtblkno[unit] = gttab.recno + (rp->fc&0xffff);
				gtnxrec[unit] = gtblkno[unit] - 1;
			}
		} else
			gtblkno[unit] = gttab.recno;
		break;
	default:
		return;
	}
	gtstart();
}

gt78intr(bp)
register struct buf *bp;
{
	register unit, state;
	register struct device *rp;
	register slave;
	int	ci, cf;
	extern	gtprint();

	unit = gtpos(bp->b_dev);
	slave = gtslave(bp->b_dev);
	rp = (struct device *)&mbadev[gtdrive(bp->b_dev)];
	if ((mbactl.sr&MBATTN) && (slave != ((rp->ndc>>8)&03))) {
		rp->as = 1<<(gtdrive(bp->b_dev));
		return;
	}
	state = gttab.b_active;
	gttab.b_active = 0;
	gttab.io_addr = (physadr)rp;
	gttab.io_nreg = NDEVREG-2;
	if (state == SIO) {
		switch(rp->dtc&0x3f) {
		case CTM:
			mbactl.bcr = -bp->b_bcount;
			gtnxrec[unit] = gttab.recno;
			goto done;
		case CDONE:
			if (mbactl.sr & MBADTABT)
				goto retry;
		done:
			gtblkno[unit]++;
			if (gttab.io_erec)
				logberr(&gttab,bp->b_flags&B_ERROR);
			gttab.b_errcnt = 0;
			gttab.b_actf = bp->av_forw;
			bp->b_resid = (-mbactl.bcr) & 0xffff;
			iodone(bp);
			gtstat[unit].io_resp += lbolt - bp->b_start;
			gtstat[unit].io_act += lbolt - gttab.io_start;
			gttab.io_start = 0;
			break;
		case CEOT:
			gteot[unit]++;
			goto done;
		case CRETRY:
		case CFPT:
		retry:
			gtstat[unit].ios.io_misc++;
			gttab.io_stp = &gtstat[unit].ios;
			fmtberr(&gttab, 0);
			if (++gttab.b_errcnt > 10) {
				bp->b_flags |= B_ERROR;
				goto done;
			}
			if (gttab.b_errcnt > 4)
				prcom(gtprint, bp, rp->dtc&0xffff, mbactl.sr);
			break;
		case CSHREC:
		case CLOREC:
			if (!(bp->b_flags&B_PHYS))
				bp->b_flags |= B_ERROR;
			goto done;
		case CTMFLTB:
		case CMBFLT:
		case CKEYFLT:
			rp->id = TMCLR;
		default:
			gtopenf[unit] |= OFATAL;
			prcom(gtprint, bp, rp->dtc&0xffff, mbactl.sr&0xffff);
			gttab.io_stp = &gtstat[unit].ios;
			fmtberr(&gttab, 0);
			bp->b_flags |= B_ERROR;
			goto done;
		}
	} else {
		ci = (gtsndt[unit]>>8)&0377;
		cf = (rp->ndt[slave]>>8)&0377;
		switch(rp->ndc&0x3f) {
		case CEOT:
			gteot[unit]++;
			goto ndone;
		case CTMFLTB:
		case CMBFLT:
		case CKEYFLT:
			rp->id = TMCLR;
			if (state == SCOM)
				goto ndone;
		default:
			gtopenf[unit] |= OFATAL;
			gttab.io_stp = &gtstat[unit].ios;
			fmtberr(&gttab, 0);
		case CDONE:
		case CFPT:
		case CLEOT:
		case CREW:
		ndone:
			switch(state) {
			case SCOM:
				if (gttab.io_erec)
					logberr(&gttab, bp->b_flags&B_ERROR);
				gttab.b_errcnt = 0;
				gttab.b_actf = bp->av_forw;
				bp->b_resid = rp->ds78;
				iodone(bp);
				break;
			case SSFOR:
				gtblkno[unit] += ci;
				break;
			case SSREV:
				gtblkno[unit] -= ci;
				break;
			}
			break;
		case CTM:
		case CNOC:
			switch(state) {
			case SSFOR:
				gtblkno[unit] += (ci-cf) + 1;
				gtnxrec[unit] = gtblkno[unit] - 1;
				break;
			case SSREV:
				gtblkno[unit] -= (ci-cf) + 1;
				gtnxrec[unit] = gtblkno[unit];
				break;
			}
			break;
		}
	}
	rp->as = 1<<(gtdrive(bp->b_dev));
	mbactl.sr = mbactl.sr;
	gtstart();
}

gtread(dev)
{
	gtphys(dev);
	physio(gtstrategy, 0, dev, B_READ);
}

gtwrite(dev)
{
	gtphys(dev);
	physio(gtstrategy, 0, dev, B_WRITE);
}

gtphys(dev)
{
	register unit;
	daddr_t a;

	unit = dev&03;
	a = u.u_offset >> BSHIFT;
	gtblkno[unit] = a;
	gtnxrec[unit] = a+1;
}

gtprint(dev, str)
char *str;
{
	register pos;

	pos = gtpos(dev);
	if (gttype[pos] == GTTM03)
		printf("%s on TU16 drive %d\n", str, gtslave(dev));
	else
		printf("%s on TU78 drive %d\n", str, gtslave(dev));
}
