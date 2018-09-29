/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/hp7220.d/hpd.d/mbp.c	1.2"
#include <stdio.h>
#include "dev.h"

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
