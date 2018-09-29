/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:io/opts.c	1.1"
/**********************************************************************
 * 	PSEUDO TELETYPE DRIVER
 * 	(Actually two drivers, requiring two entries in 'cdevsw')
 *
 * 	A pseudo-teletype is a special device which is similar to a pipe.
 *	It is used to communicate between two processes.  However, it allows
 *	one to simulate a teletype, including mode setting, interrupt, and
 *	multiple end of files (all not possible on a pipe).
 *
 *	There are really two drivers here.  One is the device which looks like
 *	a terminal and can be thought of as the slave device, and hence
 *	its routines are prefixed with 'pts' (PTY Slave).
 *	The other driver can be thought of as the controlling device,
 *	and its routines are prefixed
 *	by 'ptc' (PTY Controller).  To type on the simulated keyboard of the
 *	PTY, one does a 'write' to the controlling device.  To get the
 *	simulated printout from the PTY, one does a 'read' on the controlling
 *	device.  Normally, the controlling device is called 'ptyx' and the
 *	slave device is called 'ttyx' (where x is a number)
 *	(to make programs like 'who' happy).
 *********************************************************************/

/* TTY HEADERS FOR PTY'S */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/sbd.h"
#include "sys/signal.h"
#include "sys/immu.h"
#include "sys/errno.h"
#include "sys/sysmacros.h"
#include "macros.h"
#include "sys/systm.h"
#include "sys/termio.h"
#include "sys/tty.h"
#include "sys/fs/s5dir.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/conf.h"
#include "sys/buf.h"
#include "sys/file.h"
#include "sys/region.h"
#include "sys/proc.h"

extern int	pts_cnt;
extern struct tty pts_tty[];

int	ptsproc(), nulldev();

/**********************************************************************
 *				SLAVE HALF
 *********************************************************************/


/**********************************************************************
 *				PTSOPEN
 *	Open slave half (waiting for controlling half, just like
 *	real hardware would wait for carrier).
 *********************************************************************/

ptsopen(dev, flag)
dev_t dev;
{
	register struct tty *tp;

	if (minor(dev) >= pts_cnt) {
		u.u_error = ENXIO;
		return;
	}

	tp = &pts_tty[minor(dev)];

	if ((tp->t_state & ISOPEN) == 0) {
		ttinit(tp);		/* Set up default chars */
	}

	if (tp->t_proc == ptsproc)	/* Ctrlr still around. */
		tp->t_state |= CARR_ON;

	while ((tp->t_state & CARR_ON) == 0) {
		tp->t_state |= WOPEN;
		sleep((caddr_t) & tp->t_rawq, TTIPRI);
	}

	(*linesw[tp->t_line].l_open)(tp);
}


/***********************************************************************
 *				PTSCLOSE
 *
 *	Close slave half.
 **********************************************************************/

ptsclose(dev)
dev_t dev;
{
	register struct tty *tp;

	tp = &pts_tty[minor(dev)];
	(*linesw[tp->t_line].l_close)(tp);
	wakeup((caddr_t) & tp->t_outq.c_cf); /* jab reader of controlling half */
}


/*********************************************************************
 *				PTSREAD
 *
 *	Read from PTY, i.e., data from the controlling "hardware"
 ********************************************************************/

ptsread(dev)
dev_t dev;
{
	register struct tty *tp;

	tp = &pts_tty[minor(dev)];
	if (tp->t_proc == ptsproc) {
		(*linesw[tp->t_line].l_read)(tp);
		/* Wakeup other half if sleeping */
		wakeup((caddr_t) & tp->t_rawq.c_cf);
	}
}


/*********************************************************************
 *				PTSWRITE
 *
 *	Write on PTY, i.e., data to the controlling "hardware"
 ********************************************************************/

ptswrite(dev)
dev_t dev;
{
	register struct tty *tp;

	tp = &pts_tty[minor(dev)];

	/* Make sure controlling half is really there */
	if (tp->t_proc == ptsproc)
		(*linesw[tp->t_line].l_write)(tp);
}


/*********************************************************************
 *				PTSPROC
 *
 *	Perform driver "proc" functions (suspend, resume, etc.)
 ********************************************************************/

ptsproc(tp, cmd)
register struct tty *tp;
{
	register unit;
	int	dev;
	extern ttrstrt();

	switch (cmd) {

	case T_TIME:
		tp->t_state &= ~TIMEOUT;
		goto start;

	case T_WFLUSH:
		tp->t_tbuf.c_size -= tp->t_tbuf.c_count;
		tp->t_tbuf.c_count = 0;

	case T_RESUME:
		tp->t_state &= ~TTSTOP;
		goto start;

	case T_OUTPUT:
start:
		 {
			register struct ccblock *tbuf;

			if (tp->t_state & (BUSY | TTSTOP))
				break;
			tbuf = &tp->t_tbuf;
			if (tbuf->c_ptr == NULL || tbuf->c_count == 0) {
				if (tbuf->c_ptr)
					tbuf->c_ptr -= tbuf->c_size - tbuf->c_count;
				if (!(CPRES & (*linesw[tp->t_line].l_output)(tp)))
					break;
			}
			tp->t_state |= BUSY;
			wakeup((caddr_t) & tp->t_outq.c_cf);
			break;
		}

	case T_SUSPEND:
		tp->t_state |= TTSTOP;
		break;

	case T_BLOCK:
		tp->t_state &= ~TTXON;
		tp->t_state |= TTXOFF | TBLOCK;
		break;

	case T_RFLUSH:
		if (!(tp->t_state & TBLOCK))
			break;
	case T_UNBLOCK:
		tp->t_state &= ~(TTXOFF | TBLOCK);
		tp->t_state |= TTXON;
		break;

	case T_BREAK:
		break;
	}
}
