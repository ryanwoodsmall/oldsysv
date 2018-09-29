/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:nudnix/rfcanon.c	10.5.3.2"

#include "sys/types.h"
#include "sys/sema.h"
#include "sys/param.h"
#include "sys/fs/s5dir.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/immu.h"
#include "sys/stream.h"
#include "sys/comm.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/inode.h"
#include "sys/message.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/nserve.h"
#include "sys/cirmgr.h"
#include "sys/debug.h"
#include "sys/rdebug.h"
#include "sys/fs/s5param.h"
#include "sys/fs/s5macros.h"
#include "sys/buf.h"
#include "sys/rbuf.h"
#include "sys/sysinfo.h"
#include "sys/dirent.h"
#include "sys/hetero.h"
#include "sys/fcntl.h"
#include "sys/file.h"


/*
 *	tocanon routine for RFS
 *
 *	convert all RFS header and data parts to canonical formats
 *	called by sndmsg() before passing data to protocol module
 *
 *  WARNING: This routine will make the output data bigger.
 *	It is assumed that the "bp" is big enough for the enlarged data.
 *	See the warning message in fileop.c, right before "alocbuf".
 */

rftocanon (bp, hetero)
register mblk_t	*bp;
register int hetero;
{
	register struct response *msg;
	register struct message *mp;
	register int i = 0;

	if (hetero == NO_CONV)
		return;

	/*
	 *	convert RFS data portion to canonical form
	 */
	msg = (struct response *)PTOMSG(bp->b_rptr);
	if (msg->rp_type == RESP_MSG && msg->rp_errno == 0) {
 		switch (msg->rp_opcode)  {
		case DUFCNTL:
			switch (msg->rp_rval) {
			case	F_GETLK:
				i = tcanon("ssllss", msg->rp_data, msg->rp_data, 1);
				break;
			}
			break;
		case DUFSTAT:
			i = tcanon("sssssssllll", msg->rp_data, msg->rp_data, 1);
			break;

		case DUFSTATFS:
			i = tcanon("sllllllc6c6", msg->rp_data, msg->rp_data, 1);
			break;

		case DUGETDENTS:
			i = dentcanon(msg->rp_count, msg->rp_data, msg->rp_data, 1);
			if (i)
				msg->rp_count = i;
			break;

		case DUSTAT:
			i = tcanon("sssssssllll", msg->rp_data, msg->rp_data, 1);
			break;

		case DUSTATFS:
			i = tcanon("sllllllc6c6", msg->rp_data, msg->rp_data, 1);
			break;

		case DUUTSSYS:
			i = tcanon("lsc6c6", msg->rp_data, msg->rp_data, 1);
			break;

		case DUCOPYOUT:
			if (u.u_syscall == DUGETDENTS) {
				i = dentcanon(msg->rp_count, msg->rp_data, msg->rp_data, 1);
				if (i)
					msg->rp_count = i;
			}
			break;
		case DUCOPYIN:
			if (u.u_syscall == DUUTIME) 
				i = tcanon("ll",msg->rp_data,msg->rp_data,1);
			break;
		default:
			break;
		}

	}
	if (msg->rp_type == REQ_MSG ) {	  /*rp_type first in req & resp msg*/
		register struct request *rmsg;

		rmsg = (struct request *)PTOMSG(bp->b_rptr);
 		switch (rmsg->rq_opcode)  {
		case DUFCNTL:
			if (rmsg->rq_fflag & FRCACH) {
				switch (rmsg->rq_cmd) {
				case	F_GETLK:
				case	F_SETLK:
				case	F_SETLKW:
				case	F_CHKFL:
				case	F_FREESP:
					i = tcanon("ssllss", rmsg->rq_data, rmsg->rq_data, 1);
					break;
				default:
					break;
				}
			}
			break;
		default:
			break;
		}
	}

	/* adjust the stream write pointer due to conversion expansion */
	if (i) {
		bp->b_wptr = bp->b_rptr + sizeof(struct message) 
			     + sizeof(struct response) - DATASIZE + i;
		mp = (struct message *)bp->b_rptr;
		mp->m_size = bp->b_wptr - bp->b_rptr;
	}
	if (hetero == DATA_CONV)
		return;

	/*
	 *	convert RFS communication header to canonical form
	 */
	tcanon("llllllll", bp->b_rptr, bp->b_rptr, 0);

	/*
	 *	convert RFS request/response common header to canonical form
	 */
	tcanon("llllllllllllllllllll", PTOMSG(bp->b_rptr), PTOMSG(bp->b_rptr), 0);

}





/*
 *	frcanon routine for RFS
 *
 *	convert all RFS header and data parts back to local formats
 *	called by arrmsg() before passing data to GDP module
 */

rffrcanon (bp, hetero)
register mblk_t	*bp;
register int hetero;
{
	register struct response *msg;
	register int i = 0;

	if (hetero == NO_CONV)
		return;

	if (hetero == ALL_CONV) {
		/*
		 *	convert RFS communication header back to local form
		 */
		fcanon("llllllll", bp->b_rptr, bp->b_rptr);

		/*
		 *	convert RFS request/response common header back to local form
		 */
		fcanon("llllllllllllllllllll", PTOMSG(bp->b_rptr), PTOMSG(bp->b_rptr));
	}

	/*
	 *	convert RFS data portion back to local form
	 */
	msg = (struct response *)PTOMSG(bp->b_rptr);
	if (msg->rp_type == RESP_MSG && msg->rp_errno == 0) {
 		switch (msg->rp_opcode)  {
		case DUFCNTL:
			switch (msg->rp_rval) {
			case	F_GETLK:
				i = fcanon("ssllss", msg->rp_data, msg->rp_data);
				break;
			}
			break;
		case DUFSTAT:
			i = fcanon("sssssssllll", msg->rp_data, msg->rp_data);
			break;

		case DUFSTATFS:
			i = fcanon("sllllllc6c6", msg->rp_data, msg->rp_data);
			break;

		case DUGETDENTS:
			i = denfcanon(msg->rp_count, msg->rp_data, msg->rp_data);
			break;

		case DUSTAT:
			i = fcanon("sssssssllll", msg->rp_data, msg->rp_data);
			break;

		case DUSTATFS:
			i = fcanon("sllllllc6c6", msg->rp_data, msg->rp_data);
			break;

		case DUUTSSYS:
			i = fcanon("lsc6c6", msg->rp_data, msg->rp_data);
			break;

		default:
			break;
		}
		if (i)
			msg->rp_count = i;

	}
	if (msg->rp_type == REQ_MSG ) {	  /*rp_type first in req & resp msg*/
		register struct request *rmsg;

		rmsg = (struct request *)PTOMSG(bp->b_rptr);
 		switch (rmsg->rq_opcode)  {
		case DUFCNTL:
			switch (rmsg->rq_cmd) {
			case	F_GETLK:
			case	F_SETLK:
			case	F_SETLKW:
			case	F_CHKFL:
			case	F_FREESP:
				i = fcanon("ssllss", rmsg->rq_data, rmsg->rq_data);
				rmsg->rq_prewrite = i;
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
	}
}


static char rfs_dbuf[2048];


/*This routine is written to convert directory entries to canon form for getdent
*/

dentcanon(count,from,to,flag)
register int count,flag;
register char *from, *to;

{
	register int tlen, tcc;
	struct dirent *dir;
	register char *tmp;

	tlen = 0;
	tmp = rfs_dbuf;

	while(count > 0){
		dir = (struct dirent *)from;
		tcc = tcanon("llsc0",from,tmp,flag);
		tcc = (tcc + 3) & ~3;
		tmp += tcc;
		tlen += tcc;
		from += dir->d_reclen;
		count -= dir->d_reclen;
	}
	bcopy(rfs_dbuf, to, tlen);
	return(tlen);
}




/*
 *This routine is called to convert directory entries from canon form 
 *to local form.
 */

 denfcanon(count,from,to)
 register int count;
 register char *from;
 register char *to;
 {
	
	register int tlen, tcc, tmp;
	struct dirent *dir;

	tlen = 0;
	while(count > 0){
		tcc = 4*sizeof(long) + ((strlen(from + 4*sizeof(long)) +1 + 3) & ~3);
		tmp = fcanon("llsc0",from,to);
		dir = (struct dirent *)to;
		to += dir->d_reclen;
		tlen += dir->d_reclen;
		from += tcc;
		count -= tcc;
	}
	return(tlen);
}
