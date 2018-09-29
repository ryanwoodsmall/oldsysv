/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/main.c	10.7"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/psw.h"
#include "sys/sbd.h"
#include "sys/sysmacros.h"
#include "sys/pcb.h"
#include "sys/immu.h"
#include "sys/systm.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/mount.h"
#include "sys/inode.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/var.h"
#include "sys/debug.h"
#include "sys/conf.h"
#include "sys/cmn_err.h"

int	physmem;	/* Physical memory size in clicks.	*/
int	maxmem;		/* Maximum available memory in clicks.	*/
int	freemem;	/* Current available memory in clicks.	*/
inode_t	*rootdir;


extern int	icode[];
extern int	szicode;
extern int	userstack[];

/*
 *	Initialization code.
 *	fork - process 0 to schedule
 *	     - process 1 execute bootstrap
 *
 *	loop at low address in user mode -- /etc/init
 *	cannot be executed.
 */

main()
{
	register int	(**initptr)();
	extern int	(*io_init[])();
	extern int	(*init_tbl[])();
	extern int	(*io_start[])();
	extern int	sched();
	extern int	vhand();
	extern int	bdflush();
	extern int	mappodflag;

	startup();
	clkstart();
	dmainit();


	/*	Call all system initialization functions.
	*/

	for (initptr= &io_init[0]; *initptr; initptr++) (**initptr)();
	for (initptr= &init_tbl[0]; *initptr; initptr++) (**initptr)();
	for (initptr= &io_start[0]; *initptr; initptr++) (**initptr)();


	cmn_err(CE_CONT,
		"Available memory   = %d\n\n", ctob(freemem));

	prt_where = PRW_CONS;

	printf("***********************************************************************\n\n");
	printf("Copyright (c) 1984 AT&T - All Rights Reserved\n\n");
	printf("THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T INC.\n");
	printf("The copyright notice above does not evidence any actual or\n");
	printf("intended publication of such source code.\n\n");
	printf("***********************************************************************\n\n");


	u.u_start = time;

	/*	This call of swapadd must come after devinit in case
	 *	swap is moved by devinit.  It must also come after
	 *	dskinit so that the disk is don'ed.  This call should
	 *	return 0 since the first slot of swaptab should be used.
	 */

	if (swapdev != NODEV)
		if (swapadd(swapdev, swplo, nswap) != 0)
			cmn_err(CE_PANIC, "main - swapadd failed");

	/*
	 * make init process
	 * enter scheduling loop
	 * with system process
	 */

	if (newproc(0)) {
		register preg_t	*prp;
		register reg_t	*rp;
		register int	npgs;
		register sde_t	*sp;
		u.u_cstime = u.u_stime = u.u_cutime = u.u_utime = 0;

		/*	Set up the text region to do an exec
		**	of /etc/init.  The "icode" is in misc.s.
		*/

		rp = allocreg(NULL, RT_PRIVATE, 0);
		prp = attachreg(rp, &u, UVTEXT, PT_TEXT, SEG_RW);
		npgs = btoc(szicode);
		growreg(prp, npgs, DBD_DFILL);
		loadmmu(u.u_procp, SCN2);
		regrele(rp);

		if (copyout((caddr_t)icode, (caddr_t)(UVTEXT), szicode))
			cmn_err(CE_PANIC, "main - copyout of icode failed");

		/*	The following software workaround is for 
		**	the WE32000 which does not have the
		**	capability of getting write and execute
		**	access to a segment at the same time.
		**	The kernel normally wants to read and
		**	write the user's text but in this case
		**	we must execute the user's text in
		**	kernel mode because it is going to
		**	change %pcbp to switch to user mode.
		**	Therefore, we must change the mode of the
		**	text segment.
		*/

		if (!is32b()) {
			/*	We need a loop here if szicode
			**	ever gets larger than a segment.
			*/

			sp = (sde_t *)u.u_procp->p_srama[SCN2 - SCN2];
			sp[snum(UVTEXT)].seg_prot = KRE | URE;
			flushmmu(UVTEXT, btoc(szicode));
		} else {
			/*	Give kernel RWE and user RE access.
			*/

			chgprot(prp, SEG_RO);
		}

		/*	Allocate a stack region and grow it to
		**	SSIZE pages.
		*/

		rp = allocreg(NULL, RT_PRIVATE, 0);
		prp = attachreg(rp, &u, userstack, PT_STACK, SEG_RW);
		growreg(prp, SSIZE, DBD_DFILL);
		loadmmu(u.u_procp, SCN3);
		u.u_pcb.sub = (int *)((uint)userstack + ctob(SSIZE));
		regrele(rp);
		return(UVTEXT);
	}
	if (newproc(0)) {
		u.u_procp->p_flag |= SLOAD|SSYS;
		u.u_cstime = u.u_stime = u.u_cutime = u.u_utime = 0;
		bcopy("vhand", u.u_psargs, 6);
		bcopy("vhand", u.u_comm, 5);
		if(mappodflag)
			mappod(u.u_procp);
		return((int)vhand);
	}

	if (newproc(0)) {
		u.u_procp->p_flag |= SLOAD|SSYS;
		u.u_cstime = u.u_stime = u.u_cutime = u.u_utime = 0;
		bcopy("bdflush", u.u_psargs, 8);
		bcopy("bdflush", u.u_comm, 7);
		if(mappodflag)
			mappod(u.u_procp);
		return((int)bdflush);
	}
	bcopy("sched", u.u_psargs, 6);
	bcopy("sched", u.u_comm, 5);
	if(mappodflag)
		mappod(u.u_procp);
	return((int)sched);
}


