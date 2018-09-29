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
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/proc.h"
#include "sys/reg.h"
#include "sys/psl.h"
#include "sys/trap.h"
#include "sys/mmu.h"
#include "sys/seg.h"
#include "sys/sysinfo.h"

#define	USER	040		/* user-mode flag added to type */

/*
 * Called from the trap handler when a processor trap occurs.
 */
trap(r3, r2, r1, sp, r0, type, pc, psr_mod)
register type;
{
	register i;
	register time_t syst;
	int kludge;	/* make registers end up same as syscall.c */
	extern kdebug;
	/* if you change the local vars, change reg.h */

	syst = u.u_stime;
	u.u_ar0 = (int *)((int)&u + ptob(USIZE) - 4*4);
	u.u_fpsaved = 0;
	if (USERMODE(psr_mod))
		type |= USER;
	switch (type) {

	/*
	 * Trap not expected.
	 * Usually a kernel mode bus error.
	 */
	default:
		panicstr = "trap";	/* fake it for printfs */
		printf("proc = ");
		printf("%x ", u.u_procp);
		printf("\n");
		printf("psr_mod = %x\n", psr_mod);
		printf("pc = %x\n", pc);
		printf("msr = %x\n", smr(MSR));
		printf("eia = %x\n", smr(EIA));
		printf("trap type %x\n", type);
		if (kdebug) {
			asm("	bpt");
		}
		panic(NULL);

	case NMIFLT:		/* non-maskable interrupt */
	case NMIFLT + USER:
		nmi(pc, psr_mod);
		/* no return */

	case ILLFLT + USER:	/* illegal operation trap */
	case UNDFLT + USER:	/* undefined instruction trap */
		i = SIGILL;
		break;

	case SYSCALL + USER:	/* sys call */
		panic("syscall");
	calcpri:
	{
		register struct proc *pp;

		pp = u.u_procp;
		pp->p_pri = (pp->p_cpu>>1) + PUSER + pp->p_nice - NZERO;
		curpri = pp->p_pri;
		if (runrun == 0)
			goto out;
	}

	case RESCHED + USER:	/* Allow process switch */
		qswtch();
		goto out;

	case FPUFLT + USER:
	case DVZFLT + USER:
		i = SIGFPE;
		break;

	/*
	 * If the user SP is below the stack segment,
	 * grow the stack automatically.
	 */
	case SEGFLT + USER:	/* mmu abort */
		i = smr(EIA) & 0xffffff;
		if (grow(sp) || grow(i))
			goto calcpri;
		i = SIGSEGV;
		break;

	case PROTFLT + USER:
		i = SIGBUS;
		break;

	case BPTFLT + USER:	/* bpt instruction fault */
	case TRCTRAP + USER:	/* trace trap */
		psr_mod &= ~PS_T;	/* turn off trace bit */
		i = SIGTRAP;
		break;

	case FLGFLT + USER:	/* flag trap */
		i = SIGEMT;
		break;

	case PGRDFLT + USER:	/* read error during page fault */
		i = SIGKILL;
		printf("pid %d killed due to page fault read error\n",
			u.u_procp->p_pid);
		break;

	case ASTTRAP + USER:	/* user level profiling */
		addupc((caddr_t) pc, &u.u_prof, 1);
		return;

	}
	psignal(u.u_procp, i);

out:
	if (u.u_procp->p_sig && issig())
		psig();
	if (u.u_prof.pr_scale && syst != u.u_stime)
		addupc((caddr_t)pc, &u.u_prof, (int)(u.u_stime-syst));
	if (u.u_fpsaved)
		restfp(u.u_fps);
}
/*
 *	Analyze the cause of a NMI delivery.
 *	This code is for the Nat. Semi. SYS32 only.
 */

nmi(pc,psl)
{
	register e;
	register unsigned esr0,esr1;
	register int emb;

	spl7();
	e = *(short *)0xFFF200;
	e &= 7;
	printf("\nmachine check: pc=%x ps=%x nmi_status=%x * ",pc,psl,e);
	sync();

	if (e == 1) {	/* PFINT */
		printf("POWER\n");
	}
	else if (e == 0) {
		printf("UNKNOWN\n");
	}
	else {
		if (e & 1) printf("POWER ");
		if (e & 2) printf("MEMORY ");
		if (e & 4) printf("EXTERNAL ");
		printf("\n");

		/* and try to read ESRs from all three boards */
		for(emb=1;emb<4;emb++) {
			switch (emb) {
			case 1:
				/* first read P1632 ESR location */
				esr0 = 0xF0FFFFFF;
				esr1 = * (short *) 0xFFE808;
				/* if bit9 != 0, must be M1000 board */
				if ((esr1 & 0x0200) != 0) { 
					esr0 = * (short *) 0xFFE840;
					esr1 = * (short *) 0xFFE880;
				}
				break;
			case 2:
				/* first read P1632 ESR location */
				esr0 = 0xF0FFFFFF;
				esr1 = * (short *) 0xFFE608;
				/* if bit9 != 0, must be M1000 board */
				if ((esr1 & 0x0200) != 0) { 
					esr0 = * (short *) 0xFFE640;
					esr1 = * (short *) 0xFFE680;
				}
				break;
			case 3:
				/* first read P1632 ESR location */
				esr0 = 0xF0FFFFFF;
				esr1 = * (short *) 0xFFE408;
				/* if bit9 != 0, must be M1000 board */
				if ((esr1 & 0x0200) != 0) { 
					esr0 = * (short *) 0xFFE440;
					esr1 = * (short *) 0xFFE480;
				}
				break;
			}
			if (esr0 == 0xF0FFFFFF) { /* for P1632 boards */
			  printf("ESR%x=%x [force_parity=%x bank=%x byte=%x]\n",
			  emb, (esr1 & 0x3FF), ((esr1>>8)& 1), ((esr1>>4)& 0xF),
			  (esr1 & 0xF));
			}
			else { /* for M1000 boards */
				esr1=((esr1 & 0xFFFF)<<16)|(esr0 & 0xFFFF);
		        	printf(
		    "ESR%x=%x%x [overflow=%x error=%x syndrome=%x addr=%x]\n",	
		    emb, ((esr1>>4)& 0xFFFFFFF),(esr1 & 0xF),
		    ((esr1>>31)& 1), ((esr1>>30)& 1), ((esr1>>24)& 0x3F),
		    (esr1 & 0xFFFFFF));
			}
		}
	}
	panic("machine check");
}
