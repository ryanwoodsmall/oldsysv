/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/hp7220.d/hpd.d/mba.c	1.2"
#include <stdio.h>
#include "dev.h"

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
