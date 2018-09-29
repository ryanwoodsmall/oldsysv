/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libns:ns_getpass.c	1.2"
#include <stdio.h>
#include "nserve.h"

char *
ns_getpass(name)
char *name;
{
	struct nssend send;
	struct nssend *rtn;
	struct nssend *ns_getblock();

	/*
	 *	Initialize the information structure to send to the
	 *	name server.
	 */

	send.ns_code = NS_GETPASS;
	send.ns_name = name;
	send.ns_type = 0;
	send.ns_flag = 0;
	send.ns_desc = NULL;
	send.ns_path = NULL;
	send.ns_addr = NULL;
	send.ns_mach = NULL;

	if ((rtn = ns_getblock(&send)) == (struct nssend *)NULL)
		return((char *)NULL);

	if (rtn->ns_desc == NULL)
		rtn->ns_desc = "";

	return(rtn->ns_desc);
}
