/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libgen:smemcpy.c	1.2"
/*	Safe memory copy.
	Fast if the to and from strings do not overlap,
	slower but safe if they do.
*/

#include	<memory.h>
#include	"libgen.h"


char *
smemcpy( to, from, count )
register char		*to, *from;
register unsigned	count;
{
	char	*savedto;

	if( &to[ count ] <= from  ||  &from[ count ] <= to )
		return  memcpy( to, from, count );

	if( to == from )
		return  to;

	savedto = to;
	if( to < from )
		while( count-- )
			*(to++) = *(from++);
	else {
		to += count;
		from += count;
		while( count-- )
			*(--to) = *(--from);
	}

	return  savedto;
}
