/*	@(#)fmtlib.c	1.4	*/
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
