/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/lib/tektext.c	1.2"
/* <: t-5 d :> */
#include <stdio.h>
#include "tek.h"
#include "util.h"
extern struct control ct;

tektext(cdptr,table)
struct command *cdptr;
char *table[];
{
/*
 *  tektext creates tek scope code to draw the text string
 *  pointed to by text at the location cdptr->x1,cdptr->y1
 */
	if(cdptr->tsiz/ct.wratio<4){
		if(cdptr->tsiz>0) cdptr->tsiz = -cdptr->tsiz;
	}
	chplot(cdptr,table);
	return;
}
