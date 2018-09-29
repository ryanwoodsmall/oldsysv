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
/* @(#)hp45.c	6.1 */
/*
 * Hewlett Packard Model HP 2645A Display Station Interface
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/tty.h"
#include "sys/termio.h"
#include "sys/crtctl.h"


char maphp45[] = {
	0341,	LCA,
	0101,	CUP,
	0102,	CDN,
	0103,	CRI,
	0104,	CLE,
	0150,	HOME,
	0344,	STB,	/* STB and SPB included to accomodate old code.	*/
	0344,	SPB,
	0344,	DVID,	/* Use of DVID preferred.	*/
	0150,	CS,
	0114,	IL,
	0115,	DL,
	0121,	IC,
	0120,	DC,
	0113,	EEOL,
	0112,	EEOP,
	0123,	USCRL,
	0124,	DSCRL,
	0160,	HOME,
	0161,	EEOP,
	0162,	USCRL,
	0163,	DSCRL,
	0164,	DL,		/* kludge for sccs browser. should be CLE */
	0165,	IL,		/* kludge for sccs browser. should be CRI */
	0166,	CUP,
	0167,	CDN,
	0
};

char hp45str[] = {' ', ESC, 'R', 010, 0};
char hp_vid_chars[6] = {
	'D',	/* Underline	*/
	'A',	/* Blink	*/
	'B',	/* Reverse vid.	*/
	'H',	/* Dim video	*/
	'B',	/* Change bold to Reverse vid. */
	'H'	/* Dim for Off	*/
};

hp45output(ac, atp)
struct tty *atp;
{
	register struct clist *qp;
	register char *cp, *ap, vid_attr;
	register c;

	c = ac;
	qp = &atp->t_outq;

	if (c == DVSCN) {
		ttdvscn(atp);
		return;
	}
	if (c == UVSCN) {
		ttuvscn(atp);
		return;
	}
	for(cp=maphp45; *cp++;)
	if (c == *cp++) {
		putc(ESC, qp);
		if ((c=cp[-2]) < 0)
			putc('&', qp);
		qputc(c&0177, qp);
		switch(ac) {
			case LCA:
				c = atp->t_row;
				qputc(c/10 + '0', qp);
				qputc(c%10 + '0', qp);
				qputc('r', qp);
				c = atp->t_col;
				qputc(c/10 + '0', qp);
				qputc(c%10 + '0', qp);
				qputc('C', qp);
				break;

			case IC:
				for(cp=hp45str; *cp; cp++)
					putc(*cp, qp);
				break;

			case STB:		/* Should use DVID	*/
				putc(001,qp);
				putc(' ',qp);
				break;

			case SPB:		/* Should use DVID	*/
				putc(000,qp);
				putc(' ',qp);
				break;

			case DVID:
				c = atp->t_dstat & 077;
				ap = hp_vid_chars;
				for(vid_attr='@'; c; ap++, c >>= 1)
				    if (c & 01) vid_attr |= *ap;
				putc(vid_attr, qp);
				continue;

			case CS:
				putc(ESC, qp);
				putc(0112, qp);
				break;
		}

		break;
	}
}

hp45input(ac, atp)
struct tty *atp;
{
	register struct tty *tp;
	register char *cp;
	register c;

	c = ac;
	tp = atp;

	if (tp->t_tmflag&TERM_BIT) {
		tp->t_tmflag &= ~TERM_BIT;
		for (cp=maphp45; *cp++;)
			if (c == (cp++)[-1]) {
				c = cp[-1];
				if (tp->t_tmflag&TERM_CTLECHO) {
					ttyctl(c, tp);
					if (tp->t_tmflag&TERM_INVIS) {
						tp->t_state &= ~TTSTOP;
						(*tp->t_proc)(tp,T_OUTPUT);
					}
				}
				if (tp->t_tmflag&TERM_INVIS) {
					return(-1);
				}
				return(c|CPRES);
			}
	} else if (c == ESC) {
		tp->t_tmflag |= TERM_BIT;
		return(-1);
	}
	return(c);
}

hp45ioctl(tp, flag, nvrow)
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
