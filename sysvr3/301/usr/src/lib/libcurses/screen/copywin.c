/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/copywin.c	1.6.1.1"
/*
    This routine writes parts of Srcwin onto Dstwin,
    either non-destructively or destructively.
*/

#include	"curses.ext"
#include	<ctype.h>

int
copywin(Srcwin, Dstwin,
	minRowSrc, minColSrc, minRowDst, minColDst, maxRowDst, maxColDst,
	overlay)
register WINDOW *Srcwin, *Dstwin;
int minRowSrc, minColSrc, minRowDst, minColDst, maxRowDst, maxColDst;
int overlay;
{
    register int numcopied;
    register chtype *spSrc, *spDst;
    register int ySrc, yDst, width_bytes, numblanks;
    register int maxColSrc, diffy, maxRowSrc;
    chtype dstattrs, blank;
    int diffx, width, height;
    chtype **_yDst, **_ySrc;
    int usingcurscr = 0;

# ifdef DEBUG
    if (outf)
	fprintf (outf, "COPYWIN(%0.2o, %0.2o);\n", Srcwin, Dstwin);
# endif

    if (!Srcwin || !Dstwin)
	return ERR;

    diffy = maxRowDst - minRowDst;
    diffx = maxColDst - minColDst;
    maxColSrc = minColSrc + diffx;
    maxRowSrc =	minRowSrc + diffy;
    width = diffx + 1;
    height = diffy + 1;

    /* are selected pieces within the windows? */
    if (Srcwin == curscr)
	{
	minRowSrc += Dstwin->_yoffset;
	maxRowSrc += Dstwin->_yoffset;
	usingcurscr++;
	}

    /* check error conditions */
    if (maxRowSrc >= Srcwin->_maxy ||
	maxColSrc >= Srcwin->_maxx ||
	maxRowDst >= Dstwin->_maxy ||
	maxColDst >= Dstwin->_maxx ||
	minRowSrc < 0 ||
	minRowDst < 0 ||
	minColSrc < 0 ||
	minColDst < 0 ||
	height < 1 ||
	width < 1 ||
	Dstwin == curscr)
	return ERR;

    dstattrs = Dstwin->_attrs;
    blank = ' ' | dstattrs;

    if (usingcurscr)
	minRowSrc++;			/* std_body[] is 1 based */
    else
	{
	_ySrc = Srcwin->_y;
	if (!overlay)
	    width_bytes = width * sizeof (chtype);
	}

    _yDst = Dstwin->_y;

    /* for each Row */
    for (ySrc = minRowSrc, yDst = minRowDst;
	    height-- > 0;
	    ySrc++, yDst++)
	{
	/* copy all Cols in Row */
	if (usingcurscr)
	    {
	    /* Only the characters from 0 to lineptr->length */
	    /* are valid. Characters beyond that are blank. */
	    /* If there is no line, then it is blank. */
	    struct line *lineptr = SP->std_body[ySrc];
	    int length;
	    if (lineptr)	length = lineptr->length;
	    else		length = 0;
	    numcopied = length - minColSrc;
	    spDst = &_yDst[yDst][minColDst];
	    /* copy portion that is valid */
	    if (numcopied > 0)
		{
		if (numcopied > width)
			numcopied = width;
		numblanks = width - numcopied;
		spSrc = &SP->std_body[ySrc]->body[minColSrc];
		if (overlay)
		    if (dstattrs)
			for ( ; numcopied-- > 0; spDst++, spSrc++)
			    {
			    /* ... copy non-blanks */
			    if (*spSrc != ' ')
				*spDst = *spSrc | dstattrs;
			    }
		    else
			for ( ; numcopied-- > 0; spDst++, spSrc++)
			    {
			    /* ... copy non-blanks */
			    if (*spSrc != ' ')
				*spDst = *spSrc;
			    }
		else
		    if (dstattrs)
			for ( ; numcopied-- > 0; spDst++, spSrc++)
			    *spDst = *spSrc | dstattrs;
		    else
		        {
			/* ... copy all chtypes */
			(void) memcpy ((char *) spDst, (char *) spSrc,
				       numcopied*(int)sizeof(chtype));
			spDst += numcopied;
			}
		}
	    else
	        numblanks = width;
	    /* blank out the rest */
	    if (numblanks > 0 && !overlay)
		{
		(void) memSset (spDst, blank, numblanks);
		}
	    }
	else
	    {
	    spSrc = &_ySrc[ySrc][minColSrc];
	    spDst = &_yDst[yDst][minColDst];
	    /* for each Col in Row ... */
	    if (overlay)
		if (dstattrs)
		    for (numcopied = width; numcopied-- > 0; spSrc++, spDst++)
			{
			/* ... copy non-blanks */
			if (*spSrc != ' ')
			    *spDst = *spSrc | dstattrs;
			}
		else
		    for (numcopied = width; numcopied-- > 0; spSrc++, spDst++)
			{
			/* ... copy non-blanks */
			if (*spSrc != ' ')
			    *spDst = *spSrc;
			}
	    else /* destructive */
	        if (dstattrs)
		    for (numcopied = width; numcopied-- > 0; spSrc++, spDst++)
			*spDst = *spSrc | dstattrs;
		else
		    /* ... copy all chtype's */
		    (void) memcpy ((char *) spDst, (char *) spSrc,
		        width_bytes);
	    }

	/* note that the line has changed */
	if (Dstwin->_firstch[yDst] == _NOCHANGE)
	    {
	    Dstwin->_firstch[yDst] = minColDst;
	    Dstwin->_lastch[yDst] = maxColDst;
	    }
	else
	    {
	    if (minColDst < Dstwin->_firstch[yDst])
		Dstwin->_firstch[yDst] = minColDst;
	    if (minColDst > Dstwin->_lastch[yDst])
		Dstwin->_lastch[yDst] = maxColDst;
	    }
	}

    /* note that something in Dstwin has changed */
    Dstwin->_flags |= _WINCHANGED;

    return OK;
}
