/* @(#)ubm.c	1.2 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/seg.h"
#include "sys/map.h"
#include "sys/buf.h"
#include "sys/systm.h"
#include "sys/var.h"

#define	btos(X)	((long)X + 8191) >> 13

struct map ubmap[12] = {mapdata(12)};
static ubmflag;
static paddr_t bufoffs;

ubmalloc(size)
unsigned size;
{
	register nseg, base;
	register s;

	if (cputype != 70)
		return(0);
	nseg = btos(size);
	s = spl6();
	while ((base = malloc(ubmap, nseg)) == NULL) {
		ubmflag = 1;
		sleep(ubmap, PSWP+1);
	}
	splx(s);
	return(base);
}

static
ubmload(base, addr, size)
union { short w[2]; long l; } addr;
unsigned size;
{
	register nseg;
	register short *up;

	nseg = btos(size);
	up = (short *)(&UBMAP->r[2*base]);
	while (nseg--) {
		*up++ = addr.w[1];
		*up++ = addr.w[0];
		addr.l += 8192;
	}
}

ubmfree(base, size)
unsigned size;
{
	register nseg;

	if (cputype != 70)
		return;
	nseg = btos(size);
	mfree(ubmap, nseg, base);
	if (ubmflag) {
		wakeup(ubmap);
		ubmflag = 0;
	}
}

ubminit()
{
#define	ntob(X)	((X-1)<<13)+1
#define	SSR3	((physadr)0172516)
#define	UBMON	040
	register n;

	if (cputype != 70)
		return;
	ubmload(0, 0L, ntob(6));	/* Kernel D space */
	n = (v.v_buf + 15) >> 4;	/* ubm segments needed */
	if (n > 8)
		panic("Too many external buffers");
	ubmload(6, bufbase, ntob(n));	/* buffers */
	bufoffs = bufbase - (6L << 13);
	n += 6;	/* addr buffers plus external buffers */
	mfree(ubmap, 31-n, n);
	SSR3->r[0] |= UBMON;
}

paddr_t
ubmaddr(bp, base)
register struct buf *bp;
{

	if (cputype != 70)
		return(bp->b_paddr);
	if (bp->b_flags&B_PHYS) {
		ubmload(base, bp->b_paddr, bp->b_bcount);
		bp->b_flags |= B_MAP;
		return((paddr_t)base<<13);
	}
	if (bp->b_paddr >= (paddr_t)(unsigned)sabuf[0]
	 && bp->b_paddr <= (paddr_t)(unsigned)sabuf[v.v_sabuf])
		return(bp->b_paddr);
	return(bp->b_paddr - bufoffs);
}
