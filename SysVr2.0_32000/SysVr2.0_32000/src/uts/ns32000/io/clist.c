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
/* @(#)clist.c	1.2 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/tty.h"

getc(p)
register struct clist *p;
{
	register struct cblock *bp;
	register int c, s;

	s = spl6();
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

	s = spl6();
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

	s = spl6();
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

	s = spl6();
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

	s = spl6();
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

	s = spl6();
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

getcbp(p, cp, n)
struct clist *p;
register char *cp;
register n;
{
	register struct cblock *bp;
	register char *op;
	register on;
	register char *acp = cp;

	while (n) {
		if ((bp = p->c_cf) == NULL)
			break;
		op = &bp->c_data[bp->c_first];
		on = bp->c_last - bp->c_first;
		if (n >= on) {
			bcopy(op, cp, on);
			cp += on;
			n -= on;
			if ((p->c_cf = bp->c_next) == NULL)
				p->c_cl = NULL;
			bp->c_next = cfreelist.c_next;
			cfreelist.c_next = bp;
		} else {
			bcopy(op, cp, n);
			bp->c_first += n;
			cp += n;
			n = 0;
			break;
		}
	}
	n = cp - acp;
	p->c_cc -= n;
	return(n);
}

putcbp(p, cp, n)
struct clist *p;
register char *cp;
register n;
{
	register struct cblock *bp, *obp;
	register char *op;
	register on;
	register char *acp = cp;

	while (n) {
		if ((bp = p->c_cl) == NULL || bp->c_last == cfreelist.c_size) {
			obp = bp;
			if ((bp = cfreelist.c_next) == NULL)
				break;
			cfreelist.c_next = bp->c_next;
			bp->c_next = NULL;
			bp->c_first = bp->c_last = 0;
			if (obp == NULL)
				p->c_cf = bp;
			else
				obp->c_next = bp;
			p->c_cl = bp;
		}
		op = &bp->c_data[bp->c_last];
		on = cfreelist.c_size - bp->c_last;
		if (n >= on) {
			bcopy(cp, op, on);
			cp += on;
			bp->c_last += on;
			n -= on;
		} else {
			bcopy(cp, op, n);
			cp += n;
			bp->c_last += n;
			n = 0;
			break;
		}
	}
	n = cp - acp;
	p->c_cc += n;
	return(n);
}
