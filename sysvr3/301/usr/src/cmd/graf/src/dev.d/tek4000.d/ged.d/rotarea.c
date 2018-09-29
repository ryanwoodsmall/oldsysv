/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/ged.d/rotarea.c	1.2"
/* <: t-5 d :> */
#include <stdio.h>
#include "ged.h"
extern int dbuf[], *endbuf;

rotarea(px,py,arptr,rot,kopyf,echof,table)
int px, py, rot, kopyf, echof;
struct area *arptr;
char **table;
{
	struct command cd;
	int *bptr=dbuf, *eptr, *p, cnt, *oend=endbuf, *getgeds();
	double x, y;
/*
 *  rotarea reads buffer and rotates any commands that fall
 *  within defined area from the location px,py by rot
 */
	for( ; (eptr=getgeds(bptr,oend,&cd))!=NULL; bptr=eptr ) {
		p = cd.aptr;
		if( inarea(*p,*(p+1),arptr) ) {
			x = *p; y = *(p+1);
			gslswing((double)px,(double)py,&x,&y,rot);
			*p++ = clipu((long)x);
			*p++ = clipu((long)y);
			switch(cd.cmd) {
			case ARCS:
			case LINES:
				for( cnt=cd.cnt-4; cnt>0; cnt-=2 ) {
					x = *p; y = *(p+1);
					gslswing((double)px,(double)py,&x,&y,rot);
					*p++ = clipu((long)x);
					*p++ = clipu((long)y);
				} break;
			case TEXT: cd.trot += rot; break;
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

