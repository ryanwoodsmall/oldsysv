/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:nudnix/rnami.c	10.3"

#include "sys/types.h"
#include "sys/sema.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/fs/s5dir.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/immu.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/comm.h"
#include "sys/inode.h"
#include "sys/file.h"
#include "sys/mount.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/debug.h"
#include "sys/inline.h"
#include "sys/rdebug.h"

extern	struct	inode *rem_inode(); 

/* come here from namei() when root dir or current dir is REMOTE */
/* return code:  -1: error, 0: cross mount point again */ 

rnamei1(dirp, dpp)
register struct inode *dirp;	/* either root dir or current dir */
struct inode **dpp;
{
	register struct inode *dp;
	extern short dufstyp;

 	*dpp = dp = remote_call(dirp,(caddr_t)NULL); 
	if (dp == NULL) 
		return (-1);
 	if (u.u_rflags & U_DOTDOT) {
		u.u_rflags &= ~U_DOTDOT;
		return (0);
 	}
 	if (u.u_rflags & U_RSYS)
 		return (1);
	/* remote inode case for dufst */
	if (dp->i_fstyp == dufstyp)
		return (1);
	panic ("rnamei1");	
}



/* come here from namei() when dp->i_flag & (IDOTDOT|ILBIN) is true */
/* check if I am server(), if so, if I want to do '..' at a REMOTE mount point */
/* return code: 1: lbin mount, server continues 
		0 (NULL): cross mount again, server should go back */


/* come here from namei() when mp->m_rflags & MLBIN is true */
/* have to send back the '..' */
/* return code: -1: error, 0: went REMOTE, but server cross mount point again
		 1: found dp, sys call finished on REMOTE */

rnamei2(cp, dp, buf)
register char *cp;
register struct inode *dp;
register char *buf;
{
	register struct srmnt *smp;

	smp = &srmount[u.u_mntindx];
	if (dp == smp->sr_rootinode) 
		if (dp->i_flag &IDOTDOT)  {
			iput(dp);
			goback(smp->sr_mntindx, DOT_DOT);
			u.u_arg[0] += (u.u_nextcp - buf);
			return(0);
		}
		else  /* lbin case, now mnt index is parent */
			u.u_mntindx = smp->sr_dotdot;
	return(1);
}
rnamei3(dpp)
struct inode **dpp;
{
	register struct inode *ip;
	register struct mount *mp;
	register char c;

	/*
	 * Back up over "..".  Note that a terminating slash may have
	 * been zapped to NUL by namei().
	 */
	while ((c = *--u.u_nextcp) == '/' || c == '\0')
		*u.u_nextcp = '/';
	u.u_nextcp--;
	mp = (*dpp)->i_mntdev;
 	*dpp = ip = remote_call(mp->m_up,(caddr_t)NULL);
	if (ip == NULL) 
		return (-1);
 	if (u.u_rflags & U_DOTDOT) {
		u.u_rflags &= ~U_DOTDOT;
		return (0);
 	}
 	if (u.u_rflags & U_RSYS)
 		return (1);
	if (ip->i_fstyp == dufstyp)
		return (1);
/* how do we come here??? remote_call() sets u.u_error to one of two values */
/*
 	plock (ip);
 	ip->i_count++;
	return (0);
*/
	panic ("rnamei3");
}

/* come here from namei() when iget() returns non-zero u.u_error which */
/* indicates possibility of having gone REMOTE */
/* return code: -1: error 1: found dp, syscall ended on REMOTE
		 0: server cross mount point again, continue
		 2: somehow, it did not go REMOTE?  */

rnamei4(dp)
register struct inode *dp;
{
 	if (u.u_rflags & U_DOTDOT) {
		/*  server side of LBIN mount or client side of remote DOTDOT
 		 *  if we are on the server side, it is LBIN - return.
 		 *  if on client side, continue marching down the path.
 		 */
 		if (server())
 			panic ("rnamei4 if server()");
		u.u_rflags &= ~U_DOTDOT;
		return (0);
 	}
 	else if (u.u_rflags & U_RSYS)
 		return (1);
	return (2);  /* did not go REMOTE after all */
}

