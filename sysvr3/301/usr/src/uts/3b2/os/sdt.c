/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/sdt.c	10.7"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/immu.h"
#include "sys/systm.h"
#include "sys/sysmacros.h"
#include "sys/pfdat.h"
#include "sys/signal.h"
#include "sys/fs/s5dir.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/inode.h"
#include "sys/var.h"
#include "sys/buf.h"
#include "sys/debug.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/cmn_err.h"

chkstbl(procp, prp, rgsize)
struct proc	*procp;
preg_t		*prp;
register int	rgsize; /* region size in pages */
{
	register int	otseg, ntseg;
	register int 	len;
	int		size;
	int 		sid;
	union {
		unsigned intvar;
		VAR vaddr;
	}		virtaddr;

	/* get sdt table limit */

	virtaddr.intvar = (uint)(prp->p_regva);
	sid = virtaddr.vaddr.v_sid;
	otseg = virtaddr.vaddr.v_ssl;
	if (sid==3 && otseg==0) /* overlap with u-block */
		return(-1);

	switch (sid) {
		case 2:
		case 3:
			size = *((int *)&procp->p_sramb[sid - SCN2]);
			if (size == -1)
				len = 0;
			else
				len = procp->p_sramb[sid - SCN2].SDTlen +1; 
			break;
		default:
			return(-1);
	}

	/*
	 *	ntseg == segment id of first segment beyond region
	 */

	ntseg = 1 +  snum(prp->p_regva + ctob(rgsize));
	if (ntseg > len) {
		if (growsdt(procp, sid, ntseg, NOSLEEP)<0)
				return(-1);
	}
	return(0);
}

/*
 * loadstbl(up, prp, change)
 *
 * Change the content of a process's segment descriptor entries:
 *    	change > 0 -> load sdt table using the entries of prp->r_list
 *	change = 0 -> load sdt table using prp->r_list.
 *	change < 0 -> invalidate affected sdt entries. 
 *
 */
loadstbl(up, prp, change)
user_t		*up;
preg_t		*prp;
register int	change;
{
	register reg_t	*rp;
	register sde_t	*st;
	register pde_t	**lp, **lplim;
	register int	otseg, ntseg, stseg;
	register int 	len;
	proc_t		*procp;
	sde_t		*tabtop;
	int	 	prot;
	int		pgsize;
	int 	sid, i;
	union {
		unsigned intvar;
		VAR vaddr;
	}		virtaddr;

	/* get sdt table limit */

	procp = up->u_procp;
	virtaddr.intvar = (uint)(prp->p_regva);
	sid = virtaddr.vaddr.v_sid;


	switch (sid) {
		case 2:
		case 3:
			tabtop = (sde_t *) phystokv(procp->p_srama[sid - SCN2]);
			if (*((int *)&procp->p_sramb[sid - SCN2]) == -1)
				len = 0;
			else
				len = procp->p_sramb[sid - SCN2].SDTlen + 1;
			break;
		default:
			cmn_err(CE_PANIC,
				"loadstbl - bad section id");
	}

	rp = prp->p_reg;
	pgsize = rp->r_pgsz;

	/*
	 *	stseg =  segment id of first segment in region
	 *	otseg == segment id of first segment entry to modify
	 *	ntseg == segment id of first segment beyond expanded region
	 */

	 stseg = virtaddr.vaddr.v_ssl;
	 virtaddr.intvar += ctob(pgsize+change);
	 ntseg = sgnum(virtaddr.intvar + NBPS -1);

	if (ntseg > len)
		cmn_err(CE_PANIC, "loadstbl - segment table too short.");

	if (change < 0) {
		otseg = 1 + snum(prp->p_regva + ctob(pgsize));
		for (i = ntseg; i < otseg ;++i)
			sde_clrvalid(tabtop+i);
		if ((change + pgsize) == 0) {
			sde_clrvalid(tabtop+stseg);
			return;
		}
		st = tabtop + ntseg;
	} else {
		prot = (prp->p_flags & PF_RDONLY) ? SEG_RO: SEG_RW;
		st = tabtop + stseg;
		lp = rp->r_list;
		lplim = &lp[ctos(rp->r_pgsz + change)];

		if (lplim == lp)
			return;

		for (; lp < lplim ; lp++, st++) {
			st->wd2.address =  svirtophys(*lp);
			st->seg_prot = prot;
			st->seg_flags = SDE_flags;
			st->seg_len = (stob(1)>>3) -1;
		}
	}

	/* adjust the length of the last entry */
	pgsize = virtaddr.vaddr.v_psl;
	--st;
	st->seg_len = (ctob(pgsize)>>3) -1;
}

/*
 *  growsdt(procp, sid, nentries, slpflg)
 *
 *  sid = section id 
 *  nentries = number of entries in the new SDT[sid] table
 *
 *  slpflg&NOSLEEP!=0 -> don't wait for pages
 */

growsdt(procp, sid, nentries, slpflag)
register proc_t *procp;
register sid, nentries;
{
	register otseg, ntseg;
	register osize, nsize;
	register int top, newtop;
	register int i;

	if ((nentries>MAXSDTSEG) || (nentries<0))
		return(-1);
	top = phystokv(procp->p_srama[sid - SCN2]);
	if (*(int *)&procp->p_sramb[sid - SCN2] == -1)
		otseg = 0;
	else
		otseg = procp->p_sramb[sid - SCN2].SDTlen + 1;

	/* nsize = size of the new SDT table in page tables
	 * osize = size of the old SDT table in page tables
   	 */

	nsize = btopt(nentries<<3);
	osize = btopt(otseg << 3);

	ntseg = pttob(nsize)>>3;
	if (nsize <= osize) {
		for (i=nentries; i<ntseg; ++i)
			sde_clrvalid((sde_t *)top+i);
		uptfree(top+pttob(nsize), osize-nsize);

		/* update SDT size in the proc table */

		if (nentries == 0)
			*(int *)&procp->p_sramb[sid - SCN2] = -1;
		else
			*(int *)&procp->p_sramb[sid - SCN2] =
				(nentries - 1) << SRAMBSHIFT;
	} else {
		/* allocate new SDT table */

		if ((newtop = (int) ptalloc(nsize,PHYSCONTIG|slpflag))==NULL)
			return(-1);

		/* copy entries */

		if (otseg>0)
			bcopy(top,newtop,(2*otseg)<<2);
		for (i=otseg; i<ntseg; ++i)
			sde_clrvalid((sde_t *)newtop+i);

		/* free old SDT table */

		if (osize != 0  && procp->p_pid)
			uptfree(top, osize);

		/* update SDT addr and size in the proc table */

		procp->p_srama[sid - SCN2] = kvtophys(newtop);
		*(int *)&procp->p_sramb[sid - SCN2] =
			(nentries - 1) << SRAMBSHIFT;

	}
	return(0);
}

