/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:fs/du/duiget.c	10.4"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/fstyp.h"
#include "sys/fs/s5param.h"
#include "sys/immu.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/mount.h"
#include "sys/fs/s5dir.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/fs/s5inode.h"
#include "sys/inode.h"
#include "sys/file.h"
#include "sys/ino.h"
#include "sys/fs/s5filsys.h"
#include "sys/stat.h"
#include "sys/buf.h"
#include "sys/var.h"
#include "sys/conf.h"
#include "sys/region.h"
#include "sys/debug.h"
#include "sys/rdebug.h"
#include "sys/cmn_err.h"
#include "sys/message.h"

struct inode *
duiread(ip)
register struct inode *ip;
{
	extern short dufstyp;

	DUPRINT2 (DB_FSS,"duiread: ip = %x \n",ip);
	ip->i_fstyp = dufstyp;
	return (ip);
}

duiput(ip)
register struct inode *ip;
{
	DUPRINT4 (DB_FSS,"duiput: ip = %x   sndd = %x syscall %d\n",
			ip, ip->i_fsptr,u.u_syscall);
	del_sndd (ip->i_fsptr);
	ip->i_fsptr = NULL;
}

duitrunc (ip)
register struct inode *ip;
{
	DUPRINT3 (DB_FSS,"duitrunc: ip = %x   sndd = %x \n",ip, ip->i_fsptr);
}



duinit()
{
	register i;
	extern short dufstyp;

	for (dufstyp = 0, i=0; i < nfstyp; i++) {
		if (fstypsw[i].fs_init == duinit) {
			dufstyp = i;
			break;
		}
	}

	if (dufstyp == 0 || dufstyp == nfstyp) {
		cmn_err(CE_PANIC, "duinit: duinit no found in fstypsw\n");
	}
}


duiupdat (ip, ta, tm)
register struct inode *ip;
time_t *ta, *tm;
{
	short tmpsyscall;
	if (ip->i_flag & ISYN) {
		/* update for sync writes only */
		tmpsyscall = u.u_syscall;
		u.u_syscall = DUIUPDATE;
		remote_call (ip, (caddr_t)NULL);
		u.u_syscall = tmpsyscall;
	}
}
		
