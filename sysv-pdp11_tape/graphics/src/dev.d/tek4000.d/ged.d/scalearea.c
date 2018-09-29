static char SCCSID[]="@(#)scalearea.c	1.1";
/* <: t-5 d :> */
#include <stdio.h>
#include "ged.h"
extern int dbuf[], *endbuf;

scalearea(px,py,arptr,percent,kopyf,echof,table)
int px, py, kopyf, echof;
struct area *arptr;
double percent;
char **table;
{
	struct command cd;
	double k=(percent-1);
	int *bptr=dbuf, *eptr, *p, cnt, *oend=endbuf, *getgeds();
/*
 *  scalearea reads buffer and scales any commands that fall
 *  within defined area from the location px,py by percent
 */
	for( ; (eptr=getgeds(bptr,oend,&cd))!=NULL; bptr=eptr ) {
		p = cd.aptr;
		if( inarea(*p,*(p+1),arptr) ) {
			*p = clipu((long)(*p+(*p-px)*k)); p++;
			*p = clipu((long)(*p+(*p-py)*k)); p++;
			switch(cd.cmd) {
			case ARCS:
			case LINES:
				for( cnt=cd.cnt-4; cnt>0; cnt-=2 ) {
					*p = clipu((long)(*p+(*p-px)*k)); p++;
					*p = clipu((long)(*p+(*p-py)*k)); p++;
				} break;
			case TEXT: cd.tsiz = cd.tsiz*percent; break;
			}
			if(kopyf) {
				int *q=endbuf;
				putbufe(&cd);
				if(echof) dispbuf(q,endbuf,table);
			} else {
				putbuf(bptr,eptr,&cd);
				if(echof) dispbuf(bptr,eptr,table);
			}
		}
	}
}

