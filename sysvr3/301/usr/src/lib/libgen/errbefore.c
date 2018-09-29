/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:errbefore.c	1.2"

/*
	Routine called before error message has been printed.
	Command and library version.
*/

#include	<varargs.h>
#include	"errmsg.h"


void
errbefore(severity, format, print_args)
int      severity;
char     *format;
va_list  print_args;
{
	switch( severity ) {
	case EHALT:
	case EERROR:
	case EWARN:
	case EINFO:
		break;
	}
	return;
}
