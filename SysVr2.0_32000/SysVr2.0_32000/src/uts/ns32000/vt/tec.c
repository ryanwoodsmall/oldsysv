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
/* @(#)tec.c	6.1 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/tty.h"
#include "sys/termio.h"
#include "sys/crtctl.h"

char maptec[] = {
	0154,	LCA,
	0170,	CUP,
	0150,	CDN,
	0147,	CRI,
	0167,	CLE,
	0151,	HOME,
	0173,	STB,
	0174,	SPB,
	0146,	CS,
	0166,	CM,
	0145,	IL,
	0165,	DL,
	0144,	IC,
	0164,	DC,
	0143,	EEOL,
	0163,	EEOP,
	0175,	KBL,
	0160,	KBU,
	0200 | DL,	UVSCN,
	0200 | IL,	DVSCN,
	0
};

tecoutput(ac, atp)
struct tty *atp;
{
	register struct clist *qp;
	register char *cp;
	register c;

	c = ac;
	qp = &atp->t_outq;
	for(cp = maptec; *cp++;)
	if (c == *cp++) {
		if (c == UVSCN) {
			ttuvscn(atp);
			break;
		}
		if (c == DVSCN) {
			ttdvscn(atp);
			break;
		}
		qputc(cp[-2], qp);
		if (c == LCA) {
			qputc(~atp->t_col, qp);
			qputc(~atp->t_row, qp);
		}
		break;
	}
}

tecinput(ac, atp)
struct tty *atp;
{
	register struct tty *tp;
	register c;
	register char *cp;


	c = ac;
	for(cp=maptec; *cp++;)
	if (c == (cp++)[-1])
		return(cp[-1] | 0100000);
	return(c);

}

tecioctl(tp, flag, nvrow)
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
