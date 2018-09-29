/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/lp/freelist.c	1.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

extern void		free();

/**
 ** freelist() - FREE ALL SPACE USED BY LIST
 **/

void			freelist (list)
	char			**list;
{
	register char		**pp;

	if (list) {
		for (pp = list; *pp; pp++)
			free (*pp);
		free ((char *)list);
	}
	return;
}
