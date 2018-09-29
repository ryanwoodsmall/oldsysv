/* @(#)mem.c	6.1 */
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
#include "sys/mtpr.h"
extern int *mmpte;
extern char *mmvad;

mmread(dev)
{
	register unsigned n;
	register c;

	while(u.u_error==0 && u.u_count!=0) {
		n = min(u.u_count, ctob(1));
		if (dev == 0) {
			c = (u.u_offset >> 9) & PG_PFNUM;
			*mmpte = c | PG_V | PG_KR;
			mtpr(TBIS, mmvad);
			c = u.u_offset & 0x1ff;
			n = min(n, 512-c);
			if (copyout(&mmvad[c], u.u_base, n))
				u.u_error = ENXIO;
		} else if (dev == 1) {
			if (copyout(u.u_offset|0x80000000, u.u_base, n))
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
			c = (u.u_offset >> 9) & PG_PFNUM;
			*mmpte = c | PG_V | PG_KW;
			mtpr(TBIS, mmvad);
			c = u.u_offset & 0x1ff;
			n = min(n, 512-c);
			if (copyin(u.u_base, &mmvad[c], n))
				u.u_error = ENXIO;
		} else if (dev == 1) {
		  	if (copyin(u.u_base, u.u_offset|0x80000000, n))
				u.u_error = ENXIO;
		}
		u.u_offset += n;
		u.u_base += n;
		u.u_count -= n;
	}
}
