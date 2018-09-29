/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/glib.d/textbox.c	1.1"
/* <: t-5 d :> */
#include <stdio.h>
#include <ctype.h>
#include "debug.h"
#include "util.h"
#include "gsl.h"
#include "gpl.h"
#define TXSZ  100
#define XCH(tsiz)	(.8 * tsiz)
#define YCH(tsiz)	(1.2 * tsiz)
#define HMARG(tsiz)		( .25 * XCH(tsiz))
#define VMARG(tsiz)		( .25 * YCH(tsiz))
#define BREAKCHAR(c)	(c == '.' ||  c == ';' )

textbox(x1,y1,x2,y2,str,rot,bcol,bwt,bst,tsiz,tcol,tfont)
double x1,y1,x2,y2;
int rot,bcol,bwt,bst,tsiz,tcol,tfont;
char *str;
{
/*
 *  textbox locates and centers lines of text from str
 *  inside a box determined by x1,y1,x2,y2
 */

	if(x2 <x1) gslswap(&x1,&x2);
	if(y2 <y1) gslswap(&y1,&y2);
	gslscale(&x1,&y1);
	gslscale(&x2,&y2);
	gsltbox(x1,y1,x2,y2,str,rot,bcol,bwt,bst,tsiz,tcol,tfont);
}
gsltbox(x1,y1,x2,y2,str,rot,bcol,bwt,bst,tsiz,tcol,tfont)
double x1,y1,x2,y2;
int rot,bcol,bwt,bst,tsiz,tcol,tfont;
char *str;
{	char buff[TEXLEN];
	double xprint,yprint,span,ceil();
	int yfill,xchars,ychars,trunc;

	if(tsiz==0) return(FAIL);
	gslbox(x1,y1,x2,y2,rot,bcol,bwt,bst);
	x1 += HMARG(tsiz);	y1 += VMARG(tsiz);
	x2 -= HMARG(tsiz);	y2 -= VMARG(tsiz);
	span = (x2-HMARG(tsiz))-(x1+HMARG(tsiz));
	xchars = ceil(span/XCH(tsiz));
	span = (y2-VMARG(tsiz))-(y1+VMARG(tsiz));
	ychars = ceil(span/YCH(tsiz));
	yfill =  gslfill(str,buff,xchars,ychars,&trunc);
	xprint = x1 + ((x2-x1)/2.);
	yprint = y2-VMARG(tsiz) - ((((double)ychars+.5- yfill)/2.) * YCH(tsiz));
	if(rot) gslswing(x1,y1,&xprint,&yprint,rot);
	gsltcent(xprint,yprint,buff,rot,tsiz,tcol,tfont);
	return(SUCCESS);
}

gslfill(str,bptr,xchars,ychars,count)
char *str,*bptr;
int xchars,ychars,*count;
{	char *lptr=str,*rptr=str,*sptr;
	int yfilled,sp;
/*
 *  fill adds null terminated strings for each text line
 *  within textbox into buff separated by '\n'
 *  truncation returned in count
 */
	for(yfilled=0; ychars-->0 && *rptr !=EOS;lptr=rptr,yfilled++){
		for(sp=0;(rptr+1-lptr)<xchars && *rptr !=EOS;rptr++) 
			if(isspace(*rptr))sp++; 
		if(!sp && !isspace(*rptr)){
			sptr=rptr;
			while((!isspace(*rptr)) && *rptr != EOS)rptr++;
		}
		else if(*rptr==EOS) sptr=rptr-1;
		else if(!isspace(*rptr) && (isspace(*(rptr+1)) || *(rptr+1)==EOS)){
			sptr=rptr++; 
		}else{
			while(!isspace(*rptr))rptr--; 
			sptr=rptr;
			for(;isspace(*sptr);sptr--);
		}
		if(*rptr!=EOS) for(;isspace(*rptr);rptr++);
		for(;lptr<=sptr;lptr++)*bptr++= *lptr;
		*bptr++='\n';
	}
	*bptr=EOS; 
	*count=strlen(rptr);
	return(yfilled);
}
gslswap(x,y)
double *x,*y;
{	double temp;
	temp = *x;
	*x = *y;
	*y = temp;
}
