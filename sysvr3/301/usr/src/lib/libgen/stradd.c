/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libgen:stradd.c 	1.2"
/*LINTLIBRARY*/
/*
 * Add s2 to s1.  S1's space must be large enough.
 * Return the address of the last character written, i.e. the '\0' character.
 */

char *
stradd(s1, s2)
register char *s1, *s2;
{
	while(*s1++ = *s2++)
		;
	return(--s1);
}
