/* @(#)uba.c	6.5 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/systm.h"
#include "sys/mtpr.h"
#include "sys/proc.h"
#include "sys/uba.h"
#include "sys/map.h"
#include "sys/buf.h"
#include "sys/page.h"

#define ISTK 1

struct map ubmap[MAXUBA][UAMSIZ] = {
	{mapdata(UAMSIZ)},
	{mapdata(UAMSIZ)},
	{mapdata(UAMSIZ)},
	{mapdata(UAMSIZ)},
};

extern Xuba;		/* trap.s interrupt entry point */
int	ubazero;	/* incremented by uba code in trap.s */
extern int *ubaaddr[];
extern int ubacf[];
static	char	bdpwant[MAXUBA];
static	int	bdpuse[MAXUBA];
paddr_t	ubcoffs;
extern	_sdata, end;
extern char *bufstart;

mubmalloc(size, bdpflg, ubnum)
unsigned size;
register ubnum;
{
/*
 * Allocate as many contiguous UBA mapping registers
 * as are necessary to do transfer of 'size' bytes.
 * Wait for enough map registers.
 * if 'bdpflg' is non-zero: a buffered data path (BDP) is used;
 * else a direct data path (DDP).
 * Return
 *   |23 - - 22|21 - - 18|17 - -  9|8  - -  0|
 *   | UBA #   |  BDP #  |    #    |  start  |
 *   |         | map reg | map reg |	     |
 */
	register nseg, base;
	register bdp = 0;
	register s;

	/* This is a worst case allocation; guaranteed to handle all cases.
	 * Doesn't need to be this bad for block I/O buffers since they're
	 * always on page boundaries and never use buffered data paths.
	 * (See mubmaddr.)
	 */
	nseg = btoc(size + ctob(1) - 1);
	if (bdpflg) nseg++;	/* Need an extra map reg for bdp prefetches */
	s = spl6();
	while ((base = malloc(ubmap[ubnum], nseg)) == NULL) {
		if (bdpflg & (UBA_CANTWAIT | UBA_BDPPREF)) {
			splx(s);
			return(0);
		}
		mapwant(ubmap[ubnum])++;
		sleep((caddr_t)ubmap[ubnum], PSWP);
	}
	if (bdpflg) for(;;) {
		for (bdp = ((cputype == 780) ? NDP780 : NDP750); --bdp;)
			if (!(bdpuse[ubnum]&(1<<bdp)))
				break;
		if (bdp == 0) {
			if (bdpflg & UBA_BDPPREF) {
				splx(s);
				return((ubnum<<22) | (nseg<<9) | base);
			 } else {
				if (bdpflg & UBA_CANTWAIT) {
					mfree (ubmap[ubnum],nseg,base);
					splx(s);
					return(0);
				}
				bdpwant[ubnum]++;
				sleep((caddr_t)&bdpuse[ubnum], PSWP);
			}
		} else {
			bdpuse[ubnum] |= (1<<bdp);
			break;
		}
	}
	splx(s);
	return((ubnum<<22) | (bdp<<18) | (nseg<<9) | base);
}

ubmfree(umd)
{
	register nseg, base, bdp;
	register s;
	register ubnum;

	s = spl6();
	bdp = umdbdp(umd);
	ubnum = umduba(umd);
	if (bdp) {
		if (cputype == 780)
			ubavad[ubnum]->ubadpr[bdp] |= BNE; /* purge 780 */
		else
			ubavad[ubnum]->ubabdp[bdp] |= BDPURGE;	/* purge 750 */
		bdpuse[ubnum] &= ~(1<<bdp);
		if (bdpwant[ubnum]) {
			bdpwant[ubnum] = 0;
			wakeup((caddr_t)&bdpuse[ubnum]);
		}
	}
	nseg = umdnseg(umd);
	base = umdbase(umd);
	mfree(ubmap[ubnum], nseg, base);
	splx(s);
}

struct uba *ubavad[MAXUBA];
static int umemvad[MAXUBA];
extern int uba_cnt;
ubainit()
{
	register n, i;
	register int **up, *iv;
	int br, nex;

	if (uba_cnt < 0) {
		printf("DANGER: negative unibus adapter count\n");
		printf("  resetting configuration to include 0 adapters\n");
		uba_cnt = 0;
	}
	if ((cputype != 780) && (uba_cnt > 1)) {
		printf("DANGER: multiple unibus adapters configured\n");
		uba_cnt = 1;
	}
	for (i = 0; i < uba_cnt; i++) {
		ubavad[i] = (struct uba *)sptalloc(8, PG_V | PG_KW,
			btoc((cputype == 780) ? UNEX780(ubacf[i]) : 0xf30000));
		umemvad[i] = sptalloc(16, PG_V | PG_KW,
			btoc((cputype == 780) ? UMEM780(ubacf[i]) : 0xffe000));
	}
	if (cputype == 780) {
		int *pt;
		pt = (int *) sptalloc(1, PG_V|PG_KW, btoc(mfpr(SCBB)));
		for (br = 4; br < 7; br++) {
			for (i = 0; i < uba_cnt; i++) {
				pt[64 + (br - 4) * 16 +
					ubacf[i]] = (int) &Xuba + (8 * i) + ISTK;

			}
		}
		sptfree(pt, 1, 0);
	}
	up = ubaaddr;
	while (iv = *up++) {
		*iv = UBADEV(*iv) + umemvad[UBANUM(*iv)] - 0160000;
	}
	n = (((int)(&end)) - ((int)(&_sdata)) + 0x1ff) >> 9; /* size of kernel
							  data + bss in pages */
	if (n>=496) {
		printf("WARNING: clist area too big for unibus map\n");
		printf("  %d pages truncated to 495 pages\n", n);
		n = 495;
	}
	ubaclr();
	for (i = 0; i < uba_cnt; i++) {
		mfree(ubmap[i], 496-n, n); /* add remaining map regs to list */
	}
	ubcoffs = (int)(&_sdata) & ~0x1ff; /*
					    * starting address of data space
					    * used in macro ubmdata to find
					    * unibus map register corresponding
					    * to location
					    */
}

mubmdev(addr, ubnum)
register ubnum;
{
	return(addr - umemvad[ubnum] + 0160000);
}

ubaindex(nex)
register nex;
{
	register i;

	for (i = 0; i < uba_cnt; i++)
		if (nex == ubacf[i])
			return(i);
	return(-1);
}

paddr_t
mubmaddr(bp, umd, ubnum)
register struct buf *bp;
register ubnum;
{
	register *pt, *io, pf;
	register nseg, pfn;
	int	bdp, base, count;

	pfn = 0;
	if (umd == 0)
		umd = bp->b_umd;	/* avoid reallocation of map regs */
	if (bdp = umdbdp(umd)) {
		if (cputype == 780)
			ubavad[ubnum]->ubadpr[bdp] |= BNE;
		else
			ubavad[ubnum]->ubabdp[bdp] |= BDPURGE;
		if (paddr(bp) & 01)
			pfn |= BO;
	}
	pfn |= MRV | (bdp << 21);
	nseg = umdnseg(umd);
	count = btoc(bp->b_bcount + (paddr(bp)&0x1ff));
	if (count<nseg)
		nseg = count;
	base = umdbase(umd);
	if (bp->b_flags&B_PHYS) {

		io = &ubavad[ubnum]->map[base];
		pf = svtoc(paddr(bp));
		switch ( (paddr(bp)>>30) & 3) {
		case 2:
			pt = (int *)sbrpte + pf;
			break;
		case 1:
			/* I/O to stack */
			pf += bp->b_proc->p_ptsize*128 - 0x200000;
		case 0:
			pt = (int *)baseseg[bp->b_proc->p_ptaddr] + pf;
			break;
		default:
			panic ("ubmaddr") ;
		}
		while (--nseg >= 0)
			if ((*io++ = ((*pt++ & PG_PFNUM) | pfn)) == pfn)
				panic("uba, zero entry");
		if (bdp) *io = 0;  /* prevent prefetch segment violations */
		return((base<<9)|(paddr(bp)&0x1fe));
	} else {
		if (paddr(bp) < (paddr_t)&end)
			return(paddr(bp) - ubcoffs);
		if (paddr(bp) < (paddr_t)bufstart) {
			panic("Bad I/O address");
		}
		{
		/*
		 * At this point, all that's left is Block I/O with
		 * or without buffered data paths.
		 * i.e. driver could preallocate map regs if it wished
		 */
	
			if (nseg == 0) {
				/* keep map reg use to a minimum; fool mubmalloc */
				umd = mubmalloc(count - (ctob(1)-1), 0, ubnum);
				bp->b_umd = umd; /* brelse must deallocate */
				nseg = umdnseg(umd);
				base = umdbase(umd);
			}
			pf = svtoc(paddr(bp));
			switch ( (paddr(bp)>>30) & 3) {
			case 2:
				pt = (int *)sbrpte + pf;
				break;
			case 1:
				/* I/O to stack */
				pf += bp->b_proc->p_ptsize*128 - 0x200000;
			case 0:
				pt = (int *)baseseg[bp->b_proc->p_ptaddr] + pf;
				break;
			default:
				panic ("ubmaddr") ;
			}
			io = &ubavad[ubnum]->map[base];
			while (--nseg >= 0)
				if ((*io++ = ((*pt++ & PG_PFNUM) | pfn)) == pfn)
					panic("uba, BIO zero entry");
			if (bdp) *io = 0;  /* prevent prefetch segment violations */
			return(base<<9);
		}
	}
}

paddr_t
mubmvad(vad, umd, ubnum)
register ubnum;
{
	register nseg;
	register *io, *pt;
	register pfn, bdp;

	pfn = 0;
	if (bdp = umdbdp(umd)) {
		if (cputype == 780)
			ubavad[ubnum]->ubadpr[bdp] |= BNE;
		else
			ubavad[ubnum]->ubabdp[bdp] |= BDPURGE;
		if (vad & 01)
			pfn |= BO;
	}
	pfn |= MRV | (bdp << 21);
	nseg = umdnseg(umd);
	io = &ubavad[ubnum]->map[umdbase(umd)];
	pt = (int *)&sbrpte[svtoc(vad)];
	while (--nseg >= 0)
		*io++ = (*pt++ & PG_PFNUM) | pfn;
	return((umdbase(umd)<<9)|(vad&0x1fe));
}

ubasrv(vec, ubnum)
register ubnum;
{
	printf("UBA(%d): csr %x, sr %x\n", ubnum, ubavad[ubnum]->ubacsr,
		ubavad[ubnum]->ubasr);
	printf("   : fubar %o, fmer %o\n", ubavad[ubnum]->ubafubar<<2,
		ubavad[ubnum]->ubafmer<<9);
	/* clear error bits */
	ubavad[ubnum]->ubacsr = ubavad[ubnum]->ubacsr;
	ubavad[ubnum]->ubasr = ubavad[ubnum]->ubasr;
	/* possible normal interrupt here ? */
}

ubastray(vec, ubnum)
{
	printf("stray UBA interrupt at %o (UBA %d)\n",vec, ubnum);
}
ubaclr()
{
	register base, i, j, n;
	for (j = 0; j < uba_cnt; j++) {
		if (cputype == 780) {
			ubavad[j]->ubacsr = UBIC | ADPUP;
			ubavad[j]->ubacr = UBAIE;
		} else {
			mtpr(IORR, 1);		/* UNIBUS init */
			for (i = 1; i < NDP750; i++)		/* purge and clear */
				ubavad[j]->ubabdp[i] |= BDPURGE;
		}
		base = (((int)(&_sdata)) >> 9) & PG_PFNUM;
		n = (((int)(&end)) - ((int)(&_sdata)) + 0x1ff) >> 9;
		for (i=0; i<n; i++) {
			ubavad[j]->map[i] = MRV | base++;
		}
	}
}
