/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libF77:abort_.c	1.3"
#include <stdio.h>

abort_()
{
fprintf(stderr, "Fortran abort routine called\n");
_cleanup();
abort();
}
