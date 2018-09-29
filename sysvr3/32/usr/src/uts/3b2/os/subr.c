/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/subr.c	10.10.2.4"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/file.h"
#include "sys/inode.h"
#include "sys/fstyp.h"
#include "sys/fs/s5dir.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/sbd.h"
#include "sys/immu.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/buf.h"
#include "sys/mount.h"
#include "sys/var.h"
#include "sys/conf.h"
#include "sys/debug.h"
#include "sys/region.h"
#include "sys/proc.h"

/*
 * update is the internal name of 'sync'. It goes through the disk
 * queues to initiate sandbagged IO; goes through the I nodes to write
 * modified nodes; and it goes through the mount table to initiate modified
 * super blocks.
 */
update()
{
	register struct inode *ip;
	register struct mount *mp;
	static struct inode uinode;

	if (uinode.i_flag)
		return;
	uinode.i_flag++;
	uinode.i_ftype = IFBLK;
	for (mp = mount; mp < (struct mount *)v.ve_mount; mp++) {
		if (mp->m_flags & MINUSE) 
			(*fstypsw[mp->m_fstyp].fs_update)(mp);
	}
	for (ip = &inode[0]; ip < (struct inode *)v.ve_inode; ip++)
		if ((ip->i_flag & ILOCK) == 0 && ip->i_count != 0
		&& (ip->i_flag & (IACC|IUPD|ICHG))) {
			ip->i_flag |= ILOCK;
			ASSERT(addilock(ip) == 0);
			ip->i_count++;
			FS_IUPDAT(ip, &time, &time);
			iput(ip);
		}
	bflush(NODEV);
	uinode.i_flag = 0;
}

/*
 * Routine which sets a user error; placed in
 * illegal entries in the bdevsw and cdevsw tables.
 */
nodev()
{
	u.u_error = ENODEV;
}

/*
 * Null routine; placed in insignificant entries
 * in the bdevsw and cdevsw tables.
 */
nulldev()
{
}

/*
 * Generate an unused major device number.
 */

#define NDEV 128
#define max(a, b)	((a) > (b) ? (a) : (b))

int
getudev()
{
	extern char MAJOR[];
	static int next = 0;
	int maxdevcnt = max(bdevcnt, cdevcnt);

	for ( ; next < NDEV; next++)
		if (MAJOR[next] >= maxdevcnt)
			return(next++);
	return(-1);
}

/*
 * Find a proc table pointer given a process id.  The code attempts
 * to optimize the case in which repeated calls return successive
 * proc table slots (as with ps(1)) by remembering where the search
 * left off the last time.  Interspersed calls can defeat the
 * optimization but ordinarily this won't happen.
 */

struct proc *
prfind(pid)
register int pid;
{
	register struct proc *p;
	static struct proc *lastp = NULL;
	register struct proc *prend = (struct proc *) v.ve_proc;

	if (lastp == NULL || lastp >= prend)
		lastp = prend - 1;
	p = lastp;
	do {
		if (++p >= prend)
			p = &proc[0];
		if (p->p_stat != 0 && p->p_pid == pid) {
			lastp = p;
			return(p);
		}
	} while (p != lastp);
	return(NULL);
}

/*	This function is called to check that a psw is suitable for
**	running in user mode.  If not, it is fixed to make it
**	suitable.  This is necessary when a psw is saved on the user
**	stack where some sneaky devil could set kernel mode or
**	something.
*/

fixuserpsw(psp)
register psw_t	*psp;
{
	extern char	u400;	/* Set non-zero if we are using	*/
				/* the cache.			*/

	/*	We never use quick interrupt so make sure that is
	**	off.  Will crash the system otherwise since there
	**	are no quick interrupt vectors set up.
	*/

	psp->QIE	= 0;

	/*	See if we are using the cache or not and set the
	**	cache flush disable and cache disable bits accordingly.
	*/

	if(u400){
		psp->CSH_F_D	= 0;
		psp->CSH_D	= 0;
	} else {
		psp->CSH_F_D	= 1;
		psp->CSH_D	= 1;
	}

	/*	The interrupt priority must be zero so that no
	**	interrupts are blocked when in user mode.  The
	**	current and previous modes are both set to user.
	*/

	psp->IPL	= 0;
	psp->CM		= PS_USER;
	psp->PM		= PS_USER;

	/*	Turn off the register save/restore and initial psw
	**	bits.  Our caller can turn them back on if he
	**	wants them.
	*/

	psp->R		= 0;
	psp->I		= 0;
}
