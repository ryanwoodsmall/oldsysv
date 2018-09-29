/* @(#)utssys.c	1.1 */
#include "sys/param.h"
#include "sys/types.h"
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
		if(mp->m_flags == MINUSE && mp->m_dev==uap->mv) {
			register struct filsys *fp;

			fp = (struct filsys *)paddr(mp->m_bufp);
			if(copyout(&fp->s_tfree, uap->cbuf, 18))
				u.u_error = EFAULT;
			return;
		}
	}
	u.u_error = EINVAL;
	return;

case 105:			/* process locking */
	lock();
	return;

default:
	u.u_error = EFAULT;
	}
}
