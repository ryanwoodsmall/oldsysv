/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:nudnix/netboot.c	10.21"
/*  System call to start file sharing.
 */

#include "sys/types.h"
#include "sys/sema.h"
#include "sys/param.h"
#include "sys/fs/s5dir.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/immu.h"
#include "sys/stream.h"
#include "sys/comm.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/inode.h"
#include "sys/mount.h"
#include "sys/message.h"
#include "sys/var.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/adv.h"
#include "sys/nserve.h"
#include "sys/recover.h"
#include "sys/debug.h"
#include "sys/rdebug.h"

#define p_minwdlock p_trlock

mblk_t	*server_bp = NULL;		/* server stream buffers */
int	nserverbuf = 1;			/* number of 2K server stream buffers */

extern	int	bootstate;		/*  DU_UP, DU_DOWN, or DU_INTER */
extern	struct	proc *rfsdp;
extern	struct	inode *rfs_cdir;
extern	struct	proc *rec_proc;		/* sleep address for recovery */
extern	int	rec_flag;		/* set KILL bit to kill recovery */

extern comminit(), gdp_init(), auth_init();
extern void commdinit(), recover_init();

rfstart ()
{
	csect_t	s;			/*  for csect  */
	struct	proc	*p;		/*  for changing proc sysid's	*/
	extern int gdp_init ();
	register int	i;
	register mblk_t	*bp;

	DUPRINT1(DB_RFSTART, "rfstart system call\n");

	if (!suser())
		return;

	while (bootstate == DU_INTER) {
		sleep(&bootstate, PREMOTE);
	}
	
	/* This is a critical section. Only one process at a time 
	 * can execute this code.
	 */
	
	if (bootstate == DU_UP)  {
		DUPRINT1 (DB_RFSTART, "rfstart: system already booted\n");
		u.u_error = EEXIST;
		wakeup(&bootstate);
		return;
	}

	bootstate = DU_INTER;	/* RFS in an intermediate state */

	if (comminit() == FAILURE) {
		u.u_error = EAGAIN;
		bootstate = DU_DOWN;
		wakeup(&bootstate);
		return;
	}
	DUPRINT1(DB_RFSTART,"comm initialized\n");
	auth_init();
	gdp_init();
	if (u.u_error) {
		DUPRINT2 (DB_RFSTART, "rfstart u.u_error %d\n",u.u_error);
		commdinit();
		bootstate=DU_DOWN;
		wakeup(&bootstate);
		return;
	}
	DUPRINT1(DB_RFSTART,"gdp initialized\n");
	recover_init();

	/* allocate maxserve number of 2K stream buffers for server usage */
	for (i = 0; i < nserverbuf; i++) {
		if ((bp = allocb (sizeof(struct message) + sizeof(struct response), BPRI_MED)) == NULL) {
			/* fail to get enough stream buffers for server usage,
			   fail the RFS startup and free stream buffers */
			while (bp = server_bp) {
				server_bp = bp->b_next;
				freemsg(bp);
			}
			printf("WARNING: not enough stream buffers for RFS, RFS failed\n");
			u.u_error = ENOMEM;
			commdinit();
			bootstate = DU_DOWN;
			wakeup(&bootstate);
			return;
		}

		bp->b_wptr += sizeof(struct message);
		((struct message *)bp->b_rptr)->m_stat = 0;
		bp->b_next = server_bp;
		server_bp = bp;
	}

	/* start recover process */
	switch (newproc (1)) {
		case 0:
			break;
		case 1:
			u.u_procp->p_flag |= SSYS;
			u.u_cstime = u.u_stime = u.u_cutime = u.u_utime = 0;
			rec_proc = u.u_procp;
			netmemfree();
			bcopy ("recovery", u.u_comm, 8);
			bcopy ("recovery", u.u_psargs, 8);
			recovery ();
			return;
		default:
			DUPRINT1 (DB_RFSTART,"rfstart: cannot fork recovery\n");
			/* free stream buffers */
			while (bp = server_bp) {
				server_bp = bp->b_next;
				freemsg(bp);
			}
			commdinit();
			bootstate=DU_DOWN;
			wakeup(&bootstate);
			return;
	}

	/* start daemon process */
	switch (newproc (1))  {
		case 0:
			/* newproc sets u.u_rval1 to the child's pid,	*/
			/* but we don't want to return it, so clear it.	*/
			u.u_rval1 = 0;
			break;
		case 1:
			u.u_procp->p_flag |= SSYS;
			u.u_cstime = u.u_stime = u.u_cutime = u.u_utime = 0;
			rfsdp = u.u_procp;
			netmemfree();
			rfs_cdir = u.u_cdir;
			bcopy ("rfdaemon", u.u_comm, 8);
			bcopy ("rfdaemon", u.u_psargs, 8);
			rfdaemon ();
			return;
		default:
			DUPRINT1 (DB_RFSTART,"rfstart: cannot fork rfdaemon\n");
			/* free stream buffers */
			while (bp = server_bp) {
				server_bp = bp->b_next;
				freemsg(bp);
			}
			commdinit();
			rec_flag |= RFSKILL;
			wakeup (&rec_proc);
			return;
	}

	/*  now allow advertise calls, set all sysid's in proc table  */
	s = csect();
	for (p = &proc[0]; p < (struct proc *) v.ve_proc; p++)
		p->p_sysid = 0;
	ecsect(s);

	/* bootstate set when rfdaemon is running -- see rfadmin.c */
	return;
 }

netmemfree()
{
/* Try to release user memory for these kernel procs.
 * (We really need to put in code to free the regions,
 * iput the current and root directories, and close all the files.)
 */

	register struct proc *p;
	register preg_t *prp;
	register reg_t *rp;
	struct inode *ip;
	struct inode *iget();
	int i;
	
	p = u.u_procp;
	prp = p->p_region;
	while(rp = prp->p_reg) {
		if(ip = rp->r_iptr)
			plock(ip);
		reglock(rp);
		detachreg(prp,&u);
	}
	plock(u.u_cdir);
	iput(u.u_cdir);
	u.u_cdir = iget(&mount[0], (mount[0].m_mount)->i_number);
	prele(u.u_cdir);
	if (u.u_rdir) {
		plock(u.u_rdir);
		iput(u.u_rdir);
	}
	u.u_rdir = NULL;
	for (i = 0; i < NOFILE; i++)
		if (u.u_ofile[i] != NULL) {
			closef(u.u_ofile[i]);
			u.u_ofile[i] = NULL;
		}
}


/*
 *	System call to stop file sharing - if everything is quiet.
 */

rfstop()
{
	register struct mount *mp;
	register struct srmnt	*sp;
	register struct advertise *ap;
	register mblk_t	*bp;

	extern	int	nadvertise;
	extern	int	nsrmount;
	extern	int	nsndd;
	extern	int	msgflag;
	extern	rcvd_t	rd_recover;

	DUPRINT1(DB_RFSTART, "rfstop system call\n");

	if (!suser())
		return;

	/* Begin critical section.  As in rfstart, only one process at a time
	 * through this section of code.
	 */
	while (bootstate == DU_INTER) {
		sleep(&bootstate, PREMOTE);
	}

	if (bootstate == DU_DOWN) {
		DUPRINT1(DB_RFSTART, "rfstop: system already stopped\n");
		u.u_error = ENONET;
		wakeup(&bootstate);
		return;
	}

	bootstate = DU_INTER;

	/* can't stop if anything is remotely mounted  */
	/* (starting at mount[1] means we're not looking at root) */

	/* If any local proc has an SD (cdir, rdir, open file) that points
	 * into a remotely mounted filesystem, it will not be possible to
	 * unmount that filesystem.  (See rumount.)
	 */
	for (mp = &mount[1]; mp < (struct mount *)v.ve_mount; mp++) {
		if ((mp->m_flags & MINUSE) && (mp->m_inodp->i_flag & IRMOUNT)) {
			DUPRINT1 (DB_RFSTART,
				"rfstop: can't stop with remote mounts.\n");
			u.u_error = EBUSY;
			bootstate=DU_UP;
			wakeup(&bootstate);
			return;
		}
	}

	/* can't stop if this machine has clients */
	for (sp = srmount; sp < &srmount[nsrmount]; sp++) {
		if (sp->sr_flags != MFREE) {
			DUPRINT1 (DB_RFSTART,
				"rfstop: can't stop with clients.\n");
			u.u_error = ESRMNT;
			bootstate=DU_UP;
			wakeup(&bootstate);
			return;
		}
	}

	/* can't stop if anything is advertised  */
	/* adv table locked above */
	for (ap = advertise; ap < &advertise[nadvertise]; ap++) {
		if (ap->a_flags != A_FREE) {
			DUPRINT1 (DB_RFSTART,
			   "rfstop: can't stop with advertised resources.\n");
			u.u_error = EADV;
			bootstate=DU_UP;
			wakeup(&bootstate);
			return;
		}
	}

	DUPRINT1(DB_RFSTART, "rfstop: taking down links \n");
	kill_gdp();		/* cut all connections */
	commdinit();

	/* free server stream buffers */
	while (bp = server_bp) {
		server_bp = bp->b_next;
		freemsg(bp);
	}

	/* kill daemons - bootstate goes to DOWN after both die */
	DUPRINT1(DB_RFSTART, "rfstop: killing daemons \n");
	msgflag |= RFSKILL;
	wakeup (&rd_recover->rd_qslp);
	rec_flag |= RFSKILL;
	wakeup (&rec_proc);

	DUPRINT1(DB_RFSTART, "rfstop: done\n");
}
