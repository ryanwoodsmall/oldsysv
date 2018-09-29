/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/slk_set.c	1.2"
/*
 * Set a soft label, left or right justified or centered.
 */

#include "curses.ext"

#define LEFT	0
#define CENTER	1
#define RIGHT	2

extern char *strncpy();

slk_set(slknum, label, format)
register int slknum;
char *label;
register int format;
{
    register struct slkdata *SLK = SP->slk;
    register int len, left, total;
    register char *label2;
    char *blanks = "        ";

    if (format < 0 || format > 2 || slknum < 1 || slknum > 8 || !SLK)
	return ERR;

    /* numbers are one based, but stored zero based */
    slknum--;

    if (label == NULL)
	{
	SLK->nblabel[slknum][0] = '\0';
	(void) strncpy(SLK->label[slknum], blanks, SLK->len);
	SLK->label[slknum][SLK->len] = '\0';
	}
    else
	{
	/* strip label of leading blanks */
	while (*label && *label == ' ')
	    label++;
	label2 = label;

	/* strip label of control and other no-good characters */
	while (*label2)
	    if ((*label2 < ' ') || (*label2 >= 0177))
		*label2++ = '-';
	    else
		label2++;

	/* strip label of trailing blanks */
	while ((*(label2-1) == ' ') && (label2 > label))
	    label2--;
	len = label2 - label;

	/* trim too long labels down to size */
	if (len > SLK->len)
	    {
	    len = SLK->len;
	    label[len] = '\0';
	    }

	/* store no-blank version of label */
	strcpy(SLK->nblabel[slknum], label);

	/* store justified version of label */
	if (len == 0)
	    (void) strncpy(SLK->label[slknum], blanks, SLK->len);
	else if (len >= SLK->len)
	    (void) strcpy (SLK->label[slknum], label);
	else
	    {
	    left = (format == LEFT)	?	0 :
		   (format == CENTER)	?	(SLK->len - len) / 2 :
						SLK->len - len;
	    /* put the blanks on the left */
	    if (left > 0)
		(void) strncpy (SLK->label[slknum], blanks, left);
	    /* add the label */
	    (void) strncpy (SLK->label[slknum]+left, label, len);
	    /* put the blanks on the right */
	    total = left + len;
	    if (total < SLK->len)
		(void) strncpy (SLK->label[slknum] + total, blanks,
		    SLK->len - total);
	    }
	}

    SLK->label[slknum][SLK->len] = '\0';

    /* add it to the window */
    if (SLK->window)
	mvwprintw (SLK->window, 0, SLK->offset[slknum],
	    "%*s", SLK->len, SLK->label[slknum]);

    SLK->changed[slknum] = TRUE;
    SLK->fl_changed = TRUE;
    return OK;
}
