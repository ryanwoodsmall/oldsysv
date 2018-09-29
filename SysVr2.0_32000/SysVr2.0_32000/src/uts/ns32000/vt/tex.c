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
/* @(#)tex.c	6.1 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/tty.h"
#include "sys/termio.h"
#include "sys/crtctl.h"

char maptex[] = {
	0234,	LCA,
	0127,	CUP,
	0136,	CDN,
	0126,	CRI,
	0210,	CLE,
	0137,	CLE,
	0234,	HOME,
	0120,	STB,
	0100,	SPB,
	0014,	CS,
	0014,	CM,
	0131,	IL,
	0134,	DL,
	0130,	IC,
	0133,	DC,
	0132,	EEOL,
	0132,	EEOP,
/*	0xxx,	KBL,	*/
/*	0xxx,	KBU,	*/
	IL,	DVSCN,
	DL,	UVSCN,
	0
};

#define US 037

texoutput(ac, atp)
struct tty *atp;
{
	register struct clist *qp;
	register char *cp;
	register c;

	c = ac;
	qp = &atp->t_outq;

	for(cp=maptex; *cp++;)
	if (c == *cp++) {
		if (c == DVSCN) {
			ttdvscn(atp);
			break;
		}
		if (c == UVSCN) {
			ttuvscn(atp);
			break;
		}
		if ((c=cp[-2]) > 0) {
			putc(ESC, qp);
			if (ac == STB || ac == SPB) {
				putc(0126,qp);
				putc(US, qp);
			}
		}
		qputc(c&0177, qp);
		switch (ac) {
			case EEOP:
				for(c=atp->t_row+1; c<24; c++) {
					putc(QESC, qp);
					putc(0203, qp);
					qputc(034, qp);
					qputc(32, qp);
					qputc(c+32, qp);
					putc(QESC, qp);
					putc(0201, qp);
					putc(ESC, qp);
					putc(0132, qp);
				}

			case EEOL:
				putc(QESC, qp);
				putc(0203, qp);
				putc(034, qp);

			case LCA:
			case HOME:
				qputc(atp->t_col+32, qp);
				qputc(atp->t_row+32, qp);

			case IL:
			case CS:
			case DL:
				putc(QESC, qp);
				putc(0201, qp);
				break;
		}
		break;
	}
}

texinput(ac, atp)
struct tty *atp;
{
	register struct tty *tp;
	register c;
	register char *cp;

	c = ac;
	tp = atp;
	if (tp->t_tmflag&TERM_BIT) {
		tp->t_tmflag &= ~TERM_BIT;
		for (cp=maptex; *cp++;)
			if (c == (cp++)[-1])
				return(cp[-1] | CPRES);

	} else if (c == ESC) {
		tp->t_tmflag |= TERM_BIT;
		return(-1);
	}

	return(c);
}

texioctl(tp, flag, nvrow)
register struct tty *tp;
register unsigned nvrow;
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
