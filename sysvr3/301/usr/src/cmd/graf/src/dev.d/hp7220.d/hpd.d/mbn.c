/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/hp7220.d/hpd.d/mbn.c	1.2"
#include <stdio.h>
#include "dev.h"

mbn(i) 
int i;
{	int t;
/*
 *  mbn translates integers into three byte mbn format
 */
	putchar( (12<<3) + (i>>12 & 07) );
	t = ((i>>6) & 077);
	if(t>=32) putchar(t);  else putchar(t|0100);
	t = (i & 077);
	if(t>=32) putchar(t);  else putchar(t|0100);
	return;
}
