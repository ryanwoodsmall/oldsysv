/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/ged.d/deleteobj.c	1.2"
/* <: t-5 d :> */
#include <stdio.h>
#include <signal.h>
#include "ged.h"
extern interr();
extern int dbuf[], *endbuf;
extern struct control ct;

deleteobj(arptr)
struct area *arptr;
{
	struct command cd;
	int *bptr=dbuf, *eptr, *getgeds();
/*
 *  deleteobj reads buffer and deletes any objects that fall
 *  within defined area
 */
	for( ; (eptr=getgeds(bptr,endbuf,&cd))!=NULL; bptr=eptr ) {
		if( cd.cmd!=COMMENT ) 
			if( inarea(cd.aptr[0],cd.aptr[1],arptr) ) {
				IGNSIG;
				ct.change=TRUE;
				shuffle(bptr,eptr);
				CATCHSIG;
				eptr=bptr;
		}
	}
}

shuffle(p1,p2)
int *p1,*p2;
{
	char *pb1, *pb2;
	pb1 = (char *) p1;
	pb2 = (char *) p2;
/*
 *  shuffle shuffles down elements in the buffer to
 *  fill in space vacated by a deleted command
 */
	while(pb2 < (char *) endbuf)
		*pb1++ = *pb2++;
	endbuf = (int *) pb1;
}
