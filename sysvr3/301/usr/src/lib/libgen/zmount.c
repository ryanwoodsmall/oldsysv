/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:zmount.c	1.2"

/*	mount(2) with error checking
*/

#include	"errmsg.h"

int
zmount( severity, spec, dir, rwflag )
int	 severity;
char	*spec;
char	*dir;
int	 rwflag;
{

	int	err_ind;

	if( (err_ind = mount(spec, dir, rwflag )) == -1 )
	    _errmsg ( "UXzmount1", severity,
		  "Cannot mount file system on block \"%s\" to directory \"%s\" , rwflag=%d.",
		   spec, dir, rwflag);

	return err_ind;
}
