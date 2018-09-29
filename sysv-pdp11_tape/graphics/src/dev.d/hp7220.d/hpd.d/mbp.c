static char SCCSID[]="@(#)mbp.c	1.1";
#include <stdio.h>
#include "../../include/dev.h"

mbp(x,y) {
	int t;
/*
 *  mbp translates coordinates into multiple-byte pair format
 */
	putchar( (x>>10) | 0140 );
	t = (x>>4) & 077;
	if ( t >= 32 ) putchar(t);
	else putchar( t | 0100 );
	t = ( (x<<2) | ( (y>>12) & 03 ) ) & 077;
	if ( t >= 32 ) putchar(t);
	else putchar( t | 0100 );
	t = (y>>6) & 077;
	if ( t >= 32 ) putchar(t);
	else putchar( t | 0100 );
	t = y & 077;
	if ( t >= 32 ) putchar(t);
	else putchar( t | 0100 );
}
