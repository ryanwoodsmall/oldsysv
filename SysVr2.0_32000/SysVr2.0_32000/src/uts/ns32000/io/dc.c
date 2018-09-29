/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
/*
 * SYS16 Disk driver
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/errno.h"
#include "sys/buf.h"
#include "sys/elog.h"
#include "sys/iobuf.h"
#include "sys/systm.h"
#include "sys/dcu.h"
#include "sys/disk.h"
#include "sys/page.h"
#include "sys/mmu.h"

struct device
{
	unsigned char chan00;
	char fill0;
	unsigned char chan01;
	char fill1;
	unsigned char chan02;
	char fill2;
	unsigned char chan03;
	char fill3;
	unsigned char status;
	char fill4[7];
	unsigned char errack;
};

extern struct device *dc_addr[];

#define MAXDCU	7	/* Maximum number of disk units */
#define NRETRY	8

struct disktab dc_tbl[MAXDCU];
short dcinit;

struct	iotime	dcstat;
struct	iobuf	dctab = tabinit(DC0,&dcstat.ios);

struct dcu_iocb iocb;
int dskbuf[256];

#define coff	av_back
#define cylin	b_resid

struct spiral {
	short	piece;	/* piece to do or piece actually done */
	short	ndone;	/* total blocks actually done */
	short	cylrem;/* blocks left in cyl */
} dcspiral;

dcopen(dev)
{
	register int i;
	register struct iobuf *dp;

	if (dcinit == 0) {
		i = dc_addr[0]->chan01;	/* enable writes */
		for (i=0; i < MAXDCU; i++)
			if (initunit(i) == 0)
				dcinit |= 1 << i;
	}

	dev >>= 3;
	if (dev >= MAXDCU || (dcinit & (1<<dev)) == 0) {
		u.u_error = ENXIO;
		return;
	}


	if ((dctab.b_flags&B_ONCE)==0) {
		dp = &dctab;
		dp->b_flags |= B_ONCE;
		dp->io_addr = (physadr)&iocb.dc_sense;
		dp->io_nreg = sizeof(iocb.dc_sense)/sizeof(short);
		icuenable(0x8);
	}
}

dcclose(dev)
{
}

dcstrategy(bp)
register struct buf *bp;
{
	register struct iobuf *dp;
	register struct buf *ap;
	register struct disktab *tp;
	daddr_t	last;
	register unit;
	int	co;

	unit = minor(bp->b_dev);
	tp = &dc_tbl[unit>>3];
	last = tp->dt_part[unit&07].nbl;
	co = tp->dt_part[unit&07].cyloff;
	unit >>= 3;
	dp = &dctab;
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
	bp->cylin = bp->b_blkno/tp->dt_spcyl + co;
	co = bp->b_blkno%tp->dt_spcyl;
	*((int *)(&bp->coff)) = co;
	dcstat.io_cnt++;
	dcstat.io_bcnt += btod(bp->b_bcount);

	spl6();
	if (dp->b_actf == NULL) {
		dp->b_actf = bp;
		dp->b_actl = bp;
		dcspiral.ndone = 0;
	} else {
		dp->b_actl->av_forw = bp;
		dp->b_actl = bp;
	}
	if (dctab.b_active == 0)
		dcstart();
	spl0();
}

dcstart()
{
	register struct buf *bp;
	register struct iobuf *dp;
	register struct dcu_iocb *ip;
	register struct disktab *tp;
	int nbl, i, *pt, unit;

	dp = &dctab;
	ip = &iocb;
	if ((bp = dp->b_actf) == NULL) {
		return;
	}
	dctab.b_active++;
	blkacty |= (1<<DC0);
	dp->io_start = lbolt;
	unit = minor(bp->b_dev) >> 3;
	tp = &dc_tbl[unit];

	if (dcspiral.ndone == 0 && dp->b_errcnt == 0) { /* first time */
		nbl = btod(bp->b_bcount);
		if (nbl > MAXBLK || nbl < 0)
			panic("bad bcount");
		if (bp->b_flags & B_PHYS) {
			physmap(bp, nbl, dskbuf);
		} else {
			pt = (int *)&kpte2[svtoc(paddr(bp))];
			for (i=0; i<nbl; i++)
				dskbuf[i] = (*pt++) & PFNMASK;
		}
		i = (int)bp->coff;
		dcspiral.cylrem = tp->dt_spcyl - i;
		if (nbl > dcspiral.cylrem)
			nbl = dcspiral.cylrem;
	} else {
		nbl = btod(bp->b_bcount) - dcspiral.ndone;
		if (dcspiral.cylrem == 0) {
			dcspiral.cylrem = tp->dt_spcyl;
			bp->cylin++;
			bp->coff = 0;
		} else
			*((int *)(&bp->coff)) += dcspiral.piece;
		if (nbl > dcspiral.cylrem)
			nbl = dcspiral.cylrem;
	}
	if (bp->b_flags & B_READ)
		ip->dc_opcode = DC_READ;
	else
		ip->dc_opcode = DC_WRITE;
	ip->dc_count = nbl;
	dcspiral.piece = nbl;
	ip->dc_cylinder = bp->cylin;
	i = (int)bp->coff;
	ip->dc_drhdsec = (unit<<13) | ((i/tp->dt_nsec)<<8) |
			 (i%tp->dt_nsec);
	ip->dc_buffer = (int)&dskbuf[dcspiral.ndone];
	ip->dc_chain = 0;
	ip->dc_sense = 0;
	ip->dc_ecc = 0;
	ip->dc_status = 0;

	dc_cmd(DC_START, &ip->dc_opcode);
}

dcintr()
{
	register struct buf *bp;
	register struct device *rp;
	register struct iobuf *dp;
	unsigned char status;
	int done = 0;

	if (dctab.b_active) {	/* data transfer underway */
		blkacty &= ~(1<<0);
		dp = &dctab;
		bp = dp->b_actf;
		rp = dc_addr[0];
		if ((rp->status & DS_READY) == 0)
			panic("dc intr, not ready");
		if (rp->status & DS_ERINT)
			rp->errack = 0;
		status = STATUS0(rp->status);
		if (status != DS_IDLE)
			rp->status = 0;
		if (status != DS_DONE) {
			if (++dp->b_errcnt < NRETRY) {
				int unit;

				unit = minor(bp->b_dev);
				fmtberr(dp, dc_tbl[unit>>3].dt_part[unit&07].cyloff);
				dcspiral.piece -= iocb.dc_resid;
				if (dcspiral.piece > 0)
					dcspiral.piece--; /* dont ask! */
				if ((iocb.dc_sense & BAD_ERR) == 0 &&
				    (iocb.dc_sense & CTL_FAULT) == ECC_ERR &&
				    (bp->b_flags & B_READ) &&
				    ecc_fix(iocb.dc_ecc,
				    dskbuf[dcspiral.ndone+dcspiral.piece])
					== 0) {
					dcspiral.piece++;
					goto ok;
				}
				dcspiral.ndone += dcspiral.piece;
				dcspiral.cylrem -= dcspiral.piece;
				dcstart();
				return;
			}
			printf("dc hard error block %d\n", bp->b_blkno);
			logberr(dp, 1);
			bp->b_flags |= B_ERROR;
			bp->b_resid = bp->b_bcount - dtob(dcspiral.ndone);
			done++;
		} else {
	ok:
			if (dp->io_erec)
				logberr(dp, 0);
			dcspiral.ndone += dcspiral.piece;
			dcspiral.cylrem -= dcspiral.piece;
			if (dcspiral.ndone >= btod(bp->b_bcount)) {
				done++;
				bp->b_resid = 0;
			}
		}
		dp->b_active = 0;
		dp->b_errcnt = 0;
		dcstat.io_act += lbolt - dp->io_start;
		if (done) {
			dcstat.io_resp += lbolt - bp->b_start;
			dp->b_actf = bp->av_forw;
			iodone(bp);
			dcspiral.ndone = 0;
		}
	} else {
		rp = dc_addr[0];
		if ((rp->status & DS_READY) == 0)
			return;
		if (rp->status & DS_ERINT)
			rp->errack = 0;
		status = STATUS0(rp->status);
		if (status != DS_IDLE)
			rp->status = 0;
	}
	if (dctab.b_actf != NULL)
		dcstart();
}

dcprint(dev, str)
char *str;
{
	printf("%s on DCU drive %d, slice %d\n", str, (dev>>3)&7, dev&7);
}

dc_cmd(cmd, args)
unsigned char cmd;
{
	register struct device *dp;
	register char *p;

	dp = dc_addr[0];
	while ((dp->status & DS_READY) == 0)
		printf("dcu not ready\n");
	p = (char *)&args;
	dp->chan00 = *p++;
	dp->chan01 = *p++;
	dp->chan02 = *p;
	dp->chan03 = cmd;
}

dc_poll(cmd, args)
unsigned char cmd;
{
	register struct device *dp;
	register unsigned char status;
	register char *p;

	dp = dc_addr[0];
	while ((dp->status & DS_READY) == 0)
		;
	p = (char *)&args;
	dp->chan00 = *p++;
	dp->chan01 = *p++;
	dp->chan02 = *p;
	dp->chan03 = cmd;
wait:
	status = dp->status;
	if ((status & DS_READY) == 0)
		goto wait;
	if (status & DS_SELFTEST)
		goto wait;
	status = STATUS0(status);
	if (status == DS_BUSY)
		goto wait;
	if (status != DS_IDLE)
		dp->status = 0;
	if (status == DS_DONE) {
		return (0);
	} else {
		return (-1);
	}
}

initunit(unit)
register int unit;
{
	register int rc;

	if (unit < 0 || unit >= 8)
		return (-1);
	if ((rc=dc_poll(DC_SETBURST, 7)) != 0)
		return(rc);
	if ((rc=dc_poll(DC_ASSDRIVE, DD_DRIVE(unit))) != 0)
		return(rc);
	if ((rc=dc_poll(DC_SPECIFIC, DD_ENABLE(1))) != 0)
		return(rc);
	if ((rc=get_dtab(unit)) != 0)
		return(rc);
	return (0);
}

get_dtab(unit)
{
	register struct dcu_iocb *ip;
	static char *bp;

	bp = (char *)((int)&dskbuf[128] & ~0x1ff);
	ip = &iocb;
	ip->dc_opcode = DC_READ;
	ip->dc_count = 1;
	ip->dc_cylinder = 0;
	ip->dc_drhdsec = DD_DHS(unit,0,0);
	ip->dc_buffer = (int)&bp;
	ip->dc_chain = 0;
	ip->dc_sense = 0;
	ip->dc_ecc = 0;
	ip->dc_status = 0;
	if (dc_poll(DC_START, &ip->dc_opcode) != 0) {
		printf("Bad header on unit %d\n", unit);
		return (-1);
	}
	if (*(int *)bp != DMAGIC) {
		printf("Bad block 0 magic\n");
		return (-1);
	}
	bcopy(bp, &dc_tbl[unit], sizeof(struct disktab));
	return (0);
}

dcread(dev)
{
	if (aln_chk()) {
		fak_phys(dcstrategy, dev, B_READ, 512);
	} else if (physck(dc_tbl[dev>>3].dt_part[dev&07].nbl, B_READ))
		physio(dcstrategy, 0, dev, B_READ);
}

dcwrite(dev)
{
	if (aln_chk()) {
		fak_phys(dcstrategy, dev, B_WRITE, 512);
	} else if (physck(dc_tbl[dev>>3].dt_part[dev&07].nbl, B_WRITE))
		physio(dcstrategy, 0, dev, B_WRITE);
}

/* ECC routine for the disk.  The following routine takes the four ECC bytes
 * from the sense bytes, and uses them to generate a mask and offset, and
 * performs an XOR to correct the data in the buffer. The algorithm is
 * from the article "Error Correction Method for Disk" (September 26, 1980),
 * Volume 1, page 12", by Data Systems Technology Corp, Broomfield, Col.
 */

#define RECIP	0xa8a06080		/* reciprocal polynomial bits */
#define	R1	0xff000000		/* syndrome bits x0 - x7 */
#define	R2	0x00ff0000		/* syndrome bits x8 - x15 */
#define	R3	0x0000ff00		/* syndrome bits x16 - 23 */
#define	R4	0x000000ff		/* syndrome bits x23 - x31 */
#define	R3LOW5	0x00001f00		/* low order 5 bits of R3 */
#define R4LOW1	0x00000001		/* low order bit of R4 */


ecc_fix(ecc, data)
register unsigned char *data;	/* pointer to data buffer */
register unsigned long ecc;	/* ecc data for correction */
{
	register unsigned char *addr;	/* address of byte to ECC correct */
	register j = 517 * 8 - 25;	/* bit counter for loop */
					/* 517 = 1 sync + 4 ecc + 512 data */
	register af = 0;		/* alignment flag */

	if (ecc == 0) return(1);	/* zero ECC should not occur */

	ecc = ((ecc&R1)>>24) | ((ecc&R2)>>8) | ((ecc&R3)<<8) | ((ecc&R4)<<24);
					/* reverse order of the bytes */

	while ((ecc & R1) == 0) {	/* normalize so R1 is nonzero */
		j += 8;
		ecc <<= 8;
	}

	do {				/* loop over all bit positions */
		if (ecc & R4LOW1) {		/* shift R1-R4 right 1 bit */
			ecc >>= 1;
			ecc ^= RECIP;		/* XOR with reciprocal poly */
		} else ecc >>= 1;		/* bypass XOR if R4low is 0 */
		if (ecc & R1) continue;		/* test R1 */
		if (af == 0) {			/* test align flag */
			if (ecc & (R4|R3LOW5))	/* test R4, 5 low order of R3 */
				continue;
			af = 1;			/* set align flag = 1 */
		}
		if ((j & 7) == 0) goto goodecc;	/* test j modulo 8 */
	} while (j-- > 0);			/* test j, set j = j - 1 */
	return(1);				/* failed */

/* Here if the data is correctable.  J contains the bit offset in the data
 * buffer of where to apply the ECC fix, and the mask to XOR with is in
 * R2, R3, and R4.  (Also, j is always a multiple of 8, and R2 is the lowest
 * address byte).  The data record referenced by the ECC data consists of
 * one sync byte, 512 data bytes, and 4 ECC bytes, so therefore the data
 * in memory begins at byte offset 1 and ends with byte offset 512.  The
 * XORs to be performed must therefore be range checked before use.
 */

goodecc:
	j >>= 3;				/* get byte offset in sector */
	addr = data + j - 1;			/* point to memory address */
	if ((j > 0) && (addr < data + NBPC))
		piput(addr, piget(addr) ^ ((ecc & R2) >> 16));
	addr++;
	if (addr < data + NBPC)
		piput(addr, piget(addr) ^ ((ecc & R3) >> 8));
	addr++;
	if ((j > 0) && (addr < data + NBPC))
		piput(addr, piget(addr) ^ (ecc & R4));
	return(0);
}
