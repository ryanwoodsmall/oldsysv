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
/* @(#)startup.c	1.7 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/page.h"
#include "sys/region.h"
#include "sys/mmu.h"
#include "sys/proc.h"
#include "sys/pfdat.h"
#include "sys/var.h"
#include "sys/map.h"
#include "sys/tty.h"
#include "sys/utsname.h"
#include "sys/debug.h"


struct pfdat	*pfhead, *pfdat;

#define NDEVSEG	4
short dev_seg[NDEVSEG] = {
	0xa0,	/* sio */
	0xd0,	/* disk, tape */
	0xe0,	/* monitor */
	0xff,	/* cpu board devices */
};

seg_t		**kpte1;
spte_t		*kpte2;
seg_t		**upte1;
spte_t		*upte2;
seg_t		*zeropage;
spte_t		*copypte;
caddr_t		copyvad;
spte_t		*mmpte;
caddr_t		mmvad;
spte_t		*pmapte;
caddr_t		pmavad;
struct user	*u2;
seg_t		**u2pte1;
spte_t		*u2pte2;
spte_t		*u3pte2;

extern int	maxpmem;	/* Configured physical memory limit */

extern int	etext;
extern int	end;

/*
 * Each process has a level one page table (1K) at p_addr.
 * When the process is running, two entries in kpte1 point to it;
 * thus it is at kernal virtual address upte1.
 * To the kernal, this page is part of the level 2 kernal page table.
 * Mainly, this page points to user page tables, but we also steal
 * entries to point to the U block and to the page itself.
 */

/*
 * Ualloc allocates the level 1 page table and ublock.
 * Most entries point to zeropage.
 * The exceptions are 1 entry to point to itself, and
 * USIZE entries to point to the ublock.
 */

ualloc()
{
	pte_t t[1+USIZE];
	register i, adr;
	register pte_t *pp;

	for (i=0; i<1+USIZE; i++)
		t[i].pgi.pg_pte = 0;
	sptmemall(t, 1+USIZE);
	for (i=0; i<1+USIZE; i++) {
		t[i].pgm.pg_prot = PTE_KW;
		t[i].pgm.pg_prot2 = PTE_KW;
	}
	/* map page 1 */
	copypte[0] = t[0];
	invsatb(SYSATB, copyvad);
	pp = (pte_t *)copyvad;

	for (i=0; i<NPG_PG; i++) {
		pp[i].pgi.pg_pte = (int)zeropage;
		pp[i].pgi.pg_pte2 = (int)zeropage;
	}

	pp[SELF_P] = t[0];
	for (i=0; i<USIZE; i++)
		pp[UDOT_P + i] = t[1 + i];

	return(t[0].pgm.pg_pfn);
}

/*
 * Ufree frees up the level 1 page table and ublock.
 */

ufree(adr)
{
	pte_t t[1+USIZE];
	register i;
	register pte_t *pp;

	u2load(adr);
	pp = (pte_t *)u2pte1;
	for (i=0; i<USIZE; i++)
		t[i] = pp[UDOT_P + i];
	t[USIZE] = pp[SELF_P];
	pfree(NULL, t, NULL, 1+USIZE);
}

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
vadrspace(lastaddr)
{
	register i, p;
	register pte_t *pp;
	int nkptp, ubase, j;

	icuinit();
	icuenable(0x200); /* console recv intr */
	physmem = btop(lastaddr);
	if (maxpmem && maxpmem < physmem)
		physmem = maxpmem;

	/*
	 * Number of pages needed for system page table to map
	 * entire physical memory into system space.
	 */
	nkptp = physmem/NPG_PG;

	kpte1 = (seg_t **)ptob(btop(&end));
	kpte2 = (spte_t *)(kpte1 + NL1_SEG);
	upte2 = (pte_t *)stob(U1_SEG);
	upte1 = (seg_t **)((int)upte2 + ptob(SELF_P));
	u2 = (struct user *)((int)&u + stob(2));
	u2pte2 = (pte_t *)stob(U1_SEG+2);
	u2pte1 = (seg_t **)((int)u2pte2 + ptob(SELF_P));
	u3pte2 = (pte_t *)stob(U1_SEG+4);

	/*
	 * Setup level 1 system page table
	 */

	for (i=0; i<(nkptp*NSG_PG); i++)
		kpte1[i] = (seg_t *)((int)&kpte2[i*NPGPT] | PG_KW | PG_V);

	/*
	 * Setup text system page table entries (RO)
	 * Setup system data and bss and page table itself (RW)
	 */

	for (i=0; i < btop(&etext); i++) {
		p = ptob(i) | PG_KR | PG_V;
		kpte2[i].pgi.pg_pte = p;
		kpte2[i].pgi.pg_pte2 = PG_PE2(p);
	}
	for ( ; i < btotp(kpte2)+nkptp; i++) {
		p = ptob(i) | PG_KW | PG_V;
		kpte2[i].pgi.pg_pte = p;
		kpte2[i].pgi.pg_pte2 = PG_PE2(p);
	}
	mfree(sptmap, nkptp*NPG_PG - i, i);

	/*
	 * Map the devices so that virtual addr == physical addr
	 * and allocate a zero segment page
	 */

	pp = (pte_t *)ptob(i);
	for (i=0; i<NDEVSEG; i++) {
		kpte1[dev_seg[i]] = (seg_t *)((int)pp | PG_KW | PG_V);
		for (j=0; j<NPGPT; j++) {
			p = stob(dev_seg[i]) | ptob(j) | PG_KW | PG_V;
			pp->pgi.pg_pte = p;
			pp->pgi.pg_pte2 = PG_PE2(p);
			pp++;
		}
	}
	zeropage = (seg_t *)((int)pp | 0x1f);
	pp += NPGPT;
	ubase = btop(pp);

	/*
	 * Turn on the MMU
	 */

	lmr(PTB0, kpte1);
	lmr(MSR, MS_TU|MS_TS|MS_DS);

	/*
	 * Allocate physical memory for dynamic tables 
	 */

	ubase = mktables(ubase);

	/*
	 * Initialize queue of free pages
	 */
	meminit(ubase, physmem);

	/*
	 * Virtual address space to assist physical page to physical page 
	 * copy
	 */
	copyvad = (caddr_t)sptalloc(2, 0, -1);
	copypte = svtopte(copyvad);

	/*
	 * Virtual address space for memory driver
	 */
	mmvad = (caddr_t)sptalloc(1, 0, -1);
	mmpte = svtopte(mmvad);

	/*
	 * Virtual address space for general disk driver
	 */
	pmavad = (caddr_t)sptalloc(1, 0, -1);
	pmapte = svtopte(pmavad);

	reginit();
	p0init();
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
p0init()
{
	proc[0].p_addr = ualloc();
	uload(proc[0].p_addr);
	u.u_procp = &proc[0];

#	ifdef LOCKLOGON
	p0init_done = 1;
#	endif
}

/*
 * uload - load the new process's ublock, etc.
 */
uload(adr)
{
	register x;

	x = mkpte(PG_V|PG_KW, adr);
	kpte1[U1_SEG] = (seg_t *)x;
	kpte1[U1_SEG+1] = (seg_t *)PG_PE2(x);

	lmr(PTB0, kpte1);
	lmr(PTB1, adr<<PG_PNSH);
}

/*
 * u2load - load a 2nd process's ublock
 * used for fork and exit processing
 */
u2load(adr)
{
	register x;

	x = mkpte(PG_V|PG_KW, adr);
	kpte1[U1_SEG+2] = (seg_t *)x;
	kpte1[U1_SEG+3] = (seg_t *)PG_PE2(x);

	lmr(PTB0, kpte1);
}

/*
 * u3load - load a 3rd process's ublock
 * used for device handlers
 */
u3load(adr)
{
	register x;

	x = mkpte(PG_V|PG_KW, adr);
	kpte1[U1_SEG+4] = (seg_t *)x;
	kpte1[U1_SEG+5] = (seg_t *)PG_PE2(x);

	lmr(PTB0, kpte1);
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
	ffguess = btop(&end);
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
