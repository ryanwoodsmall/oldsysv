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
/* @(#)vt100.c	6.1 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/tty.h"
#include "sys/termio.h"
#include "sys/crtctl.h"


struct CRTLANG {
	char	code;
	char	inchar;
	char	outchar;
	char	tpindx;
} mapvt100[] = {
	LCA,	0,	0,	2,
	CUP,	'A',	'A',	0,
	CDN,	'B',	'B',	0,
	CRI,	'C',	'C',	0,
	CLE,	'D',	'D',	0,
	HOME,	'H',	'H',	0,
	STB,	0,	'5',	3,
	SPB,	0,	'0',	4,
	DVID,	0,	0,	015,
	CS,	0,	0,	5,
	CM,	0,	'c',	013,
	IL,	0200|'S',	0,	012,
	DL,	0200|'R',	0,	6,
	UVSCN,	0,	0,	7,
	DVSCN,	0,	0,	010,
	EEOL,	0,	'K',	0,
	EEOP,	0,	'J',	0,
	SVSCN,	0,	0,	011,
	CUP,	0200|'P',	0,	0,
	CDN,	0200|'Q',	0,	0,
	0
};

/*
 *	table of internal sequences for commands
 *	  the following codes are used to construct
 *	  these sequences:
 *		0200	output character from map table
 *		0201	output ESC then map table character
 *		0202	output ESC BRAC then map table char
 *		0203	output t_row in proper form
 *		0204	output t_lrow (last row)
 *		0205	output t_vrow (variable home row)
 *		0206	output t_col (column)
 *		0207	insert delay if required at this speed
 *		0210	process video attributes
 */
char *indx_vt100[] = {
	/*  0: normal ESC BRAC char sequence */
	"\202",
	/*  1: ESC char sequence */
	"\201",
	/*  2: load cursor address */
	"\202\203;\206H",
	/*  3: start video attribute sequence */
	" \202m",
	/*  4: end video attribute sequence */
	"\202m ",
	/*  5: clear screen sequence */
	"\202H\202J",
	/*  6: delete line  */
	"\314\307\311",
	/*  7: scroll bottom of screen up */
	"\202\204H\012",
	/*  010: scroll screen down from variable home */
	"\202\205H\201M",
	/*  011: define scrolling boundaries of screen */
	"\202\205r\302",
	/* 012: insert line	*/
	"\314\201M\311",
	/* 013: CRT reset with timing	*/
	"\201\207",
	/* 014: Temporary Set Scrolling Region to Current Line */
	"\202\203r\302",
	/* 015: Define Video Attributes */
	"\202m\202\210m"
};

#define BRAC	'['
#define BRAC_BIT	0100
#define OH	'O'
#define OH_BIT		040
#define SEMI	';'

#define TDELAY	0377

char vid_chars[6] = { '4', '5', '7', 0, '1', 0 };

vt100output(c, tp)
register c;
register struct tty *tp;
{
	register struct CRTLANG *cp;
	for (cp=mapvt100; cp->code; cp++)
		if (c == cp->code) {
			vttrans(tp, cp->tpindx, cp->outchar);
			return;
		}
}
vttrans(tp, c, aoc)
struct tty *tp;
register c;
char  aoc;
{
	register char *str;
	register struct clist *qp;
	char *ap;

	qp = &tp->t_outq;

	for(str = indx_vt100[c]; *str; str++) {
		c = *str;
		if (c&0200) {
			if (c&0100) {
				vttrans(tp, c&077, 0);
				continue;
			} else {
				switch (c&077) {
				case 0:
					c = aoc;
					break;
				case 1:
					putc(ESC, qp);
					c = aoc;
					break;
				case 2:
					putc(ESC, qp);
					qputc(BRAC, qp);
					c = aoc;
					break;
				case 3:
					cvtdec(tp->t_row, qp);
					continue;
				case 4:
					cvtdec(tp->t_lrow, qp);
					continue;
				case 5:
					cvtdec(tp->t_vrow, qp);
					continue;
				case 6:
					cvtdec(tp->t_col, qp);
					continue;
				case 7:
					putc(QESC, qp);
					c = TDELAY;
					qputc(c, qp);
					putc(QESC, qp);
					break;
				case 010:
					c = tp->t_dstat&077;
					ap = vid_chars;
					while(c) {
						if ((c&01) && *ap) {
							putc(SEMI, qp);
							putc(*ap, qp);
						}
						ap++;
						c >>= 1;
					}
					continue;
				}
			}
		}
		if (c) {
			qputc(c, qp);
		}
	}
}
vt100input(c, tp)
register struct tty *tp;
register c;
{
	register struct CRTLANG *cp;

	if (tp->t_tmflag&BRAC_BIT) {
		if (tp->t_tmflag&OH_BIT)
			c |= 0200;
		tp->t_tmflag &= ~(BRAC_BIT | OH_BIT);
		for (cp=mapvt100; cp->code; cp++)
			if ((c&0377) == (cp->inchar &0377))
				return(cp->code | CPRES);
		c &= 0177;
	} else { if (tp->t_tmflag&TERM_BIT) {
			tp->t_tmflag &= ~TERM_BIT;
			if (c == BRAC) {
				tp->t_tmflag |= BRAC_BIT;
				return(-1);
			}
			if (c == OH) {
				tp->t_tmflag |= BRAC_BIT | OH_BIT;
				return(-1);
			}
		} else if (c == ESC) {
			tp->t_tmflag |= TERM_BIT;
			return(-1);
		}
	}
	return(c);
}

vt100ioctl(tp, flag, nvrow)
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
	tp->t_vrow = nvrow;
	vt100output(SVSCN, tp);
	(*tp->t_proc)(tp,T_OUTPUT);
}

cvtdec(value, aqp)
int value;
struct clist *aqp;
{
	register i;
	register struct clist *qp;

	if (value == 0)
		return;
	qp = aqp;
	i = value + 1;
	if (i>9)
		putc(i/10 + '0', qp);
	putc(i%10 + '0', qp);
}
