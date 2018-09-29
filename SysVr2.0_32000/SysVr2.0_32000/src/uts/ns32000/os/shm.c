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
/* @(#)shm.c	6.3 */
#include "sys/types.h"
#include "sys/param.h"
#include "sys/dir.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/ipc.h"
#include "sys/shm.h"
#include "sys/seg.h"
#include "sys/page.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/systm.h"
#include "sys/sysmacros.h"
#include "sys/mmu.h"

#define SHMEND	(stob(SELF_P))
#define SHMINC	(stob(1))

extern	struct	shmid_ds	shmem[];	/* shared memory headers */
extern	struct	shminfo		shminfo;	/* shared memory info structure */
extern	time_t		time;			/* system idea of date */
extern	pte_t		*mshmget();

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

	if((sp = shmconv(uap->shmid)) == NULL)
		return;
	if(ipcaccess(&sp->shm_perm, SHM_R))
		return;
	if((uap->flag & SHM_RDONLY) == 0)
		if(ipcaccess(&sp->shm_perm, SHM_W))
			return;
	if(uap->flag & SHM_RND)
		uap->addr = uap->addr & ~(SHMLBA - 1);
	rp = sp->shm_reg_rw;
	reglock(rp);
	if (uap->flag & SHM_RDONLY) {
		if (sp->shm_reg_ro == NULL) {
			sp->shm_reg_ro = dupreg(rp, SEG_RO);
			if (sp->shm_reg_ro == NULL) {
				u.u_error = ENOMEM;
				regrele(rp);
				return;
			}
			if (rp->r_flags & RG_NOFREE)
				sp->shm_reg_ro->r_flags |= RG_NOFREE;
			regrele(sp->shm_reg_ro);
		}
		regrele(rp);
		rp = sp->shm_reg_ro;
		reglock(rp);
	}
	if(uap->addr == 0) {
		struct pregion *tprp;
		uint va;

		if ((tprp = findpreg(u.u_procp, PT_DATA)) == NULL)
			if ((tprp = findpreg(u.u_procp, PT_TEXT)) == NULL)
				panic("shmat: prp");
		va = (uint)(tprp->p_regva +
			(tprp->p_reg->r_pgsz + shminfo.shmbrk)*NBPP );
		va = (va + NBPS - 1) & ~(NBPS - 1);
		for(uap->addr = va; uap->addr <= SHMEND; uap->addr += SHMINC) {
			if((prp = attachreg(rp, &u, uap->addr, PT_SHMEM,
			              uap->flag & SHM_RDONLY? SEG_RO : SEG_RW)) == NULL)
				continue;
			if(sp->shm_perm.mode & SHM_INIT) {
				if((growreg(&u, prp, btop(sp->shm_segsz), 0, DBD_DZERO)) < 0) {
					detachreg(&u, prp);
					prp = NULL;
					continue;
				}
				if (uap->flag & SHM_RDONLY) {
					chgprot(prp, SEG_RO);
				}
				sp->shm_perm.mode &= ~SHM_INIT;
			}
			break;
		}
		if(prp == NULL) {
			regrele(rp);
			return;
		}
	} else {
		if((prp = attachreg(rp, &u, uap->addr, PT_SHMEM,
		   uap->flag & SHM_RDONLY? SEG_RO : SEG_RW)) == NULL) {
			regrele(rp);
			return;
		}
		if(sp->shm_perm.mode & SHM_INIT) {
			if((growreg(&u, prp, btop(sp->shm_segsz), 0, DBD_DZERO)) < 0) {
				detachreg(&u, prp);
				regrele(rp);
				u.u_error = EINVAL;
				return;
			}
			if (uap->flag & SHM_RDONLY) {
				chgprot(prp, SEG_RO);
			}
			sp->shm_perm.mode &= ~SHM_INIT;
		}
	}
	regrele(rp);
	u.u_error = 0;
	u.u_rval1 = (int) prp->p_regva;
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

	sp = &shmem[s % shminfo.shmmni];
	if(!(sp->shm_perm.mode & IPC_ALLOC)
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

	if((sp = shmconv(uap->shmid)) == NULL)
		return;

	switch(uap->cmd) {

	/* Remove shared memory identifier. */
	case IPC_RMID:
		if(u.u_uid != sp->shm_perm.uid && u.u_uid != sp->shm_perm.cuid
			&& !suser())
			return;
		rp = sp->shm_reg_rw;
		sp->shm_reg_rw = NULL;
		sp->shm_perm.mode = 0;
		sp->shm_segsz = 0;
		if(((int)(++(sp->shm_perm.seq) * shminfo.shmmni + (sp - shmem))) < 0)
			sp->shm_perm.seq = 0;
		reglock(rp);
		if(rp->r_refcnt == 0)
			freereg(rp);
		else {
			rp->r_flags &= ~RG_NOFREE;
			regrele(rp);
		}
		if (sp->shm_reg_ro != NULL) {
			rp = sp->shm_reg_ro;
			sp->shm_reg_ro = NULL;
			reglock(rp);
			if(rp->r_refcnt == 0)
				freereg(rp);
			else {
				rp->r_flags &= ~RG_NOFREE;
				regrele(rp);
			}
		}
		break;

	/* Set ownership and permissions. */
	case IPC_SET:
		if(u.u_uid != sp->shm_perm.uid && u.u_uid != sp->shm_perm.cuid
			 && !suser())
			return;
		if(copyin(uap->arg, &ds, sizeof(ds))) {
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
		if(ipcaccess(&sp->shm_perm, SHM_R))
			return;

		/*	The following is needed because
		**	a user can look at it.  In
		**	particular, the regression tests
		**	require it.
		*/

		sp->shm_nattch = sp->shm_reg_rw->r_refcnt;
		if (sp->shm_reg_ro != NULL)
			sp->shm_nattch += sp->shm_reg_ro->r_refcnt;
		sp->shm_cnattch = sp->shm_nattch;

		if(copyout(sp, uap->arg, sizeof(*sp)))
			u.u_error = EFAULT;
		break;

	/* Lock segment in memory */
	case SHM_LOCK:
		if(!suser())
			return;
		rp = sp->shm_reg_rw;
		reglock(rp);
		rp->r_flags |= RG_NOSWAP;
		regrele(rp);
		if (sp->shm_reg_ro != NULL) {
			rp = sp->shm_reg_ro;
			reglock(rp);
			rp->r_flags |= RG_NOSWAP;
			regrele(rp);
		}
		break;

	/* Unlock segment */
	case SHM_UNLOCK:
		if(!suser())
			return;
		rp = sp->shm_reg_rw;
		reglock(rp);
		rp->r_flags &= ~RG_NOSWAP;
		regrele(rp);
		if (sp->shm_reg_ro != NULL) {
			rp = sp->shm_reg_ro;
			reglock(rp);
			rp->r_flags &= ~RG_NOSWAP;
			regrele(rp);
		}
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

	/*
	 * Find matching shmem address in process region list
	 */
	for(prp = u.u_procp->p_region; prp->p_reg; prp++)
		if(prp->p_type == PT_SHMEM && prp->p_regva == uap->addr)
			break;
	if(prp->p_reg == NULL) {
		u.u_error = EINVAL;
		return;
	}
	/*
	 * Detach region from process
	 * We must remember rp here since detach clobbers p_reg
	 */
	rp = prp->p_reg;
	reglock(rp);
	detachreg(&u, prp);
	clratb(USRATB);
	/*
	 * Find shmem region in system wide table.  Update detach time
	 * and pid, and free if appropriate
	 */
	for(sp = shmem; sp < &shmem[shminfo.shmmni]; sp++)
		if((sp->shm_reg_rw == rp) || (sp->shm_reg_ro == rp))
			break;
	if(sp >= &shmem[shminfo.shmmni])
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

	if((sp = ipcget(uap->key, uap->shmflg, shmem, shminfo.shmmni, sizeof(*sp),
		&s)) == NULL)
		return;
	if(s) {

		/*
		 * This is a new shared memory segment.
		 * Allocate a region and init shmem table
		 */
		if(uap->size < shminfo.shmmin || uap->size > shminfo.shmmax) {
			u.u_error = EINVAL;
			sp->shm_perm.mode = 0;
			return;
		}
		if((rp = allocreg(NULL, RT_SHMEM)) == NULL) {
			u.u_error = ENOMEM;
			sp->shm_perm.mode = 0;
			return;
		}
		sp->shm_perm.mode |= SHM_INIT;	/* grow on first attach */
		sp->shm_segsz = uap->size;
		sp->shm_reg_rw = rp;
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
		if(uap->size && uap->size > sp->shm_segsz) {
			u.u_error = EINVAL;
			return;
		}
	}

	u.u_rval1 = sp->shm_perm.seq * shminfo.shmmni + (sp - shmem);
}

/*
 * System entry point for shmat, shmctl, shmdt, and shmget system calls.
 */
shmsys()
{
	register struct a {
		uint	id;
	}	*uap = (struct a *)u.u_ap;
	int		shmat(),
			shmctl(),
			shmdt(),
			shmget();
	static int	(*calls[])() = {shmat, shmctl, shmdt, shmget};

	if(uap->id > 3) {
		u.u_error = EINVAL;
		return;
	}
	u.u_ap = &u.u_arg[1];
	(*calls[uap->id])();
}
