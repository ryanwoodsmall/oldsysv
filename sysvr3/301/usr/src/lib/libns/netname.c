/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libns:netname.c	1.3"
#include "string.h"
#include "errno.h"
#include "nserve.h"
#include "sys/utsname.h"
#include "sys/rfsys.h"

int
netname(s)
char *s;
{
	struct utsname	name;

/*
 *	flow: 
 *
 *	1) make a call to rfsys to get domain name
 *	2) do a uname to get sysname 
 *	3) concatonate with "."
 *	4) return netnodename
 *
 */

	if(rfsys(RF_GETDNAME, s, MAXDNAME) < 0) {
		perror("netname");
		return(-1);
	}
	if(uname(&name) < 0) {
		perror("netname");
		return(-1);
	}
	(void)sprintf(s + strlen(s),"%c%s",SEPARATOR, name.nodename);
	return(0);
}
