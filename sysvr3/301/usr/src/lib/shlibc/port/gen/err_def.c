/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/err_def.c	1.1"
/*
 * Pointer to symbols
 * sys_errlist and sys_nerr
 */


 char * (* _libc_sys_errlist) = 0;
 int    (* _libc_sys_nerr) = 0;
