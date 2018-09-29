/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.qued:src/qued/scrollbuf.c	1.3"

#include <stdio.h>
#include "wish.h"
#include "token.h"
#include "winp.h"
#include "fmacs.h"
#include "terror.h"

/*
 * GROWBUF will increase the scroll buffer by size 
 */
void 
growbuf(size)
int size;
{
	unsigned oldbuffsize;

	oldbuffsize = Buffsize;
	if (Scrollbuf == NULL) {
		if ((Scrollbuf = malloc(size)) == NULL)
			fatal(NOMEM, "");
		Buffoffset = 0;
		Bufflast = size;
	}	
	else if (Buffsize != size) {
		if ((Scrollbuf = realloc(Scrollbuf, size)) == NULL)
			fatal(NOMEM, "");
	}
	if ((Buffsize = size)  > oldbuffsize)	/* initialize new block */
		memset(Scrollbuf + oldbuffsize, 0, Buffsize - oldbuffsize);
}

free_scroll_buf(fld)
ifield *fld;
{
	if (fld->scrollbuf)
		free(fld->scrollbuf);
	fld->scrollbuf = NULL;
	fld->buffsize = 0;
	fld->buffoffset = 0;
	fld->bufflast = 0;
}

/*
 * SYNCBUF will syncronize the window map with the scroll buffer
 */
void 
syncbuf(offset, start, end)
unsigned offset;
int start;
int end;
{
	register char *currptr;
	register int i;
	int saverow, savecol;

	saverow = Cfld->currow;
	savecol = Cfld->curcol;
	if (Scrollbuf == NULL)
		growbuf(FIELDBYTES);	    /* initialize scroll buffer */
	currptr = Scrollbuf + offset;
	if (Currtype == SINGLE) {
		if ((offset + Cfld->cols) >= Bufflast)
			Bufflast = offset + freadline(0, currptr, TRUE);
		else
			freadline(0, currptr, FALSE);
	}
	else {
		for (i = start; i <= end; i++, currptr += LINESIZE)
			freadline(i, currptr, TRUE);
	}
	fgo(saverow, savecol);
}

/*
 * CLEARBUF will clear the scroll buffer
 */
void
clearbuf()
{
	/*
	 * clear scroll buffer from current cursor position to end of field
	 */
	syncbuf(Buffoffset, 0, LINESINFIELD - 1);
	Bufflast = Buffoffset + (Currtype == SINGLE ? Cfld->cols : FIELDBYTES);
	growbuf(Bufflast);	    /* actually shrinks scroll buffer */
	setarrows();
}

/*
 * SHIFTBUF will shift the scroll buffer in the specified direction
 */
void 
shiftbuf(direction)
int direction;
{
	register char *sptr, *dptr;
	int startcol, startrow;
	int saverow, savecol;

	saverow = Cfld->currow;
	savecol = Cfld->curcol;
	if (Scrollbuf == NULL)
		growbuf(FIELDBYTES);
	switch(direction) {
	case UP:
		if ((startrow = Buffoffset + FIELDBYTES) >= Bufflast) {
			/* end of scroll buffer, see if there is more text */
			if (Valptr) {
				fgo(Lastrow, 0);
				Valptr = (char *) fputstring(Valptr);
			}
		}
		else {
			Bufflast -= LINESIZE;
			sptr = Scrollbuf + startrow;
			fgo(Lastrow, 0);
			fputstring(sptr);
			memshift(sptr, sptr + LINESIZE, Bufflast - startrow);
		}
		break;
	case DOWN:
		startrow = Buffoffset + FIELDBYTES;
		if ((Bufflast + LINESIZE) > Buffsize)
			growbuf(Buffsize + FIELDBYTES);
		sptr = Scrollbuf + startrow;
		memshift(sptr + LINESIZE, sptr, Bufflast - startrow);
		Bufflast += LINESIZE;
		freadline(Lastrow, sptr, TRUE);
		break;
	case LEFT:
		if ((startcol = Buffoffset + Cfld->cols) >= Bufflast)
			return;		/* fits on screen */
		sptr = Scrollbuf + startcol; 
		fgo(0, Lastcol);
		fputchar(*sptr);		/* print character shifted in */
		memshift(sptr, sptr + 1, Bufflast - startcol);
		Bufflast--;
		break;
	case RIGHT:
		startcol = Buffoffset + Cfld->cols;
		if (++Bufflast >= Buffsize)
			growbuf(Buffsize + Cfld->cols);
		sptr = Scrollbuf + startcol; 
		memshift(sptr + 1, sptr, Bufflast - startcol);
		*sptr = freadchar(0, Lastcol);	     /* character shifted out */
	}
	setarrows();
	fgo(saverow, savecol);
}
