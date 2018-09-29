/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:fs/du/dusubr.c	10.10"
#include "sys/types.h"
#include "sys/sema.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/fstyp.h"
#include "sys/stream.h"
#include "sys/comm.h"
#include "sys/nserve.h"
#include "sys/fs/s5param.h"
#include "sys/systm.h"
#include "sys/file.h"
#include "sys/fs/s5inode.h"
#include "sys/inode.h"
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


dumount(bip, mp, rdonly)
register struct inode *bip;
register struct mount *mp;
register int rdonly;
{
	DUPRINT2 (DB_FSS,"dumount: bip = %x\n",bip);
	u.u_error = EINVAL;
}

duaccess(ip, mode)
register struct inode *ip;
register mode;
{
	register struct sndd *duip = (struct sndd *)ip->i_fsptr;

	DUPRINT2 (DB_FSS,"duaccess: ip = %x\n",ip);
	switch (mode) {
	case ISUID:
	case ISGID:
	case ISVTX: 
		return (!(duip->sd_mode & mode));
	}
	return (0);
}

duopeni (ip, flag)
struct inode *ip;
{
	DUPRINT1 (DB_FSS,"duopeni: called\n");
	return;
}

duclosei (ip, mode, count, offset)
register struct inode *ip;
register int mode, count;
off_t offset;
{
	int tmpsyscall;
	struct  a{
		int mode;
		int count;
		off_t offset;
	} temp;

	DUPRINT2 (DB_FSS,"duclosei: called syscall %d\n",u.u_syscall);
	tmpsyscall = u.u_syscall;
	u.u_syscall = DUCLOSE; 
	temp.mode = mode;
	temp.count = count;
	temp.offset = offset;
	remote_call (ip,(caddr_t)&temp);
	u.u_syscall = tmpsyscall;
}

duupdate (mp)
register struct mount *mp;
{
	int tmpsyscall;
	register struct inode *ip;

	/*
	tmpsyscall = u.u_syscall;
	u.u_syscall = DUUPDATE;
	ip = (struct inode *) mp->m_down;
	remote_call (ip,NULL);
	u.u_syscall = tmpsyscall;
	if (u.u_error == ENOLINK)
		u.u_error = 0;
	*/
	DUPRINT1 (DB_FSS,"duupdate called\n");
}

int *
dufreemap (ip)
register struct inode *ip;
{
	DUPRINT1 (DB_FSS,"dufreemap called\n");
}

dugetdents (ip, buf, bufcnt)
register struct inode *ip;
register char *buf;
int bufcnt;
{
	DUPRINT2 (DB_FSS,"\ndugetdents called u.u_offset %x\n",u.u_offset);
	remfileop (ip, NULL, NULL);
	return (u.u_rval1);
}

dustatf (ip, st)
register struct inode *ip;
register struct stat *st;
{
	DUPRINT1 (DB_FSS,"dustatf: called\n");
	remfileop (ip, NULL, NULL);
}
