/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/swtch.c	10.9"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/psw.h"
#include "sys/sysmacros.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/immu.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/var.h"
#include "sys/errno.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/debug.h"
#include "sys/inline.h"

#define	SWTCH() \
{\
	asm("	MOVAW	kpcb_pswtch,%r0"); \
	asm("	CALLPS"); \
}

extern int	userstack[];
extern proc_t	*panicproc;

/*
 * put the current process on
 * the Q of running processes and
 * call the scheduler. (called by trap.c)
 */
qswtch()
{

	setrq(u.u_procp);
	SWTCH();
}


/*
 * This routine is called to reschedule the CPU.
 * if the calling process is not in RUN state,
 * arrangements for it to restart must have
 * been made elsewhere, usually by calling via sleep.
 * There is a race here. A process may become
 * ready after it has been examined.
 * In this case, idle() will be called and
 * will return in at most 1HZ time.
 * i.e. its not worth putting an spl() in.
 */

char		runrun;
int		switching;
extern int	mau_present;
extern		runqueues();
extern char	qrunflag;

pswtch()
{
	register struct proc *p, *old_curproc;
	struct proc *disp();

	switching = 1;
	sysinfo.pswitch++;
	old_curproc = p = curproc;

	switch (p->p_stat) {
		case SZOMB:

			/*	Free up remaining memory used by the
			 *	zombie process here.  This is just
			 *	the u-block and the sdt's.
			 */

			ubfree(p);
			break;

		case SONPROC:
			ASSERT(p->p_wchan == 0);
			p->p_stat = SRUN;
			break;
	}
loop:
	splhi();

	/*
	 * Search for highest-priority runnable process
	 * If no process is runnable, idle.
	 */
	runrun = 0;
	if ((p = disp()) == NULL) {
		if (qrunflag)
			runqueues();
		curpri = PIDLE;
		curproc = &proc[0];
		idle();
		goto loop;
	}

	
	/* switch context only if really process switching */
	if (curproc != old_curproc) {
		if (mau_present)
			mau_save();
		/* initialize MMU SRAMS for chosen process */
		loadmmu(p, SCN2);
		loadmmu(p, SCN3);
		if (mau_present)
			mau_restore();
	}
	spl1();

	if (u.u_pcbp == &u.u_pcb
	  && ((p->p_flag & SPRSTOP) || (p->p_sig && issig()))) {
		extern int	s_trap();

		u.u_pcbp = (pcb_t *)&u.u_kpcb;
		u.u_kpcb.ipcb.pc = s_trap;
	}
	switching = 0;
}
