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
 *	Memory special file
 *	minor device 0 is physical memory
 *	minor device 1 is kernel memory
 *	minor device 2 is EOF/NULL
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/buf.h"
#include "sys/systm.h"
#include "sys/page.h"
#include "sys/mmu.h"
extern pte_t *mmpte;
extern char *mmvad;

mmread(dev)
{
	register unsigned n;
	register c;

	while(u.u_error==0 && u.u_count!=0) {
		n = min(u.u_count, ctob(1));
		if (dev == 0) {
			c = u.u_offset & ~(NBPP-1);
			c |= PG_KR | PG_V;
			mmpte->pgi.pg_pte = c;
			mmpte->pgi.pg_pte2 = PG_PE2(c);
			invsatb(SYSATB, mmvad);
			c = u.u_offset & (NBPP-1);
			n = min(n, ctob(1)-c);
			if (copyout(&mmvad[c], u.u_base, n))
				u.u_error = ENXIO;
		} else if (dev == 1) {
			if (copyout(u.u_offset, u.u_base, n))
				u.u_error = ENXIO;
		} else
			return;
		u.u_offset += n;
		u.u_base += n;
		u.u_count -= n;
	}
}

mmwrite(dev)
{
	register unsigned n;
	register c;

	while(u.u_error==0 && u.u_count!=0) {
		n = min(u.u_count, ctob(1));
		if (dev == 0) {
			c = u.u_offset & ~(NBPP-1);
			c |= PG_KR | PG_V;
			mmpte->pgi.pg_pte = c;
			mmpte->pgi.pg_pte2 = PG_PE2(c);
			invsatb(SYSATB, mmvad);
			c = u.u_offset & (NBPP-1);
			n = min(n, ctob(1)-c);
			if (copyin(u.u_base, &mmvad[c], n))
				u.u_error = ENXIO;
		} else if (dev == 1) {
		  	if (copyin(u.u_base, u.u_offset, n))
				u.u_error = ENXIO;
		}
		u.u_offset += n;
		u.u_base += n;
		u.u_count -= n;
	}
}
