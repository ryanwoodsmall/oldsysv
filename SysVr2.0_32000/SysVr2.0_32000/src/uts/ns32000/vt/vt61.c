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
/* @(#)vt61.c	6.1 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/termio.h"
#include "sys/tty.h"
#include "sys/crtctl.h"

char mapvt61[] = {
	0131,	LCA,
	0101,	CUP,
	0102,	CDN,
	0103,	CRI,
	0104,	CLE,
	0110,	HOME,
	0312,	STB,
	0352,	SPB,
	0110,	CS,
	0322,	CM,
	0306,	IL,
	0304,	DL,
	0317,	IC,
	0323,	DC,
	0113,	EEOL,
	0112,	EEOP,
/*	0xxx,	KBL,	*/
/*	0xxx,	KBU,	*/
	0121,	IL,			/* Input only */
	0122,	EEOL,			/* Input only */
	IL,	DVSCN,
	DL,	UVSCN,
	0
};


#define TDELAY	0203

int vt61speed = 12;
char vt61str[] = {' ', ESC, 'P', 0157, 010, 0};


vt61output(ac, atp)
struct tty *atp;
{
	register struct clist *qp;
	register char *cp;
	register c;

	c = ac;
	qp = &atp->t_outq;

	for (cp=(char *)mapvt61; *cp++;)
	if (c == *cp++) {
		if (ac == DVSCN) {
			ttdvscn(atp);
			break;
		}
		if (ac == UVSCN) {
			ttuvscn(atp);
			break;
		}
		if (ac == STB)
			putc(' ', qp);
		putc(ESC, qp);
		if ((c=cp[-2]) < 0)
			if (ac == STB || ac == SPB || ac == CM)
				putc('O', qp);
			else    putc('P', qp);
		qputc(c&0177, qp);
		switch(ac) {
			case LCA:
				qputc(atp->t_row+32, qp);
				qputc(atp->t_col+32, qp);
				break;

			case IC:
				for(cp=vt61str; *cp; cp++)
					putc(*cp, qp);
				break;

			case SPB:
				putc(' ', qp);
				break;

			case CS:
				putc(ESC, qp);
				putc(0112, qp);

			case IL:
			case DL:
			case EEOL:

				if ((atp->t_cflag&CBAUD) >= vt61speed)
					putc(QESC, qp);
					putc(TDELAY, qp);
				break;
		}
		break;
	}
}
vt61input(ac, atp)
struct tty *atp;
{
	register struct tty *tp;
	register c;
	register char *cp;

	c = ac;
	tp = atp;

	if (tp->t_tmflag&TERM_BIT) {
		tp->t_tmflag &= ~TERM_BIT;
		for (cp=mapvt61; *cp++;)
		if (c == (cp++)[-1])
			return(cp[-1] | CPRES);
	} else if (c == ESC) {
		tp->t_tmflag |= TERM_BIT;
		return(-1);
	}
	return(c);
}

vt61ioctl(tp, flag, nvrow)
register struct tty *tp;
{

	if (nvrow > 23) {
		u.u_error = EINVAL;
		return;
	}
	if (flag == LDOPEN) {
		tp->t_lrow = 23;
		tp->t_tmflag = ANL;
		tp->t_iflag = (tp->t_iflag&(IGNBRK|BRKINT|IGNPAR|PARMRK|INPCK))
			|ISTRIP|ICRNL|IXON|IXANY;
		tp->t_lflag = ECHO|ECHOE|ECHOK|ISIG|ICANON;
		tp->t_oflag = OPOST|ONLCR;
	}
}
