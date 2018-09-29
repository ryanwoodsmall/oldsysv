/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libI77:fmtlib.c	1.5"
#include <stdio.h>
#define MAXINTLENGTH 12
char *icvt(value,ndigit,sign) long value; int *ndigit,*sign;
{	
	/* E.T.D  [evan] 03-28-84 
	 *
	 * Redesigned to use sprintf()  instead of
	 * special code for largest negative int,long.
	*/
	static char buf[MAXINTLENGTH+1];
	*sign = 0 ;
	if ( value == 0 )
	{
		*ndigit = 1;
		buf[MAXINTLENGTH] = '0' ;
		return( &buf[MAXINTLENGTH] ) ;
	}
	*ndigit = sprintf(buf,"%ld",(long)value);
	if ( value < 0 )
	{
		*sign = 1 ;
		(*ndigit)-- ;
		return( &buf[1] ) ;
	}
	return( buf ) ;
}
