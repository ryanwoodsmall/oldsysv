static char SCCSID[]="@(#)tekflush.c	1.1";
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
