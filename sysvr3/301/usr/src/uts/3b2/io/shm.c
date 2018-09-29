/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:io/shm.c	10.10"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/dir.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/ipc.h"
#include "sys/immu.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/systm.h"
#include "sys/sysmacros.h"
#include "sys/shm.h"
#include "sys/debug.h"
#include "sys/tuneable.h"
#include "sys/cmn_err.h"
#include "sys/sbd.h"

extern struct shmid_ds	shmem[];	/* shared memory headers */
extern struct shminfo	shminfo;	/* shared memory info structure */
extern	time_t		time;		/* system idea of date */

struct	shmid_ds	*ipcget(),
			*shmconv();


/*
 * Shmat (attach shared segment) system call.
 */
shmat()
{
	register struct a {
		int	shmid;
		uint	addr;
		int	flag;
	}	*uap = (struct a *)u.u_ap;
	register struct shmid_ds	*sp;
	register struct region		*rp;
	register struct pregion		*prp;

	if ((sp = shmconv(uap->shmid)) == NULL)
		return;
	if (ipcaccess(&sp->shm_perm, SHM_R))
		return;
	if ((uap->flag & SHM_RDONLY) == 0)
		if (ipcaccess(&sp->shm_perm, SHM_W))
			return;
	if (u.u_nshmseg >= shminfo.shmseg) {
		u.u_error = EMFILE;
		return;
	}
	rp = sp->shm_reg;
	reglock(rp);
	if (uap->addr == 0)
		uap->addr = shmaddr();
	else if (uap->flag & SHM_RND)
		uap->addr = uap->addr & ~(SHMLBA - 1);
	
	if (rp->r_pgsz  &&  chkpgrowth(rp->r_pgsz) < 0) {
		regrele(rp);
		u.u_error = ENOMEM;
		return;
	}

	if ((prp = attachreg(rp, &u, uap->addr, PT_SHMEM,
	   uap->flag & SHM_RDONLY? SEG_RO : SEG_RW)) == NULL) {
		regrele(rp);
		return;
	}
	if (sp->shm_perm.mode & SHM_INIT) {
		if (chkpgrowth(btoc(sp->shm_segsz)) < 0  ||
		   growreg(prp, btoc(sp->shm_segsz), DBD_DZERO) < 0) {
			detachreg(prp, &u);
			u.u_error = ENOMEM;
			return;
		}
		sp->shm_perm.mode &= ~SHM_INIT;
	}
	regrele(rp);
	loadmmu(u.u_procp, secnum(uap->addr));
	u.u_rval1 = (int) prp->p_regva;
	u.u_nshmseg++;
	sp->shm_atime = time;
	sp->shm_lpid = u.u_procp->p_pid;
}

/*
 * Convert user supplied shmid into a ptr to the associated
 * shared memory header.
 */
struct shmid_ds *
shmconv(s)
register int	s;	/* shmid */
{
	register struct shmid_ds	*sp;	/* ptr to associated header */

	if (s < 0)
	{
		u.u_error = EINVAL;
		return(NULL);
	}
	sp = &shmem[s % shminfo.shmmni];
	if (!(sp->shm_perm.mode & IPC_ALLOC)  
		|| s / shminfo.shmmni != sp->shm_perm.seq) {
		u.u_error = EINVAL;
		return(NULL);
	}
	return(sp);
}

/*
 * Shmctl system call.
 */
shmctl()
{
	register struct a {
		int		shmid,
				cmd;
		struct shmid_ds	*arg;
	}	*uap = (struct a *)u.u_ap;
	register struct shmid_ds	*sp;	/* shared memory header ptr */
	register struct region		*rp;	/* shmem region */
	struct shmid_ds			ds;	/* hold area for IPC_SET */

	if ((sp = shmconv(uap->shmid)) == NULL)
		return;

	switch (uap->cmd) {

	/* Remove shared memory identifier. */
	case IPC_RMID:
		if (u.u_uid != sp->shm_perm.uid && u.u_uid != sp->shm_perm.cuid
			&& !suser())
			return;
		rp = sp->shm_reg;
		sp->shm_reg = NULL;
		sp->shm_perm.mode = 0;
		sp->shm_segsz = 0;
		if (((int)(++(sp->shm_perm.seq) * shminfo.shmmni + (sp - shmem))) < 0)
			sp->shm_perm.seq = 0;
		reglock(rp);
		if (rp->r_refcnt == 0)
			freereg(rp);
		else {
			rp->r_flags &= ~RG_NOFREE;
			regrele(rp);
		}
		break;

	/* Set ownership and permissions. */
	case IPC_SET:
		if (u.u_uid != sp->shm_perm.uid && u.u_uid != sp->shm_perm.cuid
			 && !suser())
			 return;
		if (copyin(uap->arg, &ds, sizeof(ds))) {
			u.u_error = EFAULT;
			return;
		}
		sp->shm_perm.uid = ds.shm_perm.uid;
		sp->shm_perm.gid = ds.shm_perm.gid;
		sp->shm_perm.mode = (ds.shm_perm.mode & 0777) |
			(sp->shm_perm.mode & ~0777);
		sp->shm_ctime = time;
		break;

	/* Get shared memory data structure. */
	case IPC_STAT:
		if (ipcaccess(&sp->shm_perm, SHM_R))
			return;

		/*	The following is needed because
		**	a user can look at it.  In
		**	particular, the regression tests
		**	require it.
		*/

		sp->shm_nattch = sp->shm_reg->r_refcnt;
		sp->shm_cnattch = sp->shm_nattch;

		if (copyout(sp, uap->arg, sizeof(*sp)))
			u.u_error = EFAULT;
		break;

	/* Lock segment in memory */
	case SHM_LOCK:
		if (!suser())
			return;

		rp = sp->shm_reg;
		reglock(rp);
		ASSERT(rp->r_noswapcnt >= 0);
		if (rp->r_noswapcnt == 0) {
			if (availrmem - rp->r_pgsz < tune.t_minarmem) {
				regrele(rp);
				cmn_err(CE_NOTE,
				   "shmctl - couldn't lock %d pages into memory",
				   rp->r_pgsz);
				u.u_error = ENOMEM;
				return;
			} else {
				availrmem -= rp->r_pgsz;
			}
		}
		++rp->r_noswapcnt;
		regrele(rp);
		break;

	/* Unlock segment */
	case SHM_UNLOCK:
		if (!suser())
			return;

		rp = sp->shm_reg;
		reglock(rp);
		if (rp->r_noswapcnt == 0) {
			/*	User didn't really lock it.
			*/

			regrele(rp);
			break;
		}
		ASSERT(rp->r_noswapcnt > 0);
		if (rp->r_noswapcnt == 1)
			availrmem += rp->r_pgsz;
		--rp->r_noswapcnt;
		regrele(rp);
		break;

	default:
		u.u_error = EINVAL;
	}
}

/*
 * Detach shared memory segment
 */
shmdt()
{
	register struct a {
		caddr_t	addr;
	} *uap = (struct a *)u.u_ap;
	register struct pregion	*prp;
	register struct region	*rp;
	register struct shmid_ds *sp;
	register short	sz;
	caddr_t		regva;

	/*
	 * Find matching shmem address in process region list
	 */

	for (prp = u.u_procp->p_region; prp->p_reg; prp++)
		if (prp->p_type == PT_SHMEM && prp->p_regva == uap->addr)
			break;
	if (prp->p_reg == NULL) {
		u.u_error = EINVAL;
		return;
	}

	/*
	 * Detach region from process
	 * We must remember rp here since detach clobbers p_reg
	 */

	ASSERT(u.u_nshmseg > 0);
	regva = prp->p_regva;
	rp = prp->p_reg;
	sz = rp->r_pgsz;
	reglock(rp);
	detachreg(prp,&u);
	u.u_nshmseg--;

	flushmmu(secnum(regva), sz);

	/*
	 * Find shmem region in system wide table.  Update detach time
	 * and pid, and free if appropriate
	 */
	for (sp = shmem; sp < &shmem[shminfo.shmmni]; sp++)
		if (sp->shm_reg == rp)
			break;
	if (sp >= &shmem[shminfo.shmmni])
		return;	/* shmem has been removed already */
	sp->shm_dtime = time;
	sp->shm_lpid = u.u_procp->p_pid;
}

/*
 * Exec, exit, and fork subroutines not needed any longer
 * There function is implemented gratis by normal region handling
 */
shmexec() { }
shmexit() { }
shmfork() { }

/*
 * Shmget (create new shmem) system call.
 */
shmget()
{
	register struct a {
		key_t	key;
		uint	size,
			shmflg;
	}	*uap = (struct a *)u.u_ap;
	register struct shmid_ds	*sp;	/* shared memory header ptr */
	register struct region		*rp;	/* shared memory region ptr */
	int				s;	/* ipcget status */

	sp = ipcget(uap->key, uap->shmflg,  shmem, shminfo.shmmni,
		    sizeof(*sp), &s);
	if (sp == NULL)
		return;
	if (s) {

		/*
		 * This is a new shared memory segment.
		 * Allocate a region and init shmem table
		 */
		if (uap->size < shminfo.shmmin || uap->size > shminfo.shmmax) {
			u.u_error = EINVAL;
			sp->shm_perm.mode = 0;
			return;
		}
		if ((rp = allocreg(NULL, RT_SHMEM, 0)) == NULL) {
			sp->shm_perm.mode = 0;
			return;
		}
		sp->shm_perm.mode |= SHM_INIT;	/* grow on first attach */
		sp->shm_segsz = uap->size;
		sp->shm_reg = rp;
		sp->shm_atime = sp->shm_dtime = 0;
		sp->shm_ctime = time;
		sp->shm_lpid = 0;
		sp->shm_cpid = u.u_procp->p_pid;
		rp->r_flags |= RG_NOFREE;
		regrele(rp);
	} else {
		/*
		 * Found an existing segment.  Check size
		 */
		if (uap->size && uap->size > sp->shm_segsz) {
			u.u_error = EINVAL;
			return;
		}
	}

	u.u_rval1 = sp->shm_perm.seq * shminfo.shmmni + (sp - shmem);
}

shminit()
{
}


/*
 * System entry point for shmat, shmctl, shmdt, and shmget system calls.
 */

int	(*shmcalls[])() = {shmat, shmctl, shmdt, shmget};

shmsys()
{
	register struct a {
		uint	id;
	}	*uap = (struct a *)u.u_ap;
	int		shmat(),
			shmctl(),
			shmdt(),
			shmget();

	if (uap->id > 3) {
		u.u_error = EINVAL;
		return;
	}
	u.u_ap = &u.u_arg[1];
	(*shmcalls[uap->id])();
}

/*
 * Select attach address based on segment size
 */

shmaddr()
{
	register uint	vaddr;
	register uint	vaddrt;
	register preg_t	*prp;

	vaddr = UVSHM;

	for (prp = u.u_procp->p_region ; prp->p_reg ; prp++) {
		vaddrt = (uint)(prp->p_regva + ctob(prp->p_reg->r_pgsz));
		if (vaddr < vaddrt)
			vaddr = vaddrt;
	}


	vaddr = (vaddr + NBPS - 1) & ~SOFFMASK;
	return(vaddr);
}

shmseg()
{
	return(shminfo.shmseg);
}
