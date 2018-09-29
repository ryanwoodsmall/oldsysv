/* @(#)mba.c	6.2 */
#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/buf.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/proc.h"
#include "sys/seg.h"
#include "sys/var.h"
#include "sys/page.h"
#include "sys/mba.h"
#include "sys/mtpr.h"

#define ISTK	1

extern struct mbacf mbacf[];
extern Xmba;
extern int mba_cnt;


mbasetup(bp, mba)
register struct buf *bp;
register struct mba *mba;
{
	register int *io, num;
	register int *pt, pf;

	io = mba->map;
	pf = svtoc(paddr(bp));
	num = svtoc(paddr(bp) + bp->b_bcount - 1) - pf + 1;
	switch ( (paddr(bp)>>30) & 3) {
	case 2:
		pt = (int *)sbrpte + pf;
		break ;
	case 1:
		/* I/O to stack */
		pf += bp->b_proc->p_ptsize*128 - 0x200000;
	case 0:
		pt = (int *)baseseg[bp->b_proc->p_ptaddr] + pf;
		break ;
	default:
		panic ("mbasetup") ;
	}
	while (--num>=0) {
		*io++ = (*pt++ & PG_PFNUM) | PG_V;
	}
	*io = 0;
	mba->ireg.regs.sr = -1;	/* clear status (error) bits */
	mba->ireg.regs.bcr = -bp->b_bcount;
	mba->ireg.regs.var = paddr(bp) & 0x1ff;
}

mbainit()
{
	register i;
	register *pt;

	for (i = 0; i < mba_cnt; i++) {
		*mbacf[i].devptr = sptalloc(8, PG_V | PG_KW,
			btoc((cputype == 780) ?
				0x20000000 + mbacf[i].nexus * 0x2000
				: 0xf28000 + 0x2000 * i));
	}
	pt = (int *)sptalloc(1, PG_V | PG_KW, btoc(mfpr(SCBB)));
	for (i = 0; i < mba_cnt; i++)
		pt[64 + (mbacf[i].sbipri - 4)*16 +
		    ((cputype == 780) ? mbacf[i].nexus : 4 + i)] =
			(int)&Xmba + (8 * i) + ISTK;
	sptfree(pt, 1, 0);
	mbaclr();
}

mbaclr()
{
	register i;

	for (i = 0; i < mba_cnt; i++)
		((struct mba *)*mbacf[i].devptr)->ireg.regs.cr = MBAIE;
}

mbasrv(mba)
register struct mba *mba;
{
	printf("MBA %x, csr %x\n", mba, mba->ireg.regs.csr);
	mba->ireg.regs.csr = mba->ireg.regs.csr;
}
