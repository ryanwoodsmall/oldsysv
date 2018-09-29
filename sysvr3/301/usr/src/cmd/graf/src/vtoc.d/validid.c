/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/vtoc.d/validid.c	1.1"
extern	char *nodename;

#include	"vtoc.h"

/*
	validid will determine if the string pointed to by ptr is
a valid id string where id is:

		(digit group)[.[(digit group)[.]]]

*/

validid(ptr)
	char	*ptr;
{
	if	(!isdigit(*ptr))
		return(FALSE);
	else	;
	while(TRUE){
		for	(; isdigit(*ptr); ptr++)
			;
		if	(*ptr == '\0')
			return(TRUE);
		else	if	(*ptr == '.' && (isdigit(*(ptr+1)) || *(ptr+1) == '\0'))
				ptr++;
			else	return(FALSE);
	}
}
