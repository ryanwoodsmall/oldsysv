/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sadmin:csub/Fread.c	1.3"
/*	
	fread(3S) with error checking
	Incomplete reads are considered errors.
	Reading End-Of-File or no items (nitem == 0) or zero-length items
	(size == 0) are considered successful reads.
*/

#include	<stdio.h>
#include	<cmderr.h>

Fread( ptr, size, nitems, stream )
char	*ptr;
int	size, nitems;
FILE	*stream;	/* file pointer */
{
	register int	countin;
	extern int	errno;

	if( feof( stream) )
		return 0;
	if( (countin = fread( ptr, size, nitems, stream )) != nitems  &&
		countin  &&  size ) {

		cmderr( CERROR, "Fread() asked for %d %d-byte item(s), got %d",
			nitems, size, countin );
	}
	return  countin;
}
