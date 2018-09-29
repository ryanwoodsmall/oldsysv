/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:zfwrite.c	1.3"

/*	fwrite(3S) with error checking
	Incomplete writes are considered errors.
	zreading End-Of-File or no items (nitem == 0) or zero-length items
	(size == 0) are considered successful reads.
*/


#include	<stdio.h>
#include	"errmsg.h"

zfwrite( severity, ptr, size, nitems, stream )
int     severity;
char	*ptr;
int	size, nitems;
FILE	*stream;	/* file pointer */
{
	register int	countin;

	if( feof( stream) )
		return 0;
	if( (countin =
		fwrite( ptr, size, nitems, stream )) != nitems  &&
		nitems  &&  size ) {

		_errmsg( "UXzfwrite1", severity, "zfwrite() asked for %d %d-byte item(s), got %d.",
			nitems, size, countin );
	}
	return  countin;
}
