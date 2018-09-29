/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:nudnix/rmove.c	10.16"
/*  remote copyin
 *  Remote copyin function to bring data from the remote system
 *  (client side) to the local system (server side).
 *  u.u_gift points back to the client awaiting a reply.
 *  May eventually have to packetize to know how many transfers
 *  it takes to get a big copyin across whichever medium used.
 */

#include "sys/types.h"
#include "sys/sema.h"
#include "sys/fs/s5dir.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/stream.h"
#include "sys/comm.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/inode.h"
#include "sys/message.h"
#include "sys/rdebug.h"

rcopyin (from, to, n)
char	*from, *to;
int	n;
{
	register struct	response *resp;
	register struct response *in_msg;
	register struct request *copyin;
	mblk_t	*bp, *in_bp;
	register rcvd_t	rd;
	rcvd_t	giftrd;
	int	ret = 0;
	int	size, i;
	extern	rcvd_t	cr_rcvd();
	extern 	rcopyfault();

	/*  client may have stashed some data with the request  */
	if (u.u_copybp)  {
		copyin = (struct request *)PTOMSG(u.u_copybp->b_rptr);
		size = (copyin->rq_prewrite<n) ? copyin->rq_prewrite:n;
		/* protect against an external memory fault
		   if a fault occurs control will return to rcopyfault in
		   ml/misc.s and bcopy will return with a -1
	 	*/
		u.u_caddrflt = (int) rcopyfault;
		if (bcopy (copyin->rq_data + copyin->rq_sofar, to, size) < 0) {
			u.u_error = EFAULT;
			return (-1);
		}
		u.u_caddrflt = 0;
		copyin->rq_sofar += size;
		copyin->rq_prewrite -= size;
		n -= size;
		if (copyin->rq_prewrite == 0) {
			if (u.u_syscall == DUWRITE || u.u_syscall == DUWRITEI)
				freemsg(u.u_copybp);
			u.u_copybp = NULL;
		}
		if (n == 0)
			return (0);
		to += size;
		from += size;
	}
	/*  send message on u.u_gift and wait for response  */
	if ((rd = cr_rcvd (FILE_QSIZE, SPECIFIC)) == NULL) {
		u.u_error = ENOMEM;
		return (-1);
	}
	giftrd = rd;
	rd->rd_inode = (struct inode *) u.u_gift;
	rd->rd_qsize = SIGQSIZE;	/* almost infinite */
	while ((bp = alocbuf(sizeof(struct response) - DATASIZE, BPRI_MED)) == NULL);
	resp = (struct response *)PTOMSG(bp->b_rptr);
	resp->rp_type = RESP_MSG;
	resp->rp_opcode = DUCOPYIN;
	resp->rp_count = n;
	resp->rp_bufptr = (int) from;
	if(sndmsg (u.u_gift, bp,
	  sizeof (struct response) - DATASIZE, giftrd) == FAILURE) {
		free_rcvd(rd);
		return(-1);
	}
	giftrd = NULL;
	for ( ; n > 0; n -= DATASIZE, to += DATASIZE)  {
		if(de_queue (rd, &in_bp, NULL, &size) == FAILURE) {
			free_rcvd(rd);
			return(-1);
		}
		in_msg = (struct response *) PTOMSG(in_bp->b_rptr);

		if (in_msg->rp_errno){
			ret = -1;
			freemsg (in_bp);
			break;
		} 

		if (ret != -1) {
			u.u_caddrflt = (int) rcopyfault;
			if (bcopy (in_msg->rp_data, to, in_msg->rp_count) < 0) 
				ret = -1;
			else {
				ret = 0;
				u.u_caddrflt = 0;
			}
		}
		freemsg (in_bp);
	}  /*  end for  */
	free_rcvd(rd);
	return (ret);
}


/*  remote copyout
 *  If there is a message ready to send, send it away and allocate
 *  a new message.  If not, allocate a new message.  In both cases,
 *  copy the data into the message and set u.u_copyout to the new
 *  message.
 */

rcopyout (from, to, n)
char	*from, *to;
int	n;
{
	int	size, i;
	mblk_t	*bp;
	rcvd_t	giftrd = NULL;
	mblk_t	*in_bp;
	int	retsize;
	extern 	rcopyfault();

	/*allocate and fill buffer for no-delay copy*/
	if(u.u_rflags & U_RCOPY){
		while ((bp = alocbuf (sizeof (struct response), BPRI_MED)) == NULL);
		u.u_copymsg = (struct response *)PTOMSG(bp->b_rptr);
		u.u_copymsg->rp_type = RESP_MSG;
		size = (n > DATASIZE) ? DATASIZE : n;
		/* protect against an external memory fault
		   if a fault occurs control will return to rcopyfault in
		   ml/misc.s and bcopy will return with a -1
	 	*/
		u.u_caddrflt = (int)rcopyfault;
		if (bcopy (from, u.u_copymsg->rp_data, size) < 0) 
			goto badcopy;
		u.u_caddrflt = 0;
		u.u_copymsg->rp_bufptr = (int) to;
		u.u_copymsg->rp_count = size;
		u.u_copymsg->rp_bp = (long)bp;
		u.u_copymsg->rp_errno = (long)u.u_error;
		u.u_msgend = u.u_copymsg->rp_data + size;
	}
	for (; n > 0; n -= DATASIZE, from += DATASIZE, to += DATASIZE)  {
		/*  if there is already a message ready to go, send it	*/
		if (u.u_copymsg)  {
			u.u_gift->sd_copycnt++;
			u.u_copymsg->rp_opcode = DUCOPYOUT;
			/*  check if count exceeds remote queue size
			 *  if it is, create a gift so that remote side
			 *  can send back positive response before sending more
			 */
			u.u_copymsg->rp_copysync = 0;
			if (u.u_gift->sd_copycnt >= FILE_QSIZE) {
				u.u_copymsg->rp_copysync = 1;
				
				if ((giftrd == NULL) && (giftrd = cr_rcvd (FILE_QSIZE, SPECIFIC)) == NULL) {
					u.u_error = ENOMEM;
					return (-1);
				}
				giftrd->rd_inode = (struct inode *) u.u_gift;

				DUPRINT1(DB_RMOVE, "rcopyout: send gift\n");
				if(sndmsg (u.u_gift, (mblk_t *)u.u_copymsg->rp_bp, 
					sizeof (struct response) - DATASIZE + 
					u.u_copymsg->rp_count,
			 		giftrd) == FAILURE) {
						u.u_copymsg = NULL;
						free_rcvd(giftrd);
						return(-1);
					}
				if(de_queue (giftrd, &in_bp, NULL, &retsize) == FAILURE) {
					u.u_gift->sd_copycnt = 0;
					u.u_copymsg = NULL;
					free_rcvd(giftrd);
					return(-1);
				}
				freemsg (in_bp);
				u.u_gift->sd_copycnt = 0;
			}
			else
				if(sndmsg (u.u_gift, (mblk_t *)u.u_copymsg->rp_bp, 
					sizeof (struct response) - DATASIZE 
					+ u.u_copymsg->rp_count, (rcvd_t)NULL) == FAILURE) {
						u.u_copymsg = NULL;
						if (giftrd)
							free_rcvd(giftrd);
						return(-1);
				}
		}
		if((u.u_rflags & U_RCOPY) && n <= DATASIZE){
			u.u_copymsg = NULL;
			u.u_msgend = 0;
			break;
		}
		/*  allocate a new one and fill in the count  */
		while ((bp = alocbuf (sizeof (struct response), BPRI_MED)) == NULL);
		u.u_copymsg = (struct response *)PTOMSG(bp->b_rptr);
		u.u_copymsg->rp_type = RESP_MSG;
		size = (n > DATASIZE) ? DATASIZE : n;
		u.u_caddrflt = (int)rcopyfault;
		if (bcopy (from, u.u_copymsg->rp_data, size) < 0) 
			goto badcopy;
		u.u_caddrflt = 0;
		u.u_copymsg->rp_bufptr = (int) to;
		u.u_copymsg->rp_count = size;
		u.u_copymsg->rp_bp = (long)bp;
		u.u_copymsg->rp_errno = (long)u.u_error;
		u.u_msgend = u.u_copymsg->rp_data + size;
	}
	if (giftrd)
		free_rcvd(giftrd);
	return (0);
badcopy:
	u.u_copymsg = NULL;
	freemsg (bp);
	return (-1);
}

/*remio(): This routine is called by physio when a remote process tries
 *to write to a raw device.  The data is prefetched from the remote 
 *machine and the normal sequence of physio continues.  This would
 *occur when a B_WRITE is set (ie write from client to server)
*/

remio(rbase,lbase,count)
char *lbase, *rbase;
register int count;
{
	register n;
	if(rcopyin(rbase,lbase,count)){
		u.u_count = count;
		return(u.u_error = EFAULT);
	}
	return(0);
}


/*unremio(): After the data has been read from the raw device, 
 *unremio() has the task of sending the data back across the netowrk
 *to the requesting machine,  This would occur when B_READ is set
 *(ie reading from the server to the client)
*/

unremio(lbase,base,count)
caddr_t base, lbase;
register count;
{
	register n;

	u.u_count = count;
	u.u_base = base;

	while(u.u_count != 0){
		n = min(u.u_count,(DATASIZE*(FILE_QSIZE -1)));
		if( rcopyout(lbase,u.u_base,n)){
			DUPRINT2(DB_GDPERR, "unremio: rcopyout failed count=%d\n",u.u_count);
			return(u.u_error = EFAULT);
		}
		u.u_count -= n;
		u.u_base += n;
		lbase += n;
	}
	return(0);
}


/*
 *rfubyte(): This routine is called by fubyte to copy one byte from the 
 *remote client machine.
*/
rfubyte(from)
unsigned char *from;
{
	char to[sizeof(char)+1];
	if( rcopyin(from, to, sizeof(char))){
		DUPRINT1(DB_GDPERR, "rfubyte: rcopyin failed\n");
		return(-1);
	}
	return(*to);
}
/*
 *rfuword(): This routine is called to copy a word from the remote 
 *client machine.
*/
rfuword(from)
unsigned int *from;
{
	register char to[sizeof(int)+1];
	DUPRINT2(DB_RMOVE,"fuword: from=%x\n",from);
	if(rcopyin(from, to, sizeof(int))){
		DUPRINT2(DB_GDPERR, "rfuword: copyin failed errno=%d\n",u.u_error);
		return(-1);
	}
	return((unsigned)(*to));
}
/*
 *rsubyte(): This routine passes c back to the remote user. 
*/
rsubyte(to,c)
unsigned char *to;
unsigned char c;
{
	u.u_rflags |= U_RCOPY;
	DUPRINT2(DB_RMOVE,"rsubyte: setting byte%x\n",c);
	if(rcopyout((char *)(&c),to,sizeof(char))){
		DUPRINT2(DB_GDPERR, "rsubyte: rcopyout failed err=%d\n",u.u_error);
		u.u_rflags &= ~U_RCOPY;
		return(-1);
	}
	u.u_rflags &= ~U_RCOPY;
	return(0);
}
/*
 *rsuword(): This routine passes back w to the remote user's space.
 *
*/
rsuword(to,c)
unsigned int *to;
unsigned int c;
{
	u.u_rflags |= U_RCOPY;
	DUPRINT2(DB_RMOVE,"rsuword:setting word %x\n",c);
	if(rcopyout((char *)(&c), to, sizeof(int))){
		DUPRINT2(DB_GDPERR, "rsuword: rcopyout failed err=%d\n",u.u_error);
		u.u_rflags &= ~U_RCOPY;
		return(-1);
	}
	u.u_rflags &= ~U_RCOPY;
	return(0);
}
