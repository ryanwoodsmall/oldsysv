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
/* @(#)prefix.c	1.3 */
#include "uucp.h"

/*
 * check s for prefix pre
 *	pre	-> prefix string
 *	s	-> string to be checked
 * returns:
 *	FALSE	-> no match
 *	TRUE	-> match
 */
prefix(pre, s)
register char *pre, *s;
{
	register char c;

	while ((c = *pre++) == *s++)
		if (c == '\0')
			return(TRUE);
	return((c == '\0')?TRUE:FALSE);
}
