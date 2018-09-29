/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/vtoc.d/upperstr.c	1.1"
/* <: t-5 d :> */
#include <stdio.h>
upperstr(ptr)
	char *ptr; {
	for(;*ptr != '\0';ptr++) {
		if(*ptr >= 'a'&& *ptr <= 'z' ) 	*ptr = toupper(*ptr);
	}
	return;
}
