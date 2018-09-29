/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:nudnix/cache.c	10.1.2.5"

#include "sys/types.h"
#include "sys/sema.h"
#include "sys/sysmacros.h"
#include "sys/signal.h"
#include "sys/file.h"
#include "sys/fs/s5dir.h"
#include "sys/param.h"
#include "sys/stream.h"
#include "sys/comm.h"
#include "sys/message.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/inode.h"
#include "sys/var.h"
#include "sys/errno.h"
#include "sys/nserve.h"
#include "sys/recover.h"
#include "sys/cirmgr.h"
#include "sys/debug.h"
#include "sys/rfsys.h"
#include "sys/rdebug.h"
#include "sys/immu.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/buf.h"
#include "sys/cmn_err.h"
#include "sys/sysinfo.h"


extern	time_t	lbolt;
extern	sndd_t cache_sd;	/* SD for sending cache disable messages */
extern	rcvd_t cache_rd;	/* RD for getting response back */


/*
 *	Check if cache can be re-enabled.
 *	If writer machine count is 0, all clients can be re-enabled.
 *	If writer machine count is 1, only that client can be re-enabled.
 */

enable_cache(rd)
register rcvd_t	rd;
{
	register struct rd_user *rduptr;
	register struct rd_user *tmprdup = NULL;
	register int write_cnt = 0;

	DUPRINT2(DB_CACHE, "enable_cache: rd %x\n", rd);
	/* count how many machines still writing to the file */
	if (rd->rd_inode->i_flag & IWROTE)
		write_cnt++;
	for (rduptr = rd->rd_user_list; rduptr; rduptr = rduptr->ru_next) {
		if (rduptr->ru_cflag & CACHE_WRITE) {
			write_cnt++;
			if (write_cnt > 1)
				return;		/* can't enable cache */
			tmprdup = rduptr;
		}
	}
	if (write_cnt == 0) {
		/* cache can be enabled for all remote machines */
		DUPRINT1(DB_CACHE, "enable_cache: enable for all machines\n");
		for (rduptr = rd->rd_user_list; rduptr; rduptr = rduptr->ru_next) {
			if (rduptr->ru_cflag & CACHE_DISABLE) {
				rduptr->ru_cflag &= ~CACHE_DISABLE;
				rduptr->ru_cflag |= CACHE_REENABLE;
			}
		}
	}
	else if (write_cnt == 1) {
		/* cache can be enabled only for that machine */
		if (tmprdup == NULL)
			return;		/*local process has write outstanding */
		DUPRINT2(DB_CACHE, "enable_cache: enable for mntindx %x\n", rduptr->ru_srmntindx);
		if (tmprdup->ru_cflag & CACHE_DISABLE) {
			tmprdup->ru_cflag &= ~CACHE_DISABLE;
			tmprdup->ru_cflag |= CACHE_REENABLE;
		}
	}
}



/*
 *	Write happens to the file,
 *	check if any remote cache has to be disabled.
 */

disable_cache(ip)
register struct inode *ip;
{
	register struct rd_user *rduptr;
	register rcvd_t rd;
	register sndd_t sd;
	int	s;

	s = spl6();
	ip->i_rcvd->rd_mtime = lbolt;
	splx(s);

	/* get a SD for sending cache disable messages */
	while ((sd = cr_sndd()) == NULL) {
		/* try to use the reserved one for cache */
		if (cache_sd->sd_refcnt == 1) {
			cache_sd->sd_refcnt++;
			sd = cache_sd;
			break;
		}
		else {
			cache_sd->sd_stat |= SDWANT;
			sleep(&cache_sd, PZERO);
		}
	}

	/* get a RD for getting response back */
	while ((rd = cr_rcvd(1, SPECIFIC)) == NULL) {
		/* try to use the reserved one for cache */
		if (cache_rd->rd_refcnt == 1) {
			cache_rd->rd_refcnt++;
			rd = cache_rd;
			break;
		}
		else {
			cache_rd->rd_stat |= RDWANT;
			sleep(&cache_rd, PZERO);
		}
	}

	for (rduptr = ip->i_rcvd->rd_user_list; rduptr; rduptr = rduptr->ru_next) {
		if (server() && (rduptr->ru_srmntindx == u.u_mntindx)) {
			/* no need to disable cache for the machine
			   that is currently writing to the file */
			rduptr->ru_cflag |= CACHE_WRITE;
			continue;
		}
		if (rduptr->ru_cflag & CACHE_DISABLE)
			continue;
		if (rduptr->ru_cflag & CACHE_REENABLE) {
			rduptr->ru_cflag &= ~CACHE_REENABLE;
			rduptr->ru_cflag |= CACHE_DISABLE;
			continue;
		}
		if (rduptr->ru_cflag & CACHE_ENABLE) {
			rduptr->ru_cflag &= ~CACHE_ENABLE;
			rduptr->ru_cflag |= CACHE_DISABLE;
			snd_disable_msg(rduptr, ip, sd, rd);
			rcinfo.snd_dis++;
			continue;
		}
	}

	/* done with disable cache messages, free SD and RD */
	if (rd == cache_rd) {
		rd->rd_refcnt--;
		if (rd->rd_stat & RDWANT) {
			rd->rd_stat &= ~RDWANT;
			wakeup(&cache_rd);
		}
	}
	else
		free_rcvd(rd);

	if (sd == cache_sd) {
		sd->sd_refcnt--;
		if (sd->sd_stat & SDWANT) {
			sd->sd_stat &= ~SDWANT;
			wakeup(&cache_sd);
		}
	}
	else
		free_sndd(sd);
}



/*
 *	Send out disable messages to rmeote machines.
 *	This implementation will send out disable messages
 *	and wait for response back.
 *	Otherwise, an inconsistency may happen -- A writes to a file on C,
 *	C sends disable message to B and returns write response to A.
 *	A then uses remote IPC to notify B to read the file, but this
 *	gets to B before the disable message from C, so B reads the
 *	data in the cache buffer which is not the correct one --
 */

snd_disable_msg(rduptr, ip, sd, rd)
register struct rd_user *rduptr;
register struct inode *ip;
register sndd_t sd;
register rcvd_t rd;
{
	register mblk_t	*bp;
	register struct request	*request;
	int	size;
	mblk_t	*rp_bp;
	struct response *resp;
	long	sig;
	int	cursig;

	DUPRINT3(DB_CACHE, "snd_disable_msg: mntindx %x, fhandle %x\n", rduptr->ru_srmntindx, ip);

	if (((struct gdp *)rduptr->ru_queue->q_ptr)->flag != GDPCONNECT)
		return;
	sd->sd_stat = SDUSED;
	set_sndd(sd, rduptr->ru_queue, RECOVER_RD, RECOVER_RD);
	sig = u.u_procp->p_sig;
	cursig = u.u_procp->p_cursig;
	u.u_procp->p_sig = 0;
	u.u_procp->p_cursig = 0;
	while ((bp = alocbuf(sizeof(struct request)-DATASIZE, BPRI_MED)) == NULL) {
		sig |= u.u_procp->p_sig;
		if (u.u_procp->p_cursig) {
			if (cursig)
				sig |= (1L << (u.u_procp->p_cursig - 1));
			else
				cursig = u.u_procp->p_cursig;
		}
		u.u_procp->p_sig = 0;
		u.u_procp->p_cursig = 0;
	}
	u.u_procp->p_sig = sig;
	u.u_procp->p_cursig = cursig;
	request = (struct request *) PTOMSG(bp->b_rptr);
	request->rq_type = REQ_MSG;
	request->rq_opcode = DUCACHEDIS;
	request->rq_mntindx = rduptr->ru_srmntindx;
	request->rq_fhandle = (long)ip;
	if (sndmsg(sd, bp, sizeof(struct request)-DATASIZE, rd) == FAILURE) {
		/* can't send to remote machine due to link down, just return */
		return;
	}
	if (de_queue(rd, &rp_bp, NULL, &size) != FAILURE) {
		resp = (struct response *)PTOMSG(rp_bp->b_rptr);
		ASSERT(resp->rp_opcode == DUCACHEDIS);
		freemsg(rp_bp);
	}
}



/*
 *	Invalidate cache on the client machine
 */

invalidate_cache(qp, mntindx, fhandle)
register queue_t *qp;
register int mntindx;
register long fhandle;
{
	register sndd_t sd;

	rcinfo.rcv_dis++;
	/* find out which SD to disable cache */
	for (sd = sndd; sd < &sndd[nsndd]; sd ++) {
		if (sd->sd_stat & SDUSED &&
		    sd->sd_queue == qp &&
		    sd->sd_mntindx == mntindx &&
		    sd->sd_fhandle == fhandle) {
			sd->sd_stat &= ~SDCACHE;
			rfinval(sd, -1, 0);
			break;
		}
	}

}
