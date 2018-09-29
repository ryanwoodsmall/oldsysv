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
/* @(#)utssys.c	6.3 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/page.h"
#include "sys/buf.h"
#include "sys/filsys.h"
#include "sys/mount.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/var.h"
#include "sys/utsname.h"

utssys()
{
	register i;
	register struct a {
		char	*cbuf;
		int	mv;
		int	type;
	} *uap;
	struct {
		daddr_t	f_tfree;
		ino_t	f_tinode;
		char	f_fname[6];
		char	f_fpack[6];
	} ust;

	uap = (struct a *)u.u_ap;
	switch(uap->type) {

case 0:		/* uname */
	if (copyout(&utsname, uap->cbuf, sizeof(struct utsname)))
		u.u_error = EFAULT;
	return;

/* case 1 was umask */

case 2:		/* ustat */
	for(i=0; i<v.v_mount; i++) {
		register struct mount *mp;

		mp = &mount[i];
		if (mp->m_flags==MINUSE && brdev(mp->m_dev)==brdev(uap->mv)) {
			register struct filsys *fp;

			fp = mp->m_bufp->b_un.b_filsys;
			ust.f_tfree = FsLTOP(mp->m_dev, fp->s_tfree);
			ust.f_tinode = fp->s_tinode;
			bcopy(fp->s_fname, ust.f_fname, sizeof(ust.f_fname));
			bcopy(fp->s_fpack, ust.f_fpack, sizeof(ust.f_fpack));
			if (copyout(&ust, uap->cbuf, 18))
				u.u_error = EFAULT;
			return;
		}
	}
	u.u_error = EINVAL;
	return;

default:
	u.u_error = EFAULT;
	}
}
