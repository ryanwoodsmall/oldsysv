/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/subr.c	10.10"
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

#ifdef u3b2

#define NDEV 128
#define max(a, b)	((a) > (b) ? (a) : (b))

int
getudev()
{
	extern char MAJOR[];
	static int next = 0;

	for ( ; next < NDEV; next++)
		if (MAJOR[next] >= max(bdevcnt, cdevcnt))
			return(next++);
	return(-1);
}

#else

#define NDEV 256
int
getudev()
{
	static int next = 0;

	if (next == 0)
		next = bdevcnt;
	return(next < NDEV ? next++ : -1);
}

#endif
