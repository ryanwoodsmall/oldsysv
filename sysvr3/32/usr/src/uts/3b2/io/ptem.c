/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:io/ptem.c	10.2"
/*
 *	         Copyright (c) 1987 AT&T
 *		   All Rights Reserved
 *
 *     THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
 *   The copyright notice above does not evidence any actual
 *   or intended publication of such source code.
 *
 * Description: The PTEM streams module is used as a pseudo
 *		driver emulator. Its purpose is to simulate
 *		the ioctl() functions of a real device driver 
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/stream.h"
#include "sys/stropts.h"
#include "sys/termio.h"
#include "sys/immu.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/ttold.h"
#include "sys/errno.h"
#include "sys/cmn_err.h"
#include "sys/jioctl.h"
#include "sys/ptem.h"

#ifdef DBUG
#define DEBUG(a)	 if (ptem_debug) cmn_err(CE_CONT,a) 
#else
#define DEBUG(a)
#endif 

extern struct ptem ptem[];
extern int ptem_cnt;
extern nulldev();

int ptem_debug = 0;

#define PTEM_ID	43981	/* 0xabcd */

/*
 * stream data structure definitions
 */
int ptemopen(), ptemclose(), ptemrput(), ptemwput();

static struct module_info ptem_info = { PTEM_ID, "ptem", 0, INFPSZ, 4096, 1024};

static struct qinit ptemrinit = { ptemrput, NULL, ptemopen, ptemclose, nulldev, &ptem_info, NULL};

static struct qinit ptemwinit = { ptemwput, NULL, ptemopen, ptemclose, nulldev, &ptem_info, NULL};

struct streamtab pteminfo = { &ptemrinit, &ptemwinit, NULL, NULL };


/*
 * ptemopen - open routine gets called when the
 *	       module gets pushed onto the stream.
 */

ptemopen( q, dev, oflag, sflag)
register queue_t *q;	/* pointer to the read side queue */
int dev;		/* the device number(major and minor) */
int sflag;		/* Open state flag */
int oflag;		/* The user open(2) supplied flags */
{
	register struct ptem *ntp;	/* Pointer to a ptem entry for this PTEM module */
	register mblk_t *mop;		/* Pointer to a setopts message block */


	DEBUG(("ptem: open()\n"));

	if ( sflag != MODOPEN) {
		u.u_error = EINVAL;
		return( OPENFAIL);
	}

	if ( q->q_ptr != NULL)			/* already attached */
		return( 1);

	/*
	 * Find a free ptem entry
	 */
	for ( ntp = ptem; ntp->state&INUSE; ntp++)
		if ( ntp >= &ptem[ptem_cnt-1]) {
			DEBUG(("No ptem structures.\n"));
			u.u_error = ENODEV;
			return( OPENFAIL);
		}

	/*
	 * save the process group of the controlling tty in the ptem record
	 */
	if ( ( u.u_procp->p_pid == u.u_procp->p_pgrp)
		&& ( u.u_ttyp == NULL) && ( ntp->ttypgid == 0)) {
		u.u_ttyp = (short *)&ntp->ttypgid;
		u.u_ttyd = dev;
		ntp->ttypgid = u.u_procp->p_pgrp;
	}

	/*
	 * allocate a message block, used to pass the "zero length message
	 * for "stty 0"
	 * NOTE: its better to find out if such a message block can be
	 *	 allocated before its needed than to not be able to
	 *	 deliver(for possible lack of buffers) when a hang-up
	 *	 occurs.
	 */
	if ( ( ntp->dack_ptr = (mblk_t *)allocb( 4, BPRI_MED)) == NULL) {
		u.u_error = EAGAIN;
		return ( OPENFAIL);
	}

	/*
	 * set up hi/lo water marks on stream head read queue
	 */
	if ( mop = allocb( sizeof( struct stroptions), BPRI_MED)) {
		register struct stroptions *sop;
		
		mop->b_datap->db_type = M_SETOPTS;
		mop->b_wptr += sizeof(struct stroptions);
		sop = (struct stroptions *)mop->b_rptr;
		sop->so_flags = SO_HIWAT | SO_LOWAT;
		sop->so_hiwat = 512;
		sop->so_lowat = 256;
		putnext( q, mop);
	} else {
		freemsg( ntp->dack_ptr);
		ntp->dack_ptr = NULL;
		u.u_error = ENOMEM;
		return ( OPENFAIL);
	}

	q->q_ptr = (caddr_t)ntp;
	WR(q)->q_ptr = (caddr_t)ntp;
	/*
	 * Assign default cflag values cf termio(7)
	 * zero out the windowing paramters
	 */
	ntp->cflags = B300|CS8|CREAD|HUPCL;
	ntp->state = INUSE;

	ntp->wsz.ws_row = 0;
	ntp->wsz.ws_col = 0;
	ntp->wsz.ws_xpixel = 0;
	ntp->wsz.ws_ypixel = 0;
	
	return ( 1);
}


/*
 * ptemclose - This routine gets called when the module
 *              gets popped off of the stream.
 */

ptemclose( q)
register queue_t *q;	/* Pointer to the read queue */
{
	register struct ptem *ntp = (struct ptem *)q->q_ptr;

	DEBUG(("ptem: ntclose()\n"));

	ntp->state = 0;
	ntp->wsz.ws_row = 0;
	ntp->wsz.ws_col = 0;
	ntp->wsz.ws_xpixel = 0;
	ntp->wsz.ws_ypixel = 0;
	q->q_ptr = (caddr_t)NULL;
	WR(q)->q_ptr = (caddr_t)NULL;
	if ( ntp->dack_ptr) {
		freemsg( ntp->dack_ptr);
		ntp->dack_ptr = NULL;
	}
	return;
}


/*
 * ptemrput - Module read queue put procedure.
 *             This is called from the module or
 *	       driver downstream.
 */

ptemrput( q, mp)
register queue_t *q;	/* Pointer to the read queue */
register mblk_t *mp;	/* Pointer to the current message block */
{
	struct iocblk *iocp;	/* Pointer to the M_IOCTL data */
	struct copyresp *resp;


	DEBUG("ptem: ptemrput()\n");

	switch( mp->b_datap->db_type) {

	case M_DELAY:
	case M_READ:
		DEBUG( "ptemrput: case M_DELAY|M_READ freemsg\n");
		freemsg( mp);
		return;

	case M_IOCTL:
		DEBUG( "ptemrput: case M_IOCTL\n");
		iocp = (struct iocblk *)mp->b_rptr;

		switch ( iocp->ioc_cmd) {
		case TCSBRK:
			/*
			 * Send a break message upstream
			 */
			DEBUG( "ptemrput: got TSCBRK send M_BREAK upstream\n");
			if ( putctl( q->q_next, M_BREAK))
				/*
				 * Then send an ACK reply back
				 */
				mp->b_datap->db_type = M_IOCACK; 
			else
				/*
				 * Or send an NCK reply back
				 */
				mp->b_datap->db_type = M_IOCNAK; 

			iocp->ioc_count = 0;
			DEBUG( "ptemrput: and send a M_IOCACK downstream\n");
			qreply( q, mp);
			return;

		case TIOCSWINSZ:
			DEBUG( "ptemrput: case TIOCSWISZ\n");
			if ( iocp->ioc_count == TRANSPARENT) {
				register struct copyreq *get_buf_p;

				get_buf_p = ( struct copyreq *)mp->b_rptr;
				get_buf_p->cq_private = NULL;
				get_buf_p->cq_flag = 0;
				get_buf_p->cq_size = sizeof( struct winsize);
				get_buf_p->cq_addr = (caddr_t) (*(long *)(mp->b_cont->b_rptr));
				mp->b_datap->db_type = M_COPYIN;
				qreply( q, mp);
			} else
				ptioc( q, mp, RDSIDE);

			return;

		case JWINSIZE:
		case TIOCGWINSZ:
			DEBUG( "ptemrput: case JWINSIZE|TIOCGWINSZ\n");
			ptioc( q, mp, RDSIDE);
			return;

		default:
			putnext( q, mp);
			return;
		}
		break;

	case M_IOCDATA:
		DEBUG( "ptemrput: case M_IOCDATA\n");
		resp = (struct copyresp *)mp->b_rptr;
		if ( resp->cp_rval)  {
			/*
			 * Just return on failure
			 */
			freemsg( mp);
			return;
		}

		/*
		 * Only need to copy data for the SET case
		 */
		switch ( resp->cp_cmd) {

			case  TIOCSWINSZ:
				DEBUG("ptem: ptemrput() TIOCSWINSZ\n");
				ptioc( q, mp, RDSIDE);
				break;

			case JWINSIZE:
				DEBUG("ptem: ptemrput() JWINSIZE\n");

			case  TIOCGWINSZ:
				DEBUG("ptem: ptemrput() TIOCGWINSZ\n");
				iocp = (struct iocblk *)mp->b_rptr;
				iocp->ioc_error = 0;
				iocp->ioc_count = 0;
				iocp->ioc_rval = 0;
				mp->b_datap->db_type = M_IOCACK;
				qreply( q, mp);
				break;

			default:
				freemsg( mp);
		}	
		return;

	default:
		putnext( q, mp);
		break;
	}
	return;
}


/*
 * ptemwput - Module write queue put procedure.
 *             This is called from the module or
 *	       stream head upstream.
 */

ptemwput( q, mp)
register queue_t *q;	/* Pointer to the read queue */
register mblk_t *mp;	/* Pointer to current message block */
{
	struct iocblk *iocp;	/* pointer to the out going ioctl structure */
	struct termio *cb;
	struct sgttyb *gb;
	struct ptem *ntp;
	struct copyresp *resp;

	mblk_t *dack_ptr;	/* pointer to the disconnect message ACK block */
	mblk_t *ctlmp;

	DEBUG(("ptem: ptemwput()\n"));

	ntp = (struct ptem *)q->q_ptr;

	switch ( mp->b_datap->db_type) {

	case M_STOP:
		DEBUG( "ptemwput: case M_STOP\n");
		freemsg( mp);
		ntp->q_ptr = q;
		/*
		 * send ASCII DC3 - Stop character
		 */
		stop_fctl( ntp);
		break;
	
	case M_START:
		DEBUG( "ptemwput: case M_START\n");
		freemsg( mp);
		ntp->q_ptr = q;
		/*
		 * send ASCII DC1 - Start character
		 */
		start_fctl( ntp);
		break;

	case M_IOCDATA:
		DEBUG( "ptemwput: case M_IOCDATA\n");
		resp = (struct copyresp *)mp->b_rptr;
		if ( resp->cp_rval)  {
			/*
			 * Just return on failure
			 */
			freemsg( mp);
			return;
		}

		/*
		 * Only need to copy data for the SET case
		 */
		switch ( resp->cp_cmd) {

			case  TIOCSWINSZ:
				DEBUG("ptem: ptemwput() TIOCSWINSZ\n");
				ptioc( q, mp, WRTSIDE);
				break;

			case JWINSIZE:
				DEBUG("ptem: ptemwput() JWINSIZE\n");

			case  TIOCGWINSZ:
				DEBUG("ptem: ptemwput() TIOCGWINSZ\n");
				iocp = (struct iocblk *)mp->b_rptr;
				iocp->ioc_error = 0;
				iocp->ioc_count = 0;
				iocp->ioc_rval = 0;
				mp->b_datap->db_type = M_IOCACK;
				qreply( q, mp);
				break;

			default:
				freemsg( mp);
		}	
			
		return;

	case M_IOCTL:
		DEBUG( "ptemwput: case M_IOCTL\n");
		iocp = (struct iocblk *)mp->b_rptr;
		switch ( iocp->ioc_cmd) {

		case TCSETA:
		case TCSETAF:
		case TCSETAW:
			DEBUG( "ptemwput: case TCSETA|TCSETAF|TCSETAW\n");
			cb = (struct termio *)mp->b_cont->b_rptr;
			ntp->cflags = cb->c_cflag;
			mp->b_datap->db_type = M_IOCACK;
			iocp->ioc_count = 0;

			if ( ( cb->c_cflag&CBAUD) == B0) {
				/*
				 * hang-up: Send a zero lenght message
				 */
				DEBUG(("ptemwput():Hang-up: Send a zero length message\n"));
				dack_ptr = ntp->dack_ptr;

				if ( dack_ptr) {
					ntp->dack_ptr = NULL;	/* use only once */
					/*
					 * Send a zero lenght message downstream
					 */
					dack_ptr->b_rptr = dack_ptr->b_wptr 
					 = (unsigned char *)dack_ptr->b_datap->db_base;
					putnext( q, dack_ptr);
				}
			}
			/*
			 * Send ACK upstream
			 */
			qreply( q, mp);
			return;

		case TCGETA:
			DEBUG( "ptemwput: case TCGETA\n");
			if ( !( mp->b_cont = ( allocb( sizeof (struct termio), BPRI_MED)))) {
				mp->b_datap->db_type = M_IOCNAK;
				iocp->ioc_error = EAGAIN;
				iocp->ioc_count = 0;
				qreply( q, mp);
				return;
			}
			mp->b_cont->b_wptr =  mp->b_cont->b_rptr + sizeof(struct termio);
			cb = (struct termio *)mp->b_cont->b_rptr;
			cb->c_cflag = ntp->cflags;
			mp->b_datap->db_type = M_IOCACK;
			iocp->ioc_count = sizeof( struct termio);
			qreply( q, mp);
			return;

		case TCSBRK:
			DEBUG( "ptemwput: case TCSBRK\n");
			mp->b_datap->db_type = M_IOCACK;
			iocp->ioc_count = 0;
			qreply( q, mp);
			/*
			 * TCSBRK meaningful if data part of message is 0
			 * cf. termio(7)
			 */
			if ( !(*(int *)mp->b_cont->b_rptr)) {
				DEBUG( "ptemwput: got TCSBREAK send M_BREAK downstream\n");
				putctl( q->q_next, M_BREAK);
			}
			return;

		case JWINSIZE:
			DEBUG( "ptemwput: case JWINSIZE\n");

		case TIOCGWINSZ:
			DEBUG( "ptemwput: case TIOCGWINSZ\n");
				ptioc( q, mp, WRTSIDE);
			return;

		case TIOCSWINSZ:
			DEBUG( "ptemwput: case TIOCSWISZ\n");
			if ( iocp->ioc_count == TRANSPARENT) {
				register struct copyreq *get_buf_p;

				get_buf_p = ( struct copyreq *)mp->b_rptr;
				get_buf_p->cq_private = NULL;
				get_buf_p->cq_flag = 0;
				get_buf_p->cq_size = sizeof( struct winsize);
				get_buf_p->cq_addr = (caddr_t) (*(long*)(mp->b_cont->b_rptr));
				mp->b_datap->db_type = M_COPYIN;
				qreply( q, mp);
			} else
				ptioc( q, mp, WRTSIDE);

			return;

		case TIOCGETP:
			DEBUG( "ptemwput: case TIOCGETP\n");
			if ( ( mp->b_cont = allocb( sizeof( struct sgttyb), BPRI_MED)) == NULL) {
				mp->b_datap->db_type = M_IOCNAK;
				iocp->ioc_error = EAGAIN;
				qreply( q, mp);
				return;
			}

			gb = (struct sgttyb *)mp->b_cont->b_rptr;

			gb->sg_ispeed = ntp->cflags&CBAUD;
			gb->sg_ospeed = gb->sg_ispeed;

			gb->sg_flags = 0;

			if ( ntp->cflags&HUPCL)
				gb->sg_flags |= O_HUPCL;

			if ( ntp->cflags&PARODD)
				gb->sg_flags |= O_ODDP;

			mp->b_cont->b_wptr += sizeof( struct sgttyb);
			mp->b_datap->db_type = M_IOCACK;
			iocp->ioc_error = 0;
			iocp->ioc_count = sizeof(struct sgttyb);
			qreply( q, mp);
			return;

		case TIOCSETP:
			DEBUG( "ptemwput: case TIOCSETP\n");
			gb = (struct sgttyb *)mp->b_cont->b_rptr;

			ntp->cflags = (gb->sg_ispeed&CBAUD)|CREAD;
			if ( ( ntp->cflags&CBAUD) == B110)
				ntp->cflags |= CSTOPB;
			if ( gb->sg_flags&O_HUPCL)
				ntp->cflags |= HUPCL;
			if ( !(gb->sg_flags&O_RAW))
				ntp->cflags |= CS7|PARENB;
			if ( gb->sg_flags&O_ODDP)
				if ( !(gb->sg_flags&O_EVENP))
					ntp->cflags |= PARODD;

			mp->b_datap->db_type = M_IOCACK;
			iocp->ioc_count = 0;
			iocp->ioc_error = 0;

			if ( (ntp->cflags&CBAUD) == B0) {
				/*
				 * hang-up: Send a zero lenght message
				 */

				DEBUG(("ptemwput():Hang-up: Send a zero length message\n"));

				dack_ptr = ntp->dack_ptr;

				if ( dack_ptr) {
					ntp->dack_ptr = NULL;	/* use only once */
					/*
					 * Send a zero lenght message downstream
					 */
					dack_ptr->b_rptr = dack_ptr->b_wptr 
					 = (unsigned char *)dack_ptr->b_datap->db_base;
					putnext( q, dack_ptr);
				}
			}
			qreply( q, mp);
			return;

		default:
			DEBUG( "ptemwput: default IOCTL cmd type\n");
			putnext( q, mp);
			return;
	}
	
	case M_DELAY:
	case M_READ:
		DEBUG( "ptemwput: case M_DELAY|M_READ freemsg\n");
		/*
		 * tty delays not supported at this time
		 */
		freemsg( mp);
		break;

	default:
		DEBUG( "ptemwput: default data type\n");
		putnext( q, mp);
		break;
	}
	return;
}
ptioc( q, mp, qside)
mblk_t *mp;
queue_t *q;
int qside;
{
	struct ptem *tp;
	struct iocblk *iocp;
	mblk_t *tmp;
	struct winsize *wb;
	struct jwinsize *jwb;
	register struct copyreq *send_buf_p;

	DEBUG(("ptemioc:\n"));

	iocp = (struct iocblk *)mp->b_rptr;
	tp = (struct ptem *)q->q_ptr;

	/*
	 * Message must be of type M_IOCTL for this routine to be called
	 */
	switch ( iocp->ioc_cmd) {
			
	case JWINSIZE:
		DEBUG (("ptioc:case JWINSIZE\n"));


		/*
		 * If all zeros, NAK the message for dumb terminals
		 */
		if ( ( tp->wsz.ws_row == 0) && ( tp->wsz.ws_col == 0) && 
			( tp->wsz.ws_xpixel == 0) && ( tp->wsz.ws_ypixel == 0)) {
				mp->b_datap->db_type = M_IOCNAK;
				iocp->ioc_count = 0;
				iocp->ioc_error = EINVAL;
				iocp->ioc_rval = -1;
				qreply( q, mp);
				return;
		}

		if ( ( tmp = allocb( sizeof( struct jwinsize), BPRI_MED)) == NULL) {
			mp->b_datap->db_type = M_IOCNAK;
			iocp->ioc_count = 0;
			iocp->ioc_error = EAGAIN;
			iocp->ioc_rval = -1;
			qreply( q, mp);
			return;
		}
		
		if ( iocp->ioc_count == TRANSPARENT) {
			send_buf_p = ( struct copyreq *)mp->b_rptr;
			send_buf_p->cq_addr = (caddr_t) (*(long *)(mp->b_cont->b_rptr));
			freemsg( mp->b_cont);
			mp->b_cont = tmp;
			tmp->b_wptr += sizeof( struct jwinsize);
			send_buf_p->cq_private = NULL;
			send_buf_p->cq_flag = 0;
			send_buf_p->cq_size = sizeof( struct jwinsize);
			mp->b_datap->db_type = M_COPYOUT;
		} else {
			if ( mp->b_cont)
				freemsg( mp->b_cont);
			mp->b_cont = tmp;
			mp->b_datap->db_type = M_IOCACK;
			tmp->b_wptr += sizeof( struct jwinsize);
			iocp->ioc_count = sizeof( struct jwinsize);
			iocp->ioc_error = 0;
			iocp->ioc_rval = 0;
		}
		jwb = ( struct jwinsize *)mp->b_cont->b_rptr;

		jwb->bytesx = tp->wsz.ws_row;
		jwb->bytesy = tp->wsz.ws_col;
		jwb->bitsx = tp->wsz.ws_xpixel;
		jwb->bitsy = tp->wsz.ws_ypixel;

		qreply( q, mp);
		return;

	case TIOCGWINSZ:
		DEBUG(("ptioc:case TIOCGWINSZ\n"));

		/*
		 * If all zeros NAK the message for dumb terminals
		 */
		if ( ( tp->wsz.ws_row == 0) && ( tp->wsz.ws_col == 0) && 
		   ( tp->wsz.ws_xpixel == 0) && ( tp->wsz.ws_ypixel == 0)) {
			mp->b_datap->db_type = M_IOCNAK;
			iocp->ioc_count = 0;
			iocp->ioc_error = EINVAL;
			iocp->ioc_rval = -1;
			qreply( q, mp);
			return;
		}

		if ( ( tmp = allocb( sizeof( struct winsize), BPRI_MED)) == NULL)  {
			mp->b_datap->db_type = M_IOCNAK;
			iocp->ioc_count = 0;
			iocp->ioc_error = EAGAIN;
			iocp->ioc_rval = -1;
			qreply( q, mp);
			return;
		}

		if ( iocp->ioc_count == TRANSPARENT) {
			send_buf_p = ( struct copyreq *)mp->b_rptr;
			send_buf_p->cq_addr = (caddr_t)(*(long *)(mp->b_cont->b_rptr));
			freemsg( mp->b_cont);
			mp->b_cont = tmp;
			tmp->b_wptr += sizeof( struct winsize);
			send_buf_p->cq_private = NULL;
			send_buf_p->cq_flag = 0;
			send_buf_p->cq_size = sizeof( struct winsize);
			mp->b_datap->db_type = M_COPYOUT;
		} else {
			if ( mp->b_cont)
				freemsg( mp->b_cont);
			mp->b_cont = tmp;
			tmp->b_wptr += sizeof( struct winsize);
			mp->b_datap->db_type = M_IOCACK;
			iocp->ioc_count = sizeof( struct winsize);
			iocp->ioc_error = 0;
			iocp->ioc_rval = 0;
		}

		wb = (struct winsize *)mp->b_cont->b_rptr;
		wb->ws_row = tp->wsz.ws_row;
		wb->ws_col = tp->wsz.ws_col;
		wb->ws_xpixel = tp->wsz.ws_xpixel;
		wb->ws_ypixel = tp->wsz.ws_ypixel;

		qreply( q, mp);
		return;

	case TIOCSWINSZ:
		DEBUG(("ptioc:case TIOCSWINSZ\n"));

		wb = (struct winsize *)mp->b_cont->b_rptr;

	        /*
		 * Send a window change signal if the information is new
		 */
		if ( ( tp->wsz.ws_row != wb->ws_row) || 
		     ( tp->wsz.ws_col != wb->ws_col) ||
		     ( tp->wsz.ws_xpixel != wb->ws_xpixel) ||
		     ( tp->wsz.ws_ypixel != wb->ws_xpixel))  {

		        /*
			 * SIGWINCH is always sent upstream
			 */
			if ( qside == WRTSIDE)
				putctl1( RD(q)->q_next, M_SIG, SIGWINCH);
			else if( qside == RDSIDE)
				putctl1( q->q_next, M_SIG, SIGWINCH);
		}

		tp->wsz.ws_row = wb->ws_row;
		tp->wsz.ws_col = wb->ws_col;
		tp->wsz.ws_xpixel = wb->ws_xpixel;
		tp->wsz.ws_ypixel = wb->ws_ypixel;

		iocp->ioc_count = 0;
		iocp->ioc_error = 0;
		iocp->ioc_rval = 0;

		/*
		 * Send it downstream if the ioctl is from
		 * write side, so other modules can see it,
		 * there is an assumption that the drivers
		 * will ACK this ioctl
		 */

		mp->b_datap->db_type = M_IOCACK;
		qreply( q, mp);
		return;

	default:
		putnext( q, mp);
		return;

	}
}
/*
 * stop_fctl: is called if an allocb is needed to send
 *		send an ASCII DC3 downstream
 *	    if a message block cannot be allocated bufcall
 *	    is invoked. Note only 3 tries to allocb() are made.
 *	    A Medium priority message is tried for first, then
 *	    if this fails two more attempts to get a High priority
 *	    buffer are made.
 */
int
stop_fctl( ntp)
struct ptem *ntp;
{
	static short no_of_retries = 0;

	mblk_t *ctlmp;
	

	if ( ctlmp = allocb( 4, ( !no_of_retries)?BPRI_MED:BPRI_HI)) {
		no_of_retries = 0;
		/*
		 * send ASCII DC3 - Stop character
		 */
		*ctlmp->b_wptr++ = 023;	
		putnext( ntp->q_ptr, ctlmp);
	} else {
		/*
		 * Stop retrying after 3 failed allocb() calls
		 */
		if ( ++no_of_retries <= 3)
			bufcall( 4, BPRI_HI, stop_fctl, ntp);
		else {
			cmn_err( CE_WARN, "Streams module PTEM: can't get buffer to send ASCII DC3\n");
			no_of_retries = 0;
		}
	}

	return;
}
/*
 * start_fctl: is called if an allocb is needed to send
 *		send an ASCII DC1 downstream
 *	    if a message block cannot be allocated bufcall
 *	    is invoked. Note only 3 tries to allocb() are made.
 *	    A Medium priority message is tried for first, then
 *	    if this fails two more attempts to get a High priority
 *	    buffer are made.
 */
int
start_fctl( ntp)
struct ptem *ntp;
{
	static short no_of_retries = 0;

	mblk_t *ctlmp;
	

	if ( ctlmp = allocb( 4, ( !no_of_retries)?BPRI_MED:BPRI_HI)) {
		no_of_retries = 0;
		/*
		 * send ASCII DC1 - Start character
		 */
		*ctlmp->b_wptr++ = 021;	
		putnext( ntp->q_ptr, ctlmp);
	} else {
		/*
		 * Stop retrying after 3 failed allocb() calls
		 */
		if ( ++no_of_retries <= 3)
			bufcall( 4, BPRI_HI, start_fctl, ntp);
		else {
			cmn_err( CE_WARN, "Streams module PTEM: can't get buffer to send ASCII DC1\n");
			no_of_retries = 0;
		}
	}

	return;
}
