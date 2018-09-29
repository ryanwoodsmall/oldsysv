/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:include/lp.set.h	1.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

/*
 * How far should we check for "compressed" horizontal pitch?
 * Keep in mind that (1) too far and the user can't read it, and
 * (2) some Terminfo entries don't limit their parameters like
 * they should. Keep in mind the other hand, though: What is too
 * compact for you may be fine for the eagle eyes next to you!
 */
#define MAX_COMPRESSED	30	/* CPI */

#define	E_SUCCESS	0
#define	E_FAILURE	1
#define	E_BAD_ARGS	2
#define	E_MALLOC	3

#define	OKAY(P)		((P) && (*P))
#define R(F)		(int)((F) + .5)

#if	!defined(CHARSETDIR)
# define CHARSETDIR	"/usr/lib/charset"
#endif

extern int		set_pitch(),
			set_size(),
			set_charset();
