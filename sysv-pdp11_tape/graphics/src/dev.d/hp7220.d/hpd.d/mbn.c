static char SCCSID[]="@(#)mbn.c	1.1";
#include <stdio.h>
#include "../../include/dev.h"

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
