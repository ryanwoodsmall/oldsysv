/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sadmin:csub/stddef.c	1.2"
/*	
	Special module that "compiles" the external definitions in
	stddef.h, thus putting many declarations of global space in one
	module.
*/

#define	EXTERN
#include	<stddef.h>
