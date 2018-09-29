/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/lib/getpos.c	1.5"
/* <: t-5 d :> */
#include <stdio.h>
#include "util.h"
#include "tek.h"

getpos(tkptr)
struct tekaddr *tkptr;
{
/*
 *  getpos gets and sets values of 4 byte tek addresses
 */
	char s[5];
	int i;
#if u370
	char *sptr;
	for (i=0, sptr=s; i < 5 && fread(sptr,1,1,stdin) == 1; i++, sptr++);
	if(i >= 5) {
#else
	i = fread(s,1,5,stdin);
	if( i == 5 ) {
#endif
		tkptr->ch=s[0] & 0177;
		tkptr->xh=s[1] & 0177;
		tkptr->xl=s[2] & 0177;
		tkptr->yh=s[3] & 0177;
		tkptr->yl=s[4] & 0177;
		return(SUCCESS);
	}
	else return(FAIL);
}
