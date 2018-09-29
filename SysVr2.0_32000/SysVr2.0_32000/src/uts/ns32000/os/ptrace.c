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
/* @(#)ptrace.c	1.2 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/page.h"
#include <sys/region.h>
#include "sys/proc.h"
#include "sys/inode.h"
#include "sys/file.h"
#include "sys/reg.h"
#include "sys/text.h"
#include "sys/seg.h"
#include "sys/var.h"
#include "sys/psl.h"
#include "sys/pfdat.h"
/*
 * Priority for tracing
 */
#define	IPCPRI	PZERO

/*
 * Tracing variables.
 * Used to pass trace command from
 * parent to child being traced.
 * This data base cannot be
 * shared and is locked
 * per user.
 */
struct
{
	int	ip_lock;
	int	ip_req;
	int	*ip_addr;
	int	ip_data;
} ipc;

/*
 * Enter the tracing STOP state.
 * In this state, the parent is
 * informed and the process is able to
 * receive commands from the parent.
 */
stop()
{
	register struct proc *pp;
	register struct proc *cp;
	register preg_t	     *prp;

	/* 	Put the region sizes into the u-block for the
	 *	parent to look at if he wishes.
	 */

	pp = u.u_procp;
	if (prp = findpreg(pp, PT_TEXT))
		u.u_tsize = prp->p_reg->r_pgsz;
	else	
		u.u_tsize = 0;
	if (prp = findpreg(pp, PT_DATA))
		u.u_dsize = prp->p_reg->r_pgsz;
	else
		u.u_dsize = 0;
	if (prp = findpreg(pp, PT_STACK))
		u.u_ssize = prp->p_reg->r_pgsz;
	else
		u.u_ssize = 0;
loop:
	cp = u.u_procp;
	if (cp->p_ppid != 1)
	for (pp = &proc[0]; pp < (struct proc *)v.ve_proc; pp++)
		if (pp->p_pid == cp->p_ppid) {
			wakeup((caddr_t)pp);
			cp->p_stat = SSTOP;
			swtch();
			if ((cp->p_flag&STRC)==0 || procxmt())
				return;
			goto loop;
		}
	exit(fsig(u.u_procp->p_sig));
}

/*
 * sys-trace system call.
 */
ptrace()
{
	register struct proc *p;
	register struct a {
		int	req;
		int	pid;
		int	*addr;
		int	data;
	} *uap;

	uap = (struct a *)u.u_ap;
	if (uap->req <= 0) {
		u.u_procp->p_flag |= STRC;
		return;
	}
	for (p=proc; p < (struct proc *)v.ve_proc; p++) 
		if (p->p_stat==SSTOP
		 && p->p_pid==uap->pid
		 && p->p_ppid==u.u_procp->p_pid)
			goto found;
	u.u_error = ESRCH;
	return;

    found:
	while (ipc.ip_lock)
		sleep((caddr_t)&ipc, IPCPRI);
	ipc.ip_lock = p->p_pid;
	ipc.ip_data = uap->data;
	ipc.ip_addr = uap->addr;
	ipc.ip_req = uap->req;
	p->p_flag &= ~SWTED;
	setrun(p);
	while (ipc.ip_req > 0)
		sleep((caddr_t)&ipc, IPCPRI);
	u.u_rval1 = ipc.ip_data;
	if (ipc.ip_req < 0)
		u.u_error = EIO;
	ipc.ip_lock = 0;
	wakeup((caddr_t)&ipc);
}

int ipcreg[] = {R0, R1, R2, R3, R4, R5, R6, R7, FP, SP, PC};
/*
 * Code that the child process
 * executes to implement the command
 * of the parent process in tracing.
 */
procxmt()
{
	register int i;
	register *p;

	if (ipc.ip_lock != u.u_procp->p_pid)
		return(0);
	i = ipc.ip_req;
	ipc.ip_req = 0;

	switch (i) {

	/* read user I */
	case 1:
		ipc.ip_data = fuiword((caddr_t)ipc.ip_addr);
		break;

	/* read user D */
	case 2:
		ipc.ip_data = fuword((caddr_t)ipc.ip_addr);
		break;

	/* read u */
	case 3:
		i = (int)ipc.ip_addr;
		if (i<0 || i >= ptob(USIZE))
			goto error;
		ipc.ip_data = ((physadr)&u)->r[i>>2];
		break;

	/* write user I */
	/* Must set up to allow writing */
	case 4: {
		register preg_t *prp;

		prp = findpreg(u.u_procp, PT_TEXT);
		if (prp && (prp->p_flags & PF_RDONLY)) {
			if (prp->p_reg->r_refcnt != 1 ||
			    prp->p_reg->r_flags & RG_NOFREE)
				goto error;
			chgprot(prp, SEG_RW);
		} else
			prp = NULL;
		i = suiword((caddr_t)ipc.ip_addr, ipc.ip_data);
		if (prp)
			chgprot(prp, SEG_RO);
		if (i >= 0) {
			/*
			 * The write worked.  Break the disk association
			 * of the page since it has been modified.
			 * Note: the word can straddle two pages.
			 */
			dirtypage((caddr_t)ipc.ip_addr);
			dirtypage(((caddr_t)ipc.ip_addr)+NBPW-1);
		} else
			goto error;
		break;

	}
	/* write user D */
	case 5:
		if (suword((caddr_t)ipc.ip_addr, 0) < 0)
			goto error;
		suword((caddr_t)ipc.ip_addr, ipc.ip_data);
		break;

	/* write u */
	case 6:
		i = (int)ipc.ip_addr;
		p = (int *)&((physadr)&u)->r[i>>2];
		for (i=0; i<11; i++)
			if (p == &u.u_ar0[ipcreg[i]])
				goto ok;
		if (p == &u.u_ar0[PSR_MOD]) {
			ipc.ip_data |= (PS_I|PS_S|PS_U);
			ipc.ip_data &=  ~PS_P;
			goto ok;
		}
		goto error;

	ok:
		*p = ipc.ip_data;
		break;

	/* set signal and continue */
	/* one version causes a trace-trap */
	case 9:
		u.u_ar0[PSR_MOD] |= PS_T;
	case 7:
		if ((int)ipc.ip_addr != 1)
			u.u_ar0[PC] = (int)ipc.ip_addr;
		u.u_procp->p_sig = 0L;
		if (ipc.ip_data)
			psignal(u.u_procp, ipc.ip_data);
		wakeup((caddr_t)&ipc);
		return(1);

	/* force exit */
	case 8:
		wakeup((caddr_t)&ipc);
		exit(fsig(u.u_procp->p_sig));

	default:
	error:
		ipc.ip_req = -1;
	}
	wakeup((caddr_t)&ipc);
	return(0);
}

/*
 * Break the disk association of a page based on the
 * virtual address for the current running process.
 * Note: code assumes address is legit.
 */
dirtypage(vaddr)
register caddr_t vaddr;
{
	register tmp;
	register pfd_t *pfd;
	register dbd_t *dbd;
	register preg_t *prp;
	register pte_t *pt;

	prp = findreg(u.u_procp, vaddr);
	tmp = btotp(vaddr - prp->p_regva);
	pt = &prp->p_reg->r_list[tmp/NPGPT][tmp%NPGPT];
	dbd = (dbd_t *)pt + NPGPT;
	pfd = &pfdat[pt->pgm.pg_pfn];
	if (pfd->pf_flags & P_HASH)
		premove(pfd);
	if (dbd->dbd_type == DBD_SWAP)
		swfree1(dbd);
	dbd->dbd_type = DBD_NONE;
}
