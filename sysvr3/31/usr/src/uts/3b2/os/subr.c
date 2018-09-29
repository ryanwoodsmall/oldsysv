/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/subr.c	10.10.2.3"
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
