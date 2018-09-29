static char SCCSID[]="@(#)addrbuf.c	1.1";
/* <: t-5 d :> */
#include <stdio.h>
#include "tek.h"
extern char obuf[],*optr;
#define MAXLENGTH 7

addrbuf(x1,y1)
int x1,y1;
{
	int tk[4]; /* hy, ly, hx, lx */
	int i;
/*
 *  addrbuf translates screen x and y values to tek 4 byte
 *  addresses and writes necessary bytes to obuf
 *  also checks for full out buffer & if found calls tekflush
 */
	xytek(x1,y1,tk);

	for( i=0; i<4; i++ ) *optr++ = tk[i];

	if(optr + MAXLENGTH > obuf + BUFSIZ) {
		tekflush();
	}
	return;
}
