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
/* @(#)ds40.c	6.1 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/tty.h"
#include "sys/termio.h"
#include "sys/crtctl.h"

#define	SDESC	0200		/* output escape flag */
#define	NONE	0177		/* no direct conversion flag */

char ds40map[] = {
	LCA,	NONE,		/* Load Cursor Adress (simulated) */
	CUP,	SDESC|'7',	/* Move Cursor Up */
	CDN,	SDESC|'B',	/* Move Cursor Down */
	CRI,	SDESC|'C',	/* Move Cursor Right */
	CLE,	010,		/* Move Cursor Left */
	NL,	NONE,		/* Special DS40 newline */
	CRTN,	SDESC|'G',	/* Return Cursor to start of line */
	HOME,	SDESC|'H',	/* Home Cursor */
	CS,	SDESC|'R',	/* Clear Screen */
	EEOP,	SDESC|'J',	/* Erase to end of page */
	EEOL,	NONE,		/* Erase to end of line */
	IL,	SDESC|'L',	/* Insert Line */
	DL,	SDESC|'M',	/* Delete Line */
	IC,	SDESC|'^',	/* Insert Char */
	DC,	SDESC|'P',	/* Delete Char */
	STB,	SDESC|'3',	/* Start Blink */
	SPB,	SDESC|'4',	/* Stop Blink */
	BPRT,	SDESC|'W',	/* Begin Protect */
	EPRT,	SDESC|'X',	/* End Protect */
	STAB,	SDESC|'0',	/* Set Single Tab */
	ATAB,	SDESC|'1',	/* Set Column of Tabs */
	CTAB,	SDESC|'2',	/* Clear Tabs */
	USCRL,	SDESC|'S',	/* Scroll Up */
	DSCRL,	SDESC|'T',	/* Scroll Down */
	ASEG,	SDESC|'U',	/* Advance Segment */
/*	KBL,	NONE,		/* Keyboard Lock */
/*	KBU,	NONE,		/* Keyboard Unlock */
	UVSCN,	NONE,
	DVSCN,	NONE,
	0,
};

extern int rowpres,colpres;
/*.s'ds40output'Translate control into ds40 equivalent'*/
ds40output(ac, tp)
struct tty *tp;
{
	register struct clist *qp;
	register char *cp;
	register int c;

	c = ac;
	qp = &tp->t_outq;
	for(cp=ds40map;*cp;cp+=2) {
		if (c == *cp) {
			/*
			 * If mapping given, output hardware control sequence.
			 */
			if (*++cp != NONE) {
				if (*cp&SDESC)
					putc(ESC,qp);
				qputc(*cp&0177,qp);
			}
			/*
			 * Add in control timing delays, or simulate
			 * functions not in hardware.
			 */
			switch(c) {

			case DL: case DSCRL: case DC:
			case IL: case USCRL: case IC:
				putc(QESC,qp);
				putc(0213,qp);
				break;

			case NL:
				putc(ESC,qp);
				putc('G',qp);
				putc(ESC,qp);
				putc('B',qp);
				break;

			case CS:
				putc(QESC,qp);
				putc(0213,qp);

			case HOME:
				tp->t_tmflag |= TERM_BIT;
				break;

			case EEOL:
				for(c=0;c<80;c++)
					ds40output(DC,tp);
				break;

			case EEOP:
				putc(QESC,qp);
				putc(0216,qp);
				break;

			case CTAB:
				putc(QESC,qp);
				putc(0212,qp);
				break;

			case SPB:
			case STB:
				tp->t_col--;
				break;

			case LCA:
				if ((tp->t_tmflag&TERM_BIT) == 0) {
					ds40output(HOME,tp);
					rowpres = colpres = 0;
				}
				ds40lca(tp->t_row,tp->t_col,tp);
				break;
			case UVSCN:
				ttuvscn(tp);
				break;
			case DVSCN:
				ttdvscn(tp);
				break;

			}
		}
	}
}
/*.s'ds40lca'Simulate LCA'*/
ds40lca(row, col, tp)
struct tty *tp;
{
	register int rowmoves,colmoves,aseg;
	int rowdir,coldir,x;

	/*
	 * Compute the number of rows and columns to move relative
	 * to the current position of the cursor.
	 */

	rowmoves = row - rowpres%(tp->t_lrow+1);
	if (rowmoves < 0) {
		rowdir = CUP;
		rowmoves = -rowmoves;
	} else
		rowdir = CDN;

	colmoves = col - ((colpres > 79) ? 79: colpres);
	if (colmoves < 0) {
		coldir = CLE;
		colmoves = -colmoves;
	} else
		coldir = CRI;
	if (colmoves > (col-1)) {
		colmoves = col;
		coldir = CRTN;
	}

	/*
	 * Compute the number of moves if the cursor is homed.
	 */
	x = 1 + row/24 + row%24 + col;	/* "home" move sequence */

	if (x < rowmoves+colmoves+1) {
		rowdir = HOME;
		rowmoves = row%24;
		coldir = CRI;
		colmoves = col;
		aseg = row/24;
	} else
		aseg = 0;

	if (rowdir == HOME) {
		ds40output(HOME,tp);
		rowdir = CDN;
	}
	while(rowmoves--) ds40output(rowdir,tp);
	while(aseg--) ds40output(ASEG,tp);
	if (coldir == CRTN) {
		ds40output(CRTN,tp);
		coldir = CRI;
	}
	while(colmoves--) ds40output(coldir,tp);
}
/*.s'ds40input'Input character mapping'*/
#define	GS	035
#define	EOT	004
#define	CR	015

ds40input(ac, tp)
struct tty *tp;
{
	register int c;

	c = ac&0177;
	if (c == GS)
		c = EOT;

	if (c == CR)
		ttyoutput('\n',tp);

	return(c);

}


/*.t'ds40ioctl'Dummy routine for ioctl'*/
ds40ioctl(tp, flag, nvrow)
register struct tty *tp;
register unsigned nvrow;
{

	if (nvrow > 71) {
		u.u_error = EINVAL;
		return;
	}
	if (flag == LDOPEN) {
		tp->t_lrow = 71;
		tp->t_tmflag = ANL;
		tp->t_iflag = (tp->t_iflag&(IGNBRK|BRKINT|IGNPAR|PARMRK|INPCK))
			|ISTRIP|ICRNL;
			/* echo should be off for half duplex terminal */
		tp->t_lflag = (tp->t_lflag & (ECHO|ECHOE|ECHOK))|ISIG|ICANON;
		tp->t_oflag = OPOST|ONLCR;
	}
}
