static char SCCSID[]="@(#)mba.c	1.1";
#include <stdio.h>
#include "../../include/dev.h"

double tab[] ={180.,90.,45.,22.5,11.25,5.63,2.81,1.40,.703,.352,
			.176,.088,.044,.022,.011,.005,0.};
mba(ang)
double ang;
{	unsigned i,k;
	int t;
/*
 *  mba translates int ang value into three-byte angle values
 */
	for(k=0,i=0; tab[k] > 0.; k++){
		i = i<<1;
		if(ang >= tab[k]){
			i |= 01;
			ang -= tab[k];
		}
	}
	putchar((6 << 4) + ((i >> 12) & 017));
	t=((i>>6) & 077) ;
	if( t>= 32) putchar(t);
	else putchar(t | 0100);
	t=(i & 077) ;
	if( t>= 32) putchar(t);
	else putchar(t | 0100);
}
