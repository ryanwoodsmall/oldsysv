/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/lib/tekflush.c	1.2"
/* <: t-5 d :> */
#include <stdio.h>
#include <signal.h>
#include "tek.h"
char obuf[BUFSIZ],*optr=obuf;
extern struct tekterm tm;

tekflush()
{
	if((optr - obuf) >1) {
		fwrite(obuf,1,(unsigned)(optr-obuf),stdout);
		tm.curlw = UNSET;  tm.curls = UNSET;
		optr = obuf;
	}
	return;
}

tekclr() /* reset output buffer */
{
	tm.curlw = UNSET; tm.curls = UNSET;
	optr = obuf;
}
