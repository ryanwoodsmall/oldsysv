/* @(#)kl.c	1.1 */
/*
 *   KL/DL-11 driver
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

struct device *kl_addr[];
int	kl_cnt;
struct tty kl_tty[];
int	klproc(), ttrstrt();
char	partab[];
static	diag_out	= 1;

#define DSRDY	02
#define	RDRENB	01

struct device {
	short rcsr, rbuf;
	short tcsr, tbuf;
};

klopen(dev, flag)
{
	register struct device *addr;
	register struct tty *tp;

	if (dev >= kl_cnt) {
		u.u_error = ENXIO;
		return;
	}
	tp = &kl_tty[dev];
	addr = kl_addr[dev];
	tp->t_proc = klproc;
	if ((tp->t_state&ISOPEN) == 0) {
		ttinit(tp);
		tp->t_state = ISOPEN|CARR_ON;
		tp->t_iflag |= ICRNL|IXON|IXANY|ISTRIP;
		tp->t_oflag |= OPOST|ONLCR|TAB3;
		tp->t_lflag |= ISIG|ICANON|ECHO|ECHOK;
	}
	addr->rcsr |= IENABLE|DSRDY|RDRENB;
	addr->tcsr |= IENABLE;
	(*linesw[tp->t_line].l_open)(tp);
}

klclose(dev)
{
	register struct tty *tp;

	tp = &kl_tty[dev];
	(*linesw[tp->t_line].l_close)(tp);
	tp->t_state = 0;
}

klread(dev)
{
	register struct tty *tp;

	tp = &kl_tty[dev];
	(*linesw[tp->t_line].l_read)(tp);
}

klwrite(dev)
{
	register struct tty *tp;

	tp = &kl_tty[dev];
	(*linesw[tp->t_line].l_write)(tp);
}

klxint(dev)
{
	register struct tty *tp;
	register struct device *addr;

	sysinfo.xmtint++;
	tp = &kl_tty[dev];
	addr = kl_addr[dev];
	if (addr->tcsr&DONE) {
		tp->t_state &= ~BUSY;
		klproc(tp, T_OUTPUT);
	}
}

klrint(dev)
{
	register struct device *addr;
	register struct tty *tp;
	register flg;
	int	c;

	sysinfo.rcvint++;
	tp = &kl_tty[dev];
	addr = kl_addr[dev];
	c = addr->rbuf;
	addr->rcsr |= RDRENB;
	if (dev || (c&0177) != (037&'o')) {
		if (!(tp->t_state & ISOPEN))
			return;
		if (tp->t_rbuf.c_ptr == NULL)
			return;
		flg = tp->t_iflag;
		if (flg&IXON) {
			register ctmp;

			ctmp = c & 0177;
			if (tp->t_state&TTSTOP) {
				if (ctmp == CSTART || flg&IXANY)
					(*tp->t_proc)(tp, T_RESUME);
			} else {
				if (ctmp == CSTOP)
					(*tp->t_proc)(tp, T_SUSPEND);
			}
			if (ctmp == CSTART || ctmp == CSTOP)
				return;
		}
		if (c&PERROR && !(flg&INPCK))
			c &= ~PERROR;
		if (c&(FRERROR|PERROR|OVERRUN)) {
			if ((c&0377) == 0) {
				if (flg&IGNBRK)
					return;
				if (flg&BRKINT) {
					signal(tp->t_pgrp, SIGINT);
					ttyflush(tp, (FREAD|FWRITE));
					return;
				}
			} else {
				if (flg&IGNPAR)
					return;
			}
		} else {
			if (flg&ISTRIP)
				c &= 0177;
			else {
				c &= 0377;
			}
		}
		*tp->t_rbuf.c_ptr = c;
		tp->t_rbuf.c_count--;
		(*linesw[tp->t_line].l_input)(tp);
	} else {
		if (diag_out)
			printf("\nO.S. messages -- OFF\n");
		diag_out = !diag_out;
		if (diag_out)
			printf("\nO.S. messages -- ON\n");
	}
}

klioctl(dev, cmd, arg, mode)
register dev;
{
	ttiocom(&kl_tty[dev], cmd, arg, mode);
}

klproc(tp, cmd)
register struct tty *tp;
{
	register struct device *addr;

	switch (cmd) {

	case T_TIME:
		tp->t_state &= ~TIMEOUT;
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
		tp->t_state |= BUSY;
	/* parity ? */
		addr = kl_addr[tp - kl_tty];
		addr->tbuf = *tbuf->c_ptr++;
		tbuf->c_count--;
		}
		break;

	case T_SUSPEND:
		tp->t_state |= TTSTOP;
		break;

	case T_BLOCK:
		tp->t_state |= TBLOCK;
		break;

	case T_RFLUSH:
	case T_UNBLOCK:
		tp->t_state &= ~TBLOCK;
		break;
	}
}

putchar(c)
register c;
{
	register s;
	register struct device *addr;

	addr = kl_addr[0];
	while((addr->tcsr&DONE) == 0)
		;
	if (c == 0)
		return;
	if (diag_out == 0)
		return;
	s = addr->tcsr;
	addr->tcsr = 0;
	addr->tbuf = c;
	if (c == '\n') {
		putchar('\r');
		putchar(0177);
		putchar(0177);
	}
	putchar(0);
	addr->tcsr = s;
}

klclr()
{
	register dev;
	register struct device *addr;
	register struct tty *tp;

	for (dev = 0; dev < kl_cnt; dev++) {
		tp = &kl_tty[dev];
		if ((tp->t_state&(ISOPEN|WOPEN)) == 0)
			continue;
		addr = kl_addr[dev];
		addr->rcsr |= IENABLE|DSRDY|RDRENB;
		addr->tcsr |= IENABLE;
		tp->t_state &= ~BUSY;
		klproc(tp, T_OUTPUT);
	}
}
