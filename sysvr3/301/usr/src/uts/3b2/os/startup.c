/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/startup.c	10.10"
#include "sys/param.h"
#include "sys/types.h"
#include "sys/psw.h"
#include "sys/boot.h"
#include "sys/sbd.h"
#include "sys/firmware.h"
#include "sys/sysmacros.h"
#include "sys/immu.h"
#include "sys/nvram.h"
#include "sys/pcb.h"
#include "sys/systm.h"
#include "sys/pfdat.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/map.h"
#include "sys/buf.h"
#include "sys/reg.h"
#include "sys/utsname.h"
#include "sys/tty.h"
#include "sys/var.h"
#include "sys/debug.h"
#include "sys/cmn_err.h"


/*	The following define fields in the firmware vector
**	table which is mapped into the kernel's virtual
**	address space at VBASE.
**
**		VSIZOFMEM	Size of mainstore in bytes.
*/

#define	VSIZOFMEM	(*VBASE->p_memsize)

/*	The following page table is used for the first
**	segment of the kernel's virtual space, that is,
**	the gate table.  This is result of the hardware
**	implementation.  It seems that when writing the
**	result of an SPOP instruction, write protection
**	doesn't work.  You can write anywhere that you
**	have read permission.  
*/

int	gateptbl[NPGPT + 8];
int	*gateptbl0;


/* Routine called before main, to initialize various data structures.

/* Routine called before main, to initialize various data structures.
** The arguments are:
**	physclick - The click number of the first available page
**		    of memory.
*/

mlsetup(physclick)
int	physclick;
{
	register unsigned	nextclick;
	register unsigned	i;
	register int		*pde;
	register sde_t		*sde;
	extern int		sbss[];		/* Start of kernel bss. */
	extern int		bssSIZE[];	/* Size of bss in bytes.*/
	extern int		firstfree;	/* First free click.	*/
	extern int		maxfree;	/* Last free click + 1.	*/
	extern char		u400;		/* Set for 3b2/400.	*/


	/*	Reinitialize the data area to support
	**	booting from an absolute file which
	**	was written after the system was
	**	initialized.
	*/

	datainit();

	/*	Zero all BSS
	*/

	bzero(sbss, bssSIZE);

	/*	Set up the page table for segment 0.  First
	**	get the physical addresses.  Then build the
	**	page table.  Note that a page table must
	**	be aligned on a 32 byte boundary.
	*/

	nextclick = btoc(svirtophys(0));
	pde = gateptbl;
	pde = (int *)(((int)pde + 31) & ~0x1f);
	gateptbl0 = pde;

	for(i = 0  ;  i < NPGPT  ;  i++, nextclick++)
		pde[i] = mkpde(PG_P | PG_COPYW, nextclick);
	pde[i-1] |= PG_LAST;
	sde = (sde_t *)srama[0];
	ASSERT(sde->seg_flags & SDE_C_bit);
	sde->wd2.address = svirtophys(pde);
	sde->seg_flags &= ~SDE_C_bit;
	flushmmu(0, NPGPT);

	if (fast32b())
		cache_on();
	else
		cache_off();

	/*	No user page tables available yet.
	 */
	
	ptfree.pf_next = &ptfree;
	ptfree.pf_prev = &ptfree;

	/*	Set up memory parameters.
	*/

	i = btoct(VSIZOFMEM);
	if (v.v_maxpmem  &&  (v.v_maxpmem < i))
		i = v.v_maxpmem;
	physmem = i;
	maxclick = btoc(MAINSTORE) + i;
	firstfree = physclick;
	maxfree = maxclick;

	ASSERT(physclick < maxclick);

	/*	Zero all unused physical memory.
	**	Note that all physical memory is
	**	mapped to the indentical address
	**	in section 0 of the kernel's virtual
	**	address space.
	*/

	bzero(ctob(physclick), ctob(maxclick - physclick));

	/*	Initialize memory mapping for sysseg,
	**	the segment from which the kernel
	**	dynamically allocates space for itself.
	*/

	nextclick = sysseginit(physclick);

	/*	Initialize the kernel window segments.
	*/

	windowinit();

	/*	Initialize the map used to allocate
	**	kernel virtual space.
	*/

	mapinit(sptmap, v.v_sptmap);
	mfree(sptmap, SYSSEGSZ, btoc(syssegs));

	/*	Note that reginit must be called before
	**	mktables because reginit calculates the
	**	value of pregpp which is used by mktables.
	*/

	reginit();
	
	/*	Allocate some kernel tables.
	*/

	nextclick = mktables(nextclick);

	/*	Initialize the physical memory manager.
	*/

	kpbase = ctob(nextclick);
	meminit(nextclick, maxclick);

	/*	Initialize process 0.
	*/

	p0init();
}


/*	The unix object file is written by a user program
**	(mkunix) after the system is booted.  Therefore,
**	the data are has been modified from its initial
**	state.  This routine saves the initial state
**	on the first (config) boot and restores it
**	on all subsequent boots.
*/

datainit()
{
	static int		first_time = 1;
	static struct var	copyv;
	extern int		mpid;
	extern int		mappodflag;

	if (first_time)
		copyv = v;
	else
	{
		v = copyv;
		mpid = 0;
	}
	first_time = 0;
	mappodflag = 0;
}


/*	Allocate page tables for the kernel segment sysseg and
**	and initialize the segment table for it.
*/

sysseginit(physclick)
register int	physclick;
{
	register int		pgcnt;
	register sde_t		*sptr;
	register char		*ptptr;
	sde_t			sdeproto;

	/*	Set up a prototype with the permissions
	**	and flags we want.
	*/

	sdeproto.seg_prot  = KRWE;
	sdeproto.seg_len   = ctomo(NCPS);
	sdeproto.seg_flags = SDE_V_bit | SDE_P_bit;

	/*	Now loop through all of the segment table
	**	entries for sysseg and initialize them
	**	to point to the correct kernel page
	**	tables.
	*/

	sptr = kvtokstbl(syssegs);
	ptptr = (char *)ctob(physclick);

	for (pgcnt = 0  ;  pgcnt < SYSSEGSZ ; pgcnt += NPGPT, sptr++) {
		*sptr = sdeproto;
		sptr->wd2.address = (int)ptptr;
		ptptr += sizeof(ptbl_t);
	}

	/*	Set the address of the kernel page table.
	**	We are actually using a physical address.
	**	This works since all of physical memory
	**	is mapped 1-to-1 into the kernel's virtual
	**	address space.
	*/

	kptbl = (pde_t *)ctob(physclick);

	return(btoc(ptptr));
}

/*	Initialize the kernel window segments.
*/

windowinit()
{
	register sde_t	*sptr;

	/*	Initialize the u-block window used to access a
	**	u-block other than that of the currently running
	**	process.
	*/

	sptr = kvtokstbl(win_ublk);
	sptr->seg_prot = KRWE;
	sptr->seg_len = ctomo(USIZE);
	sptr->seg_flags = SDE_V_bit | SDE_P_bit;
}

/*
 * Create system space to hold page allocation and
 * buffer mapping structures and hash tables
 */

mktables(nextfree)
{
	register int	m;
	register int	i;
	register preg_t	*prp;
	extern int	pregpp;

	/*	Allocate space for the pfdat.
	*/

	i = btoc((maxclick - nextfree) * sizeof(struct pfdat));
	pfdat = ((struct pfdat *)sptalloc(i, PG_P, nextfree, 0));
	nextfree +=i;
	
	/*	Compute the smallest power of two larger than
	 *	the size of physical memory.
	 */

	m = physmem;
	while (m & (m - 1))
		 m = (m | (m - 1)) + 1;
	phashmask = (m>>3) - 1;

	/*	Allocate space for the page hash bucket
	 *	headers.
	 */

	i = btoc((m >> 3) * sizeof(*phash));
	phash = ((struct pfdat **)sptalloc(i, PG_P, nextfree, 0));
	nextfree +=i;

	/*	Allocate space for the pregion tables for each process
	 *	and link them to the process table entries.
	 *	The maximum number of regions allowed for is process is
	 *	3 for text, data, and stack plus the maximum number
	 *	of shared memory regions allowed.
	 */
	
	i = btoc(pregpp * sizeof(preg_t) * v.v_proc);
	prp = (preg_t *)sptalloc(i, PG_P, nextfree, 0);
	nextfree +=i;
	for (i = 0  ;  i < v.v_proc  ;  i++, prp += pregpp)
		proc[i].p_region = prp;
	
	/*	Return next available physical page.
	*/

	return(nextfree);
}

/*	The following pcb is used to initialize the u_kpcb
**	field of the u-block.  This is so that the CALLPS
**	done in ttrap.s to switch to the kernel stack for
**	a system call will go off to the system call
**	handler, systrap, in trap.c
*/

extern int	systrap();

struct kpcb	kpcb_syscall = {

			/*	The initial PSW with CM = kernel,
			**	PM = user, and the I and R bits
			**	set.  This is followed by the
			**	initial PC and initial SP.
			*/
			
			{{0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 1, 1, 0, 0, 0},
			systrap,
			0},

			/*	Save area for PSW, PC, and SP.
			*/

			ZPSW, 0, 0,

			/*	Stack lower bound and stack upper
			**	bound.
			*/

			0,
			(int *)&u + ctob(USIZE) / sizeof(int),

			/*	Savearea for ap, fp, r0 - r10.
			*/

			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

			/*	The following indicates no block
			**	moves.
			*/

			0,
	} ;


/*	Set up proc0
*/

psw_t	p0init_psw = IPSW;

p0init()
{
	register int	ii;
	register pde_t	*pdptr;
	extern int	userstack[];

	/*	Fix up the syscall pcb.
	*/

	kpcb_syscall.ipcb.sp = u.u_stack;
	kpcb_syscall.slb = u.u_stack;

	/*	Initialize proc 0's ublock
	*/

	u.u_pcb.psw = p0init_psw;
	u.u_pcb.sp  = userstack;
	u.u_pcb.pc  = (int (*)())UVTEXT;
	u.u_pcb.slb = (int *)UVBASE;
	u.u_pcb.sub = 0;
	u.u_pcb.regsave[K_AP] = (unsigned) userstack;
	u.u_pcb.regsave[K_FP] = (unsigned) userstack;
	u.u_tsize   = 0;
	u.u_dsize   = 0;
	u.u_ssize   = 0;
	u.u_ar0     = &u.u_pcb.regsave[K_R0];
	u.u_kpcb    = kpcb_syscall;
	u.u_procp   = &proc[0];
	u.u_cmask   = CMASK;
	u.u_limit   = CDLIMIT;

	/*	initialize process data structures
	*/

	curproc = &proc[0];
	curpri = 0;

	proc[0].p_srama[SCN2 - SCN2] = NULL;
	((int *)proc[0].p_sramb)[SCN2 - SCN2] = -1;

	proc[0].p_srama[SCN3 - SCN2] = srama[3];
	((int *)proc[0].p_sramb)[SCN3 - SCN2] = NS3SDE - 1;
	proc[0].p_size = USIZE;
	proc[0].p_stat = SRUN;
	proc[0].p_flag = SLOAD | SSYS;
	proc[0].p_nice = NZERO;

	pdptr = ubptbl(&proc[0]);
	for (ii = 0 ; ii < USIZE ; ii++, pdptr++)
		pdptr->pgi.pg_pde =
			mkpde(PG_P, btoc(svirtophys((char *)&u + ctob(ii))));
}

/*	Machine-dependent startup code
*/

startup()
{
	char		*nvrsysname;
	char		sysname[9];
	extern char	*release;
	extern char	*version;
	extern int	nofiles_min;
	extern int	nofiles_max;
	extern int	nofiles_cfg;

	/*	Check to see if the configured value of NOFILES
	**	was illegal and changed by vstart_s.  If so,
	**	print a message.  We do this here because
	**	vstart_s is in assembly language and hard to
	**	print from.
	*/

	if (nofiles_cfg != v.v_nofiles) {
		if (nofiles_cfg < nofiles_min)
			cmn_err(CE_NOTE,
				"Configured value of NOFILES (%d) is less than min (%d)",
				  nofiles_cfg, nofiles_min);
		else
			cmn_err(CE_NOTE,
				"Configured value of NOFILES (%d) is greater than max (%d)",
				  nofiles_cfg, nofiles_max);
		cmn_err(CE_CONT, "        NOFILES set to %d.", v.v_nofiles);

	}
	
	/* retrieve system name from nvram */

	nvrsysname = (char *) &(UNX_NVR->sys_name[0]);
	rnvram(nvrsysname, sysname, 9);
	if (sysname[0] != '\0') {
		str8cpy(utsname.sysname, sysname);
		str8cpy(utsname.nodename, sysname);
	}

	/*	Get the release and version of the system.
	*/

	if (release[0] != '\0')
		str8cpy(utsname.release, release);
	if (version[0] != '\0')
		str8cpy(utsname.version, version);


	cmn_err(CE_CONT,
		"\nUNIX System V Release %s AT&T %s Version %s\n",
		utsname.release, utsname.machine, utsname.version);
	cmn_err(CE_CONT,
		"Node %s\n", utsname.nodename);
	cmn_err(CE_CONT,
		"Total real memory  = %d\n", ctob(physmem));

	machinit();
	devinit();
}

/*
 * Adjust for appropriate boot options.
 */

machinit()
{
}

/*
 * Adjust devs based on boot options
 */

devinit()
{
	/*	Fix up the devices if we were booted
	**	from floppy disk.
	*/

	if (VBASE->p_cmdqueue->b_dev == FLOPDISK) {
		rootdev = makedev(emajor(rootdev), FLOPMINOR);
		pipedev = rootdev;
		swapdev = NODEV;
		nswap   = 0;
	}
}

