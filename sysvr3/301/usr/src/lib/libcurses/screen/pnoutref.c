/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/pnoutref.c	1.6"
/*
 * make the current screen look like "win" over the area covered by
 * win.
 *
 */

# include	"curses.ext"

/* Put out pad but don't actually update screen. */
pnoutrefresh(pad, pminrow, pmincol, sminrow, smincol, smaxrow, smaxcol)
register WINDOW	*pad;
int pminrow, pmincol, sminrow, smincol, smaxrow, smaxcol;
{
	register int copycnt, temp, pr, sr;
	register chtype	*nsp;
	register int fastupdate = 0;

	if (pad == NULL)
		return ERR;

	if (sminrow > smaxrow || smincol > smaxcol) return ERR;

	/* Make sure everything fits */
	if (pminrow < 0) pminrow = 0;
	if (pmincol < 0) pmincol = 0;
	if (sminrow < 0) sminrow = 0;
	if (smincol < 0) smincol = 0;
	if ((smaxrow + pad->_yoffset) >= (lines - SP->Ybelow)) 
		smaxrow = lines - SP->Ybelow - pad->_yoffset - 1;
	temp = pad->_maxy - 1 - pminrow;
	if ((smaxrow - sminrow) > temp)
		smaxrow = sminrow + temp;
	if (smaxcol >= columns)
		smaxcol = columns - 1;
	temp = pad->_maxx - 1 - pmincol;
	if ((smaxcol - smincol) > temp)
		smaxcol = smincol + temp;

	if (pminrow == pad->_pminrow && pmincol == pad->_pmincol &&
	    sminrow == pad->_sminrow && smincol == pad->_smincol &&
	    smaxrow == pad->_smaxrow && smaxcol == pad->_smaxcol)
		fastupdate++;
	else {
		pad->_pminrow = pminrow;
		pad->_pmincol = pmincol;
		pad->_sminrow = sminrow;
		pad->_smincol = smincol;
		pad->_smaxrow = smaxrow;
		pad->_smaxcol = smaxcol;
		pad->_flags |= _WINCHANGED;
	}
# ifdef DEBUG
	if(outf) fprintf(outf, "PREFRESH(pad %x, pcorner %d,%d, smin %d,%d, smax %d,%d)", pad, pminrow, pmincol, sminrow, smincol, smaxrow, smaxcol);
	_dumpwin(pad);
	if(outf) fprintf(outf, "PREFRESH:\n\tfirstch\tlastch\n");
#endif /* DEBUG */

	/* Copy it out, like a refresh, but appropriately offset */
	if (pad->_flags & _WINCHANGED) {
		smaxrow += pad->_yoffset;
		for (pr=pminrow,sr=sminrow+pad->_yoffset; sr <= smaxrow; sr++,pr++)
			if (!fastupdate || pad->_firstch[pr] != _NOCHANGE) {
				nsp = &pad->_y[pr][pmincol];
				_ll_move(sr, smincol);
				copycnt = pad->_maxx - pmincol;
				temp = columns - SP->virt_x /* + 1 */;
				if (temp < copycnt) copycnt = temp;
				temp = smaxcol - smincol + 1;
				if (temp < copycnt) copycnt = temp;
				if (copycnt > 0)
					memcpy ((char *)SP->curptr,
						(char *)nsp,
						copycnt*sizeof(*nsp));
				SP->curptr += copycnt;
				SP->virt_x += copycnt;
				pad->_firstch[pr] = _NOCHANGE;
			}
		pad->_flags &= ~_WINCHANGED;
	}
	pad->_flags &= ~_WINMOVED;

	SP->leave_lwin = pad;
	SP->leave_use_idl |= pad->_use_idl==2 ? pad->_need_idl : pad->_use_idl;
	if (pad->_leave ||
	    ((SP->leave_x = pad->_curx - pmincol + smincol) < 0) ||
	    (SP->leave_x >= columns) ||
	    ((SP->leave_y = pad->_cury - pminrow + sminrow + pad->_yoffset) < 0) ||
	    (SP->leave_y >= lines))
		SP->leave_leave = TRUE;
	return OK;
}
