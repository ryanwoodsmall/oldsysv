/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:io/ldterm.c	1.5"
/* 
 * tty line discipline 0:
 * pushable streams module
 */

#include "sys/param.h"
#include "sys/signal.h"
#include "sys/types.h"
#include "sys/immu.h"
#include "sys/systm.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/fs/s5dir.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/stream.h"
#include "sys/stropts.h"
#include "sys/conf.h"
#include "sys/ttold.h"
#include "sys/termio.h"
#include "sys/jioctl.h"
#include "sys/ldterm.h"
#include "sys/errno.h"

extern struct ldterm ldterm_tty[];
extern int	ldterm_cnt;

static char	maptab[] = {		/* upper-lower case conversion */
	000, 000, 000, 000, 000, 000, 000, 000,
	000, 000, 000, 000, 000, 000, 000, 000,
	000, 000, 000, 000, 000, 000, 000, 000,
	000, 000, 000, 000, 000, 000, 000, 000,
	000, '|', 000, 000, 000, 000, 000, '`',
	'{', '}', 000, 000, 000, 000, 000, 000,
	000, 000, 000, 000, 000, 000, 000, 000,
	000, 000, 000, 000, 000, 000, 000, 000,
	000, 000, 000, 000, 000, 000, 000, 000,
	000, 000, 000, 000, 000, 000, 000, 000,
	000, 000, 000, 000, 000, 000, 000, 000,
	000, 000, 000, 000, '\\', 000, '~', 000,
	000, 'A', 'B', 'C', 'D', 'E', 'F', 'G',
	'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
	'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
	'X', 'Y', 'Z', 000, 000, 000, 000, 000
};


/* parity table for ascii characters */
/* if 6 lo bits = 0, no special output processing required */
static char	partab[] = {
	0001, 0201, 0201, 0001, 0201, 0001, 0001, 0201,
	0202, 0004, 0003, 0201, 0005, 0206, 0201, 0001,
	0201, 0001, 0001, 0201, 0001, 0201, 0201, 0001,
	0001, 0201, 0201, 0001, 0201, 0001, 0001, 0201,
	0200, 0000, 0000, 0200, 0000, 0200, 0200, 0000,
	0000, 0200, 0200, 0000, 0200, 0000, 0000, 0200,
	0000, 0200, 0200, 0000, 0200, 0000, 0000, 0200,
	0200, 0000, 0000, 0200, 0000, 0200, 0200, 0000,
	0200, 0000, 0000, 0200, 0000, 0200, 0200, 0000,
	0000, 0200, 0200, 0000, 0200, 0000, 0000, 0200,
	0000, 0200, 0200, 0000, 0200, 0000, 0000, 0200,
	0200, 0000, 0000, 0200, 0000, 0200, 0200, 0000,
	0000, 0200, 0200, 0000, 0200, 0000, 0000, 0200,
	0200, 0000, 0000, 0200, 0000, 0200, 0200, 0000,
	0200, 0000, 0000, 0200, 0000, 0200, 0200, 0000,
	0000, 0200, 0200, 0000, 0200, 0000, 0000, 0201
};



int ldtermopen(), ldtermclose(), ldtermin(), ldterminsrv(), ldtermosrv(), ldtermout();
void ldflush_buf(), ldvmin_ready();
extern nulldev();
struct module_info ldtermmiinfo = { 
	0x0bad, "ldterm", 0, INFPSZ, 300, 100 };


struct module_info ldtermmoinfo = { 
	0x0bad, "ldterm", 0, INFPSZ, 300, 200 };


struct qinit ldtermrinit = {
	ldtermin, ldterminsrv, ldtermopen, ldtermclose, nulldev, &ldtermmiinfo, NULL};


struct qinit ldtermwinit = {
	ldtermout, ldtermosrv, ldtermopen, ldtermclose, nulldev, &ldtermmoinfo, NULL};


struct streamtab ldtrinfo = { 
	&ldtermrinit, &ldtermwinit };


ldtermopen(q, dev, oflag, sflag)
register queue_t *q;
int	dev, oflag, sflag;
{
	register struct ldterm *tp;
	register mblk_t *mp;
	struct stroptions *sop;


	DEBUG4 (("ldtermopen called\n"));

	if (sflag != MODOPEN) {
		u.u_error = EINVAL;
		return (OPENFAIL);
	}
	if (q->q_ptr != NULL) {	/* already attached */
		return(1);
	}
	if ((mp = allocb(sizeof(struct stroptions), BPRI_MED)) == NULL) {
		u.u_error = EAGAIN;
		return (OPENFAIL);
	}
	mp->b_datap->db_type = M_SETOPTS;
	mp->b_wptr += sizeof (struct stroptions);
	sop = (struct stroptions *)mp->b_rptr;
	sop->so_flags = SO_READOPT|SO_NDELON;
	sop->so_readopt = RMSGN;
	putnext (q, mp);


	for (tp = ldterm_tty; tp->ld_state & TTUSE; tp++)
		if (tp >= &ldterm_tty[ldterm_cnt-1]) {
			DEBUG1(("ldterm: No ldterm_tty structures.\n"));
			u.u_error = ENOSPC;
			return(OPENFAIL);
		}

	tp->ld_state 	= TTUSE;
	tp->ld_iflags 	= 0;
	tp->ld_oflags 	= 0;
	tp->ld_lflags 	|= ISIG | ICANON | ECHO;
	tp->ld_col 	= 0;
	tp->ld_erase 	= CERASE;
	tp->ld_kill 	= CKILL;
	tp->ld_intrc 	= CINTR;
	tp->ld_quitc 	= CQUIT;
	tp->ld_startc 	= CSTART;	/* ^Q */
	tp->ld_stopc 	= CSTOP;	/* ^S */
	tp->ld_eofc 	= CEOF;
	tp->ld_eol 	= 0;
	tp->ld_eol2 	= 0;
	tp->ld_swtchc 	= CSWTCH;

	tp->op 		= tp->canonb;   /* array of 256 statically allocated */
	tp->lmode_flg 	= 0;		/* Fix for type ahead */
	tp->rawlength 	= 0;		/* counter for vmin   */
	q->q_ptr 	= (caddr_t)tp;
	WR(q)->q_ptr 	= (caddr_t)tp;
	tp->echobuf 	= NULL;
	tp->raw_message = NULL;		/* buffer for raw read (vmin) */
	return(1);
}

ldtermclose(q)
register queue_t *q;
{
	register struct ldterm *tp = (struct ldterm *)q->q_ptr;
	register mblk_t *mp;
	struct stroptions *sop;

	DEBUG4 (("ldtermclose\n"));
	if (mp = allocb(sizeof(struct stroptions), BPRI_MED)) {
		mp->b_datap->db_type = M_SETOPTS;
		mp->b_wptr += sizeof (struct stroptions);
		sop = (struct stroptions *)mp->b_rptr;
		sop->so_flags = SO_READOPT|SO_NDELOFF;
		sop->so_readopt = RNORM;
		putnext (q, mp);
	}

	tp->ld_state = 0;
	/* Dump line buffer contents, then unlink the structure. */
	tp->op = tp->canonb;
	q->q_ptr = NULL;

	ldflush_buf (&tp->echobuf);
	ldflush_buf (&tp->raw_message);
	return;
}


/*
 *  put procedure for input from driver end of stream (read queue)
 */
ldtermin(q, mp)
register queue_t *q;
register mblk_t *mp;
{

	register c;
	register struct ldterm *tp;
	queue_t * wrq = WR(q);
	mblk_t * bp, *nbp, *tmp, *mop;
	mblk_t * eb = NULL;
	mblk_t * swb = NULL;
	struct termio *cb;
	struct sgttyb *gb;
	struct iocblk *iocp;
	register flag;
	int	size;
	register bytes = 0;
	char	*readp;

	DEBUG4 (("ldtermin:\n"));

	tp = (struct ldterm *)q->q_ptr;

	if (mp->b_datap->db_type != M_DATA) {
		switch (mp->b_datap->db_type) {

		default:
			DEBUG1(("ldtermin: default\n"));
			putnext(q, mp);
			return;

		case M_BREAK:
			DEBUG1(("ldtermin: M_BREAK\n"));
			if (!(tp->ld_iflags & IGNBRK) && (tp->ld_iflags & BRKINT))
				ldtermsig(q, SIGINT);
			freemsg(mp);
			return;

		case M_START:
		case M_STOP:
			DEBUG1(("ldtermin: M_START or STOP\n"));
			freemsg(mp);
			return;

		case M_IOCACK:	/* snag return TCGETA ioctl msg here */
			DEBUG1(("ldtermin: M_IOCACK\n"));
			iocp = (struct iocblk *)mp->b_rptr;
			switch ( iocp->ioc_cmd) {
			case TCGETA:
				/* since it is the driver who allocates the
				   message block, we must wait now to 
				   return the line discipline parameters.
				   allocate a block in case driver missed! */

				if (!mp->b_cont) {
					if ((tmp = allocb(sizeof(struct termio),
					    BPRI_HI)) == NULL)  {
						mp->b_datap->db_type = M_IOCNAK;
						iocp->ioc_error = EAGAIN;
						putnext(q, mp);
						return;
					}
					mp->b_cont = tmp;
				}

				cb = (struct termio *)mp->b_cont->b_rptr;

				cb->c_oflag = tp->ld_oflags;	/* all except the */
				cb->c_iflag = tp->ld_iflags;	/* cb->c_cflag  */
				cb->c_lflag = tp->ld_lflags;
				cb->c_line = 0;

				cb->c_cc[0] = tp->ld_intrc;
				cb->c_cc[1] = tp->ld_quitc;
				cb->c_cc[2] = tp->ld_erase;
				cb->c_cc[3] = tp->ld_kill;
				cb->c_cc[4] = tp->ld_eofc;
				cb->c_cc[5] = tp->ld_eol;
				cb->c_cc[6] = tp->ld_eol2;
				cb->c_cc[7] = tp->ld_swtchc;

				/* acknowldge the ioctl */
				iocp->ioc_count = sizeof(struct termio );
				iocp->ioc_error = 0;
				iocp->ioc_rval = 0;
				mp->b_datap->db_type = M_IOCACK;
				putnext(q, mp);
				return;

			case TCSETAW:
			case TCSETA:
			case TCSETAF:
				/* if ICANON flag is not set, set options in 
				 * the stream head to receive M_READ messages
				 * Send M_SETOPTS only if anything changed 
				 * check if VMIN is zero before turning on
				 */
				 DEBUG4 (("CHG_TO_RAW = %x, CHG_TO_CANON = %x, VMIN = %x\n", CHG_TO_RAW, CHG_TO_CANON, VMIN));

				if ((CHG_TO_RAW /*&& ((char) V_MIN == 0) */) || CHG_TO_CANON) {
					if ((mop = allocb(sizeof(struct stroptions ), 
					    BPRI_MED)) == NULL) {
						DEBUG1 (("ldtermin: outof blocks\n"));
						mp->b_datap->db_type = M_IOCNAK;
						iocp->ioc_error = EAGAIN;
						putnext(q, mp);
						return;
					} else {
						register struct stroptions *sop;

						mop->b_datap->db_type = M_SETOPTS;
						mop->b_wptr += sizeof(struct stroptions );
						sop = (struct stroptions *)mop->b_rptr;
						if (CANON_MODE) {
							
							sop->so_flags = SO_READOPT|SO_MREADOFF;
							sop->so_readopt = RMSGN;
						} else {
							sop->so_flags = SO_READOPT;
							sop->so_readopt = RNORM;
							if ((char) V_MIN == 0)

								sop->so_flags|= SO_MREADON;
						}
						DEBUG1 (("ldtermin: M_SETOPTS sent\n"));
						    putnext(q, mop); 
					}
				}
				putnext(q, mp); 
				    /*
				 * if M_IOCACK is TCSETxx and the caller 
				 * required RAW mode and the canonical buffer
				 * has some data, then send the data upstream
				 * otherwise, data will hang around the 
				 * canonical buffer until returning to 
				 * cannonical mode;
				 */
				if (CHG_TO_RAW && (tp->op > tp->canonb)) {
					/* Get a block and put it in */
					if ((nbp = allocb((tp->op - tp->canonb),
					    BPRI_HI)) == NULL) {
						DEBUG1(("ldtermin out of blocks\n")); 
					} else {
						bcopy(tp->canonb, nbp->b_wptr,
						    (tp->op - tp->canonb)); 
					    	nbp->b_wptr += (tp->op - tp->canonb); 
					    	putnext(q, nbp); 
					    	tp->op = tp->canonb; 
					    	tp->lmode_flg = 0; 
					    	nbp = NULL; 
					}
				}
				return; 

			    case TIOCGETP:
				gb = (struct sgttyb *)mp->b_cont->b_rptr; 

				gb->sg_erase = tp->ld_erase; 
				gb->sg_kill = tp->ld_kill; 
				flag = 0; 
				if (RAW_MODE)
				    flag |= O_RAW; 
				if (tp->ld_lflags & XCASE)
				    flag |= O_LCASE; 
				if (ECHO_ON)
				    flag |= O_ECHO; 
				if (!(tp->ld_lflags & ECHOK))
				    flag |= O_NOAL; 
				if (!(gb->sg_flags & O_ODDP))
				    if (tp->ld_iflags & INPCK)
					flag |= O_EVENP; 
				    else
					flag |= O_ODDP | O_EVENP; 

				if (tp->ld_oflags & ONLCR) {
					flag |= O_CRMOD; 
					if (tp->ld_oflags & CR1)
						flag |= O_CR1; 
					if (tp->ld_oflags & CR2)
					    	flag |= O_CR2; 
				} else {
					if (tp->ld_oflags & CR1)
					    	flag |= O_NL1; 
				    	if (tp->ld_oflags & CR2)
					   	 flag |= O_NL2; 
				}

				if ((tp->ld_oflags & TABDLY) == TAB3)
				    	flag |= O_XTABS; 
			    	else if (tp->ld_oflags & TAB1)
				    flag |= O_TBDELAY; 
			    	if (tp->ld_oflags & FFDLY)
				    flag |= O_VTDELAY; 
			    	if (tp->ld_oflags & BSDLY)
				    flag |= O_BSDELAY; 

			    	gb->sg_flags |= flag; 
			    	iocp->ioc_error = 0; 
			    	iocp->ioc_rval = 0; 
			    	putnext(q, mp); 
			    	return; 

			    default:
				putnext(q, mp); 
			    	return; 
			}

		case M_FLUSH:
			DEBUG1(("ldtermin: M_FLUSH %d\n", *mp->b_rptr)); 
		    	if (*mp->b_rptr & FLUSHR)
		    		flushq(q, 0); 
		    	if (*mp->b_rptr & FLUSHW) {
				flushq(wrq, 0); 
			    	ldflush_buf (&tp->echobuf);
			}
			/* note that FLUSHRW will be satisfied by both the
			 * "if" statements
			 */
			putnext(q, mp); 
		    	return; 
		}
		/* everything but M_DATA should be gone by now */
	}

	/* flow control: send start message if blocked and q->q_next low. */
	if (tp->ld_iflags & IXOFF && tp->ld_state & TTBLOCK && canput(q)) {
		tp->ld_state &= ~TTBLOCK; 
		    putctl(wrq->q_next, M_START); 
	}
	/* look at all input for special characters */
	bp = mp; 
    	readp = (char *)bp->b_rptr; 
	size = 0; 

more:	
    	while (readp < (char *)bp->b_wptr) {
		c = *readp++; 
		if ( tp->ld_iflags & ISTRIP ) {
			c &= 0177; 
			    *(readp - 1) = c; 
		}

		/* If stopped, start if you can; if running, 
		 * stop if you must 
		 */

		if (tp->ld_iflags & IXON) {
			if (tp->ld_state & TTSTOP) {
				if (tp->ld_iflags & IXANY ) {
					tp->ld_state &= ~TTSTOP; 
					qenable(wrq); 
				}
				if (c == tp->ld_startc) {
					tp->ld_state &= ~TTSTOP; 
					qenable(wrq); 
				}
			} else {
				if ( c == tp->ld_stopc )
					tp->ld_state |= TTSTOP; 
			}
		}
		if (c == tp->ld_stopc || c == tp->ld_startc || c == tp->ld_eofc)
			continue; 

		    /* process special characters */

		if ( tp->ld_lflags & ISIG ) {
			if (c == tp->ld_intrc) {
				ldtermsig(q, SIGINT); 
				continue; 
			}
			if (c == tp->ld_quitc) {
				ldtermsig(q, SIGQUIT); 
				continue; 
			}
			if (c == tp->ld_swtchc) {
				/* Shouldn't ever see this; ignore it. */
				swb = allocb(4, BPRI_HI); 
			    	swb->b_datap->db_type = M_CTL; 
				*swb->b_wptr++ = 'Z'; 
				putnext(q, swb); 
				continue; 

			}
		}

		if (c == '\\') {
			if (tp->ld_state & TTESC1)
			    	tp->ld_state &= ~TTESC1; 
			else
				tp->ld_state |= TTESC1; 
		    	if (ECHO_OFF)
			    	continue; 
		}

		/* Map upper case input to lower case if IUCLC flag set */

		if ( tp->ld_iflags & IUCLC && c >= 'A' && c <= 'Z' ) {
			c += 'a' - 'A'; 
		    	*(readp - 1) = c; 
		}


		if ( c == '\r' ) {
			if ( tp->ld_iflags & IGNCR )
			    	continue; 

		    	if ( tp->ld_iflags & ICRNL ) {
				c = '\n'; 
			    	*(readp - 1) = c; 
			}
		} else if ( c == '\n' && tp->ld_iflags & INLCR ) {
			c = '\r'; 
		    	*(readp - 1) = c; 
		}

		/* Echo turned off, but ECHOE and ECHONL still applies */

		if (ECHO_OFF) {
			if (RAW_MODE)
			    	continue; 

			/* Echo NL when ECHO turned off, and ECHONL flag  set */
			if ( c == '\n' && tp->ld_lflags & ECHONL ) {
				if ( ( eb = allocb(4, BPRI_MED) ) == NULL ) {
					DEBUG1(("ldtermin: no blocks for NL echo\n"));
					continue;
				}

				*eb->b_wptr++ = '\n';
				(*wrq->q_qinfo->qi_putp)(wrq, eb);
				eb = NULL;
				continue;
			}


			/* Echo SP-BS on ERASE char if ECHO is off, ECHOE is 
			 * set, and the ERASE char isn't escaped
			 */

			if ( c == tp->ld_erase && tp->ld_lflags & ECHOE ) {
				if (tp->ld_state & TTESC1) {
					tp->ld_state &= ~TTESC1;
					continue;
				}

				if ( ( eb = allocb(4, BPRI_MED) ) == NULL ) {
					DEBUG1(("!ldtermin: no blocks for ECHOE\n"));
					continue;
				}

				*eb->b_wptr++ = '\040';  /* space */
				*eb->b_wptr++ = '\010';  /* backspace */
				(*wrq->q_qinfo->qi_putp)(wrq, eb);
				eb = NULL;
			}

			continue;
		}

		/* ECHO ON Processing */

		if ( ECHO_ON ) {
			if (eb == NULL || bytes == 0) {
				if (eb != NULL) {
					if (!(canput(wrq)) || tp->echobuf != NULL) {
						bytes = eb->b_wptr - eb->b_rptr;
						if (tp->echobuf == NULL) {
							if (bytes <= 256) {
								if ((tp->echobuf = allocb(256, BPRI_MED)) == NULL) {
/* code folded from here */
	DEBUG2( ("allocb failed\n"));
	freemsg(eb);
	--readp;
	eb = NULL;
	continue;
/* unfolding */
								}

							} else {
								tp->echobuf= eb;
								goto getblk;
							}
						}


						if (((tp->echobuf->b_datap->db_lim - tp->echobuf->b_wptr) - 1) < bytes ) {
							freemsg(eb);
							goto getblk;
						}

						bcopy(eb->b_rptr, tp->echobuf->b_wptr, bytes);
						tp->echobuf->b_wptr += bytes;
						freemsg(eb);
					} else
						(*wrq->q_qinfo->qi_putp)(wrq, eb);
				}

getblk:
				bytes = (int)(((char *)bp->b_wptr - readp) + 1);

				if ((eb = allocb((bytes + size), BPRI_MED)) == NULL) {
					DEBUG1(("!ldtermin: no blocks for echo\n"));
					break;
				}

				bytes = (int)((eb->b_datap->db_lim - eb->b_rptr) - 1);

				size = 0;
			}

			if (CANON_MODE) {
				/* Echo ERASE as BS-SP-BS here if not escaped */

				if ( c == tp->ld_erase && tp->ld_lflags & ECHOE
				     && !(tp->ld_state & TTESC1) ) {
					if (bytes < 3) {
						readp--;
						size = 3;
						bytes = 0;
					} else {
						/* echo <backspace> <space>
						 * <backspace>
						 */

						*eb->b_wptr++ = '\010';
						*eb->b_wptr++ = '\040';
						*eb->b_wptr++ = '\010';
						bytes -= 3;
					}
					continue;
				}

				/* Echo NL after KILL char if not escaped. */

				if ( c == tp->ld_kill && tp->ld_lflags & ECHOK
				     && !(tp->ld_state & TTESC1) ) {
					if (bytes < 2) {
						readp--;
						size = 2;
						bytes = 0;
					} else {
						*eb->b_wptr++ = c;
						*eb->b_wptr++ = '\n';
						bytes -= 2;
					}
					continue;
				}

				/* This little hack causes backslash to be 
				 * echoed as '\', not '\\', when XCASE is set. 
				 */

				if ( c == '\\' ) {
					if ( tp->ld_lflags & XCASE )
						*eb->b_wptr++ = (c | 0200);
					else
						*eb->b_wptr++ = c;
					bytes--;
				} else {

					if ( tp->ld_state & TTESC1 )
						tp->ld_state &= ~TTESC1;

					*eb->b_wptr++ = c;
					bytes--;
				}
			} else {
				*eb->b_wptr++ = c;
				bytes--;
			}
		}
	}


	/* flow control: send stop message if q->q_next getting full */
	if (tp->ld_iflags & IXOFF && !(tp->ld_state & TTBLOCK)
	     && !(canput(q)) ) {
		tp->ld_state |= TTBLOCK;
		DEBUG3( ("Sending M_STOP to driver\n"));
		putctl(wrq->q_next, M_STOP);
	}

	if (eb != NULL) {
		if (!(canput(wrq)) || tp->echobuf != NULL) {
			bytes = eb->b_wptr - eb->b_rptr;
			if (tp->echobuf == NULL) {
				if (bytes <= 256) {
					if ((tp->echobuf = allocb(256, BPRI_MED)) == NULL) {
						DEBUG2( ("ldtermin: allocb failed\n"));
						freemsg(eb);
						goto putblk;
					}

				} else {
					tp->echobuf = eb;
					goto putblk;
				}
			}

			if (((tp->echobuf->b_datap->db_lim - tp->echobuf->b_wptr) - 1) < bytes ) {
				DEBUG4(("ldtermin: no space in echobuf\n"));
				freemsg(eb);
				goto putblk;
			}

			bcopy(eb->b_rptr, tp->echobuf->b_wptr, bytes);
			tp->echobuf->b_wptr += bytes;
			freemsg(eb);
		} else
			(*wrq->q_qinfo->qi_putp)(wrq, eb);
	}

putblk:
	/* check the other blocks in the message */
	if ( bp->b_cont != NULL ) {
		bp = bp->b_cont;
		readp = (char *)bp->b_rptr;
		goto more;
	}

	/* queue the message for service procedure */

	putq(q, mp);
	return;
}


/*
 * line discipline input server processing.  Erase/kill and escape ('\')
 * processing, gathering into lines,  upper/lower case input mapping.
 */

ldterminsrv(q)
queue_t *q;
{
	struct ldterm *tp;
	mblk_t * mp, *bp, *bpt;
	register unsigned char	c;
	int blksz, s;

	DEBUG4(("ldterminsrv:\n"));

	if (!(canput(q->q_next))) {
		DEBUG2 (("ldterminsrv: canput failed on q = \n", q->q_next));
		return;
	}

	tp = (struct ldterm *)q->q_ptr;
	bpt = NULL;


	while ((mp = getq(q)) != NULL) {
		/* All queued messages should be M_DATA 
		 * pass on everything else
		 */
		if (mp->b_datap->db_type != M_DATA) {
			putnext(q, mp);
			continue;
		}

		bp = mp;

		blksz = (int)(bp->b_wptr - bp->b_rptr);

more:	
		while (bp->b_rptr < bp->b_wptr) {

			c = *bp->b_rptr++;

			/* Throw away CR here if IGNCR flag set */
			if (c == '\r' && tp->ld_iflags & IGNCR) {
				if ( tp->ld_state & TTESC )
					tp->ld_state &= ~TTESC;
				continue;
			}

			/* Throw away INTR, QUIT, and SWTCH chars if 
			 * ISIG flag is set
			 */

			if ( tp->ld_lflags & ISIG ) {
				if ( c == tp->ld_intrc || c == tp->ld_quitc
				     || c == tp->ld_swtchc ) {
					if ( tp->ld_state & TTESC )
						tp->ld_state &= ~TTESC;
					continue;
				}
			}

			/* Throw away START and STOP characters here 
			 * if and only if flow control is enabled
			 */

			if ( c == tp->ld_stopc || c == tp->ld_startc ) {
				if ( tp->ld_state & TTESC )
					tp->ld_state &= ~TTESC;
				if (tp->ld_iflags & IXON)
					continue;
			}


			/* Raw Mode processing; Dictated by VMIN/VTIME */	

			if (RAW_MODE) { 
				DEBUG4(("RAW MODE\n"));
				s=spltty();
				if ((bpt = tp->raw_message) == NULL) {
					if ((bpt = allocb (max(blksz, V_MIN) , BPRI_MED)) == NULL) {
						printf ("ldterminsrv: no more blocks\n");
						splx(s);
						return;
					}
					tp->raw_message = bpt;
				}
				*bpt->b_wptr++ = c;
				tp->rawlength++;

				/* do the processing until message is 
				 * is complete 
				 */
				DEBUG4(("ldterminsrv: vmin = %d, vtime = %d, rawlength = %d\n", V_MIN, V_TIME, tp->rawlength));

				if (bp->b_rptr < bp->b_wptr) 
				{
					DEBUG4(("ldterminsrv: more data in current block\n"));
					splx(s);
					continue;	      
				} else {
				DEBUG4 (("ldterminsrv: bp = %x, bp->b_cont = %x, mp = %x\n", bp, bp->b_cont, mp));
					if (bp->b_cont) {
						bp = bp->b_cont;
						mp->b_cont = NULL;
						if (mp) freeb(mp);
						mp = bp;
						splx(s);
						DEBUG4 (("ldterminsrv: reading from cont block\n"));
						continue;
					}
				}

				/* At this point, all the data has been
				 * copied.
				 */

				/* if VMIN chars have been collected, send up
				 * and cancel any timeouts in progress. VTIME
				 * is handled in an intelligent fashion to 
				 * eliminate unsetting callout table entries
				 * and minimize PIR_9  interrupts. When the 
				 * first char arrives, if VTIME is set, timer
				 * is started. Subsequent characters only 
				 * clears RTO flag (does not untimeout). If
				 * N characters arrived before the first timer
				 * fired, we only issue PIR_9 once. If an 
				 * expected char did not arrive after a timeout
				 * call was made RTO stays set and we ship
				 * everything we have upto that point, to the
				 * stream head.
				 */

				tp->ld_state &= ~RTO;


				if ( tp->rawlength >= V_MIN ) {
					ldvmin_ready (q, tp);
					bpt = NULL;
				} else if ( V_TIME ) {
					if (!(tp->ld_state & TACT ))
						ldraw_timeout(q);
				}
				splx(s);
			} else if (CANON_MODE) {
				DEBUG4(("CANON_MODE\n"));	

				/* If the ICANON flag is set, collect a line and
				 * send up 
				 */

				if ( c == '\\' ) {

					/* Escape processing here. */
					if ( tp->ld_state & TTESC ) {

						/* XCASE processing here. */
						if ( tp->ld_lflags & XCASE && maptab[c] )
							c = maptab[c];
						tp->ld_state &= ~TTESC;
					} else
						tp->ld_state |= TTESC;
				}

				/* ERASE processing here */
				if ( c == tp->ld_erase ) {
					if ( tp->op > tp->canonb )
						tp->op--;

					if ( !(tp->ld_state & TTESC) )
						continue;
				}

				/* KILL processing here */
				if ( c == tp->ld_kill) {
					if (tp->ld_state & TTESC) {
						if ( tp->op > tp->canonb )
							tp->op--;
					} else {
						tp->op = tp->canonb;
						continue;
					}
				}

				if (c != '\\' && tp->ld_state & TTESC )
					tp->ld_state &= ~TTESC;

				/* If the line's too long, silently dump it.*/
				if ( tp->op >= &tp->canonb[CANBSIZ-1] ) {
					DEBUG2( ("dumped line\n"));
					tp->op = tp->canonb;
				}

				/* Push the character into the line. */
				*tp->op++ = c;

				/* If you're at the end of the line, send it. */
				if (c == '\n' || c == tp->ld_eofc
				     || c == tp->ld_eol || c == tp->ld_eol2 ) {
					/* ^D ends the line, but don't send ^D */
					if ( c == tp->ld_eofc )
						tp->op--;

					/* Get a block and put it in */
					if ((bpt = allocb((tp->op) - (tp->canonb), BPRI_MED))
					     == NULL ) {
						DEBUG2(("ldterminsrv: out of blocks\n"));
						/* recovery:
						 * back up connacnical buffer 
						 * and input buffer to line
						 * delimeter, put buffer back
						 * on queue, and try again later
						 */
						putbq(q, mp);
						bufcall((tp->op) - (tp->canonb), BPRI_MED, qenable, q);
						if ( c != tp->ld_eofc )
							tp->op--;
						bp->b_rptr--;
						return;
					}
					bcopy( tp->canonb, bpt->b_wptr, (tp->op - tp->canonb));
					bpt->b_wptr += (tp->op - tp->canonb);

					/* Send the block up the stream */
					putnext(q, bpt);
					tp->op = tp->canonb;
					bpt = NULL;
				}
			}
		}

		s=spltty();
		if (mp) {
			mp->b_cont = NULL;
			freeb(mp);
		}

		if (bp && bp->b_cont) 
		{
			bp = bp->b_cont;
			mp = bp;
			splx(s);
			goto more;
		}
		splx(s);

		/* flow control: send start message if blocked and q->q_next low */
		if (tp->ld_iflags & IXOFF && tp->ld_state & TTBLOCK
		     && q->q_next->q_count <= q->q_next->q_qinfo->qi_minfo->mi_lowat) {
			tp->ld_state &= ~TTBLOCK;
			putctl(WR(q), M_START);
		}
	}
	return;
}


/*
 * Line discipline output queue put procedure: speeds M_IOCTL
 * messages.
 */
ldtermout(q, mp)
register queue_t *q;
register mblk_t *mp;
{
	register struct ldterm *tp;
	struct iocblk *iocp;
	int	n;

	DEBUG4(("ldtermout:\n"));

	tp = (struct ldterm *)q->q_ptr;
	iocp = (struct iocblk *)mp->b_rptr;

	switch (mp->b_datap->db_type) {

	case M_IOCTL:
		DEBUG1(("ldtermout: M_IOCTL\n"));
		switch (iocp->ioc_cmd) {

		case TCSETA: /* immediate; put at head of queue */
			putbq(q, mp);
			qenable(q);
			return;

		case TCFLSH:
			if ( *(int *)mp->b_cont->b_rptr == 0 ) {
				putctl1(q->q_next, M_FLUSH, FLUSHR);
				mp->b_datap->db_type = M_IOCACK;
			} else if ( *(int *)mp->b_cont->b_rptr == 1 ) {
				putctl1(RD(q)->q_next, M_FLUSH, FLUSHW);
				mp->b_datap->db_type = M_IOCACK;
			} else if ( *(int *)mp->b_cont->b_rptr == 2 ) {
				putctl1(q->q_next, M_FLUSH, FLUSHR);
				putctl1(RD(q)->q_next, M_FLUSH, FLUSHW);
				mp->b_datap->db_type = M_IOCACK;
			} else {
				mp->b_datap->db_type = M_IOCNAK;
				iocp->ioc_error = EINVAL;
			}
			iocp->ioc_count = 0;
			qreply(q, mp);
			return;

		case TCXONC:
			n = *(int *)mp->b_cont->b_rptr;
			/* bug fix - successive TCXONC failures */
			/* if( n==0 && !(tp->ld_state&TTSTOP)) */
			if (n == 0) {
				tp->ld_state |= TTSTOP;
				mp->b_datap->db_type = M_IOCACK;
			} /* else if( n==1 && tp->ld_state&TTSTOP) */
			else if (n == 1) {
				qenable(q);
				tp->ld_state &= ~TTSTOP;
				mp->b_datap->db_type = M_IOCACK;
			} else {
				iocp->ioc_error = EINVAL;
				mp->b_datap->db_type = M_IOCNAK;
			}

			iocp->ioc_count = 0;
			qreply(q, mp);
			return;

		default:
			putq(q, mp);
			qenable (q);
			return;
		}
	case M_FLUSH:

		DEBUG1(("ldtermout: M_FLUSH %d\n", *(mp->b_rptr)));
		if (*mp->b_rptr == FLUSHW || *mp->b_rptr == FLUSHRW) {
			flushq(q, 0);
			ldflush_buf (&tp->echobuf); 
		}
		if (*mp->b_rptr == FLUSHR || *mp->b_rptr == FLUSHRW)
			flushq(RD(q), 0);

		putnext(q, mp);
		break;

/**	case M_DATA:  if OPOST flag is not set, characters are transmitted
		      * without change. This could improve performance
		if (canput (q->q_next))
			if (!(tp->ld_oflags & OPOST)) {
				putnext (q, mp);
				break;
			}
		 else fall through 	*/
	default:
		/* Queue everything else for the service procedure.	*/
		putq(q, mp);
		qenable (q);
		break;
	}
	return;
}


/*
 * line discipline output service procedure: delays, tab and CR/NL
 * mapping
 */
ldtermosrv(q)
queue_t *q;
{
	struct ldterm *tp;
	mblk_t * mp, *bpt;
	struct iocblk *iocp;
	int	x;
	int runout_flag = 0;

	DEBUG4(("ldterminsrv:\n"));

	tp = (struct ldterm *)q->q_ptr;

more:
	while (!(tp->ld_state & TTSTOP) && (mp = getq(q)) != NULL) {
		switch (mp->b_datap->db_type) {

		case M_IOCTL:
			DEBUG1(("ldtermosrv: M_IOCTL\n"));
			iocp = (struct iocblk *)mp->b_rptr;
			switch(iocp->ioc_cmd)
			{
				case TCGETA:
				case TCSBRK:
				case TIOCGETP:
				case TIOCGWINSZ:
				case TIOCSWINSZ:
				case JWINSIZE:
					putnext(q, mp);
					break;
				default:
					ldtermioc(q, mp);
			}
			continue;

		case M_DATA:
			if (!(canput(q->q_next))) {
				putbq(q, mp);
				return;
			}
			/** if (!(tp->ld_lflags & OPOST)) {
				putnext (q, mp);
				continue;
			}*/
			if (!(ldoutconv(q, mp)))
				return;
			continue;

		case M_READ:
			/* Stream head needs data to satisfy timed read.
			 * Has meaning only if ICANON flag is off indicating
			 * raw mode 
			 */
			DEBUG1(("ldtermosrv: M_READ \n"));
			/* only cases 3 and 4 (see SVID) need be handled here.
			 * we will cheat ldraw_timeout() to do it for us
			 */

			DEBUG4 (("Case M_READ: RAW_MODE = %d, V_MIN = %d\n",
				RAW_MODE, V_MIN));
			if ( RAW_MODE && (char) V_MIN == 0 ) { /* handle only vmin = 0 case */
				x=spltty();
				if ((char) V_TIME == 0) 
					tp->ld_state |= RTO;
				else 	
					tp->ld_state &= ~RTO;
				DEBUG4 (("ldtermout: calling ldraw_timeout\n"));
				if ((bpt = tp->raw_message) == NULL) {
					if((bpt = allocb(4, BPRI_MED))== NULL) {
					/* at this point, there is no point in
					 * forwarding the M_READ because
					 * subsequent allocb's will fail.
					 * we will use the M_READ's message
					 * block for read allocation,
					 * so that the reader will not block 
					 */
						DEBUG1(("ldterminsrv: no blocks\n"));
						mp->b_datap->db_type = M_DATA;
						mp->b_rptr = mp->b_wptr = mp->b_datap->db_base;
						bpt = mp;/* isn't this neat?? */
						runout_flag = 1;
					}
					tp->raw_message = bpt;
				}
				ldraw_timeout (RD (q));
			}
			splx(x);
			if (!runout_flag)
				putnext (q, mp);
			continue;

		default:
			DEBUG1(("ldtermosrv: default \n"));
			putnext(q, mp);
			continue;
		}
	}

	if (tp->echobuf != NULL) {
		if (canput(q)) {
			x = splhi();
			putq(q, tp->echobuf);
			tp->echobuf = NULL;
			splx(x);
			goto more;
		}
	}
}


/*
 * Output processing for contents of M_DATA messages.
 * Called by output service routine.
 * 
 * returns 0 if message (imp) was not processed and returned to output queue (q) * returns 1 is message (imp) was correctly processed.
 */
ldoutconv(q, imp)
queue_t *q;
mblk_t *imp;
{

	register struct ldterm *tp;
	register bytes;
	register c;
	register mblk_t *omp = NULL;
	register mblk_t *bp;
	int	size, ctype; 
	int	count = 0;

	DEBUG4(("ldoutconv:\n")); 
	tp = (struct ldterm *)q->q_ptr;

	if ( imp == NULL ) {
		DEBUG1( ("ldoutconv: null message pointer\t"));
		return(0);
	}

	bp = imp;

	if ((tp->ld_oflags & (CRDLY | TABDLY | VTDLY | NLDLY | BSDLY | FFDLY))
	     && !(tp->ld_oflags & OFILL)) {
		if (tp->ld_oflags & OPOST && ((tp->ld_oflags & TABDLY) != TAB3))
			bytes = 64;
		else
			bytes = (int)(bp->b_wptr - bp->b_rptr);
	} else
		bytes = (int)(bp->b_wptr - bp->b_rptr);

	if (bytes > 2048)
		bytes = 2048;

	if ((omp = allocb(bytes, BPRI_MED)) == NULL) {
		bufcall(bytes, BPRI_MED, qenable, q);
		putbq(q, imp);
		return(0);
	}

	bytes = (int)(omp->b_datap->db_lim - omp->b_rptr) - 1;

	size = 0;

more:
	while (bp->b_rptr < bp->b_wptr && !(tp->ld_state & TTSTOP) ) {
		/* Make sure there's room for TWO more chars here; you */
		/* might need to push in a '\' before the current char.*/
		if (bytes <= 0) {
			if ((tp->ld_oflags & (CRDLY | TABDLY | VTDLY | NLDLY | BSDLY | FFDLY))
			     && !(tp->ld_oflags & OFILL)) {
				if (tp->ld_oflags & OPOST && ((tp->ld_oflags & TABDLY) != TAB3))
					bytes = 64;
				else
					bytes = (int)(bp->b_wptr - bp->b_rptr);
			} else
				bytes = (int)(bp->b_wptr - bp->b_rptr);

			if (omp != NULL)
				putnext(q, omp);

			if ((omp = allocb((bytes + size), BPRI_MED)) == NULL) {
				bufcall((bytes + size), BPRI_MED, qenable, q);

				putbq(q, imp);
				return(0);
			}

			bytes = (int)(omp->b_datap->db_lim - omp->b_rptr) - 1;

			size = 0;

			if (count && tp->ld_oflags & OFILL)
				goto fill;
		}

		/* Get a character from the input msg block */

		c = *bp->b_rptr++;

		/* Take care of XCASE processing before other output stuff. */
		if ((tp->ld_lflags & XCASE) && CANON_MODE ) {
			if (bytes < 2) {
				bytes = 0;
				bp->b_rptr--;
				continue;
			}

			if ( c >= 'A' && c <= 'Z' ) {
				/* precede with '\' */
				*omp->b_wptr++ = '\\';
				bytes--;
			} else if ( c == '`' ) {
				/* map '`' to "\'" */
				*omp->b_wptr++ = '\\';
				bytes--;
				c = '\047';  /* ''' */
			} else if ( c == '|' ) {
				/* map '|' to "\!" */
				*omp->b_wptr++ = '\\';
				bytes--;
				c = '!';
			} else if ( c == '~' ) {
				/* map '~' to "\^" */
				*omp->b_wptr++ = '\\';
				bytes--;
				c = '^';
			} else if ( c == '\173' )  /* left curly brace */ {
				/* map '{' to "\(" */
				*omp->b_wptr++ = '\\';
				bytes--;
				c = '(';
			} else if ( c == '\175' )  /* right curly brace */ {
				/* map '}' to "\)" */
				*omp->b_wptr++ = '\\';
				bytes--;
				c =  ')';
			} else if ( (c & 0177) == '\\' ) {
				if ( c == '\\' ) {
					/* map '\' to "\\" */
					*omp->b_wptr++ = '\\';
					bytes--;
				} else {
					/* '\' being echoed; do nothing */
					c &= 0177;
				}
			}

		}
		/* If no other output processing is required, push the */
		/* character into the block and get another.           */
		if ( !(tp->ld_oflags & OPOST) ) {
			*omp->b_wptr++ = c;
			bytes--;
			continue;
		} else { 		/* OPOST output flag is set */

			/* Map lower case to upper case if OLCUC is set.   */
			if ( tp->ld_oflags & OLCUC && c >= 'a' && c <= 'z' )
				c -= 'a' - 'A';

			/* Push characters into output buffer until character is found */
			/* that needs further output processing.  */
			if ((ctype = partab[c = c&0177] & 077) == 0) {
				tp->ld_col++;
				*omp->b_wptr++ = c;
				bytes--;
				continue;
			}

			/* Map '\t' to spaces if TAB3 flag is set.	*/
			if (c == '\t' && (tp->ld_oflags & TABDLY) == TAB3) {
				if ((8 - (tp->ld_col & 07)) > bytes) {
					bytes = 0;
					bp->b_rptr--;
					size = 8;
					continue;
				}

				for (; ; ) {
					*omp->b_wptr++ = ' ';
					bytes--;
					tp->ld_col++;
					if ((tp->ld_col & 07) == 0)/* every 8th */
						break;
				}
				continue;
			}


			/* Map <CR> to <NL> on output if OCRNL flag set.*/
			if (c == '\r' && tp->ld_oflags & OCRNL)
				c = '\n';

			/* Ignore <CR> in column 0 if ONOCR flag set.	*/
			if (c == '\r' && tp->ld_col == 0 && tp->ld_oflags & ONOCR)
				continue;

			/* Map <NL> to <CR><NL> on output if ONLCR flag is set.	*/
asm(".globl onlcr");
asm("onlcr:");
			if (c == '\n' && tp->ld_oflags & ONLCR) {
				if ((tp->ld_state & TTCR) == 0) {
					tp->ld_state |= TTCR;
					c = '\r';
					ctype = partab['\r'] & 077;
					--bp->b_rptr;
				} else
					tp->ld_state &= ~TTCR;
			}

			/* Push the character into the output message.	*/
			*omp->b_wptr++ = c;
			bytes--;


			/* Delay values and column position calculated here.	*/
			count = 0;
			switch (ctype) {

				/* Ordinary characters; these should not get this far. */
			case 0:
				tp->ld_col++;
				break;
				/* Non-printing characters; nothing happens.	*/
			case 1:
				break;

				/* Backspace */
			case 2:
				if (tp->ld_col)
					tp->ld_col--;
				if ( tp->ld_oflags & BSDLY ) {
					if ( tp->ld_oflags & OFILL )
						count = TICK1;
					else
						count = TICK3;
				}
				break;

				/* Newline: Delay and column depend on tty type and flags. */
			case 3:
				if ( tp->ld_oflags & ONLRET )
					goto cr;

				if ( (tp->ld_oflags & NLDLY) == NL1 ) {
					count = TICK2;
				}

				break;


				/* tab */
			case 4:
				tp->ld_col |= 07;
				tp->ld_col++;

				if (tp->ld_oflags & OFILL) {
					if (tp->ld_oflags & TABDLY)
						count = TICK2;
					break;
				}

				if ((tp->ld_oflags & TABDLY) == TAB2 && !(tp->ld_oflags & OFILL))
					count = TICK6;
				if ((tp->ld_oflags & TABDLY) == TAB1 && !(tp->ld_oflags & OFILL)) {
					count = 1 + (tp->ld_col | ~07);
					if (count < 5)
						count = 0;
				}
				break;

				/* vertical motion */
			case 5:
				if (tp->ld_oflags & VTDLY && !(tp->ld_oflags & OFILL))
					count = MANYTICK;
				break;

				/* carriage return */
			case 6:
cr:			
				if ((tp->ld_oflags & CRDLY) == CR1 )
					if ( tp->ld_oflags & OFILL )
						count = TICK2;
					else
						count = tp->ld_col % 2;
				else if ((tp->ld_oflags & CRDLY) == CR2)
					if ( tp->ld_oflags & OFILL )
						count = TICK4;
					else
						count = TICK6 + 4;
				else if ((tp->ld_oflags & CRDLY) == CR3 )
					if ( tp->ld_oflags & OFILL )
						count = 0;
					else
						count = TICK9 + 6;
				tp->ld_col = 0;
				break;

			}
fill:
			if ( count && tp->ld_oflags & OFILL ) {
				if (count > bytes) {
					bytes = 0;
					size = count;
					continue;
				}

				for ( ; count; count--) {
					bytes--;
					if ( tp->ld_oflags & OFDEL )
						*omp->b_wptr++ = 0177;
					else
						*omp->b_wptr++ = CNUL;
				}
			} else if (count) {
				putnext(q, omp);
				putctl1(q->q_next, M_DELAY, count);
				omp = NULL;
				bytes = 0;
			}
		}
	}
	/* If output is blocked, put remaining data back on queue and return. */
	/* Be sure to enable (q) when the output is unblocked!		      */
	if ( tp->ld_state & TTSTOP && bp->b_rptr < bp->b_wptr ) {
		if (omp != NULL)
			putnext(q, omp);

		putbq(q, bp);
		return(0);
	}

	/* free up message block as soon as it is processed */
	bp = bp->b_cont;
	imp->b_cont = NULL;
	if (imp)
		freeb(imp);
	imp = bp;

	if (bp)		/* are we done yet? */ {
		bytes = 0;
		size = 0;
		goto more;
	}

	if (omp != NULL)
		putnext(q, omp);

	return(1);
}


/*
 * Signal generated by the reader: M_SIGNAL and M_FLUSH messages sent.
 */

ldtermsig(q, sig)
queue_t *q;
int	sig;
{
	struct ldterm *tp = (struct ldterm *)q->q_ptr;

	DEBUG4(("ldtermsig: q = %x, sig = %d\n", q, sig));

	putctl1(q->q_next, M_PCSIG, sig);

	if ( !(tp->ld_lflags & NOFLSH) ) {
		tp->op = tp->canonb; 	/* drop cannonical buffer */
		ldflush_buf (&tp->raw_message);
		tp->rawlength = 0;
		putctl1(q->q_next, M_FLUSH, FLUSHW);
		putctl1(WR(q)->q_next, M_FLUSH, FLUSHR);
		tp->ld_state &= ~TTSTOP; /* clear all hazards */
	}

	qenable (q);	 	/* may be an overkill, but sure is a
	qenable (WR(q));	 * bug fix for break hanging problem.
					 */


}


ldtermioc(q, mp)
mblk_t *mp;
queue_t *q;
{
	struct ldterm *tp;
	struct termio *cb;
	struct sgttyb *gb;
	struct iocblk *iocp;
	register flag;

	DEBUG4(("ldtermioc:\n"));

	iocp = (struct iocblk *)mp->b_rptr;
	cb = (struct termio *)mp->b_cont->b_rptr;
	tp = (struct ldterm *)q->q_ptr;

	/* Message must be of type M_IOCTL for this routine to be called.  */
	DEBUG4(("ldtermioc:iocp->ioc_cmd = %x\n", iocp->ioc_cmd));

	switch (iocp->ioc_cmd) {
			
	case TCSETAW:	/* Set current parameters and special characters. */
	case TCSETA:	/* do this BEFORE passing ioctl to driver below */
	case TCSETAF:	/* let the driver send the ACK */
			/* Also, check ICANON mode in tp->ld_lflags
		 	 * and cb->c_flag. If it's different from
			 * each other, send the information downstream.
			*/
		tp->lmode_flg = 0;	/* initial set */
		if (CANON_MODE && !( cb->c_lflag & ICANON))
			tp->lmode_flg = B_RAW;
		else if (RAW_MODE && ( cb->c_lflag & ICANON ))
			tp->lmode_flg = B_CANON;
		else 
			tp->lmode_flg = 0;	/* no change. */

		tp->ld_oflags = cb->c_oflag;
		tp->ld_iflags = cb->c_iflag;
		tp->ld_lflags = cb->c_lflag;

		tp->ld_intrc = cb->c_cc[0];
		tp->ld_quitc = cb->c_cc[1];
		tp->ld_erase = cb->c_cc[2];
		tp->ld_kill = cb->c_cc[3];
		tp->ld_eofc = cb->c_cc[4];
		tp->ld_eol = cb->c_cc[5];
		tp->ld_eol2 = cb->c_cc[6];
		tp->ld_swtchc = cb->c_cc[7];

		/*  Keep these zeroed in the driver, */
		/*  except for the iflags not handled here. */
		cb->c_iflag &= IGNBRK|BRKINT|IGNPAR|PARMRK|INPCK|IXON|IXOFF|IXANY;
		cb->c_lflag = 0;
		cb->c_oflag = 0;
		cb->c_cc[4] = 1; /* 	 set MIN to get 1-byte messages */

		iocp->ioc_count = sizeof(struct termio );
		iocp->ioc_error = 0;
		iocp->ioc_rval = 0;

		putnext(q, mp);
		return;

	case TIOCSETP:
		gb = (struct sgttyb *)mp->b_cont->b_rptr;
		tp->ld_iflags = 0;
		tp->ld_oflags = 0;
		tp->ld_lflags = 0;
		tp->ld_erase = gb->sg_erase;
		tp->ld_kill = gb->sg_kill;
		flag = gb->sg_flags;

		if (flag & O_XTABS)
			tp->ld_oflags |= TAB3;
		if (flag & O_TBDELAY)
			tp->ld_oflags |= TAB1;
		if (flag & O_LCASE) {
			tp->ld_iflags |= IUCLC;
			tp->ld_oflags |= OLCUC;
			tp->ld_lflags |= XCASE;
		}
		if (flag & O_ECHO)
			tp->ld_lflags |= ECHO;
		if (!(flag & O_NOAL))
			tp->ld_lflags |= ECHOK;
		if (flag & O_CRMOD) {
			tp->ld_iflags |= ICRNL;
			tp->ld_oflags |= ONLCR;
			if (flag & O_CR1)
				tp->ld_oflags |= CR1;
			if (flag & O_CR2)
				tp->ld_oflags |= ONOCR | CR2;
		} else {
			tp->ld_oflags |= ONLRET;
			if (flag & O_NL1)
				tp->ld_oflags |= CR1;
			if (flag & O_NL2)
				tp->ld_oflags |= CR2;
		}
		if (flag & O_RAW) {
			tp->ld_eol = 1;
			tp->ld_eofc = 6;
			tp->ld_iflags &= ~(ICRNL | IUCLC);
		} else {
			tp->ld_eofc = CEOF;
			tp->ld_eol = 0;
			tp->ld_eol2 = 0;
			tp->ld_iflags |= BRKINT | IGNPAR | ISTRIP | IXON | IXANY;
			tp->ld_oflags |= OPOST;
			tp->ld_lflags |= ICANON | ISIG;
		}
		tp->ld_iflags |= INPCK;
		if (flag & O_ODDP)
			if (flag & O_EVENP)
				tp->ld_iflags &= ~INPCK;
		if (flag & O_VTDELAY)
			tp->ld_oflags |= FFDLY;
		if (flag & O_BSDELAY)
			tp->ld_oflags |= BSDLY;

		iocp->ioc_count = sizeof(struct termio );
		iocp->ioc_error = 0;
		iocp->ioc_rval = 0;

		putnext(q, mp);
		return;
	case LDSETT:
	case LDGETT:
		mp->b_datap->db_type = M_IOCACK;
		iocp->ioc_count = 0;
		qreply(q, mp);
		return;

	default:
		/* not a tty ioctl, deliver to module downstream */
		putnext(q, mp);

	}
}


ldraw_timeout(q)
register struct queue *q;

{

	register struct ldterm *tp;
	register int s;

	DEBUG4(("ldraw_timeout:\n"));

	s=spltty();
	tp = (struct ldterm *)q->q_ptr;

	tp->ld_state &= ~TACT;
	if (CANON_MODE) {
		splx(s);
		return;
	}
	/* if VMIN has any value, the timer is to be started after one
	 * char has been received only (if need be). Hence raw_length
	 * should never be equal to zero here if VMIN > 0.
	 */

	if ((tp->rawlength == 0) && V_MIN) {
		splx(s);		     
		return;				
	}
	if (!(tp->ld_state & RTO )) {
		tp->ld_state |= RTO | TACT;
		DEBUG3( ("calling timeout from ldraw_timeout\n"));
		timeout (ldraw_timeout, q, (int)(V_TIME * (HZ / 10)));
		splx(s);
		return;
	}
	/* Send RAW blocks off here */
	tp->ld_state &= ~RTO;
	ldvmin_ready (q, tp);
	splx(s);
}
void
ldvmin_ready (q, tp)
register struct queue *q;
register struct ldterm *tp;

{
	int s;

	DEBUG3 (("VMIN READY\n"));

	s=spltty();
	if (tp->raw_message)			/* the test here is for break */
		putnext(q, tp->raw_message);	/* handling. Break processing */
						/* is done at interrupt level */
	tp->raw_message = NULL;
	tp->rawlength = 0;
	splx(s);
	return;
}
void
ldflush_buf (buf)
mblk_t **buf;
{
	int s;
	s=spltty();
	if (*buf) {
		freemsg (*buf);
		*buf = NULL;
	}
	splx(s);
}

