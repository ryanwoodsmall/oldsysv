/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/vtoc.d/sizefac.c	1.1"
#include <stdio.h>
extern char *nodename;
#include"vtoc.h"
extern struct node *nd[];
#define TEXLEN 256

sizefac(maxindx,maxwd,xchars,ychars)
	int maxindx,maxwd;
	double *xchars,*ychars; {
	int len;
	char *str,line[TEXLEN];
	int cfilled,lfilled;
	int trunc = 1;
/*
 *  sizefac places in xchars the number of characters, and
 *  in ychars the number of lines required
 *  to accomodate the longest textstring and 
 *  still maintain a golden ratio box
 */
	for(*xchars=0,str=nd[maxindx]->text,len=strlen(str)
	; (*xchars < maxwd) || trunc ;) {
		chop(len,xchars,ychars);
		if(*xchars < maxwd) len++;
		else {
			gslfill(str,line,(int)*xchars,(int)*ychars,&trunc);
			if(trunc) len += trunc;
			else len++;
		}
	}
	*xchars +=.5; *ychars += .5;  /* allow for margins  */
	return;
}
chop(len,xchars,ychars)
int len;
double *xchars,*ychars;
{	double sqrt();
/*
 *  given a length, chop sets xchars and ychars to the proper values
 *  to give golden ratio.
 */

	*ychars = (int)(sqrt((double)((len)/GOLDRAT)));
	*xchars = (int)((*ychars * GOLDRAT) /XYRAT);
	return;
}
