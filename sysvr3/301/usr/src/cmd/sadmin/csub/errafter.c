/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sadmin:csub/errafter.c	1.2"
/*	
	Routine called after error message has been printed.
	Command and library version.
*/

#include	"cmderr.h"
#include	<stdio.h>


void
errafter( severity, format, ErrArgList )
int	severity;
char	*format;
int	ErrArgList;
{
	extern int	errno;

	switch( severity ) {
	case CHALT:
	case CERROR:
		if( errno  &&  (int)format != CERRNO ) {
			fputc( '\t', stderr );
			perror("");
		}
	}
	switch( severity ) {
	case CHALT:
		abort();
	case CERROR:
		exit( errexit );
	case CWARN:
	case CINFO:
		break;
	}
	return;
}
