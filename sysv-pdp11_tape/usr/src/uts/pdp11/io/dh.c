/* @(#)dh.c	1.1 */
/*
 *	DH11 & DM11-BB driver for multiple units
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

#define	DHCSCAN	2	/* clock ticks for character scan	*/
#define	DH_CHAR	01	/* character arrived flag (dh_work)	*/
#define	DH_ACT	02	/* character in buffer flag (dh_work)	*/
char	dh_work[64];

struct device *dh_addr[];
int	dh_cnt;
struct tty dh_tty[];
struct inter dm_inter[];
short	dhsar[8];
static char dh_xon = CSTART;
static char dh_xoff = CSTOP;

#define	BITS6	01
#define	BITS7	02
#define	TWOSB	04
#define	PENABLE	020
#define	OPAR	040

#define	IENAB	030100
#define	XINT	0100000
#define	CLRNEM	0400

struct device {
	short dhcsr, dhnxch, dhlpr, dhcar;
	short dhbcr, dhbar, dhbrk, dhsilo;
};

struct devdm *dm_addr[];

#define	DONE	0200
#define	SCENABL	040
#define	BSY	020
#define	TURNON	07	/* RTS, DTR, line enable */
#define	TURNOFF	1	/* line enable only */
#define	CARRIER	0100

struct devdm {
	short	dmcsr, dmlstat;
};

dhopen(dev, flag)
{
	register struct tty *tp;
	extern dhproc();

	if (dev >= dh_cnt) {
		u.u_error = ENXIO;
		return;
	}
	tp = &dh_tty[dev];
	if ((tp->t_state&(ISOPEN|WOPEN)) == 0) {
		ttinit(tp);
		tp->t_proc = dhproc;
		dhparam(dev);
	}
	spl5();
	if (tp->t_cflag&CLOCAL || dmctl(dev, TURNON)&CARRIER)
		tp->t_state |= CARR_ON;
	if (!(flag&FNDELAY))
	while ((tp->t_state&CARR_ON)==0) {
		tp->t_state |= WOPEN;
		sleep(&tp->t_canq, TTIPRI);
	}
	(*linesw[tp->t_line].l_open)(tp);
	spl0();
}

dhclose(dev)
{
	register struct tty *tp;

	tp = &dh_tty[dev];
	(*linesw[tp->t_line].l_close)(tp);
	if (tp->t_cflag&HUPCL) {
		spl5();
		dmctl(dev, TURNOFF);
		spl0();
	}
}

dhread(dev)
{
	register struct tty *tp;

	tp = &dh_tty[dev];
	(*linesw[tp->t_line].l_read)(tp);
}

dhwrite(dev)
{
	register struct tty *tp;

	tp = &dh_tty[dev];
	(*linesw[tp->t_line].l_write)(tp);
}

dhioctl(dev, cmd, arg, mode)
register dev;
{
	switch(cmd) {
	default:
		if (ttiocom(&dh_tty[dev], cmd, arg, mode))
			dhparam(dev);
	}
}

dhparam(dev)
{
	register struct tty *tp;
	register flags, lpr;
	register struct device *rp;
	static	scanf;

	tp = &dh_tty[dev];
	rp = dh_addr[dev>>4];
	if (!scanf) {
		dhscan();
		scanf++;
	}
	flags = tp->t_cflag;
	lpr = (flags&CBAUD)<<10;
	if (flags&CREAD)
		lpr |= (flags&CBAUD)<<6;
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
	spl5();
	rp->dhcsr &= ~017;
	rp->dhcsr |= (dev&017) | IENAB;
	if ((flags&CBAUD) == 0) {
		/* Hang up line */
		if (tp->t_state&CARR_ON) {
			rp->dhbcr = -1;
			dmctl(dev, TURNOFF);
		}
		spl0();
		return;
	}
	rp->dhlpr = lpr;
	spl0();
}

dhrint(dev)
{
	register struct tty *tp;
	register c;
	register struct device *rp;
	int	line;

	sysinfo.rcvint++;
	rp = dh_addr[dev];
	while ((c = rp->dhnxch) < 0) {	/* char. present */
		line = ((c>>8)&017)|(dev<<4);
		if (line >= dh_cnt)
			continue;
		tp = &dh_tty[line];
		if (!(tp->t_state&(ISOPEN|WOPEN)))
			continue;
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
		if (tp->t_rbuf.c_ptr == NULL)
			return;
		while (lcnt) {
			*tp->t_rbuf.c_ptr++ = lbuf[--lcnt];
			if (--tp->t_rbuf.c_count == 0) {
				dh_work[line] = 0;
				tp->t_rbuf.c_ptr -=
					tp->t_rbuf.c_size - tp->t_rbuf.c_count;
				(*linesw[tp->t_line].l_input)(tp);
			} else	
				dh_work[line] = DH_ACT;
				/* note DH_CHAR bit cleared */
		}
	    }
	}
}

/*
 * DH11 transmitter interrupt.
 * Restart each line which used to be active but has
 * terminated transmission since the last interrupt.
 */
dhxint(dev)
{
	register struct tty *tp;
	register ttybit, bar;
	struct device *rp;
	short *sbar;

	sysinfo.xmtint++;
	rp = dh_addr[dev];
	rp->dhcsr &= ~XINT;
	rp->dhcsr |= CLRNEM;
	sbar = &dhsar[dev];
	bar = *sbar & ~rp->dhbar;
	dev <<= 4;
	ttybit = 1;
	for (; bar; dev++, ttybit <<= 1) if (bar&ttybit) {
		tp = &dh_tty[dev];
		bar &= ~ttybit;
		lobyte(rp->dhcsr) = (dev&017) | IENAB;
		if (tp->t_state&BUSY) {
			register n;

			tp->t_state &= ~BUSY;
			if (tp->t_tbuf.c_count) {
				n = rp->dhcar - (int)tp->t_tbuf.c_ptr;
				tp->t_tbuf.c_count -= n;
				tp->t_tbuf.c_ptr += n;
			}
		}
		if (tp->t_state&TTXON) {
			tp->t_state &= ~TTXON;
			rp->dhcar = &dh_xon;
			rp->dhbcr = -1;
			rp->dhbar |= ttybit;
			continue;
		}
		if (tp->t_state&TTXOFF) {
			tp->t_state &= ~TTXOFF;
			rp->dhcar = &dh_xoff;
			rp->dhbcr = -1;
			rp->dhbar |= ttybit;
			continue;
		}
		*sbar &= ~ttybit;
		dhproc(tp, T_OUTPUT);
	}
}

dhproc(tp, cmd)
register struct tty *tp;
{
	register struct device *rp;
	int	dev, ttybit;
	extern ttrstrt();

	dev = tp - dh_tty;
	rp = dh_addr[dev>>4];
	ttybit = 1 << (dev&017);
	switch(cmd) {

	case T_TIME:
		tp->t_state &= ~TIMEOUT;
		rp->dhbrk &= ~ttybit;
		goto start;

	case T_WFLUSH:
	case T_RESUME:
		tp->t_state &= ~TTSTOP;
		goto start;

	case T_OUTPUT:
	start:
	{
		register struct ccblock *tbuf;

		if (tp->t_state&(TIMEOUT|TTSTOP|BUSY))
			break;
		tbuf = &tp->t_tbuf;
		if (tbuf->c_ptr == NULL || tbuf->c_count == 0) {
			if (tbuf->c_ptr)
				tbuf->c_ptr -= tbuf->c_size;
			if (!(CPRES & (*linesw[tp->t_line].l_output)(tp)))
				break;
		}
		lobyte(rp->dhcsr) = (dev&017) | IENAB;
		rp->dhcar = tbuf->c_ptr;
		rp->dhbcr = -tbuf->c_count;
		rp->dhbar |= ttybit;
		dhsar[dev>>4] |= ttybit;
		tp->t_state |= BUSY;
		break;
	}

	case T_SUSPEND:
		tp->t_state |= TTSTOP;
		if (tp->t_state&BUSY) {
			lobyte(rp->dhcsr) = (dev&017) | IENAB;
			rp->dhbcr = -1;
		}
		break;

	case T_BLOCK:
		tp->t_state |= TBLOCK;
		tp->t_state &= ~TTXON;
		lobyte(rp->dhcsr) = (dev&017) | IENAB;
		if (tp->t_state&BUSY) {
			tp->t_state |= TTXOFF;
			rp->dhbcr = -1;
		} else {
			rp->dhcar = &dh_xoff;
			rp->dhbcr = -1;
			rp->dhbar |= ttybit;
			dhsar[dev>>4] |= ttybit;
		}
		break;

	case T_RFLUSH:
		if (!(tp->t_state&TBLOCK))
			break;

	case T_UNBLOCK:
		tp->t_state &= ~(TTXOFF|TBLOCK);
		lobyte(rp->dhcsr) = (dev&017) | IENAB;
		if (tp->t_state&BUSY) {
			tp->t_state |= TTXON;
			rp->dhbcr = -1;
		} else {
			rp->dhcar = &dh_xon;
			rp->dhbcr = -1;
			rp->dhbar |= ttybit;
			dhsar[dev>>4] |= ttybit;
		}
		break;

	case T_BREAK:
		rp->dhbrk |= ttybit;
		tp->t_state |= TIMEOUT;
		timeout(ttrstrt, tp, HZ/4);
		break;
	}
}

/*
 * Dump control bits into the DM registers.
 */
dmctl(dev, bits)
register dev;
{
	register struct devdm *dmaddr;
	register svcsr;

	sysinfo.mdmint++;
	dmaddr = dm_addr[dev>>4];
	dmaddr->dmcsr &= ~SCENABL;
	while((svcsr=dmaddr->dmcsr)&BSY);
	dmaddr->dmcsr = dev&017;
	dmaddr->dmlstat = bits;
	dev = dmaddr->dmlstat;
	dmaddr->dmcsr = svcsr|IENAB|SCENABL;
	return(dev);
}

/*
 * DM11 interrupt.
 * Mainly, deal with carrier transitions.
 */
dmintr(dev)
{
	register struct tty *tp;
	register struct devdm *dmaddr;
	register unit;

	dmaddr = dm_addr[dev];
	unit = (dmaddr->dmcsr&017)|(dev<<4);
	if (unit>=dh_cnt) {
		dmaddr->dmlstat = 0;
		dmaddr->dmcsr &= ~DONE;
		return;
	}
	tp = &dh_tty[unit];
	dm_inter[unit].cnt++;
	if (tp->t_cflag&CLOCAL) {
		dmaddr->dmlstat = 0;
		dmaddr->dmcsr &= ~DONE;
		return;
	}
	if (dmaddr->dmlstat&CARRIER) {
		if ((tp->t_state&CARR_ON)==0) {
			wakeup(&tp->t_canq);
			tp->t_state |= CARR_ON;
		}
	} else {
		if (tp->t_state&CARR_ON) {
			if (tp->t_state&ISOPEN) {
				signal(tp->t_pgrp, SIGHUP);
				dmaddr->dmlstat = 0;
				ttyflush(tp, FREAD|FWRITE);
			}
			tp->t_state &= ~CARR_ON;
		}
	}
	dmaddr->dmcsr &= ~DONE;
}

dhscan()
{
	register struct tty *tp;
	register int line;
	register char *workp;

	for (line = 0, workp = dh_work; line < dh_cnt; line++, workp++) {
		if (*workp&DH_CHAR) {
			tp = &dh_tty[line];
			*workp = 0;
			if (tp->t_rbuf.c_ptr) {
				tp->t_rbuf.c_ptr -=
					tp->t_rbuf.c_size - tp->t_rbuf.c_count;
				(*linesw[tp->t_line].l_input)(tp);
			}
		}
		if (*workp&DH_ACT)
			*workp |= DH_CHAR;
	}
	timeout(dhscan, 0, DHCSCAN);
}

dhclr()
{
	register dev;
	register struct tty *tp;

	for (dev = 0; dev < dh_cnt; dev++) {
		tp = &dh_tty[dev];
		if ((tp->t_state&(ISOPEN|WOPEN)) == 0)
			continue;
		dhparam(dev);
		dmctl(dev, TURNON);
		tp->t_state &= ~BUSY;
		dhproc(tp, T_OUTPUT);
	}
}
