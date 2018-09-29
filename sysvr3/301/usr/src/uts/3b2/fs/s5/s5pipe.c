/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:fs/s5/s5pipe.c	10.5"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/fstyp.h"
#include "sys/fs/s5macros.h"
#include "sys/systm.h"
#include "sys/fs/s5dir.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/sbd.h"
#include "sys/immu.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/mount.h"
#include "sys/fs/s5inode.h"
#include "sys/inode.h"
#include "sys/file.h"
#include "sys/debug.h"
#include "sys/conf.h"
#include "sys/cmn_err.h"

/*
 * Allocate an inode on the fstyp device.
 * Set the appropriate counters, if the cmd is FSG_PIPE.
 */
struct inode *
s5getinode(mp, cmd, rdev)
register struct mount *mp;
register int cmd;
int rdev;
{
	register struct inode *ip;
	register struct s5inode *s5ip;
	struct inode *s5ialloc();

	if (mp == NULL) {
		cmn_err(CE_WARN, "NULL mp in s5getinode()");
		u.u_error = ENODEV;
		return(NULL);
	}
	if (cmd == FSG_PIPE) {
		if ((ip = s5ialloc(mp, IFIFO, 0, rdev)) == NULL)
			return(NULL);
		s5ip = (struct s5inode *)ip->i_fsptr;
		ASSERT(s5ip != NULL);
		s5ip->s5i_frcnt = 1;
		s5ip->s5i_fwcnt = 1;
	} else if (cmd == FSG_CLONE) {
		if ((ip = s5ialloc(mp, IFCHR, 0, rdev)) == NULL)
			return(NULL);
		s5ip = (struct s5inode *)ip->i_fsptr;
		ASSERT(s5ip != NULL);
	}
	return(ip);
}

/*
 * Open a fifo
 * Check read and write counts, delay as necessary
 */

s5openf(ip, mode)
	register struct inode *ip;
	register mode;
{
	register struct s5inode *s5ip;

	s5ip = (struct s5inode *)ip->i_fsptr;
	ASSERT(s5ip != NULL);
	if (mode & FREAD) {
		if (s5ip->s5i_frcnt++ == 0)
			wakeup((caddr_t)&s5ip->s5i_frcnt);
	}
	if (mode & FWRITE) {
		if (mode & FNDELAY && s5ip->s5i_frcnt == 0) {
			u.u_error = ENXIO;
			return;
		}
		if (s5ip->s5i_fwcnt++ == 0)
			wakeup((caddr_t)&s5ip->s5i_fwcnt);
	}
	if (mode & FREAD) {
		while (s5ip->s5i_fwcnt == 0) {
			if (mode & FNDELAY || ip->i_size) {
				return;
			}
			sleep(&s5ip->s5i_fwcnt, PPIPE);
		}
	}
	if (mode & FWRITE) {
		while (s5ip->s5i_frcnt == 0)
			sleep(&s5ip->s5i_frcnt, PPIPE);
	}
}

/*
 * Close a fifo
 * Update counts and cleanup
 */

s5closef(ip, mode)
	register struct inode *ip;
	register mode;
{
	register i;
	register struct s5inode *s5ip;
	register daddr_t bn;

	s5ip = (struct s5inode *)ip->i_fsptr;
	ASSERT(s5ip != NULL);
	if (mode & FREAD) {
		if ((--s5ip->s5i_frcnt == 0) && (s5ip->s5i_fflag & IFIW)) {
			s5ip->s5i_fflag &= ~IFIW;
			wakeup((caddr_t)&s5ip->s5i_fwcnt);
		}
	}
	if (mode & FWRITE) {
		if ((--s5ip->s5i_fwcnt == 0) && (s5ip->s5i_fflag & IFIR)) {
			s5ip->s5i_fflag &= ~IFIR;
			wakeup((caddr_t)&s5ip->s5i_frcnt);
		}
	}
	if ((s5ip->s5i_frcnt == 0) && (s5ip->s5i_fwcnt == 0)) {
		for (i = NFADDR - 1; i >= 0; i--) {
			bn = s5ip->s5i_faddr[i];
			if (bn == (daddr_t)0)
				continue;
			s5ip->s5i_faddr[i] = (daddr_t)0;
			s5free(ip->i_mntdev, bn);
		}
		ip->i_size = 0;
		s5ip->s5i_frptr = 0;
		s5ip->s5i_fwptr = 0;
		ip->i_flag |= IUPD|ICHG;
	}
}
