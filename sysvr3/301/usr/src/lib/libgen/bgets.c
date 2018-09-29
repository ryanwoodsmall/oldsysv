/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libgen:bgets.c	1.1"
/*
	read no more that <count> characters into <buf> from stream <fp>,
	stoping at any character slisted in <stopstr>.
	stopstr == 0 uses previous value of stopstr.
*/

#include <stdio.h>

#define CHARS	256

static unsigned char	stop[CHARS];

unsigned char *
bgets( buf, count, fp, stopstr )
unsigned char	*buf;
register
  int	count;
FILE	*fp;
unsigned char	*stopstr;
{
	register unsigned char	*cp;
	register int	c;

	if( stopstr ) {
		/* clear and set stopstr array */
		for( cp = stop;  cp < &stop[CHARS]; )
			*cp++ = 0;
		for( cp = stopstr;  *cp; )
			stop[ *cp++ ] = 1;
	}
	for( cp = buf;  ; ) {
		if( --count < 0 ) {
			*cp = '\0';
			break;
		}
		if( (c = getc(fp)) == EOF ) {
			*cp = '\0';
			if( cp == buf ) {
				cp = (unsigned char *) EOF;
			}
			break;
		}
		*cp++ = c;
		if( stop[ c ] ) {
			*cp = '\0';
			break;
		}
	}

	return  cp;
}
