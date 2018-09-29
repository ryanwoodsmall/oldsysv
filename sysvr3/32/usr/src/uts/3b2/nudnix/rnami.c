/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:nudnix/rnami.c	10.3.1.4"

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
#include "sys/stream.h"
#include "sys/comm.h"
#include "sys/message.h"
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
	/* remote inode case for dufst */
	if (dp->i_fstyp == dufstyp)
		return (1);
	panic ("rnamei1");	
}



/* come here from namei() when dp->i_flag & IDOTDOT is true */
/* check if I am server(), if so, if I want to do '..' at a REMOTE mount point */
/* return code: -1: error, 0: went REMOTE, but server cross mount point again
		 1: found dp, sys call finished on REMOTE */

rnamei2(cp, dp, buf)
register char *cp;
register struct inode *dp;
register char *buf;
{
	register struct srmnt *smp;
	mblk_t *nbp;
	int len;

	smp = &srmount[u.u_mntindx];
	if (dp == smp->sr_rootinode) { 
		iput(dp);
		goback(smp->sr_mntindx);
		while ((nbp = alocbuf (sizeof (struct response), BPRI_MED)) == NULL);
		u.u_copymsg = (struct response *)PTOMSG(nbp->b_rptr);
		u.u_copymsg->rp_type = RESP_MSG;
		u.u_copymsg->rp_bp = (long)nbp;
		len = strlen(u.u_nextcp);
		bcopy(u.u_nextcp, u.u_copymsg->rp_data, len+1);
		u.u_msgend = u.u_copymsg->rp_data + len + 1;
		return(0);
	}
	return(1);
}

/* This routine is called from nami to free a strams buffer */

rnamei0()
{
	if (u.u_gift && (u.u_gift->sd_temp != NULL)) {
		freemsg((mblk_t *)u.u_gift->sd_temp);
		u.u_gift->sd_temp = NULL;
	}
}
