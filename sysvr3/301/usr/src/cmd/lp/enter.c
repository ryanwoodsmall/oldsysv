/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident "@(#)lp:enter.c	3.2"
/* enter(name, array, size, max) -- enters name into an argv-type array.

	name -- item to be entered into array
	array -- array to be added to
	size -- pointer to the # of items in array
	max -- max # of items allowed in array

	return codes: 0 for success
		     -1 for failure
*/

#include	"lp.h"


int
enter(name, array, size, max)
char *name;
char *array[];
int *size;
int max;
{
	char *p, *malloc(), *strcpy();

	if(*size >= max)	/* no room in array */
		return(-1);
	if(name == NULL)
		array[(*size)++] = NULL;
	else {
		if((p = malloc((unsigned)(strlen(name)+1))) == NULL)
			return(-1);
		array[(*size)++] = p;
		strcpy(p, name);
	}
	return(0);
}
