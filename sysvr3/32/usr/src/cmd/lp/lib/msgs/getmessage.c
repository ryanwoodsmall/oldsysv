/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/msgs/getmessage.c	1.1"
/*
**      LINTLIBRARY
*/

# include	<varargs.h>

/* VARARGS */
int
getmessage(va_alist)
va_dcl
{
    va_list	arg;
    int		rval;
    short	type;
    char	*buf;

    va_start(arg);
    buf = (char *)va_arg(arg, char *);
    type = (short)va_arg(arg, int);
    rval = _getmessage(buf, type, arg);
    va_end(arg);
    return(rval);
}
