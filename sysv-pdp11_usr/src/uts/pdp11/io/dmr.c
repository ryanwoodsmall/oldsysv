/* @(#)dmr.c	1.4 */
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

struct device *dmr_addr[];
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
int	dmr_cnt;

#define RWMAX	7
#define	MAXBUF	8192
#define RDYSCAN	16	/* loop delay for RDYI after RQI */

/* Defines for bsel0 and bsel2 */
#define	BACCI	0
#define	CNTLI	1
#define	PERR	2
#define	BASEI	3
#define	INX	0		/* transmit block */
#define	INR	4		/* read block */
#define	RQI	040		/* port request bit */
#define	IEI	0100		/* enable input interrupts */
#define	RDYI	0200		/* port ready */

#define	BACCO	0
#define	CNTLO	1
#define	OUX	0		/* transmit block */
#define	OUR	4		/* read block */
#define	IEO	0100		/* enable output interrupts */
#define	RDYO	0200		/* port available */

#define	MCLR	0100		/* DMC11 Master Clear */

/* Defines for CNTLI mode */
#define	HDPLX	02000		/* half duplex DDCMP operation */
#define	SEC	04000		/* half duplex secondary station */
#define	MAINT	0400		/* enter maintenance mode */

/* Defines for BACCI/O and BASEI mode */
#define	DMXMEM	0140000		/* xmem bit position */
#define	CCOUNT	037777		/* character count mask */
#define	RESUME	02000		/* resume (BASEI only) */

/* Defines for CNTLO */
#define	CNTMASK	01777
#define FATAL	01630

struct dmr {
	short	d_flag;
	struct buf *d_next;
	struct buf *d_rdy[2];
	struct clist d_que;
	char	d_rwq[2];
	short	d_ioc[12];
	struct buf d_rbuf[2];
	int	d_map[2];
	short	d_base[128];
};

#define	DMCACT	1
#define	DMCRUN	2
#define	DMCBUF	4
#define	DMCMAI	010			/*  DMC Maintenance mode flag  */

#define NDMC 2
struct	dmr dmr_dmr[NDMC];

dmropen(dev)
{
	register struct dmr *mp;
	paddr_t	base;

	if (dev >= dmr_cnt || dev >= NDMC) {
		u.u_error = ENXIO;
		return;
	}
	mp = &dmr_dmr[dev];
	mp->d_flag |= DMCACT;
	if ((mp->d_flag&DMCRUN) == 0) {
		mp->d_flag |= DMCRUN;
		dmr_addr[dev]->bsel2 |= IEO;
		base = ubmdata(mp->d_base);
		dmrload(mp, BASEI, loword(base), hiword(base)<<14);
		if (mp->d_flag & DMCMAI)
			dmrload(mp, CNTLI, 0, HDPLX|MAINT);
		else
			dmrload(mp, CNTLI, 0, 0);
	}
	if ((mp->d_flag&DMCBUF) == 0) {
		mp->d_flag |= DMCBUF;
		mp->d_map[0] = ubmalloc(MAXBUF);
		mp->d_map[1] = ubmalloc(MAXBUF);
	}
}

dmrclose(dev)
{
	register struct dmr *mp;
	register struct buf *bp;

	mp = &dmr_dmr[dev];
	mp->d_flag &= ~DMCACT;
	if (mp->d_flag&DMCRUN) {
		mp->d_flag &= ~DMCMAI;
		dmrload(mp, PERR, 0, 0);
	} else if (mp->d_flag&DMCBUF) {
		mp->d_flag &= ~DMCBUF;
		ubmfree(mp->d_map[0], MAXBUF);
		ubmfree(mp->d_map[1], MAXBUF);
	}
}

dmrstrategy(bp)
register struct buf *bp;
{
	register struct dmr *mp;
	register struct buf *abp;
	int sps;

	if (bp->b_bcount > MAXBUF) {
		bp->b_flags |= B_ERROR;
		iodone(bp);
		return;
	}
	mp = &dmr_dmr[minor(bp->b_dev)];
	bp->b_flags &= ~B_STALE;
	bp->av_forw = NULL;
	sps = spl5();
	if ((abp = mp->d_next) == NULL)
		mp->d_next = bp;
	else {
		while (abp->av_forw)
			abp = abp->av_forw;
		abp->av_forw = bp;
	}
	dmrstart(mp, bp->b_flags&B_READ);
	splx(sps);
}

dmrstart(mp, flag)
register struct dmr *mp;
{
	register struct buf *bp;
	register count;
	paddr_t addr;

	if ((bp = mp->d_next) == NULL)
		return;
	do {
		if ((bp->b_flags&(B_STALE|B_READ))!=flag)
			continue;
		if (mp->d_rwq[flag] >= RWMAX)
			return;
		mp->d_rwq[flag]++;
		bp->b_flags |= B_STALE;
		count = bp->b_bcount;
		addr = ubmaddr(bp, mp->d_map[flag]);
		dmrload(mp, flag<<2, loword(addr),
			(count&CCOUNT)|((hiword(addr)&03)<<14));
	} while (bp = bp->av_forw);
}

dmrload(mp, type, w0, w1)
register struct dmr *mp;
{
	register struct device *kp;
	register dev;
	register sps, n;

	dev = mp - dmr_dmr;
	kp = dmr_addr[dev];
	sps = spl5();
	if ((n = mp->d_que.c_cc) == 0)
		kp->bsel0 = type | RQI;
	else
		putc(type | RQI, &mp->d_que);
	putc(lobyte(w0), &mp->d_que);
	putc(hibyte(w0), &mp->d_que);
	putc(lobyte(w1), &mp->d_que);
	putc(hibyte(w1), &mp->d_que);
	if (n == 0)
		dmrrint(dev);
	splx(sps);
}

dmrrint(dev)
{
	register struct dmr *mp;
	register struct device *kp;
	register int n;

	kp = dmr_addr[dev];
	mp = &dmr_dmr[dev];
	while (kp->bsel0&RDYI) {
		kp->bsel4 = getc(&mp->d_que);
		kp->bsel5 = getc(&mp->d_que);
		kp->bsel6 = getc(&mp->d_que);
		kp->bsel7 = getc(&mp->d_que);
		kp->bsel0 &= ~(IEI|RQI);
		while (kp->bsel0&RDYI);
		if (mp->d_que.c_cc == 0)
			return;
		kp->bsel0 = getc(&mp->d_que);
		n = RDYSCAN;
		while (n-- && (kp->bsel0&RDYI) == 0);
	}
	if (mp->d_que.c_cc)
		kp->bsel0 |= IEI;
}

dmrxint(dev)
{
	register struct dmr *mp;
	int	arg, cmd;

	{
		register struct device *kp;

		kp = dmr_addr[dev];
		arg = kp->sel6;
		cmd = kp->bsel2&7;
		kp->bsel2 &= ~RDYO;
	}
	mp = &dmr_dmr[dev];
	switch (cmd) {
	case OUR:
	case OUX:
	{
		register struct buf *bp, *abp;
		register flag;

		flag = (cmd==OUR)?B_READ:B_WRITE;
		/*
		 * Look through the active Q for the buffer
		 * the DMC11 says is finished. Assume the DMC11
		 * returns the buffers in FIFO order.
		 */
		abp = NULL;
		if (bp = mp->d_next) do {
			if ((bp->b_flags&B_READ)!=flag)
				continue;
			mp->d_rwq[flag]--;
			mp->d_ioc[flag]++;
			bp->b_resid = bp->b_bcount - (arg&CCOUNT);
			if (abp)
				abp->av_forw = bp->av_forw;
			else
				mp->d_next = bp->av_forw;
			iodone(bp);
			dmrstart(mp, flag);
			return;
		} while (abp = bp, bp = bp->av_forw);
		printf("DMC%d lost block\n", dev);
		return;
	}
	case CNTLO:
	{
		register short *ip;
		register count;
		register struct buf *bp;

		count = arg&CNTMASK;
		if (count&FATAL) {
			dmr_addr[dev]->bsel1 = MCLR;
			mp->d_flag &= ~DMCRUN;
			mp->d_rwq[0] = mp->d_rwq[1] = 0;
			bp = mp->d_next;
			while (bp) {
				bp->b_flags |= B_ERROR;
				iodone(bp);
				bp = bp->av_forw;
				mp->d_next = bp;
			}
			if (mp->d_flag&DMCACT) {
				dmropen(dev);
			} else {
				dmrclose(dev);
			}
		}
		ip = &mp->d_ioc[2];
		while (count) {
			if (count&01)
				(*ip)++;
			ip++;
			count >>= 1;
		}
		return;
	}
	default:
		printf("DMC%d bad control %o\n", dev, cmd);
	}
}

dmrread(dev)
{
	physio(dmrstrategy,&dmr_dmr[dev].d_rbuf[B_READ],dev,B_READ);
}

dmrwrite(dev)
{
	physio(dmrstrategy,&dmr_dmr[dev].d_rbuf[B_WRITE],dev,B_WRITE);
}

dmrioctl(dev, cmd, arg, mode)
{
	register struct dmr *mp;

	mp = &dmr_dmr[dev];
	switch (cmd) {
	case IOCTYPE:
		u.u_rval1 = DIOC;
		break;

	case DIOCGETC:
		if (copyout(mp->d_ioc, arg, sizeof(mp->d_ioc)))
			u.u_error = EFAULT;
		break;
	case DIOCGETB:
		if (copyout(mp->d_base, arg, sizeof(mp->d_base)))
			u.u_error = EFAULT;
		break;
	case DIOCSETE:
		dmrload(mp, PERR, 0, 0);
		break;
	case DIOCMAIN:
		mp->d_flag |= DMCMAI;
		dmrload(mp, PERR, 0, 0);
		break;
	case DIOCNORM:
		mp->d_flag &= ~DMCMAI;
		dmrload(mp, PERR, 0, 0);
		break;
	}
}
