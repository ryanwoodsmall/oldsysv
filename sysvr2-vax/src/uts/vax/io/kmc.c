/* @(#)kmc.c	6.1 */
/*
 * KMC11 microprocessor driver
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/tty.h"
#include "sys/kmc.h"
#include "sys/buf.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"

extern struct device *kmc_addr[];
extern	kmc_cnt;

#define	MAXKMC	16
struct kmc {
	struct clist k_inq;
	short	k_stat;
	char	k_type;
	char	k_opt;
	short	k_arg[3];
	int	(*k_rint)();
} kmc[MAXKMC];

#define	KMC11A	1
#define	KMC11B	2
#define	KASIZE	1024
#define	KBSIZE	4096

#define	RUN	(1<<7)
#define	MCLR	(1<<6)
#define	CWRT	(1<<5)
#define	LUB	(1<<4)
#define	LUA	(1<<3)
#define	ROMO	(1<<2)
#define	ROMI	(1<<1)
#define	STEP	(1<<0)

#define RDYO	0200
#define RDYI	020
#define RQI	0200
#define IEI	01
#define IEO	020

#define	STYPE	017
#define SRUN	020
#define SRINT	040
#define	SOPEN	0100
#define	SLOAD	0200
#define TIMERON 0400
#define CMDFLG  01000
#define KMCOK   02000
#define KMCDCLR 04000
#define SENDOK  010000


#define TSEC  5*HZ
#define PFTSEC 15*HZ

#define KMCDEV(X) (X>>8)&017
#define SHIFTKMC(X) (X<<8)

struct device {
union {
	char	b[8];
	ushort	w[4];
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

kmcopen(dev, flag)
{
	register struct device *kp;
	register struct kmc *tp;
	register sav;

	if (dev>=kmc_cnt || (tp = &kmc[dev])->k_stat&SOPEN) {
		u.u_error = ENXIO;
		return;
	}
	tp->k_stat |= SOPEN;
	if (tp->k_type==0) {
		kp = kmc_addr[dev];
		kp->bsel1 = ROMO;
		kp->sel4 = 0;
		sav = kp->sel6;
		kp->sel6 = ~sav;
		if (kp->sel6 != sav) {
			tp->k_type = KMC11B;
			kp->sel6 = sav;
		} else
			tp->k_type = KMC11A;
		kp->bsel1 = 0;
	}
}

kmcclose(dev)
{
	kmc[dev].k_stat &= ~SOPEN;
}

kmcread(dev)
{
	register struct device *kp;
	register ad;
	int	dsize;
	ushort	sav;

	if (kmc[dev].k_stat&SRUN)
		return;
	dsize = (kmc[dev].k_type==KMC11A)?KASIZE:KBSIZE;
	kp = kmc_addr[dev];
	kp->bsel1 = 0;
	do {
		ad = u.u_offset;
		if (ad<dsize*2) {
			if (ad&1) {
				u.u_error = ENXIO;
				break;
			}
			ad >>= 1;
			kp->bsel1 = ROMO;
			kp->sel4 = ad;
			passc(kp->bsel6);
			passc(kp->bsel7);
			kp->bsel1 = 0;
		} else if (ad -= dsize*2, ad<dsize) {
			kp->bsel1 = ROMO;
			kp->sel4 = 0;
			sav = kp->sel6;
			kp->bsel1 = ROMI;
			kp->sel6 = 010000|(ad&0377);	/* mov ad,mar */
			kp->bsel1 = ROMI|STEP;
			kp->bsel1 = ROMI;
			kp->sel6 = 04000|((ad>>8)&0377);	/* mov %ad,%mar */
			kp->bsel1 = ROMI|STEP;
			kp->bsel1 = ROMI;
			kp->sel6 = 055222;	/* mov mem,csr2|mar++ */
			kp->bsel1 = ROMI|STEP;
			passc(kp->bsel2);
			kp->bsel1 = ROMI;
			kp->sel6 = sav;
			kp->bsel1 = 0;
		} else
			break;
	} while (u.u_error==0 && u.u_count);
}

kmcwrite(dev)
{
	register struct device *kp;
	register ad;
	int	dsize;
	short	ins;
	ushort	sav;

	if (kmc[dev].k_stat&SRUN)
		return;
	dsize = (kmc[dev].k_type==KMC11A)?KASIZE:KBSIZE;
	kp = kmc_addr[dev];
	kp->bsel1 = 0;
	while (u.u_error==0 && u.u_count) {
		ad = u.u_offset;
		if (ad<dsize*2) {
			if (ad&1) {
				u.u_error = ENXIO;
				break;
			}
			kp->bsel1 = ROMO;
			kp->sel4 = ad>>1;
			lobyte(ins) = cpass();
			hibyte(ins) = cpass();
			kp->sel6 = ins;
			kp->bsel1 |= CWRT;
			kp->bsel1 = 0;
		} else if (ad -= dsize*2, ad<dsize) {
			kp->bsel1 = ROMO;
			kp->sel4 = 0;
			sav = kp->sel6;
			kp->bsel1 = ROMI;
			kp->sel6 = 010000|(ad&0377);	/* mov ad,mar */
			kp->bsel1 = ROMI|STEP;
			kp->bsel1 = ROMI;
			kp->sel6 = 04000|((ad>>8)&0377);	/* mov %ad,%mar */
			kp->bsel1 = ROMI|STEP;
			kp->bsel1 = ROMI;
			kp->bsel2 = cpass();
			kp->sel6 = 0136440;	/* mov csr2,mem|mar++ */
			kp->bsel1 = ROMI|STEP;
			kp->bsel1 = ROMI;
			kp->sel6 = sav;
			kp->bsel1 = 0;
		} else
			break;
	}
}

kmcioctl(dev, cmd, arg, mode)
{
	register struct device *kp;
	register struct kmc *tp;
	struct kmcntl kk;
	short	csr[4];
	ushort	sav;

	if (cmd != KCSETA) {
		u.u_error = EINVAL;
		return;
	}
	if (copyin(arg, &kk, sizeof(kk))) {
		u.u_error = EFAULT;
		return;
	}
	kp = kmc_addr[dev];
	tp = &kmc[dev];
	switch (kk.kmd) {
	case KMCLR:
	case KRESET:
		spl7();
		kp->bsel1 = MCLR;
		spl0();
	case KSTOP:
		tp->k_stat &= ~SRUN;
		kp->bsel1 = 0;
		if (kk.kmd == KRESET) {
			tp->k_stat = 0;
			while(getc(&tp->k_inq) >= 0) ;
		}
		return;
	case KMS:
		if (tp->k_stat&SRUN)
			break;
		kp->bsel1 = ROMI|ROMO;
		sav = kp->sel6;
		kp->bsel1 = ROMI;
		kp->sel6 = kk.kval;
		kp->bsel1 = ROMI|STEP;
		kp->bsel1 = ROMI;
		kp->sel6 = sav;
		kp->bsel1 = 0;
		goto lcsr;
	case KSTEP:
		if (tp->k_stat&SRUN)
			break;
		kp->bsel1 |= STEP;
		kp->bsel1 = 0;
	case KCSR:
	lcsr:
		csr[0] = kp->sel0;
		csr[1] = kp->sel2;
		csr[2] = kp->sel4;
		csr[3] = kp->sel6;
		if (copyout(csr, kk.kcsr, sizeof csr))
			u.u_error = EFAULT;
		return;
	case KWRCR:
		if (tp->k_stat&SRINT)
			break;
		kp->sel6 = kk.kval;
		return;
	case KRUN:
		if (tp->k_stat&SRUN)
			break;
		tp->k_stat &= ~STYPE;
		tp->k_stat |= (kk.kval&STYPE)|SRUN;
		kp->bsel1 |= RUN;
		if (tp->k_stat&SRINT) {
			spl5();
			kmcrint(dev);
			spl0();
		}
		return;
	case KLU:
		kp->bsel1 = kk.kval&(LUA|LUB);
		return;
	}
	u.u_error = EIO;
}

kmcrint(dev)
{
	register struct device *kp;
	register struct kmc *tp;

	kp = kmc_addr[dev];
	tp = &kmc[dev];
	kp->sel0 &= ~IEI;
	while (kp->sel2&RDYI) {
		if ((tp->k_stat&SLOAD) ||
		  getcbp(&tp->k_inq, tp->k_arg, 6) == 6) {
			kp->sel2 = tp->k_arg[0]|RDYI;
			kp->sel4 = tp->k_arg[1];
			kp->sel6 = tp->k_arg[2];
			tp->k_stat &= ~SLOAD;
		} else {
			printf("Bad kmc %d load\n", dev);
		}
		tp->k_stat |= KMCOK;
		if (tp->k_inq.c_cc==0) {
			kp->sel0 &= ~RQI;
			kp->sel2 &= ~RDYI;
			return;
		}
		kp->sel2 &= ~RDYI;
	}
	if ((tp->k_stat&SLOAD) || tp->k_inq.c_cc)
		kp->sel0 |= IEI|RQI;
}

kmcxint(dev)
{
	register struct device *kp;
	register struct kmc *tp;
	int p1, p2, p3, p4;

	kp = kmc_addr[dev];
	tp = &kmc[dev];
	kp->sel0 &= ~IEO;
	while(kp->sel2&RDYO) {
		p1 = SHIFTKMC(dev)|(kp->bsel3);
		p2 = kp->bsel2&017;
		p3 = kp->sel4;
		p4 = kp->sel6;
		kp->sel2 &= ~RDYO;
		if (tp->k_stat&SRINT)
			(*tp->k_rint)(p1, p2, p3, p4);
	}
	kp->sel0 |= IEO;
}

kmcload(dev, p1, p2, p3)
{
	register struct device *kp;
	register struct kmc *tp;
	register unit;
	register sps;

	unit = KMCDEV(dev);
	tp = &kmc[unit];
	if (!(tp->k_stat&SRUN)) {
		if (!(tp->k_opt&OKCHK))
			return(-1);
	}
	kp = kmc_addr[unit];
	sps = spl5();
	if (tp->k_stat&SLOAD) {
		putcbp(&tp->k_inq, tp->k_arg, 6);
		tp->k_stat &= ~SLOAD;
	}
	kp->sel0 |= RQI;
	tp->k_arg[0] = (p1&017)|((dev&0377)<<8);
	tp->k_arg[1] = p2;
	tp->k_arg[2] = p3;
	if (tp->k_inq.c_cc)
		putcbp(&tp->k_inq, tp->k_arg, 6);
	else
		tp->k_stat |= SLOAD;
	kmcrint(unit);
	splx(sps);
	return(tp->k_inq.c_cc);
}

kmcset(dev, type, rint,opt)
int (*rint)();
{
	register struct device *kp;
	register struct kmc *tp;
	register unit;
	extern kmcok();

	kp = kmc_addr[dev];
	tp = &kmc[dev];
	if (!(opt&OKCHK)) {
		if ((tp->k_stat&(STYPE|SRUN|SOPEN))!=((type&STYPE)|SRUN))
			return (1);
	}
	tp->k_opt = opt;
	if (opt&OKOPT) {
		tp->k_stat |= SENDOK;
		kmcload(SHIFTKMC(dev),OKCMD,0,0);
	}
	if (!(tp->k_stat&TIMERON)) {
		tp->k_stat |= TIMERON;
		timeout(kmcok, SHIFTKMC(dev), TSEC);
	}
	tp->k_stat |= SRINT;
	tp->k_rint = rint;
	kp->sel0 |= IEO;
	return(0);
}

kmcdclr(dev)
register dev;
{
	register struct kmc *tp;
	register struct device *kp;

	if (dev < 0 || dev >= kmc_cnt)
		return;
	tp = &kmc[dev];
	while (getc(&tp->k_inq) >= 0) ;
	tp->k_stat |= KMCDCLR;
	kp = kmc_addr[dev];
	kp->sel0 = 0;
	kp->sel2 = 0;
}

kmcreset(dev)
{
	register struct kmc *tp;
	register struct device *kp;
	register s;

	tp = &kmc[dev];
	kp = kmc_addr[dev];
	s = spl7();
	kp->bsel1 = MCLR;
	splx(s);
	kp->bsel1 = 0;
	tp->k_stat = 0;
	while(getc(&tp->k_inq)>=0);
}


kmcok(dev)
{
	register struct kmc *tp;
	register unit;
	extern kmcok();


	unit = KMCDEV(dev);
	tp = &kmc[unit];

	if ( tp->k_stat&KMCDCLR ) {
		timeout(kmcok, dev, PFTSEC);
		tp->k_stat &= ~KMCDCLR;
		return;
	}
	if ((tp->k_stat&CMDFLG)&&(!(tp->k_stat&KMCOK))) {
		tp->k_stat &= ~CMDFLG;
		tp->k_stat &= ~TIMERON;
		if ( tp->k_stat&SRINT )
			(*tp->k_rint)(dev,OKCHECK,0,0);
		return;
	}
	tp->k_stat &= ~CMDFLG;
	if (tp->k_stat&SENDOK) {
		tp->k_stat |= CMDFLG;
		tp->k_stat &= ~KMCOK;
		kmcload(dev,OKCMD,0,0);
	} else {
		if (tp->k_inq.c_cc || (tp->k_stat&SLOAD)) {
			tp->k_stat |= CMDFLG;
			tp->k_stat &= ~KMCOK;
		}
	}
	timeout(kmcok, dev, TSEC);
}
