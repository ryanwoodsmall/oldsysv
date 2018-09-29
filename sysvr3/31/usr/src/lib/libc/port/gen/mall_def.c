/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/mall_def.c	1.3"
/*
 * Contains the definitions
 * of the pointers to the
 * malloc, and _assert
 */

 char * (* _libc_malloc)() = 0;
