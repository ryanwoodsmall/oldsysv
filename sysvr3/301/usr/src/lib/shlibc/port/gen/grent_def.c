/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/grent_def.c	1.1"
/*
 * Contains the definitions
 * of the pointers to the
 * functions getgrent(),
 * setgrent(), endgrent() and
 * fgetgrent().
 */

#include <grp.h>

 struct group * (* _libc_getgrent)() = 0;
 struct group * (* _libc_fgetgrent)() = 0;
 void  (* _libc_setgrent)() = 0;
 void  (* _libc_endgrent)() = 0;
