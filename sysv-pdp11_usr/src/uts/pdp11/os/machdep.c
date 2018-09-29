/* @(#)machdep.c	1.2 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/inode.h"
#include "sys/proc.h"
#include "sys/seg.h"
#include "sys/map.h"
#include "sys/reg.h"
#include "sys/buf.h"
#include "sys/var.h"
#include "sys/psl.h"
#include "sys/utsname.h"

/*
 * Machine-dependent startup code
 */
startup(firstaddr)
{
	register unsigned i;
	register unsigned bs;

	/*
	 * zero and free all of core
	 */

	printf("\nUNIX/%s: %s%s\n", utsname.release, utsname.sysname, utsname.version);
	i = firstaddr + USIZE;
	UISD->r[0] = 077406;
	for(;;) {
		UISA->r[0] = i;
		if (suiword((caddr_t)0, 0) != 0)
			break;
		clearseg(i);
		maxmem++;
		mfree(coremap, 1, i);
		i++;
	}
	printf("real mem = %D bytes\n", ctob((long)(maxmem + firstaddr + USIZE)));
	bs = btoc(BSIZE)*v.v_buf;
	i = malloc(coremap, bs);
	if (i==NULL)
		panic("buffers");
	bufbase = ctob((paddr_t)i);
	maxmem -= bs;

	maxmem -= mausinit();
	mfree(swapmap, nswap, 1);
	swplo--;
}

/*
 * Determine which clock is attached, and start it.
 * panic: no clock found
 */
static physadr lks;	/* pointer to clock device */
#define	CLOCK1	((physadr)0177546)
#define	CLOCK2	((physadr)0172540)
clkstart()
{

	/*
	 * set up user I/O physical address
	 */

	UISA->r[7] = 0177600;	/* io segment */
	UISD->r[7] = 077406;

	lks = CLOCK1;
	if(fuiword((caddr_t)lks) == -1) {
		lks = CLOCK2;
		if(fuiword((caddr_t)lks) == -1)
			panic("no clock");
	}
	lks->r[0] = 0115;
}

clkset(oldtime)
time_t	oldtime;
{
	time = oldtime;
}

clkreld(on)
{
	if (on)
		lks->r[0] = 0115;
	else
		lks->r[0] = 0;
}

/*
 * Let a process handle a signal by simulating an interrupt
 */
sendsig(p, signo)
caddr_t p;
{
	register unsigned n;

	n = u.u_ar0[R6] - 4;
	grow(n);
	suword((caddr_t)n+2, u.u_ar0[PS]);
	suword((caddr_t)n, u.u_ar0[R7]);
	u.u_ar0[R6] = n;
	u.u_ar0[PS] &= ~PS_T;
	u.u_ar0[R7] = (int)p;
}

/*
 * copy count bytes from from to to.
 */
bcopy(from, to, count)
caddr_t from, to;
register count;
{
	register char *f, *t;

	f = from;
	t = to;
	do
		*t++ = *f++;
	while(--count);
}

/*
 * create a duplicate copy of a process
 */
procdup(p)
register struct proc *p;
{
	register ushort a1, a2, n;

	n = p->p_size;
	if ((a2 = malloc(coremap, n)) == NULL)
		return(NULL);

	a1 = p->p_addr;
	p->p_addr = a2;
	while(n--)
		copyseg(a1++, a2++);
	return(1);
}
