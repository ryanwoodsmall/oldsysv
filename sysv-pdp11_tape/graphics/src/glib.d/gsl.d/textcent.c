static char SCCSID[]="@(#)textcent.c	1.1";
/* <: t-5 d :> */
#include <stdio.h>
#include "../../../include/debug.h"
#include "../../../include/util.h"
#define HRAT  (.8)
#define VRAT  (1.2)

textcent(x1,y1,str,trot,tsiz,color,font)
double x1,y1;
int trot,tsiz,color,font;
char *str; 
{
/*
 *  textcent centers string pointed to by str on the point x1, y1
 */
	gslscale(&x1,&y1);
	gsltcent(x1,y1,str,trot,tsiz,color,font);
}
gsltcent(x1,y1,str,trot,tsiz,color,font)
double x1,y1;
int trot,tsiz,color,font;
char *str;
{	double ox,oy,xhch,yhch,xprint,yprint;
	int more=TRUE;
	char *ptr;
	ox = x1;	oy = y1;
	gslhchar(&xhch,&yhch,trot,HRAT*tsiz);
	for(;more;str=ptr){
		xprint = x1 - ((gslslen(str)-1.)/2. * xhch );
		yprint = y1 - ((gslslen(str)-1.)/2. * yhch );
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
