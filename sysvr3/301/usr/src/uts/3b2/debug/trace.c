/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:debug/trace.c	10.2"
#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/immu.h>
#include	<sys/region.h>
#include	<sys/proc.h>
#include	<sys/signal.h>
#include	<sys/fs/s5dir.h>
#include	<sys/psw.h>
#include	<sys/pcb.h>
#include	<sys/user.h>
#include	<sys/cmn_err.h>
#include	<sys/sys3b.h>
#include	<sys/inline.h>

char	tracebuf[200];
int	my_stack[256];
char	*s3blookup();
int	strcmp();
proc_t	*paddr;
pcb_t	*pcbaddr;
int	t_ap, t_fp, t_pc, t_sp;

trace()
{
	register int oldsp;
	register pcb_t **isp;
	register int oldap, oldfp;
	int s;

	asm("	PUSHW  %r0");
	asm("	PUSHW  %r1");
	asm("	PUSHW  %r2");

	/*
		Since we switch u area behind demon's and UNIX's back, we
		have to make sure no interrupts occur since pcbp may not
		be pointing to any place meaningful
	*/

	s = spl7();
	asm("	MOVW	%sp,%r8");
	asm("	MOVW	%isp,%r7");
	asm("	MOVW	%ap,%r6");
	asm("	MOVW	%fp,%r5");

	/*
		Trace must use its own stack since we may be called running
		on a kernel in some u area but since we change u areas to
		the process we're going to trace, we can't use any kernel
		stacks (it must be guaranteed that "my_stack" is not 
		allocated in section 3, the u area section).
	*/

	asm("	MOVAW	my_stack,%sp");
	asm("	MOVAW	my_stack,%fp");
	asm("	MOVAW	my_stack,%ap");

	/*
		Did we get into demon via a ^P?  If we did and we're
		trying to trace the process that was running, then
		the pcb address for it is pointed to by a word on
		the isp.
	*/

	if (paddr->p_stat == SONPROC && strcmp(s3blookup(oldsp), "kstkD") == 0) 
		pcbaddr = *(isp - 1);

	/*
		Otherwise the process we're tracing was not the current process
		UNIX was running so its pcb info is pointed to by u.u_pcbp
	*/


	else 
		pcbaddr = u.u_pcbp;

	tracepcb();

	asm("	MOVW	%r8,%sp");	/* restore old stack */
	asm("	MOVW	%r6,%ap");
	asm("	MOVW	%r5,%fp");
	splx(s);
	asm("	POPW  %r2");
	asm("	POPW  %r1");
	asm("	POPW  %r0");
}

tracereg()
{
	pcb_t	fake_pcb;
	int	oldpri;

	asm("	PUSHW  %r0");
	asm("	PUSHW  %r1");
	asm("	PUSHW  %r2");

	oldpri = splhi();
	fake_pcb.pc = (int (*)())t_pc;
	fake_pcb.sp = (int *)t_sp;
	fake_pcb.regsave[K_AP] = t_ap;
	fake_pcb.regsave[K_FP] = t_fp;

	pcbaddr = &fake_pcb;
	tracepcb();
	splx(oldpri);
	asm("	POPW  %r2");
	asm("	POPW  %r1");
	asm("	POPW  %r0");
}


tracepcb()
{
	int		delay;
	extern int	dmd_delay;
	register int ap, fp, sp, pc;
	int *oldpcptr;
	int *argp;
	int first;
	char *s3bsp;
	uint srama_save;
	SRAMB sramb_save;
	int	oldpri;

	asm("	PUSHW  %r0");
	asm("	PUSHW  %r1");
	asm("	PUSHW  %r2");

	if (pcbaddr == &u.u_pcb) {
		cmn_err(CE_CONT, "^Can't trace processes in user mode\n");
		goto out;
	}

	oldpri = splhi();
	srama_save = srama[SCN3];
	sramb_save = sramb[SCN3];

	loadmmu(paddr, 3);
	ap = pcbaddr->regsave[K_AP];
	fp = pcbaddr->regsave[K_FP];
	pc = (int)pcbaddr->pc;
	sp = (int)pcbaddr->sp;
	cmn_err(CE_CONT, "^    AP		    FP		    PC		Function\n\n");

	while (sp > (int)pcbaddr->slb && sp < (int)pcbaddr->sub) {
		s3bsp = s3blookup(pc);

		/*
			Special case:  If the symbol is "nrmx_KK", then
			we took a fault in kernel mode.  So we want to
			step over the stuff on the stack left by the
			exception handler and pick up the PC on the
			stack that tells us where the fault occurred
		*/

		if (strcmp(s3bsp, "nrmx_KK") == 0) {
			pc = *((int *)(sp) - 4);
			sp = (int)((int *)ap + 2);
			s3bsp = s3blookup(pc);
		}

		sprintf(tracebuf,"%#x	%#x	%#x	%s(", ap, fp, pc, s3bsp);
		
		if (fp > ap) 
			oldpcptr = (int *)(fp - 9 * sizeof(char *));
		else
			oldpcptr = (int *)(sp - 2 * sizeof(char *));

		for (argp = (int * )ap, first = 1; argp < oldpcptr; argp++) {
			if (first != 1)
				sprintf(tracebuf, "%s, ", tracebuf);

			sprintf(tracebuf, "%s%#x", tracebuf, *argp);
			first = 0;
		}
	
		cmn_err(CE_CONT, "^%s)\n", tracebuf);

		if (dmd_delay) {
			delay = dmd_delay;
			while(delay--) ;
		}

		if (ap > sp)	/* then we ran into residue of callps */
			break;

		pc = *oldpcptr++;
		sp = ap;
		ap = *oldpcptr++;

		if (fp > sp)
			fp = *oldpcptr;
	}


	srama[SCN3] = srama_save;
	sramb[SCN3] = sramb_save;
	splx(oldpri);

out:
	asm("	POPW  %r2");
	asm("	POPW  %r1");
	asm("	POPW  %r0");
}

printsp()
{
	register int sp;

	asm("	MOVW	%sp,%r8");
	cmn_err(CE_CONT, "^sp = %x\n", sp);
}

printpc()
{
	asm("	PUSHW	%r0");
	asm("	PUSHW	%r1");
	asm("	PUSHW	%r2");

	cmn_err(CE_CONT, "^%x\n", u.u_pcbp->pc);

	asm("	POPW	%r2");
	asm("	POPW	%r1");
	asm("	POPW	%r0");
}
