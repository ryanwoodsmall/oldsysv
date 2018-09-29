/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libgen:sputs.c	1.1"
/*
	Provide machine independent transfer of shorts, ala sputl(3X).
*/

/*
 * The intent here is to provide a means to make the value of
 * bytes in an io-stream correspond to the value of the short
 * in the memory while doing the io a `short' at a time.
 * Files written and read in this way are machine-independent.
 *
 */
#include <values.h>

void
sputs(w, buffer)
register short w;
register char *buffer;
{
	register int i = BITSPERBYTE * sizeof(short);

	while ((i -= BITSPERBYTE) >= 0)
		*buffer++ = (char) (w >> i);
}
