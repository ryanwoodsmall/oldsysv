static char SCCSID[]="@(#)upperstr.c	1.1";
/* <: t-5 d :> */
#include <stdio.h>
upperstr(ptr)
	char *ptr; {
	for(;*ptr != '\0';ptr++) {
		if(*ptr >= 'a'&& *ptr <= 'z' ) 	*ptr = toupper(*ptr);
	}
	return;
}
