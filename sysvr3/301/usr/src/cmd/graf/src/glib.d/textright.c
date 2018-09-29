/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/glib.d/textright.c	1.1"
/* <: t-5 d :> */
#include <stdio.h>
#include "debug.h"
#include "util.h"
#define HRAT  (.8)
#define VRAT  (1.2)

textright(x1,y1,str,trot,tsiz,color,font)
double x1,y1;
int trot,tsiz,color,font;
char *str; 
{
/*
 *  textright places string pointed to by str to end on the point x1,y1
 */

	gslscale(&x1,&y1);
	gsltright(x1,y1,str,trot,tsiz,color,font);
}
gsltright(x1,y1,str,trot,tsiz,color,font)
double x1,y1;
int trot,tsiz,color,font;
char *str;
{	double ox,oy,xhch,yhch,xprint,yprint;
	int more=TRUE;
	char *ptr;
	ox = x1;	oy = y1;
	gslhchar(&xhch,&yhch,trot,HRAT*tsiz);
	for(;more;str=ptr){
		xprint = x1 - ((gslslen(str)-1.) * xhch );
		yprint = y1 - ((gslslen(str)-1.) * yhch );
		for(ptr=str;*ptr !='\n' && *ptr!=EOS;ptr++);
		if(*ptr=='\n'){
			*ptr++=EOS;
			y1 -= VRAT*tsiz;
			if(trot) gslswing(x1,y1+VRAT*tsiz,&x1,&y1,trot);
		}
		else more=FALSE;
		gsltext(xprint,yprint,str,trot,tsiz,color,font);
	}
	setcoord(ox,oy);  /*  reset cur parms  */
}
