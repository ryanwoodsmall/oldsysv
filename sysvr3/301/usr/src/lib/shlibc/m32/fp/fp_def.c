/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-m32:fp/fp_def.c	1.1"
/* 
 * This file contains the initialization of the
 * pointer to the imported symbol _getflthw
 */

 void (* _libc__getflthw)() = 0 ;
