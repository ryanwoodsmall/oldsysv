/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/utssys.c	10.9"
#include "sys/types.h"
#include "sys/psw.h"
#include "sys/param.h"
#include "sys/uadmin.h"
#include "sys/mount.h"
#include "sys/fs/s5dir.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/inode.h"
#include "sys/fstyp.h"
#include "sys/ustat.h"
#include "sys/statfs.h"
#include "sys/file.h"
#include "sys/var.h"
#include "sys/utsname.h"
#include "sys/immu.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/systm.h"
#include "sys/sysmacros.h"
#include "sys/debug.h"
#include "sys/conf.h"

utssys()
{
	register i;
	register struct a {
		char	*cbuf;
		int	mv;
		int	type;
	} *uap;
	struct ustat ust;
	struct statfs stfs;

	uap = (struct a *)u.u_ap;
	switch (uap->type) {
	case 0:		/* uname */
		if (copyout(&utsname, uap->cbuf,
		  sizeof(struct utsname)))
			u.u_error = EFAULT;
		return;

	/* case 1 was umask */

	case 2:		/* ustat */
		if (uap->mv < 0) {
			duustat();
			return;
		}
		for (i = 0; i < v.v_mount; i++) {
			register struct mount *mp;
			register struct inode *ip;

			mp = &mount[i];
			if ((mp->m_flags & MINUSE) && mp->m_dev == uap->mv) {
				ASSERT(mp->m_mount != NULL);
				ip = iget(mp, mp->m_mount->i_number);
				if (ip == NULL) {
					u.u_error = ENOENT;
					return;
				}
				(*fstypsw[mp->m_fstyp].fs_statfs)(ip, 
				  (char *)&stfs, 0);
				if (u.u_error) {
					iput(ip);
					return;
				}
				ust.f_tfree = (daddr_t)stfs.f_bfree;
				ust.f_tinode = (ino_t)stfs.f_ffree;
				bcopy(stfs.f_fpack, ust.f_fpack, 
				  sizeof(ust.f_fpack));
				bcopy(stfs.f_fname, ust.f_fname, 
				  sizeof(ust.f_fname));

				if (copyout(&ust, uap->cbuf, sizeof(ust)))
					u.u_error = EFAULT;
				iput(ip);
				return;
			}
		}
		u.u_error = EINVAL;
		return;

	default:
		u.u_error = EFAULT;
	}
}

/*
 * administrivia system call
 */

uadmin()
{
	register struct a {
		int	cmd;
		int	fcn;
		int	mdep;
	} *uap;
	static ualock;

	if (ualock || !suser())
		return;
	ualock = 1;
	uap = (struct a *)u.u_ap;
	switch (uap->cmd) {

	case A_SHUTDOWN:
		{
			register struct proc *p = &proc[2];

			for (; p < (struct proc *)v.ve_proc; p++) {
				if (p->p_stat == NULL)
					continue;
				if (p != u.u_procp)
					psignal(p, SIGKILL);
			}
		}
		delay(HZ);	/* allow other procs to exit */
		{
			register struct mount *mp = &mount[0];

			xumount(mp);
			update();
			srumountfun();
		}
	case A_REBOOT:
		mdboot(uap->fcn, uap->mdep);
		/* no return expected */
		break;

	case A_REMOUNT:
		{
			register struct mount *mp = &mount[0];
	
			iflush(mp);
			/* remount root file system */
			srmountfun(0);
		}
		break;

	default:
		u.u_error = EINVAL;
	}
	ualock = 0;
}

