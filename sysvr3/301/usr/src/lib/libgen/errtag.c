/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:errtag.c	1.1"

/*	Set tag to file name and line number;
	Used by errmsg() macro.
*/

#include	"errmsg.h"


void
errtag( str, num )
char	*str;
int	num;
{
	Err.tagstr = str;
	Err.tagnum = num;
}
