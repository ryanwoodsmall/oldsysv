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
/* @(#)main.c	6.6 */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/sysmacros.h>
#include <sys/systm.h>
#include <sys/dir.h>
#include <sys/signal.h>
#include <sys/user.h>
#include <sys/filsys.h>
#include <sys/mount.h>
#include <sys/page.h>
#include <sys/inode.h>
#include <sys/region.h>
#include <sys/proc.h>
#include <sys/conf.h>
#include <sys/buf.h>
#include <sys/var.h>
#include <sys/ipc.h>
#include <sys/seg.h>


struct inode *rootdir;
int	maxmem, physmem;
int	maxumem;

/*
 * Initialization code.
 * Called from cold start routine as soon as a stack and segmentation
 * have been established.
 * Proc 0 has been partially setup to accomplish above.
 *
 * Functions:
 *	machine dependent device initialization
 *	set process 0 proc[0].p_ and u.u_ values
 *	call all initialization routines
 *	fork - process 0 to schedule
 *	     - process 1 execute bootstrap
 *	     - process 2 execute pageout (vhand)
 */

main()
{
	register int (**initptr)();
	extern int (*init_tbl[])();
	extern icode[], szicode;
	register int i;

	startup();

	printf("\nCopyright (c) 1984 AT&T Technologies, Inc.\n");
	printf("	All Rights Reserved\n\n");
	printf("THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T TECHNOLOGIES, INC.\n");
	printf("The copyright notice above does not evidence any actual or intended\n");
	printf("publication of such source code.\n\n");

	/*
	 * set up system process
	 */

	curproc = &proc[0];

	proc[0].p_stat = SRUN;
	proc[0].p_flag |= SLOAD|SSYS;
	proc[0].p_nice = NZERO;

	u.u_cmask = CMASK;
	u.u_limit = CDLIMIT;
	u.u_rdir = NULL;

	/*
	 * initialize system tables and resources
	 */

	for (initptr= &init_tbl[0]; *initptr; initptr++) {
		(**initptr)();
	}
	msginit();
	seminit();

	/*
	 * get root inode for proc 0 and others
	 */
	rootdir = iget(rootdev, ROOTINO);
	rootdir->i_flag &= ~ILOCK;
	u.u_cdir = iget(rootdev, ROOTINO);
	u.u_cdir->i_flag &= ~ILOCK;
	u.u_start = time;

	/*
	 *	This call should return 0 since the first slot
	 *	of swaptab should be used.
	 */

	if(swapadd(swapdev, swplo, nswap) != 0)
		panic("startup - swapadd failed");


	/*
	 * create initial processes
	 * start scheduling task
	 */

	if (newproc(0)) {
		register preg_t *prp;
		register reg_t *rp;

		rp = allocreg(NULL, RT_PRIVATE);
		prp = attachreg(rp, &u, 0, PT_DATA, SEG_RW);
		growreg(&u, prp, btop(szicode), btop(szicode), DBD_DZERO);
		regrele(rp);
		copyout((caddr_t)icode, (caddr_t)0, szicode);
		return(0);
	}

	if (newproc(0)) {
		register struct proc *p;

		maxmem -= (USIZE + 1 + 1);
		p = u.u_procp;
		p->p_flag |= SLOAD|SSYS;
		p->p_pid = 0;
		vhand();
		/* never returns */
	}
	sched();
	/* FOR LINT */
	return (0);
}
