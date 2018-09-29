char spaxver[] = "@(#)spacing.c	1.1";		/* SCCS */

# include <stdio.h>
# include "structs.h"

int
h_space(n)
int	n ;
{
	static	struct
	{
		int	width ;
		int	charcode ;
	} table[] =
	{
		1024 ,	0364 ,
		512 ,	0365 ,
		256 ,	0366 ,
		128 ,	0367 ,
		64 ,	0370 ,
		32 ,	0371 ,
		16 ,	0372 ,
		8 ,	0373 ,
		4 ,	0374 ,
		2 ,	0375 ,
		1 ,	0376 ,
	} ;
	int	i ;
	int	num_chars ;

	num_chars = 0 ;
	for ( i=0 ; i<NEL(table) ; i++ )
		while ( table[i].width <= n )
		{
			n -= table[i].width ;
			putchar(table[i].charcode) ;
			num_chars++ ;
		}
	return(num_chars) ;
}
char
v_space(old,new)
int	old ;
int	new ;
{
	static	int	tab[4][4] =
	{
		'0' ,	'D' ,	'2' ,	'3' ,
		'0' ,	'4' ,	'5' ,	'6' ,
		'0' ,	'7' ,	'8' ,	'9' ,
		'0' ,	'B' ,	'C' ,	'0' ,
	} ;
	int	cc[4] ;
	int	num_cc ;
	int	digit ;
	int	i ;

	while ( old/STRIPSIZE < new/STRIPSIZE )
	{
		old = (old/STRIPSIZE+1) * STRIPSIZE ;
		if ( old == new )
			return('A') ;
		printf("A1 \n") ;
	}

	old %= STRIPSIZE ;
	new %= STRIPSIZE ;
	num_cc = 0 ;
	for ( i=3 ; i>=0 ; i-- )
	{
		if ( (new >> (2*i)) != (old >> (2*i)) )
		{
			digit = (new >> (2*i)) & 03 ;
			cc[num_cc++] = tab[i][digit] ;
			old = (new >> (2*i)) << (2*i) ;

		}
	}
	for ( i=0 ; i<num_cc-1 ; i++ )
		printf("%c1 \n",cc[i]) ;
	return(cc[i]) ;
}
