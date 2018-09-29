/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sadmin:check/rightlist.c	1.3"
/*
	Look for a list of right answers.  Any answer that doesn't match
	one in argv or an empty answer fails and returns 0.
	If all answers are correct, rightlist() returns the return value of
	the last call to (*rafunc)().
*/

#include "check.h"
#include <stdio.h>
#include <string.h>

#define	MAXANS	100	/* The maximum number of answers in the list. */


rightlist( answer, argc, argv, separators, rafunc )
char	*answer;
int	argc;
char	**argv;
char	*separators;
int	(*rafunc)();	/* pointer to rightanswer function;
			returns 0 on failure */
{
	char	*a;
	int	j;
	register char	**op;
	char	*outlist[ MAXANS ];

	op = outlist;
	if( (a = strtok( answer, separators )) == NULL )
		return  0;
	do {
		if( !( j = (*rafunc)( a, argv ) ) ) {
			fprintf( stderr,
				"\t'%s' is not a permitted answer\n", a );
			return  0;
		}
		*op++ = a;
	} while( (a = strtok( NULL, separators )) != NULL );
	*op = 0;
	if( echo )
		for( op = outlist; *op; op++ )
			puts( *op );
	return  j;
}
