/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.qued:src/qued/editvirt.c	1.2"

#include <stdio.h>
#include <ctype.h>
#include "wish.h"
#include "token.h"
#include "winp.h"
#include "fmacs.h"

static void fputline();
static void shift_buff();
static void freadline();
static void growbuf();

token
editvirtual(tok)
token tok;
{
	register token rettok;

	rettok = TOK_NOP;
	switch(tok) {
	case TOK_DL:
		Flags |= I_CHANGED;
		/* shift_buff(UP); */
		break;
	case TOK_IL:
		Flags |= I_CHANGED;
		/* shift_buff(DOWN); */
		break;
	case TOK_UP:
		if (scroll_up(1) == FALSE)
			beep();
		break;
	case TOK_DOWN:
	case TOK_RETURN:
		if (scroll_down(1) == FALSE)
			beep();
		break;
	case TOK_BACKSPACE:
		if (scroll_up(1) == FALSE)
			beep();
		else {
			fgo(Cfld->currow, Lastcol);
			fputchar(' ');
		}
		break;
	default:
		rettok = tok;
	}
	return(rettok);
}
