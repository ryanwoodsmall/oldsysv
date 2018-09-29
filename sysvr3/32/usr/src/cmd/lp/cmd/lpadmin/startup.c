/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpadmin/startup.c	1.6"

#include "stdio.h"

#include "lp.h"
#include "msgs.h"

#include "lpadmin.h"


/**
 ** startup() - OPEN CHANNEL TO SPOOLER
 **/

void			startup ()
{
	trap_signals ();

	if (mopen() == -1)
		scheduler_active = 0;
	else
		scheduler_active = 1;

	return;
}
