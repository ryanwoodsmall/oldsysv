/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/ged.d/copymove.c	1.2"
/* <: t-5 d :> */
#include <stdio.h>
#include "ged.h"
extern int dbuf[], *endbuf;

copy(dx,dy,ar,table,mode,echof)
int dx, dy, echof;
struct area *ar;
char **table, mode;
{
	struct command cd;
	int *bptr=dbuf, *eptr=endbuf, *getgeds();
/*
 *  copy reads buffer and copys any commands that fall
 *  within defined area to locations dx and dy 
 *  distance away
 */
	  while( (bptr=getgeds(bptr,eptr,&cd))!=NULL )
		if( displace(&cd,dx,dy,ar,mode) ) {
			putbufe(&cd);
		}
	if(echof) dispbuf(eptr,endbuf,table);
	return;
}

move(dx,dy,ar,table,mode,echof)
int dx, dy, echof;
struct area *ar;
char **table, mode;
{
	struct command cd;
	int *bptr=dbuf, *eptr=dbuf, *getgeds();
/*
 *  move reads buffer and moves any commands that fall
 *  within defined area to locations dx and dy 
 *  distance away
 */
	for( ; (eptr=getgeds(bptr,endbuf,&cd))!=NULL; bptr=eptr )
		if( displace(&cd,dx,dy,ar,mode) ) {
			putbuf(bptr,eptr,&cd);
			if(echof) dispbuf(bptr,eptr,table);
		}
	return(SUCCESS);
}
displace(cd,dx,dy,arptr,mode)
struct command *cd;
int dx,dy;
struct area *arptr;
char mode;		/*   'o' || 'p'  */
{
	int change=0, *p, cnt;
/*
 *  displace displaces all points or objects(according to mode)
 *  within area given by arptr by dx and dy
 *  number of changes to buffer is returned
 */
	if(cd->cmd == COMMENT) return(0);
	p = cd->aptr;
	if( inarea(*p,*(p+1),arptr) ) {
		*p=clipu(*p+(long)dx);
		*(p+1)=clipu(*(p+1)+(long)dy);
		change++;
	}
	if( (mode=='o'&&change) || mode=='p' )
		if( cd->cmd==ARCS || cd->cmd==LINES )
			for( cnt=cd->cnt-4,p+=2; cnt>0; p+=2,cnt-=2 )
				if( mode=='o'||inarea(*p,*(p+1),arptr) ) {
					*p=clipu(*p+(long)dx);
					*(p+1)=clipu(*(p+1)+(long)dy);
					change++;
				}
	return(change);
}
