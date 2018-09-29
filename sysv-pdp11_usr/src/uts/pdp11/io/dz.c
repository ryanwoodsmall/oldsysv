/* @(#)dz.c	1.2 */
/*
 *	DZ11 driver
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/tty.h"
#include "sys/termio.h"
#include "sys/conf.h"
#include "sys/sysinfo.h"

#define DZCSCAN	2	/* clock ticks for character scan	*/
#define DZMSCAN	20	/* scan loops for modem scan		*/

#define DZ_CHAR	01	/* character arrived flag (dz_work)	*/
#define DZ_ACT	02	/* character in buffer flag (dz_work)	*/
char dz_work[64];	/* call list for line discipline	*/
			/* needs to be made by config		*/

struct device *dz_addr[];
int	dz_cnt;
struct tty dz_tty[];
short	dz_scan;

char	dz_brk[8];	/* break bits - max 64 lines */
char	dz_speeds[16] = {
	0, 00, 01, 02, 03, 04, 0, 05,
	06, 07, 010, 012, 014, 016, 0, 0
};

#define	BITS6	010
#define	BITS7	020
#define	TWOSB	040
#define	PENABLE	0100
#define	OPAR	0200
#define	RCVENB	010000

#define	IE	040140
struct device {
	short	dzcsr, dzrbuf;
	char	dztcr, dzdtr;
	char	dztbuf, dzbrk;
};
#define	dzlpr	dzrbuf
#define	dzmsr	dzbrk

#define	ON	1
#define	OFF	0

dzopen(dev, flag)
{
	register struct tty *tp;
	extern dzproc(), dzscan();

	if (dev >= dz_cnt) {
		u.u_error = ENXIO;
		return;
	}
	tp = &dz_tty[dev];
	if ((tp->t_state&(ISOPEN|WOPEN)) == 0) {
		ttinit(tp);
		tp->t_proc = dzproc;
		dzparam(dev);
	}
	spl5();
	if (tp->t_cflag&CLOCAL || dzmodem(dev, ON))
		tp->t_state |= CARR_ON;
	else
		tp->t_state &= ~CARR_ON;
	if (!(flag&FNDELAY))
	while ((tp->t_state&CARR_ON)==0) {
		tp->t_state |= WOPEN;
		sleep((caddr_t)&tp->t_canq, TTIPRI);
	}
	(*linesw[tp->t_line].l_open)(tp);
	spl0();
}

dzclose(dev)
{
	register struct tty *tp;

	tp = &dz_tty[dev];
	(*linesw[tp->t_line].l_close)(tp);
	if (tp->t_cflag&HUPCL && !(tp->t_state&ISOPEN))
		dzmodem(dev, OFF);
}

dzread(dev)
{
	register struct tty *tp;

	tp = &dz_tty[dev];
	(*linesw[tp->t_line].l_read)(tp);
}

dzwrite(dev)
{
	register struct tty *tp;

	tp = &dz_tty[dev];
	(*linesw[tp->t_line].l_write)(tp);
}

dzioctl(dev, cmd, arg, mode)
register dev;
{
	switch(cmd) {
	default:
		if (ttiocom(&dz_tty[dev], cmd, arg, mode))
			dzparam(dev);
	}
}

dzparam(dev)
{
	register struct tty *tp;
	register flags, lpr;
	register struct device *dzaddr;

	tp = &dz_tty[dev];
	dzaddr= dz_addr[dev>>3];
	dzaddr->dzcsr = IE;
	if (dz_scan==0) {
		dzscan();
		dz_scan++;
	}
	flags = tp->t_cflag;
	if ((flags&CBAUD) == 0) {
		/* hang up line */
		dzmodem(dev, OFF);
		return;
	}
	lpr = (dz_speeds[flags&CBAUD]<<8)|(dev&07);
	if (flags&CREAD)
		lpr |= RCVENB;
	if (flags&CS6)
		lpr |= BITS6;
	if (flags&CS7)
		lpr |= BITS7;
	if (flags&PARENB) {
		lpr |= PENABLE;
		if (flags&PARODD)
			lpr |= OPAR;
	}
	if (flags&CSTOPB)
		lpr |= TWOSB;
	dzaddr->dzlpr = lpr;
}

dzrint(dev)
{
	register struct tty *tp;
	register c;
	register line;
	register struct device *dzaddr;

	sysinfo.rcvint++;
	dzaddr = dz_addr[dev];
	while ((c = dzaddr->dzrbuf) < 0) {	/* char. present */
		line = (c>>8)&07;
		tp = &dz_tty[line|(dev<<3)];
		if (tp >= &dz_tty[dz_cnt])
			continue;
		if (!(tp->t_state&(ISOPEN|WOPEN)))
			continue;
		if (tp->t_cflag&CLOCAL || dzaddr->dzmsr&(1<<line)) {
			if ((tp->t_state&CARR_ON) == 0) {
				wakeup(&tp->t_canq);
				tp->t_state |= CARR_ON;
			}
		} else {
			if (tp->t_state&CARR_ON) {
				signal(tp->t_pgrp, SIGHUP);
				tp->t_pgrp = 0;
				dzaddr->dzdtr &= ~(1<<line);
				tp->t_state &= ~CARR_ON;
				ttyflush(tp, (FREAD|FWRITE));
			}
			continue;
		}
		if (tp->t_iflag&IXON) {
			register char ctmp;
			ctmp = c & 0177;
			if (tp->t_state&TTSTOP) {
				if (ctmp == CSTART || tp->t_iflag&IXANY)
					(*tp->t_proc)(tp, T_RESUME);
			} else {
				if (ctmp == CSTOP)
					(*tp->t_proc)(tp, T_SUSPEND);
			}
			if (ctmp == CSTART || ctmp == CSTOP)
				continue;
		}
					/* Check for errors */
	    {
		register int flg;
		char lbuf[3];		/* local character buffer */
		short lcnt;		/* count of chars in lbuf */

		lcnt = 1;
		flg = tp->t_iflag;
		if (c&PERROR && !(flg&INPCK))
			c &= ~PERROR;
		if (c&(FRERROR|PERROR|OVERRUN)) {
			if ((c&0377) == 0) {
				if (flg&IGNBRK)
					continue;
				if (flg&BRKINT) {
					signal(tp->t_pgrp, SIGINT);
					ttyflush(tp, (FREAD|FWRITE));
					continue;
				}
			} else {
				if (flg&IGNPAR)
					continue;
			}
			if (flg&PARMRK) {
				lbuf[2] = 0377;
				lbuf[1] = 0;
				lcnt = 3;
				sysinfo.rawch += 2;
			} else
				c = 0;
		} else {
			if (flg&ISTRIP)
				c &= 0177;
			else {
				c &= 0377;
				if (c == 0377 && flg&PARMRK) {
					lbuf[1] = 0377;
					lcnt = 2;
				}
			}
		}
		/* stash character in r_buf */
		lbuf[0] = c;
		line |= dev<<3;
		if (tp->t_rbuf.c_ptr == NULL)
			return;
		while (lcnt) {
			*tp->t_rbuf.c_ptr++ = lbuf[--lcnt];
			if (--tp->t_rbuf.c_count == 0) {
				dz_work[line] = 0;
				tp->t_rbuf.c_ptr -=
					tp->t_rbuf.c_size - tp->t_rbuf.c_count;
				(*linesw[tp->t_line].l_input)(tp);
			} else	
				dz_work[line] = DZ_ACT;
				/* note DZ_CHAR bit cleared */
		}
	    }
	}
}

dzxint(dev)
{
	register struct tty *tp;
	register struct device *dzaddr;
	register unit;

	sysinfo.xmtint++;
	dzaddr = dz_addr[dev];
	while(dzaddr->dzcsr<0) {	/* TX rdy */
		unit = (dzaddr->dzcsr >> 8) & 07;
		tp = &dz_tty[(dev<<3)|unit];
		if (tp->t_state & TTXON) {
			dzaddr->dztbuf = CSTART;
			tp->t_state &= ~TTXON;
			continue;
		}
		if (tp->t_state & TTXOFF) {
			dzaddr->dztbuf = CSTOP;
			tp->t_state &= ~TTXOFF;
			continue;
		}
		if (tp->t_state & BUSY) {
			if (tp->t_tbuf.c_count) {
				dzaddr->dztbuf = *tp->t_tbuf.c_ptr++;
				tp->t_tbuf.c_count--;
			}
			tp->t_state &= ~BUSY;
			dzproc(tp, T_OUTPUT);
			continue;
		}
		dzaddr->dztcr &= ~(1<<unit);
	}
}

dzproc(tp, cmd)
register struct tty *tp;
{
	register unit;
	struct device *dzaddr;
	int	dev;
	extern ttrstrt();

	unit = tp - dz_tty;
	dev = unit>>3;
	dzaddr = dz_addr[dev];
	unit = 1<<(unit&07);
	switch(cmd) {

	case T_TIME:
		tp->t_state &= ~TIMEOUT;
		dz_brk[dev] &= ~unit;
		dzaddr->dzbrk = dz_brk[dev];
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

		if (tp->t_state & (BUSY|TTSTOP))
			break;
		tbuf = &tp->t_tbuf;
		if (tbuf->c_ptr == NULL || tbuf->c_count == 0) {
			if (tbuf->c_ptr) 
				tbuf->c_ptr -= tbuf->c_size - tbuf->c_count;
			if (!(CPRES & (*linesw[tp->t_line].l_output)(tp)))
				break;
		}
		tp->t_state |= BUSY;
		dzaddr->dztcr |= unit;
		break;
	    }

	case T_SUSPEND:
		tp->t_state |= TTSTOP;
		break;

	case T_BLOCK:
		tp->t_state &= ~TTXON;
		tp->t_state |= TTXOFF|TBLOCK;
		dzaddr->dztcr |= unit;
		break;

	case T_RFLUSH:
		if (!(tp->t_state&TBLOCK))
			break;
	case T_UNBLOCK:
		tp->t_state &= ~(TTXOFF|TBLOCK);
		tp->t_state |= TTXON;
		dzaddr->dztcr |= unit;
		break;

	case T_BREAK:
		dz_brk[dev] |= unit;
		dzaddr->dzbrk = dz_brk[dev];
		tp->t_state |= TIMEOUT;
		timeout(ttrstrt, tp, HZ/4);
		break;
	}
}

dzmodem(dev, flag)
{
	register struct device *dzaddr;
	register bit;

	dzaddr = dz_addr[dev>>3];
	bit = 1<<(dev&07);
	if (flag==OFF)
		dzaddr->dzdtr &= ~bit;
	else
		dzaddr->dzdtr |= bit;
	return(dzaddr->dzmsr & bit);
}

dzscan()
{
	register i;
	register struct device *dzaddr;
	register struct tty *tp;
	register int line;
	register char *workp;
	char	bit;
	static scancnt;

	for (line = 0, workp = dz_work; line < dz_cnt; line++, workp++) {
		if (*workp&DZ_CHAR) {
			tp = &dz_tty[line];
			*workp = 0;
			if (tp->t_rbuf.c_ptr) {
				tp->t_rbuf.c_ptr -=
					tp->t_rbuf.c_size - tp->t_rbuf.c_count;
				(*linesw[tp->t_line].l_input)(tp);
			}
		}
		if (*workp&DZ_ACT)
			*workp |= DZ_CHAR;
	}
	if (!(scancnt++ % DZMSCAN)) for (i=0; i<dz_cnt; i++) {
		tp = &dz_tty[i];
		if (!(tp->t_state&(ISOPEN|WOPEN)))
			continue;
		dzaddr = dz_addr[i>>3];
		bit = 1<<(i&07);
		if (tp->t_cflag&CLOCAL || dzaddr->dzmsr&bit) {
			if ((tp->t_state&CARR_ON)==0) {
				wakeup(&tp->t_canq);
				tp->t_state |= CARR_ON;
			}
		} else {
			if (tp->t_state&CARR_ON) {
				if (tp->t_state&ISOPEN) {
					signal(tp->t_pgrp, SIGHUP);
					dzaddr->dzdtr &= ~bit;
					ttyflush(tp, (FREAD|FWRITE));
				}
				tp->t_state &= ~CARR_ON;
			}
		}
	}
	timeout(dzscan, 0, DZCSCAN);
}

dzclr()
{
	register dev;
	register struct tty *tp;

	for (dev = 0; dev < dz_cnt; dev++) {
		tp = &dz_tty[dev];
		if ((tp->t_state&(ISOPEN|WOPEN)) == 0)
			continue;
		dzparam(dev);
		dzmodem(dev, ON);
		tp->t_state &= ~BUSY;
		dzproc(tp, T_INPUT);
		dzproc(tp, T_OUTPUT);
	}
}
