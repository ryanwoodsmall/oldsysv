/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/lp/strip.c	1.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

/**
 ** strip() - STRIP LEADING AND TRAILING BLANKS
 **/

char			*strip (str)
	register char		*str;
{
	register char		*p;

	if (!str || !*str)
		return (0);

	str += strspn(str, " ");
	for (p = str; *p; p++)
		;
	p--;
	for (; p >= str && *p == ' '; p--)
		;
	*++p = 0;
	return (str);
}
