/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:fs/du/durdwri.c	10.4"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/fstyp.h"
#include "sys/fs/s5param.h"
#include "sys/fs/s5inode.h"
#include "sys/inode.h"
#include "sys/mount.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/sbd.h"
#include "sys/immu.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/buf.h"
#include "sys/conf.h"
#include "sys/file.h"
#include "sys/systm.h"
#include "sys/debug.h"
#include "sys/rdebug.h"
#include "sys/message.h"
#include "sys/sysinfo.h"

dureadi(ip)
register struct inode *ip;
{
	DUPRINT3 (DB_FSS,"dureadi: ip = %x   sndd = %x \n",ip,ip->i_fsptr);
	if (u.u_syscall != DUREAD) 
		u.u_syscall = DUREADI;
	if(ip->i_ftype & IFIFO)
		prele(ip);
	remfileop (ip,NULL,NULL);
	if(ip->i_ftype & IFIFO)
		plock(ip);
}

duwritei(ip)
register struct inode *ip;
{
	DUPRINT3 (DB_FSS,"duwritei: ip = %x   sndd = %x \n",ip,ip->i_fsptr);
	if (u.u_syscall != DUWRITE) 
		u.u_syscall = DUWRITEI;
	if(ip->i_ftype & IFIFO)
		prele(ip);
	remfileop (ip,NULL,NULL);
	if(ip->i_ftype & IFIFO)
		plock(ip);
}
