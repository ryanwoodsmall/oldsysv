/* @(#)probe.c	1.1 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/mtpr.h>
#include <sys/sysmacros.h>
#include <sys/dir.h>
#include <sys/signal.h>
#include <sys/user.h>
#include <sys/page.h>
#include <sys/region.h>
#include <sys/proc.h>
#include <sys/text.h>
#include <sys/systm.h>
#include <sys/sysinfo.h>
#include <sys/file.h>
#include <sys/inode.h>
#include <sys/buf.h>
#include <sys/var.h>
#include <sys/ipc.h>
#include <sys/errno.h>

pte_t	*useracc(vaddr, length, code)
register caddr_t vaddr;
register int length;
{
	register int i, ptlen;
	register pte_t *pt;
	register pte_t *save_pt;
	int	olen;
	static char seg[]={P0LR, P1LR, SLR, 0};

	/*
	 * off end of page table
	 */
	if (((int)(pt = mapa(vaddr))) >= 0) {
		return((pte_t *)0);
	}
	save_pt = pt;
	i = (((int)vaddr) >> 30)&3;
	if (i == 1) 
		ptlen = (-(((int)vaddr) >> BPPSHIFT)) & 0x1fffff;
	else {
		ptlen = mfpr(seg[i]);
		ptlen -= (((int)vaddr) >> BPPSHIFT) & 0x1fffff;
	}

	length = ((length - 1 + (int)vaddr) >> BPPSHIFT) - (((int)vaddr) >> BPPSHIFT) + 1;
	olen = length;
	while (--length >= 0) {
		/*
		 * off end of page table
		 */
		if (--ptlen < 0) 
			return((pte_t *)0);
		/*
		 * read memory
		 */
		if ((code&B_READ) == 0) {
			if ((i = pt->pgm.pg_prot) != PTE_UR && i != 4 && i<12) 
				return((pte_t *)0);
		} else {
			/*
			 * write memory
			 */
			if (pt->pgm.pg_prot != PTE_UW) {
				if (pt->pgm.pg_cw) {
					/*
					 * if physio, then break CW;
					 * adaptor cannot do it
					 */
					if (code&B_PHYS) {
						*vaddr = *vaddr;
					}
				} else 
					return((pte_t *)0);
			}
		}
		++pt;
		vaddr += NBPP;
	}

	/*
	 * lock pages for physio
	 */
	if (code&B_PHYS) {
		length = olen;
		pt = save_pt;
		vaddr -= length * NBPP;
		while (--length >= 0) {
			/*
			 * cause page fault
			 */
			while (!pt_isacc(pt))
				i = *vaddr;
			/*
			 * and lock in working set
			 */
			pt->pgm.pg_lock = PT_LOCK; 
			++pt;
			vaddr += NBPP;
		}
	}

	return(save_pt);
}
