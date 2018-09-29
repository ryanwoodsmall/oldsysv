/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:nudnix/rfcanon.c	10.5"

#include "sys/types.h"
#include "sys/sema.h"
#include "sys/stream.h"
#include "sys/comm.h"
#include "sys/message.h"
#include "sys/dirent.h"
#include "sys/hetero.h"
#include "sys/rdebug.h"



/*
 *	tocanon routine for RFS
 *
 *	convert all RFS header and data parts to canonical formats
 *	called by sndmsg() before passing data to protocol module
 */

rftocanon (bp, hetero)
register mblk_t	*bp;
register int hetero;
{
	register struct response *msg;
	register int i;

	if (hetero == NO_CONV)
		return;

	/*
	 *	convert RFS data portion to canonical form
	 */
	msg = (struct response *)PTOMSG(bp->b_rptr);
	if (msg->rp_type == RESP_MSG && msg->rp_errno == 0) {
 		switch (msg->rp_opcode)  {
		case DUFCNTL:
			i = tcanon("ssllss", msg->rp_data, msg->rp_data, 1);

		case DUFSTAT:
			i = tcanon("sssssssllll", msg->rp_data, msg->rp_data, 1);
			break;

		case DUFSTATFS:
			i = tcanon("sssllllc6c6", msg->rp_data, msg->rp_data, 1);
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
			i = tcanon("sssllllc6c6", msg->rp_data, msg->rp_data, 1);
			break;

		case DUUTSSYS:
			i = tcanon("lsc6c6", msg->rp_data, msg->rp_data, 1);
			break;

		default:
			i = 0;
			break;
		}

		/* adjust the stream write pointer due to conversion expansion */
		if (i)
			bp->b_wptr = bp->b_rptr + sizeof(struct message) 
				     + sizeof(struct response) - DATASIZE + i;
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
	register int i;

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
			i = fcanon("ssllss", msg->rp_data, msg->rp_data);

		case DUFSTAT:
			i = fcanon("sssssssllll", msg->rp_data, msg->rp_data);
			break;

		case DUFSTATFS:
			i = fcanon("sssllllc6c6", msg->rp_data, msg->rp_data);
			break;

		case DUGETDENTS:
			i = denfcanon(msg->rp_count, msg->rp_data, msg->rp_data);
			if (i)
				msg->rp_count = i;
			break;

		case DUSTAT:
			i = fcanon("sssssssllll", msg->rp_data, msg->rp_data);
			break;

		case DUSTATFS:
			i = fcanon("sssllllc6c6", msg->rp_data, msg->rp_data);
			break;

		case DUUTSSYS:
			i = fcanon("lsc6c6", msg->rp_data, msg->rp_data);
			break;

		default:
			break;
		}

	}
}





/*This routine is written to convert directory entries to canon form for getdent
*/

dentcanon(count,from,to,flag)
register int count,flag;
register char *from, *to;

{
	register int tlen, tcc;
	struct dirent *dir;
	char cbuf[1400];
	register char *tmp;

	tlen = 0;
	tmp = cbuf;

	while(count > 0){
		dir = (struct dirent *)from;
		tcc = tcanon("llsc0",from,tmp,flag);
		tcc = (tcc + 3) & ~3;
		tmp += tcc;
		tlen += tcc;
		from += dir->d_reclen;
		count -= dir->d_reclen;
	}
	bcopy(cbuf, to, tlen);
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
	
	register int tlen, tcc;
	struct dirent *dir;

	tlen = 0;
	while(count > 0){
		tcc = fcanon("llsc0",from,to);
		dir = (struct dirent *)to;
		to += dir->d_reclen;
		tlen += dir->d_reclen;
		tcc = 4*sizeof(long) + ((strlen(from + 4*sizeof(long)) +1 + 3) & ~3);
		from += tcc;
		count -= tcc;
	}
	return(tlen);
}
