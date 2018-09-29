/* @(#)startup.c	1.7 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/page.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/seg.h"
#include "sys/map.h"
#include "sys/reg.h"
#include "sys/psl.h"
#include "sys/utsname.h"
#include "sys/mtpr.h"
#include "sys/clock.h"
#include "sys/ipc.h"
#include "sys/shm.h"
#include "sys/acct.h"
#include "sys/tty.h"
#include "sys/var.h"
#include "sys/pfdat.h"
#include "sys/buf.h"
#include "sys/debug.h"


struct pfdat	*pfhead, *pfdat;

int		*sbrpte;
pt_t		*baseseg;
spte_t		*copypte;
caddr_t		copyvad;
spte_t		*mmpte;
caddr_t		mmvad;
spte_t		*pmapte;
caddr_t		pmavad;
caddr_t		memcvad;
spte_t		*userpte;
struct user	*uservad;

extern int	maxpmem;	/* Configured physical memory limit */

extern int	etext;
extern int	end;
extern int	sigcode();

/*
 * Initially, running in physical memory without memory map and 
 * using interrupt stack.
 *
 * Allocate system virtual address space and setup virtual windows.
 *
 * Dedicate memory for boot time tables and thread rest on free list.
 *
 * Create address space for proc 0.  After vadrspace returns, we switch
 * to the hardware process context of proc 0.
 */
vadrspace(lastaddr, startpc)
{
	register i;
	register spte_t *sbr;
	int	nspte, ubase;

	if (maxpmem && maxpmem < btop(lastaddr))
		physmem = maxpmem;
	else
		physmem = btop(lastaddr);

	/*
	 * Number of pages needed for system page table to map
	 * entire physical memory into system space.
	 */
	nspte = physmem/NPGPT;

	/* we need a lot of slop to deal with that fact that one
	 * physical page often has several system virtual addresses
	 */
	nspte = 2 * nspte;
	sbr = (spte_t *)&end;
	sbrpte = (int *) sbr;
	baseseg = (pt_t *) 0x80000000;

	/*
	 * Setup System Page Table immediately after operating system
	 */

	mtpr(SBR, (int)sbr - SYSVA);
	mtpr(SLR, nspte*NPGPT);

	/*
	 * Setup text system page table entries (RO)
	 * Setup system data and bss and page table itself (RW)
	 */

	for (i = 0; i < btop((int)(&etext) - SYSVA); i++)
		sbr++->pgi.pg_pte = PG_V | PG_KR | i;
	for ( ; i < (btop((int)(&end) - SYSVA) + nspte); i++)
		sbr++->pgi.pg_pte = PG_V | PG_KW | i;

	/*
	 * For VAX allow user process to access system text
	 * for signal catcher
	 * This is only possible because of the overlapping address space.
	 */
	( (int *) sbrpte)[((int)sigcode - SYSVA)>>9] |= PG_URKR;

	/*
	 * Initiailize system page table map
	 * and turn on the MMU
	 */

	mfree(sptmap, nspte*NPGPT - i, i);
	clratb(SYSATB);
	mtpr(MAPEN, 1);

	/*
	 * Allocate physical memory for dynamic tables 
	 */

	ubase = i;
	ubase = mktables(ubase);

	/*
	 * Initialize queue of free pages
	 */
	meminit(ubase, physmem);

	/*
	 * Virtual address space for acessing ublock
	 */
	uservad = (struct user *)sptalloc(btop(sizeof u), 0, -1);
	userpte = (spte_t *)svtopte(uservad);

	/*
	 * Virtual address space to assist physical page to physical page 
	 * copy
	 */
	copyvad = (caddr_t)sptalloc(2, 0, -1);
	copypte = (spte_t *)svtopte(copyvad);

	/*
	 * Virtual address space for memory driver
	 */
	mmvad = (caddr_t)sptalloc(1, 0, -1);
	mmpte = (spte_t *)svtopte(mmvad);

	/*
	 * Virtual address space for general disk driver
	 */
	pmavad = (caddr_t)sptalloc(1, 0, -1);
	pmapte = (spte_t *)svtopte(pmavad);

	/*
	 * Virtual address space for picking up machine errors
	 */
	if (cputype == 780)
		memcvad = (caddr_t)sptalloc(1, PG_V | PG_KW, btop(0x20002000));
	else
		memcvad = (caddr_t)sptalloc(1, PG_V | PG_KW, btop(0xf20000));

	reginit();
	p0init(startpc);
}

/*
 * Create the physical address space for proc 0.  Most proc 0 parameters
 * are initialized in main(), but some must be initialized now.
 * main() is entered on the kernel stack and enough of proc 0 must be
 * set up to have a kernel stack
 */
#ifdef LOCKLOGON
int p0init_done = 0;		/* for region lock logging */
#endif
p0init(startpc)
{
	register pt_t *seg;
	register struct user *useg;
	reg_t *rp;
	preg_t *prp;
	spte_t *spt, *upt, tmp;
	int i;

	/*
	 * Hand craft a P0, P1 and U block for the Scheduler
	 */
	curproc = &proc[0];
	curproc->p_flag = SLOAD|SSYS;
	seg = (pt_t *)sptalloc(1, 0, -1);
	proc[0].p_ptaddr = svtop(seg);
	proc[0].p_ptsize = 1;
	proc[0].p_spt = (int *) seg;
	proc[0].p_nspt = 1;
	mtpr(P0BR, seg);
	mtpr(P0LR, 0);
	proc[0].p_p0size = 0;

	mtpr(P1BR, &seg[1][-REGSZPGS]);
	mtpr(P1LR, REGSZPGS);
	proc[0].p_p1size = 0;
	useg = (struct user *)sptalloc(USIZE, PG_V|PG_KW, 0);
	useg->u_procp = &proc[0];
	useg->u_ssize = 0;

	/*
	 * Handcraft process context for scheduler
	 */
	useg->u_pcb.pcb_ksp = 0x80000000;
	useg->u_pcb.pcb_esp = -1;
	useg->u_pcb.pcb_ssp = -1;
	useg->u_pcb.pcb_usp = (int)&u;
	useg->u_pcb.pcb_p0br = mfpr(P0BR);
	useg->u_pcb.pcb_p0lr = mfpr(P0LR) | (4<<24);
	useg->u_pcb.pcb_p1br = mfpr(P1BR);
	useg->u_pcb.pcb_p1lr = mfpr(P1LR);
	useg->u_pcb.pcb_pc = startpc;
	useg->u_pcb.pcb_psl = 0;

	/*
	 * attach a stack region and place the u area where it belongs
	 */
	rp = allocreg(NULL, RT_PRIVATE);
	prp = attachreg(rp, useg, SYSVA, PT_STACK, SEG_RW);
	if (growreg(useg, prp, USIZE, 0, DBD_DZERO) < 0)
		panic("p0init growreg");
	upt = (spte_t *)&sbrpte[svtop(useg)];
	spt = (spte_t *)&(baseseg[proc[0].p_ptaddr +
					proc[0].p_ptsize][-USIZE]);
	proc[0].p_addr = upt->pgm.pg_pfn;
	for (i=0; i<USIZE; i++) {
		tmp = *spt;
		*spt = *upt;
		spt->pgm.pg_lock = PT_LOCK;
		spt++;
		*upt++ = tmp;
	}
	sptfree(useg, USIZE, 0);
	regrele(rp);
	proc[0].p_p1size = USIZE;
	u.u_ssize = USIZE;
	mtpr(PCBB, ptob(proc[0].p_addr));

#	ifdef LOCKLOGON
	p0init_done = 1;
#	endif
}

/*
 * Create system space to hold page allocation and
 * buffer mapping structures and hash tables.
 */
mktables(physpage)
register int physpage;
{
	register int ffguess;
	register int pagedpages, m;
	register preg_t *prp;
	extern int	pregpp;
	register int i;

	/*
	 * Sizing of data structures to match physical memory
	 * Underestimate
	 */
	ffguess = btop(0x1fffff&(int)(&end));
	pagedpages = (physmem - ffguess);

	/*
	 * Numerical convergence to round to a power of 2
	 */
	m = pagedpages;
	while (m & (m - 1))
		 m = (m | (m - 1)) + 1;

	phashmask = (m>>3) - 1;

	/*
	 * Allocate system space for page structures
	 */
	i = btop(pagedpages*sizeof(*pfdat));
	pfdat = ((struct pfdat *)sptalloc(i, PG_V | PG_KW, physpage)) - ffguess;
	physpage += i;

	/*
	 * Hash table to find buffers
	 */
	i = btop((m>>3) * sizeof(*phash));
	phash = ((struct pfdat *)sptalloc(i, PG_V | PG_KW, physpage));
	physpage += i;

	/*
	 *	Allocate space for the pregion tables for each process
	 *	and link them to the process table entries.
	 *	The maximum number of regions allowed for is process is
	 *	3 for text, data, and stack plus the maximum number
	 *	of shared memory regions allowed.
	 */
	
	i = btop(pregpp * sizeof(preg_t) * v.v_proc);
	prp = (preg_t *)sptalloc(i, PG_V|PG_KW, physpage);
	physpage += i;
	for(i = 0  ;  i < v.v_proc  ;  i++, prp += pregpp)
		proc[i].p_region = prp;


	return(physpage);
}

/*
 * Machine-dependent startup code
 */
startup()
{

	printf("\nUNIX/%s: %s%s\n", utsname.release, utsname.sysname, utsname.version);
	printf("real mem  = %d\n", physmem*ctob(1) );
	printf("avail mem = %d\n", maxmem*ctob(1));
	mbainit();		/* setup mba mapping regs map */
	ubainit();		/* setup uba mapping regs map */
}

/*
 * Initialize clist by freeing all character blocks.
 */
struct chead cfreelist;
cinit()
{
	register n;
	register struct cblock *cp;

	for(n = 0, cp = &cfree[0]; n < v.v_clist; n++, cp++) {
		cp->c_next = cfreelist.c_next;
		cfreelist.c_next = cp;
	}
	cfreelist.c_size = CLSIZE;
}
