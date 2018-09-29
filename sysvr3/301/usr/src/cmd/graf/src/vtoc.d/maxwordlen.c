/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/vtoc.d/maxwordlen.c	1.1"
/* <: t-5 d :> */
#include <stdio.h>
#include "ctype.h"
#include "util.h"

maxwordlen(ptr)
	char *ptr; {
	int word = 0;
	int i;
/*
 *  maxwordlen returns length of longest word in string
 *  pointed to by ptr.
 */
	for(i=0; *ptr != '\0'; i++,ptr++) {
		if(isspace(*ptr)) {
			word = MAX(word,i);
			ptr++;
			i = 0;
		}
	}
	word = MAX(word,i);
	return(word);
}
