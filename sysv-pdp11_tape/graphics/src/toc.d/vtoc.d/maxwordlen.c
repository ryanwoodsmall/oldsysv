static char SCCSID[]="@(#)maxwordlen.c	1.1";
/* <: t-5 d :> */
#include <stdio.h>
#include "ctype.h"
#include "../../../include/util.h"

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
