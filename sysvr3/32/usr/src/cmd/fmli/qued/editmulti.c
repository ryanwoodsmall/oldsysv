/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.qued:src/qued/editmulti.c	1.2"

#include	<ctype.h>
#include	"token.h"
#include	"winp.h"
#include	"fmacs.h"
#include	"wish.h"

editmulti(tok)
token tok;
{
	register token rettok;
	register int row, col;
	int wrapcol;

	row = Cfld->currow;
	col = Cfld->curcol;
	rettok = TOK_NOP;
	switch(tok) {
	case TOK_UP:
		if (row == 0)
			rettok = TOK_UP;
		else 
			row--;
		break;
	case TOK_DOWN:
		if (row == Lastrow)
			rettok = TOK_DOWN;
		else
			row++;
		break;
	case TOK_RETURN:
	case TOK_ENTER:
		if (row == Lastrow)
			rettok = TOK_RETURN;
		else {
			row++;
			col = 0;
		}
		break;
	case TOK_BACKSPACE:
		/* reposition cursor for further backspaces or erases */
		if (row == 0)
			rettok = tok;
		else {
			fgo(--row, col = Lastcol);
			fputchar(' ');
		}
		break;
	case TOK_WRAP:
		if (row == Lastrow)
			rettok = tok;
		else if (Flags & I_WRAP) {
			if ((wrapcol = do_wrap()) < 0) {
				wrapcol = 0;
				beep();
			}
			if (col == Lastcol) {	/* if cursor on last col */ 
				col = wrapcol;
				row++;
			}
		}
		else if (col == Lastcol) {
			row++;
			col = 0;
		}
		else
			beep();
		break;
	default:
		rettok = tok;
	}
	fgo(row, col);
	return(rettok);
}
