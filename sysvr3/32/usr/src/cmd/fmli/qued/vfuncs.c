/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.qued:src/qued/vfuncs.c	1.2"

#include <stdio.h>
#include "wish.h"
#include "token.h"
#include "ctl.h"
#include "winp.h"
#include "fmacs.h"
#include "terror.h"
#include "vtdefs.h"

int Pagelines;
static int Pagechars;
#define LINEBYTES	(Cfld->cols)

scroll_up(num)
int num;
{
	register int i;
	register char *currptr;
	int saverow, savecol;

	Pagelines = num;
	if (Buffoffset == 0) {
		setarrows();
		return(FALSE);		/* top of file */
	}
	saverow = Cfld->currow;
	savecol = Cfld->curcol;
	if (Buffoffset < PAGEBYTES)	/* if a partial scroll */
		Pagelines = (int) (Buffoffset / LINESIZE);
	/*
	 * store bottom lines
	 */
	syncbuf(Buffoffset + FIELDBYTES - PAGEBYTES, LASTPAGE, LINESINFIELD - 1);
	Buffoffset -= PAGEBYTES;
	/*
	 * push displayed lines off the bottom
	 */
	fgo(0, 0);
	finsline(Pagelines, FALSE);
	/*
	 * put buffered text at the top
	 */
	currptr = Scrollbuf + Buffoffset;
	for (i = 0; i < Pagelines; i++, currptr += LINESIZE) {
		fgo(i, 0);
		fputstring(currptr);
	}
	if (Pagelines != 1 && (saverow = (saverow + Pagelines)) > Lastrow)
		saverow = Lastrow;
	fgo(saverow, savecol);
	setarrows();
	return(TRUE);
}

scroll_down(num)
int num;
{
	register int i;
	register unsigned fieldoffset;
	int saverow, savecol;

	Pagelines = num;

	if (Scrollbuf == NULL)		/* make sure scroll buffer exists */
		growbuf(FIELDBYTES);
	fieldoffset = Buffoffset + FIELDBYTES;
	if (fieldoffset >= Bufflast && Valptr == NULL) {
		setarrows();		/* at the end of the scroll buffer */
		return(FALSE);
	}

	saverow = Cfld->currow;
	savecol = Cfld->curcol;
	/*
	 * synchronize the scroll buffer with the window
	 */
	syncbuf(Buffoffset, 0, Pagelines - 1);
	Buffoffset += PAGEBYTES;
	if (Buffoffset + FIELDBYTES >= Buffsize)
		growbuf(Buffoffset + FIELDBYTES);   /* need more buffer space */

	/*
	 * delete displayed lines form the top of the field 
	 */
	fgo(0, 0);
	fdelline(Pagelines);

	/*
	 * put buffered text at the bottom 
	 */
	fgo(LASTPAGE, 0);
	for (i = 0; i < Pagelines; i++) {
		if (fieldoffset >= Bufflast) {	/* if end of scroll buffer */
			if (Valptr)  		/* see if more buffered text */
				Valptr = (char *) fputstring(Valptr);
			Bufflast = Buffoffset + FIELDBYTES;
			break;
		}
		fgo(LASTPAGE + i, 0);
		fputstring(Scrollbuf + fieldoffset);
		fieldoffset += LINESIZE;
	}

	if (Pagelines != 1 && (saverow = (saverow - Pagelines)) < 0)
		saverow = 0;
	fgo(saverow, savecol);
	setarrows();
	return(TRUE);
}

scroll_left(num)
int num;
{
	register int i, savecol;
	register char *currptr;

	Pagechars = num;
	if (Buffoffset == 0) {
		setarrows();
		return(FALSE);		/* top of file */
	}
	savecol = Cfld->curcol;
	if (Buffoffset < Pagechars)	/* if a partial scroll */
		Pagechars = Buffoffset;
	/*
	 * store line
	 */
	syncbuf(Buffoffset, 0, 0);
	Buffoffset -= Pagechars;
	/*
	 * shift visible window 
	 */
	fgo(0, 0);
	fclearline();
	fgo(0, 0);
	fputstring(Scrollbuf + Buffoffset);
	if ((savecol += num) > Lastcol)
		savecol = Lastcol;
	fgo(0, savecol);
	setarrows();
	return(TRUE);
}

scroll_right(num)
int num;
{
	register int i, savecol;
	register unsigned fieldoffset;

	Pagechars = num;
	fieldoffset = Buffoffset + LINEBYTES;
	if (fieldoffset >= Bufflast) {
		setarrows();
		return(FALSE);
	}

	savecol = Cfld->curcol;
	/*
	 * synchronize the scroll buffer with the window
	 */
	syncbuf(Buffoffset, 0, 0);
	Buffoffset += Pagechars;
	if (Buffoffset + LINEBYTES >= Buffsize)
		growbuf(Buffoffset + LINEBYTES);   /* need more buffer space */
	/*
	 * shift visible window ... 
	 */
	fgo(0, 0);
	fclearline();
	fgo(0, 0);
	fputstring(Scrollbuf + Buffoffset);
	if ((savecol = (savecol - num)) < 0)
		savecol = 0;
	fgo(0, savecol);
	setarrows();
	return(TRUE);
}
