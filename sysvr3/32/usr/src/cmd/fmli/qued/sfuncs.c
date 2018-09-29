/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.qued:src/qued/sfuncs.c	1.2"

#include <stdio.h>
#include "wish.h"
#include "vtdefs.h"
#include "token.h"
#include "winp.h"
#include "fmacs.h"
#include "attrs.h"

fdelchar()
{
	int saverow, savecol;

	saverow = Cfld->currow;
	savecol = Cfld->curcol;
	wdelchar();
	/*
	 * go to last column and insert a blank
	 */
	fgo(saverow, Lastcol);
	winschar(' ', Fieldattr);
	fgo(saverow, savecol);
}

finsstr(buff)
char *buff;
{
	register char *bptr;

	for (bptr = buff; *bptr != '\0'; bptr++)
		;
	bptr--;
	while (bptr >= buff)
		finschar(*bptr--);
}

finschar(c)
char c;
{
	int saverow, savecol;

	saverow = Cfld->currow;
	savecol = Cfld->curcol;
	/* 
	 * delete last character, re-position cursor and insert
	 * a character
	 */
	fgo(saverow, Lastcol);
	wdelchar();
	fgo(saverow, savecol);
	winschar(c, Fieldattr);
}
