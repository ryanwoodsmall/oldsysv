/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:io/clist.c	10.6"
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/tty.h"
#include "sys/var.h"
#include "sys/inline.h"

struct cblock	*cfree;		/* Ptr to start of memory area	*/
				/* allocated to cblocks.	*/
struct chead	cfreelist;	/* Head of cblock freelist ptr.	*/
int		cfreecnt;	/* Count of free cblock's.	*/

getc(p)
register struct clist *p;
{
	register struct cblock *bp;
	register int c, s;

	s = splhi();
	if (p->c_cc > 0) {
		p->c_cc--;
		bp = p->c_cf;
		c = bp->c_data[bp->c_first++]&0377;
		if (bp->c_first == bp->c_last) {
			if ((p->c_cf = bp->c_next) == NULL)
				p->c_cl = NULL;
			bp->c_next = cfreelist.c_next;
			cfreelist.c_next = bp;
			if (cfreelist.c_flag) {
				cfreelist.c_flag = 0;
				wakeup(&cfreelist);
			}
		}
	} else
		c = -1;
	splx(s);
	return(c);
}

putc(c, p)
register struct clist *p;
{
	register struct cblock *bp, *obp;
	register s;

	s = splhi();
	if ((bp = p->c_cl) == NULL || bp->c_last == cfreelist.c_size) {
		obp = bp;
		if ((bp = cfreelist.c_next) == NULL) {
			splx(s);
			return(-1);
		}
		cfreelist.c_next = bp->c_next;
		bp->c_next = NULL;
		bp->c_first = bp->c_last = 0;
		if (obp == NULL)
			p->c_cf = bp;
		else
			obp->c_next = bp;
		p->c_cl = bp;
	}
	bp->c_data[bp->c_last++] = c;
	p->c_cc++;
	splx(s);
	return(0);
}

struct cblock *
getcf()
{
	register struct cblock *bp;
	register int s;

	s = splhi();
	if ((bp = cfreelist.c_next) != NULL) {
		cfreelist.c_next = bp->c_next;
		bp->c_next = NULL;
		bp->c_first = 0;
		bp->c_last = cfreelist.c_size;
	}
	splx(s);
	return(bp);
}

putcf(bp)
register struct cblock *bp;
{
	register int s;

	s = splhi();
	bp->c_next = cfreelist.c_next;
	cfreelist.c_next = bp;
	if (cfreelist.c_flag) {
		cfreelist.c_flag = 0;
		wakeup(&cfreelist);
	}
	splx(s);
}

struct cblock *
getcb(p)
register struct clist *p;
{
	register struct cblock *bp;
	register int s;

	s = splhi();
	if ((bp = p->c_cf) != NULL) {
		p->c_cc -= bp->c_last - bp->c_first;
		if ((p->c_cf = bp->c_next) == NULL)
			p->c_cl = NULL;
	}
	splx(s);
	return(bp);
}

putcb(bp, p)
register struct cblock *bp;
register struct clist *p;
{
	register struct cblock *obp;
	register int s;

	s = splhi();
	if ((obp = p->c_cl) == NULL)
		p->c_cf = bp;
	else
		obp->c_next = bp;
	p->c_cl = bp;
	bp->c_next = NULL;
	p->c_cc += bp->c_last - bp->c_first;
	splx(s);
	return(0);
}

/*
 * Initialize clist by freeing all character blocks.
 */

#define	BOUND		0x10000		/* 64K boundary for I/O	*/
#define	nextbound(x)	(x / BOUND * BOUND + BOUND)

cinit()
{
	register		n;
	register struct cblock *cp;
	register int		addr;

	n = btoc(v.v_clist * sizeof(struct cblock));
	if ((cfree = (struct cblock *)kseg(n)) == NULL)
		panic("cannot allocate character buffers");

	for (n = 0, cp = &cfree[0]; n < v.v_clist; n++, cp++) {
		addr = svirtophys(cp->c_data);
		if (addr + CLSIZE <= nextbound(addr)) {
			cp->c_next = cfreelist.c_next;
			cfreelist.c_next = cp;
			cfreecnt++;
		}
	}
	cfreelist.c_size = CLSIZE;
}
