/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/fakcu.c	1.4"
/*LINTLIBRARY*/
/*
 *       This is a dummy _cleanup routine to place at the end
 *       of the C library in case no other definition is found.
 *       If the standard I/O routines are used, they supply a
 *       real "_cleanup"  routine in file flsbuf.c
 */
void
_cleanup()
{
}
