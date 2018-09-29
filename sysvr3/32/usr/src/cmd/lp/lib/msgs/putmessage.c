/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/msgs/putmessage.c	1.1"
/*
**	LINTLIBRARY
*/

# include	<varargs.h>

/* VARARGS */
int
putmessage(va_alist)
va_dcl
{
    char	*buf;
    int		size;
    short	type;
    va_list	arg;

    va_start(arg);
    buf = (char *)va_arg(arg, char *);
    type = (short)va_arg(arg, int);
    size = _putmessage(buf, type, arg);
    va_end(arg);
    return(size);
}
