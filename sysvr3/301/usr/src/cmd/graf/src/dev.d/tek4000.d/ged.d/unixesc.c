/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/ged.d/unixesc.c	1.2"
/* <: t-5 d :> */
#include <stdio.h>
#include "ged.h"

unixesc(restrict)
int restrict;
{
	int pid,rpid,retcode;

	if((pid = fork()) == 0) {
		if(!restrict) execl("/bin/sh","sh", "-t", 0);
		else execl("/bin/rsh","rsh", "-t", 0);
		exit(0);
	}
	while((rpid = wait(&retcode)) != pid && rpid != -1);
	return;
}
