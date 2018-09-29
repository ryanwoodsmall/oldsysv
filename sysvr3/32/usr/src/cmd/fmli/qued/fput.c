/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.qued:src/qued/fput.c	1.5"

#include <stdio.h>
#include "wish.h"
#include "token.h"
#include "winp.h"
#include "fmacs.h"
#include "attrs.h"

/*
 * FPUTSTRING will return NULL if the entire string fits in the field
 * otherwise it returns a pointer to the beginning of the substring that
 * does not fit
 */
char *
fputstring(str)
char *str;
{
	register char *sptr;
	register int row, col, done; 
	int i, numspaces, pos;

	col = Cfld->curcol;
	row = Cfld->currow;
	done = FALSE;
	sptr = str;
	while (!done) {
		if (*sptr == '\\') {
			switch(*(++sptr)) {
			case 'b':
				*sptr = '\b';
				break;
			case 'n':
				*sptr = '\n';
				break;
			case 't':
				*sptr = '\t';
				break;
			case 'r':
				*sptr = '\r';
				break;
			}
		}
		switch(*sptr) {
		case '\n':
			fgo(row, col);
			fclearline();
			if (row == Lastrow)
				done = TRUE;
			else
				fgo(++row, col = 0);
			sptr++;
			break;
		case '\b':
			if (col != 0)
				fgo(row, --col);
			sptr++;
			break;
		case '\t':
			numspaces = ((col + 8) & ~7) - col;
			for (i = 0; i < numspaces && col <= Lastcol; i++, col++)
				fputchar(' ');
			sptr++;
			break;
		case '\0':
			done = TRUE;
			sptr = NULL;
			continue;
		default:
			fputchar(*sptr++);
			col++;
			break;
		}
		if (col > Lastcol) {
			if (row == Lastrow) {
				if ((Flags & I_SCROLL) && (Flags & I_WRAP)) {
					/*
					 * If the word is not longer then
					 * the length of the field then
					 * clear away the word to wrap...
					 * and adjust the string pointer to
					 * "unput" the word ....
					 */
					pos = prev_bndry(row, ' ', TRUE);
					if (pos >= 0) {
						fgo(row, pos);
						fclearline();
						sptr -= (Lastcol - pos);
					}
				}
				else
					sptr++;
				done = TRUE;
			}
			else if ((Flags & I_WRAP) && (wrap() == TRUE)) {
				if ((col = do_wrap()) < 0)
					col = 0;
				fgo(++row, col);
			}
			else {
				if (*sptr != '\n')
					fgo(++row, col = 0);
			}
		}
	}
	Cfld->curcol = col;
	Cfld->currow = row;
	return(sptr);
}
