/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/mmgt.c	10.6"
/*	3B2 Specific Memory Management Routines
*/

#include "sys/types.h"
#include "sys/param.h"
#include "sys/psw.h"
#include "sys/map.h"
#include "sys/sysmacros.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/pcb.h"
#include "sys/immu.h"
#include "sys/user.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/pfdat.h"
#include "sys/debug.h"
#include "sys/cmn_err.h"
#include "sys/tuneable.h"

char ksegtbl[ctos(SYSSEGSZ)];	/* parallel segment table for kseg */

/*	Add a new segment to the kernel.  Returns the new
**	kernel segment virtual address.
*/

char *
kseg(sz)
register int	sz;
{
	register char *va;
	register pfn;

	if (sz <= 0 || sz > NPGPT)
		return(0);
	if ((pfn = ptmalloc(sptmap, sz, NPGPT-1)) == 0)
		return(0);

	reglock(&sysreg);
	memlock();

	if (availrmem - sz < tune.t_minarmem  ||
	   availsmem - sz < tune.t_minasmem) {
		mfree(sptmap, sz, pfn);
		memunlock();
		regrele(&sysreg);
		nomemmsg("kseg", sz, 0, 0);
		return(0);
	}
	availrmem -= sz;
	availsmem -= sz;

	if (ptmemall(&sysreg, pfntokptbl(pfn), sz, 1, 0))
		cmn_err(CE_PANIC, "kseg - ptmemall failed");

	memunlock();
	regrele(&sysreg);
	va = (char *) ctob(pfn);
	ksegtbl[snum(va - (char *)syssegs)] = sz;
	bzero(va, ctob(sz));
	return(va);
}

/* Free previously allocated kernel segment.
 */

unkseg(va)
register char	*va;
{
	register sz, pfn, sn;

	pfn = pnum(va);
	sn = snum(va - (char *)syssegs);
	sz = ksegtbl[sn];
	reglock(&sysreg);
	memlock();
	pfree(&sysreg, pfntokptbl(pfn), NULL, sz);
	availrmem += sz;
	availsmem += sz;
	flushmmu(va, sz);
	memunlock();
	regrele(&sysreg);
	mfree(sptmap, sz, pfn);
	ksegtbl[sn] = 0;
}


/*	This routine flushes the mmu for a range of addresses.
*/

flushmmu(va, size)
register caddr_t	va;	/* First virtual address to flush.	*/
register int		size;	/* Number of pages to flush.		*/
{
	register int	i;
	for (i = 0  ;  i < size  ;  i++)
		flushaddr(va + ctob(i));
}


