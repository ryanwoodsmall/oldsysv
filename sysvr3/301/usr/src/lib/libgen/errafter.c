/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:errafter.c	1.2"

/*
	Customized routine called after error message has been printed.
	Command and library version.
	Return a value to indicate action.
*/

#include	"errmsg.h"
#include	<stdio.h>
#include	<varargs.h>

int
errafter( severity, format, print_args )
int	severity;
char	*format;
va_list print_args;
{
	switch( severity ) {
	case EHALT:
		return EABORT;
	case EERROR:
		return EEXIT;
	case EWARN:
		return ERETURN;
	case EINFO:
		return ERETURN;
	}
	return ERETURN;
}
