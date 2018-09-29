/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/lp/cs_strcmp.c	1.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

/*
 * Compare strings ignoring case:  s1>s2: >0  s1==s2: 0  s1<s2: <0
 */

int
cs_strcmp(s1, s2)
register char *s1, *s2;
{

	if(s1 == s2)
		return(0);
	while(toupper(*s1) == toupper(*s2++))
		if(*s1++ == '\0')
			return(0);
	return(toupper(*s1) - toupper(*--s2));
}
