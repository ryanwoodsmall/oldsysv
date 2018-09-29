/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:fs/du/dusys3.c	1.3.1.4"
#include "sys/types.h"
#include "sys/sema.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/fstyp.h"
#include "sys/stream.h"
#include "sys/comm.h"
#include "sys/nserve.h"
#include "sys/cirmgr.h"
#include "sys/ustat.h"
#include "sys/fs/s5param.h"
#include "sys/systm.h"
#include "sys/file.h"
#include "sys/fs/s5inode.h"
#include "sys/inode.h"
#include "sys/nami.h"
#include "sys/mount.h"
#include "sys/fs/s5filsys.h"
#include "sys/fs/s5dir.h"
#include "sys/statfs.h"
#include "sys/conf.h"
#include "sys/open.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/sbd.h"
#include "sys/immu.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/buf.h"
#include "sys/var.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/debug.h"
#include "sys/rdebug.h"
#include "sys/message.h"
#include "sys/fcntl.h"


/* fcntl */

dufcntl(ip, cmd, arg, flag, offset)
register struct inode *ip;
int cmd, arg, flag;
off_t offset;
{
	int unlocked = 0;
	sndd_t sd;

	DUPRINT4 (DB_FSS,"dufcntl: ip %x cmd %x offset %x\n", ip, cmd, offset);
	sd = (sndd_t)ip->i_fsptr;
	if (sd->sd_stat & SDMNDLCK && cmd == F_FREESP) {
		prele(ip);
		unlocked = 1;
	}
	remfileop (ip, flag, offset);

	/* Check unlocked instead of SDMNDLCK, since file could become
	 * mandatory locked during remote operation, in which case you
	 * don't call plock here. 
	 */
	if (unlocked)
		plock(ip);
	DUPRINT2 (DB_FSS,"dufcntl: u.u_error = %d \n",u.u_error);
}

/* ioctl */
duioctl(ip, cmd, arg, flag)
register struct inode *ip;
int cmd, arg, flag;
{
	register int savesyscall;

	DUPRINT4 (DB_FSS,"duioctl: ip %x cmd %x flag %x\n", ip, cmd, flag);
	savesyscall = u.u_syscall;
	u.u_syscall = DUIOCTL;
	remfileop(ip, flag, NULL);
	u.u_syscall = savesyscall;
	DUPRINT2 (DB_FSS,"duioctl: u.u_error = %d \n",u.u_error);
}

dunotify(ip, noargp)
register struct inode *ip;
register struct argnotify *noargp;
{
	struct a {int fdes; off_t off; int sbase; } *uap = (struct a *) u.u_ap;
	ASSERT(noargp != 0);
	switch (noargp->cmd) {
	case NO_SEEK:
		if(noargp->data2 == 2) {
			uap->off -= ip->i_size;
			remfileop(ip,NULL,NULL);
			return (uap->off);
		}
	}
	return (uap->off);
}
