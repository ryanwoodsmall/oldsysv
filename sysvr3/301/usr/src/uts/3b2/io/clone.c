/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:io/clone.c	10.5"
/*
 * Clone Driver.
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/fs/s5dir.h"
#include "sys/fstyp.h"
#include "sys/file.h"
#include "sys/inode.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/stropts.h"
#include "sys/stream.h"
#include "sys/errno.h"
#include "sys/sysinfo.h"
#include "sys/systm.h"
#include "sys/sysmacros.h"
#include "sys/conf.h"
#include "sys/var.h"
#include "sys/debug.h"
#include "sys/inline.h"


int clnopen();
static struct module_info clnm_info = { 0, "CLONE", 0, 0, 0, 0 };
static struct qinit clnrinit = { NULL, NULL, clnopen, NULL, NULL, &clnm_info, NULL };
static struct qinit clnwinit = { NULL, NULL, NULL, NULL, NULL, &clnm_info, NULL };
struct streamtab clninfo = { &clnrinit, &clnwinit };

/*
 * Clone open.  Dev is the major device of the streams device to
 * open.  Look up the device in the cdevsw[].   Attach its qinit
 * structures to the read and write queues and call its open
 * with the q and the dev CLONEOPEN.  Swap in a new inode with
 * the real device number constructed from the major and the
 * minor returned by the device open.  
 */

clnopen(q, dev, flag, sflag)
queue_t *q;
{
	int i, mindev, rdev;
	register struct streamtab *st;
	struct file *fp;
	struct inode *ip, *nip;
	struct mount *mp;

	if (sflag) return(OPENFAIL);

	/*
	 * Get the device to open.
	 */
	i = MAJOR[minor(dev) & 0x7f];
	if ((i >= cdevcnt) || !(st = cdevsw[i].d_str)) {
		u.u_error = ENXIO;
		return(OPENFAIL);
	}

	/*
	 * Substitute the real qinit values for the current ones
	 */
	setq(q, st->st_rdinit, st->st_wrinit);

	/*
	 * Call the device open with the stream flag  CLONEOPEN.  The device
	 * will either fail this or return a minor device number.
	 */
	rdev = makedev((minor(dev)&0x7f), 0);
	if ((mindev = (*q->q_qinfo->qi_qopen)(q, rdev, flag, CLONEOPEN)) == OPENFAIL)
		return(OPENFAIL);

	/*
	 * Get the inode at top of this stream, allocate a new inode,
	 * and exchange the new inode for the old one.
	 */
	rdev = makedev(minor(dev)&0x7f, mindev);
	ip = ((struct stdata *)(q->q_next->q_ptr))->sd_inode;

	ASSERT(ip->i_sptr);

	/* set up dummy inode */

	/*
	 * Allocate an inode from the pipe file system.
	 */
	ASSERT(pipefstyp > 0 && pipefstyp < nfstyp);
	mp = fsinfo[pipefstyp].fs_pipe;
	ASSERT(mp != NULL);
	if ((nip = (*fstypsw[pipefstyp].fs_getinode)(mp, FSG_CLONE, rdev)) == NULL){
		(*q->q_qinfo->qi_qclose)(q, flag);
		return(OPENFAIL);
	}

	plock(ip);

	nip->i_sptr = ip->i_sptr;

	for (i=0; i<v.v_nofiles; i++) 
		if ( (fp = u.u_ofile[i]) && (fp->f_inode == ip)) break;
	ASSERT(i < v.v_nofiles);

	/* 
	 * link dummy inode to file table and to the stream 
	 */
	ip->i_sptr = NULL;	/* no other clone open will get this stream */
	nip->i_sptr->sd_inode = (fp->f_inode = nip);
	nip->i_sptr->sd_strtab = st;
	iput(ip);
	prele(nip);
	return(mindev);
}	


