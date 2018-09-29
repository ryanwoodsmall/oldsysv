/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:zptrace.c	1.2"

/*	ptrace(2) with error checking
*/

#include	"errmsg.h"

int
zptrace( severity, request, pid, addr, data )
int	 severity;
int	 request;
int	 pid;
int	 addr;
int	 data;
{

	int	err_ind;

	if( (err_ind = ptrace(request, pid, addr, data )) == -1 )
	    _errmsg ( "UXzptrace1", severity,
		  "Cannot process trace: request=%d process id=%d addr=%d data=%d.",
		   request, pid, addr, data);

	return err_ind;
}
