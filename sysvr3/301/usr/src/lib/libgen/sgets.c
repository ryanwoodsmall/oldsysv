/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libgen:sgets.c	1.1"
/*
	Provide machine independent transfer of shorts, ala sgetl(3X).
*/

/*
 * The intent here is to provide a means to make the value of
 * bytes in an io-buffer correspond to the value of a short
 * in the memory while doing the io a `short' at a time.
 * Files written and read in this way are machine-independent.
 *
 */
#include <values.h>

short
sgets(buffer)
register char *buffer;
{
	register short w = 0;
	register int i = BITSPERBYTE * sizeof(short);

	while ((i -= BITSPERBYTE) >= 0)
		w |= (short) ((unsigned char) *buffer++) << i;
	return  w;
}
