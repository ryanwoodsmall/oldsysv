/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/lp/lenlist.c	1.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

/**
 ** lenlist() - COMPUTE LENGTH OF LIST
 **/

int			lenlist (list)
	char			**list;
{
	register char		**pl;

	if (!list)
		return (0);
	for (pl = list; *pl; pl++)
		;
	return (pl - list);
}
