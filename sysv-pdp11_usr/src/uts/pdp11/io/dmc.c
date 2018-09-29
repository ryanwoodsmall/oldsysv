/* @(#)dmc.c	1.3 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/buf.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/tty.h"
#include "sys/uba.h"
#include "sys/dmcio.h"

struct device *dmc_addr[];
struct device {
union {
	char	b[8];
	short	w[4];
} un;
};

#define	bsel0	un.b[0]
#define	bsel1	un.b[1]
#define	bsel2	un.b[2]
#define	bsel3	un.b[3]
#define	bsel4	un.b[4]
#define	bsel5	un.b[5]
#define	bsel6	un.b[6]
#define	bsel7	un.b[7]
#define	sel0	un.w[0]
#define	sel2	un.w[1]
#define	sel4	un.w[2]
#define	sel6	un.w[3]

int	dmc_cnt;

#define RWMAX	7
#define	MAXBUF	8192

/* Defines for bsel0 and bsel2. */
#define	BACCI	0
#define	CNTLI	1
#define	PERR	2
#define	BASEI	3
#define	INX	0		/* transmit block */
#define	INR	4		/* read  block */
#define	RQI	040		/* port request bit */
#define	IEI	0100		/* enable input interrupts */
#define	RDYI	0200		/* port ready */

#define	BACCO	0
#define	CNTLO	1
#define	OUX	0		/* transmit block */
#define	OUR	4		/* read  block */
#define	IEO	0100		/* enable output interrupts */
#define	RDYO	0200		/* port available */

#define	MCLR	0100		/* DMC11 Master Clear */

/* Defines for CNTLI mode (set into sel6). */
#define	HDPLX	02000		/* half duplex DDCMP operation */
#define	SEC	04000		/* half duplex secondary station */
#define	MAINT	0400		/* enter maintenance mode */

/* Defines for BACCI/O and BASEI mode (set into sel6). */
#define	DMXMEM	0140000		/* xmem bit position */
#define	CCOUNT	037777		/* character count mask */
#define	RESUME	02000		/* resume (BASEI only) */

/* Defines for CNTLO (set in sel6). */
#define FATAL	01630

struct dmc {
	int	b_flags;
	struct buf *b_forw;
	struct buf *b_back;
	struct buf *av_forw;
	struct clist dm_que;
	char	dm_rwq[2];
	short	dm_ioc[16];
	struct buf dm_rbuf[2];
	int	dm_map[2];
	short	dm_base[128];
};

#define	DMCOPEN	2
#define DMCMAI	010		/*  DMC Maintenance mode flag  */

#define NDMC 2
struct	dmc dmc11[NDMC];

dmcopen(dev)
{
	register struct dmc *dmp;

	if (dev >= dmc_cnt || dev >= NDMC) {
		u.u_error = ENXIO;
		return;
	}
	dmp = &dmc11[dev];
	if((dmp->b_flags&DMCOPEN) == 0) {
		dmp->b_flags = DMCOPEN;
		dmcinit(dev);
		dmp->dm_map[0] = ubmalloc(MAXBUF);
		dmp->dm_map[1] = ubmalloc(MAXBUF);
	}
}

dmcclose(dev)
{
	register struct dmc *dmp;

	dmp = &dmc11[dev];
	dmcclr(dev);
	dmp->b_flags &= ~DMCMAI;
	ubmfree(dmp->dm_map[0], MAXBUF);
	ubmfree(dmp->dm_map[1], MAXBUF);
	dmp->b_flags &= ~DMCOPEN;
}

dmcinit(dev)
{
	register struct dmc *dmp;
	paddr_t	base;

	dmp = &dmc11[dev];
	dmcclr(dev);
	dmc_addr[dev]->bsel2 |= IEO;
	base = ubmdata(dmp->dm_base);
	dmcload(dmp, BASEI, loword(base), hiword(base)<<14);
	if (dmp->b_flags & DMCMAI) 
		dmcload(dmp, CNTLI, 0, HDPLX|MAINT);
	else
		dmcload(dmp, CNTLI, 0, 0);
}

dmcclr(dev)
{
	int sps;
	register struct dmc *dmp;
	register struct buf *bp;

	sps = spl5();
	dmc_addr[dev]->bsel1 = MCLR;
	splx(sps);

	dmp = &dmc11[dev];
	dmp->dm_rwq[0] = dmp->dm_rwq[1] = 0;

	/* Mark all buffers bad. */
	while(bp = dmp->av_forw) {
		bp->b_flags |= B_ERROR;
		dmp->av_forw = bp->av_forw;
		iodone(bp);
	}
}

dmcstrategy(bp)
register struct buf *bp;
{
	register struct dmc *dmp;
	register struct buf *abp;
	int sps;

	if (bp->b_bcount > MAXBUF) {
		bp->b_flags |= B_ERROR;
		iodone(bp);
		return;
	}
	dmp = &dmc11[minor(bp->b_dev)];
	abp = (struct buf *)dmp;
	bp->av_forw = NULL;
	sps = spl5();
	while (abp->av_forw)
		abp = abp->av_forw;
	abp->av_forw = bp;
	dmcstart(dmp, bp->b_flags&B_READ);
	splx(sps);
}

dmcstart(dmp, flag)
register struct dmc *dmp;
{
	register struct buf *bp;
	paddr_t addr;

	bp = (struct buf *)dmp;
	while (bp = bp->av_forw) {
		if ((bp->b_flags&(B_STALE|B_READ))!=flag)
			continue;
		if (dmp->dm_rwq[flag] >= RWMAX) {
			return;
		}
		dmp->dm_rwq[flag]++;
		bp->b_flags |= B_STALE;
		addr = ubmaddr(bp, dmp->dm_map[flag]);
		dmcload(dmp, flag<<2, loword(addr), 
			(bp->b_bcount&CCOUNT)|((hiword(addr)&03)<<14));
		return;
	}
}

dmcload(dmp, type, w0, w1)
register struct dmc *dmp;
{
	register struct device *kp;
	register dev;
	int	sps;

	dev = dmp-dmc11;
	kp = dmc_addr[dev];
	sps = spl5();
	kp->bsel0 |= RQI;
	putc(lobyte(w0), &dmp->dm_que);
	putc(hibyte(w0), &dmp->dm_que);
	putc(lobyte(w1), &dmp->dm_que);
	putc(hibyte(w1), &dmp->dm_que);
	putc(type, &dmp->dm_que);
	dmcrint(dev);
	splx(sps);
}

dmcrint(dev)
{
	register struct dmc *dmp;
	register struct device *kp;

	kp = dmc_addr[dev];
	kp->bsel0 &= ~IEI;
	dmp = &dmc11[dev];
	while (kp->bsel0&RDYI) {
		kp->bsel4 = getc(&dmp->dm_que);
		kp->bsel5 = getc(&dmp->dm_que);
		kp->bsel6 = getc(&dmp->dm_que);
		kp->bsel7 = getc(&dmp->dm_que);
		kp->bsel0 = getc(&dmp->dm_que)|RDYI;
		while (kp->bsel0&RDYI);
		if (dmp->dm_que.c_cc==0)
			return;
		kp->bsel0 = RQI;
	}
	if (dmp->dm_que.c_cc)
		kp->bsel0 |= IEI;
}

dmcxint(dev)
{
	register struct device *dp;
	register struct dmc *dmp;
	register struct buf *bp, *abp;
	short	*ip;
	int	count, flag;

	dp = dmc_addr[dev];
	count = dp->sel6;
	flag = dp->bsel2&7;
	dp->bsel2 &= ~RDYO;

	dmp = &dmc11[dev];
	switch(flag) {

	case OUX:
	case OUR:
		bp = (struct buf *)dmp;
		flag >>= 2;

		/*
		 * Look through the active Q for the buffer
		 * the DMC11 says is finished. Assume the DMC11
		 * returns the buffers in FIFO order.
		 */
		while(abp = bp,bp = bp->av_forw) {
			if ((bp->b_flags&B_READ)!=flag)
				continue;
			dmp->dm_rwq[flag]--;
			dmp->dm_ioc[flag]++;
			bp->b_resid = bp->b_bcount - count&CCOUNT;
			abp->av_forw = bp->av_forw;
			iodone(bp);
			dmcstart(dmp, flag);
			return;
		}
		printf("DMC%d lost block\n", dev);
		return;

	case CNTLO:
		if (count&FATAL)
			dmcinit(dev);
		ip = &dmp->dm_ioc[2];
		while(count) {
			if (count&01)
				(*ip)++;
			ip++;
			count >>= 1;
		}
	}
}

dmcread(dev)
{
	physio(dmcstrategy,&dmc11[dev].dm_rbuf[B_READ],dev,B_READ);
}

dmcwrite(dev)
{
	physio(dmcstrategy,&dmc11[dev].dm_rbuf[B_WRITE],dev,B_WRITE);
}
dmcioctl(dev, cmd, arg, mode)
{
	register struct dmc *mp;

	mp = &dmc11[dev];
	switch (cmd) {
	case IOCTYPE:
		u.u_rval1 = DIOC;
		break;

	case DIOCGETC:
		if (copyout(mp->dm_ioc, arg, sizeof(mp->dm_ioc)))
			u.u_error = EFAULT;
		break;
	case DIOCGETB:
		if (copyout(mp->dm_base, arg, sizeof(mp->dm_base)))
			u.u_error = EFAULT;
		break;
	case DIOCSETE:
		dmcload(mp, PERR, 0, 0);
		break;
	case DIOCMAIN:
		mp->b_flags |= DMCMAI;
		dmcload(mp, PERR, 0, 0);
		break;
	case DIOCNORM:
		mp->b_flags &= ~DMCMAI;
		dmcload(mp, PERR, 0, 0);
		break;
	}
}
