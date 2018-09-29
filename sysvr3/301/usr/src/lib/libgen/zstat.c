/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:zstat.c	1.2"

/*	stat(2) with error checking
*/

#include "errmsg.h"
#include <sys/stat.h>

int
zstat( severity, path, buf)
int	severity;
char	*path;
struct	stat *buf;
{
	int	rc;

	if((rc = stat(path, buf)) == -1 )
		_errmsg("UXzstat1", severity,
		      "Cannot obtain information about file:  \"%s\".",
		       path);


	return  rc;
}
