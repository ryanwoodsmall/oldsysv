/* @(#)dl.c	1.1 */
/*
 *   DL-11E driver
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

struct device *dl_addr[];
int	dl_cnt;
struct tty dl_tty[];
char	partab[];

#define	DTR	02
#define	RTS	04
#define	DSIE	040
#define	IE	0100
#define	RCVDONE	0200
#define	CARRIER	010000
#define	DSCHG	0100000
#define	XBRK	01

#define	FERROR	020000

struct device {
	short	rcsr, rbuf;
	short	tcsr, tbuf;
};

dlopen(dev, flag)
{
	register struct device *addr;
	register struct tty *tp;
	extern dlproc();

	if(dev >= dl_cnt) {
		u.u_error = ENXIO;
		return;
	}
	tp = &dl_tty[dev];
	addr = dl_addr[dev];
	if ((tp->t_state&(ISOPEN|WOPEN)) == 0) {
		ttinit(tp);
		tp->t_proc = dlproc;
		addr->rcsr |= IE;
		addr->tcsr |= IE;
	}
	spl4();
	addr->rcsr |= DSIE|RTS|DTR;
	if (addr->rcsr & CARRIER)
		tp->t_state |= CARR_ON;
	if (!(flag&FNDELAY))
	while ((tp->t_state&CARR_ON)==0) {
		tp->t_state |= WOPEN;
		sleep((caddr_t)&tp->t_canq, TTIPRI);
	}
	(*linesw[tp->t_line].l_open)(tp);
	spl0();
}

dlclose(dev)
{
	register struct tty *tp;

	tp = &dl_tty[dev];
	(*linesw[tp->t_line].l_close)(tp);
	if (tp->t_cflag&HUPCL)
		dl_addr[dev]->rcsr &= ~(RTS|DTR);
}

dlread(dev)
{
	register struct tty *tp;

	tp = &dl_tty[dev];
	(*linesw[tp->t_line].l_read)(tp);
}

dlwrite(dev)
{
	register struct tty *tp;

	tp = &dl_tty[dev];
	(*linesw[tp->t_line].l_write)(tp);
}

dlxint(dev)
{
	register struct tty *tp;
	register struct device *addr;

	tp = &dl_tty[dev];
	addr = dl_addr[dev];
	if (addr->tcsr&DONE) {
		tp->t_state &= ~BUSY;
		if (tp->t_state & TTXON) {
			addr->tbuf = CSTART;
			tp->t_state &= ~TTXON;
		} else if (tp->t_state & TTXOFF) {
			addr->tbuf = CSTOP;
			tp->t_state &= ~TTXOFF;
		} else
			dlproc(tp, T_OUTPUT);
	}
}

dlrint(dev)
{
	register int c;
	register struct device *addr;
	register struct tty *tp;

	tp = &dl_tty[dev];
	addr = dl_addr[dev];
	c = addr->rcsr;
	if (c&DSCHG) {
		if (c&CARRIER) {
			if ((tp->t_state&CARR_ON)==0) {
				wakeup(&tp->t_canq);
				tp->t_state |= CARR_ON;
			}
		} else {
			if (tp->t_state&CARR_ON) {
				if (tp->t_state&ISOPEN) {
					signal(tp->t_pgrp, SIGHUP);
					addr->rcsr &= ~(RTS|DTR);
					ttyflush(tp, (FREAD|FWRITE));
				}
				tp->t_state &= ~CARR_ON;
			}
		}
	} else if (c&RCVDONE) {
		c = addr->rbuf;
		(*linesw[tp->t_line].l_input)(tp, c, 0);
	}
}

dlioctl(dev, cmd, arg, mode)
{
	register struct tty *tp;

	tp = &dl_tty[dev];
	if (ttiocom(tp, cmd, arg, mode))
		if ((tp->t_cflag&CBAUD) == 0)
			dl_addr[dev]->rcsr &= ~(RTS|DTR);
}

dlproc(tp, cmd)
register struct tty *tp;
{
	register c;
	register struct device *addr;

	extern ttrstrt();

	addr = dl_addr[tp - dl_tty];
	switch (cmd) {

	case T_TIME:
		tp->t_state &= ~TIMEOUT;
		addr->tcsr &= ~XBRK;
		goto start;

	case T_WFLUSH:
	case T_RESUME:
		tp->t_state &= ~TTSTOP;
		goto start;

	case T_OUTPUT:
	start:
		if (tp->t_state&(TIMEOUT|TTSTOP|BUSY))
			break;
		if (tp->t_state&TTIOW && tp->t_outq.c_cc==0) {
			tp->t_state &= ~TTIOW;
			wakeup((caddr_t)&tp->t_oflag);
		}
		while ((c=getc(&tp->t_outq)) >= 0) {
			if (tp->t_oflag&OPOST && c == 0200) {
				if ((c = getc(&tp->t_outq)) < 0)
					break;
				if (c > 0200) {
					tp->t_state |= TIMEOUT;
					timeout(ttrstrt, (caddr_t)tp, (c&0177));
					break;
				}
			}
			tp->t_state |= BUSY;
	/* parity ? */
			addr->tbuf = c;
			break;
		}
		if (tp->t_state&OASLP && tp->t_outq.c_cc<=ttlowat[tp->t_cflag&CBAUD]) {
			tp->t_state &= ~OASLP;
			wakeup((caddr_t)&tp->t_outq);
		}
		break;

	case T_SUSPEND:
		tp->t_state |= TTSTOP;
		break;

	case T_BLOCK:
		tp->t_state &= ~TTXON;
		tp->t_state |= TBLOCK;
		if (tp->t_state&BUSY)
			tp->t_state |= TTXOFF;
		else
			addr->tbuf = CSTOP;
		break;

	case T_RFLUSH:
		if (!(tp->t_state&TBLOCK))
			break;
	case T_UNBLOCK:
		tp->t_state &= ~(TTXOFF|TBLOCK);
		if (tp->t_state&BUSY)
			tp->t_state |= TTXON;
		else
			addr->tbuf = CSTART;
		break;

	case T_BREAK:
		addr->tcsr |= XBRK;
		tp->t_state |= TIMEOUT;
		timeout(ttrstrt, tp, HZ/4);
		break;
	}
}

dlclr()
{
	register dev;
	register struct device *addr;
	register struct tty *tp;

	for (dev = 0; dev < dl_cnt; dev++) {
		tp = &dl_tty[dev];
		if ((tp->t_state&(ISOPEN|WOPEN)) == 0)
			continue;
		addr = dl_addr[dev];
		addr->rcsr |= IE|DSIE|RTS|DTR;
		addr->tcsr |= IE;
		tp->t_state &= ~BUSY;
		dlproc(tp, T_OUTPUT);
	}
}
