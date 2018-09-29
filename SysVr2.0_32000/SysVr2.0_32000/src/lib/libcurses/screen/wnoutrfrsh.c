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
/*
 * make the current screen look like "win" over the area covered by
 * win.
 *
 * 7/9/81 (Berkeley) @(#)refresh.c	1.6
 */

/*	@(#) wnoutrfrsh.c: 1.1 10/15/83	(1.14	3/6/83)	*/
#include	"curses.ext"

extern	WINDOW *lwin;

/* Put out window but don't actually update screen. */
wnoutrefresh(win)
register WINDOW	*win;
{
	register int wy, y;
	register chtype	*nsp, *lch;

# ifdef DEBUG
	if( win == stdscr )
	{
		if(outf) fprintf(outf, "REFRESH(stdscr %x)", win);
	}
	else
	{
		if( win == curscr )
		{
			if(outf) fprintf(outf, "REFRESH(curscr %x)", win);
		}
		else
		{
			if(outf) fprintf(outf, "REFRESH(%d)", win);
		}
	}
	if(outf) fprintf(outf, " (win == curscr) = %d, maxy %d\n", win, (win == curscr), win->_maxy);
	if( win != curscr )
	{
		_dumpwin( win );
	}
	if(outf) fprintf(outf, "REFRESH:\n\tfirstch\tlastch\n");
# endif	DEBUG
	/*
	 * initialize loop parameters
	 */

	if( win->_clear || win == curscr || SP->doclear )
	{
# ifdef DEBUG
		if (outf) fprintf(outf, "refresh clears, win->_clear %d, curscr %d\n", win->_clear, win == curscr);
# endif	DEBUG
		SP->doclear = 1;
		win->_clear = FALSE;
		if( win != curscr )
		{
			touchwin( win );
		}
	}

	if( win == curscr )
	{
#ifdef	DEBUG
	if(outf) fprintf(outf, "Calling _ll_refresh(FALSE)\n" );
#endif	DEBUG
		_ll_refresh(FALSE);
		return OK;
	}
#ifdef	DEBUG
	if(outf) fprintf(outf, "Didn't do _ll_refresh(FALSE)\n" );
#endif	DEBUG

	for( wy = 0; wy < win->_maxy; wy++ )
	{
		if( win->_firstch[wy] != _NOCHANGE )
		{
			y = wy + win->_begy;
			lch = &win->_y[wy][win->_maxx-1];
			nsp = &win->_y[wy][0];
			_ll_move(y, win->_begx);
			while( nsp <= lch )
			{
				if( SP->virt_x++ < columns )
				{
					*SP->curptr++ = *nsp++;
				}
				else
				{
					break;
				}
			}
			win->_firstch[wy] = _NOCHANGE;
		}
	}
	lwin = win;
	return OK;
}
