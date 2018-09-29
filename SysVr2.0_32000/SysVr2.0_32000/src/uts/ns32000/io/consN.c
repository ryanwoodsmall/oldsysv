/* Use this version if you have rev-N chips and the
	console hangs periodically */
/* You must also edit ml/trap.s to make the xmit interrupt level-triggered
	Change:
	movb	$0xcb,0x6(r2)
   to
	movb	$0xcf,0x6(r2)
 */
/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
/*
 *	Console interface
 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/tty.h"
#include "sys/termio.h"
#include "sys/systm.h"
#include "sys/cons.h"
#include "sys/conf.h"
#include "sys/sysinfo.h"
#include "sys/page.h"

extern struct	tty con_tty;

int	conproc();
int	ttrstrt();
int	con_diag	= 1;
int	con_brk;
char	con_xena;	/* xmit interrupt enabled */
#define	CNTLO	017
#define CNTLA	01

conopen(dev, flag)
dev_t dev;
{
	register struct tty *tp;

	if (dev != 0) {
		u.u_error = ENXIO;
		return;
	}
	tp = &con_tty;
	tp->t_proc = conproc;
	if ((tp->t_state&ISOPEN) == 0) {
		ttinit(tp);
		tp->t_state = CARR_ON;
		(*linesw[tp->t_line].l_open)(tp);
		tp->t_iflag |= ICRNL|IXON|IXANY|ISTRIP;
		tp->t_oflag |= OPOST|ONLCR|TAB3;
		tp->t_lflag |= ISIG|ICANON|ECHO|ECHOK;
		icuenable(C_RINT);
	} else
		(*linesw[tp->t_line].l_open)(tp);
}

conclose(dev)
dev_t dev;
{
	register struct tty *tp;

	tp = &con_tty;
	(*linesw[tp->t_line].l_close)(tp);
	tp->t_state = 0;
}

conread(dev)
dev_t dev;
{
	(*linesw[con_tty.t_line].l_read)(&con_tty);
}

conwrite(dev)
dev_t dev;
{
	(*linesw[con_tty.t_line].l_write)(&con_tty);
}

conxint(dev)
{
	register struct tty *tp;

	sysinfo.xmtint++;
	tp = &con_tty;
	tp->t_state &= ~BUSY;
	conproc(tp, T_OUTPUT);
	if ((tp->t_state & BUSY) == 0) {
		icudisable(C_XINT);
		con_xena = 0;
	}
}

conrint(dev)
{
	register struct tty *tp;
	register struct console *cp;
	register c, ch;

	sysinfo.rcvint++;
	tp = &con_tty;
	cp = CONADDR;
	while (cp->status & RxRDY) {
		c = cp->data;
		ch = c & 0177;
		if (ch == CNTLO) {
			if (con_diag)
				printf("\nO.S. messages -- OFF\n");
			con_diag = !con_diag;
			if (con_diag)
				printf("\nO.S. messages -- ON\n");
			continue;
		}
		if (ch == CNTLA && con_brk == 0) {
			con_brk = 1;
			printf("\nB - bpt, D - dump\n");
			continue;
		} else if (con_brk) {
			switch(ch) {
			case 'D':
				do_dump();
				/* no return */
			case 'B':
				do_bpt();
				/* no return */
			}
			con_brk = 0;
		}
		if (!(tp->t_state & ISOPEN))
			continue;
		if (tp->t_rbuf.c_ptr == NULL)
			continue;
		if (tp->t_iflag&IXON) {
			if (tp->t_state&TTSTOP) {
				if (ch == CSTART || tp->t_iflag&IXANY)
					(*tp->t_proc)(tp, T_RESUME);
			} else {
				if (ch == CSTOP)
					(*tp->t_proc)(tp, T_SUSPEND);
			}
			if (ch == CSTART || ch == CSTOP)
				continue;
		}
		if (tp->t_iflag&ISTRIP)
			c &= 0177;
		else
			c &= 0377;
		*tp->t_rbuf.c_ptr = c;
		tp->t_rbuf.c_count--;
		(*linesw[tp->t_line].l_input)(tp, L_BUF);
	}
}

conioctl(dev, cmd, arg, mode)
{
	ttiocom(&con_tty, cmd, arg, mode);
}

conproc(tp, cmd)
register struct tty *tp;
{
	register c;

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
		register struct console *cp;
		register struct ccblock *tbuf;

		cp = CONADDR;
		tbuf = &tp->t_tbuf;
		if (tp->t_state & (BUSY|TTSTOP))
			break;
		if (tbuf->c_ptr == 0 || tbuf->c_count <= 0) {
			if (tbuf->c_ptr)
				tbuf->c_ptr -= tbuf->c_size - tbuf->c_count;
			if (!(CPRES & (*linesw[tp->t_line].l_output)(tp))) {
				break;
			}
		}
		tp->t_state |= BUSY;
		cp->data = *tbuf->c_ptr++ & C_DATA;
		if (con_xena == 0) {
			icuenable(C_XINT);
			con_xena = 1;
		}
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
	register struct console *cp;
	register s;

	cp = CONADDR;
	while((cp->status & TxRDY) == 0)
		;
	if (c == 0)
		return;
	if (con_diag == 0)
		return;
	s = icudisable(C_XINT);
	cp->data = c & C_DATA;
	if (c == '\n') {
		putchar('\r');
	}
	putchar(0);
	if (C_XINT & ~s)
		icuenable(C_XINT);
}

do_bpt()
{
	extern char vector;
	extern int kdebug;
	int i, *p;

	if (kdebug == 0) {
		i = (int)&vector;
		p = (int *)kpte2;
		p[i>>9] |= PG_KW;
		asm("	movd	$0xe01544,r1");
		asm("	sprd	intbase,r2");
		asm("	addr	32(r2),r2");
		asm("	lmr	eia,r2");
		asm("	movd	32(r1),0(r2)");
	}
	asm("	bpt");
}
