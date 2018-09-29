static char SCCSID[]="@(#)bufged.c	1.1";
/* <: t-5 d :> */
#include <stdio.h>
#include <signal.h>
#include "ged.h"
extern int interr();
extern int dbuf[], *endbuf;
extern struct control ct;
extern char *nodename;

putbufe(cp)
struct command *cp;
{
	int *putgeds();

/*
 *  putbufe appends to the display buffer the command whose
 *  parameters are contained within the structure cp
 */
	if( endbuf+cp->cnt < dbuf+BUFSZ ) {
		IGNSIG;
		endbuf = putgeds(endbuf,dbuf+BUFSZ,cp);
		ct.change = TRUE;
		CATCHSIG;
		return(SUCCESS);
	}
	else {
		ERRPR0(display buffer full);
		return(FAIL);
	}
}

putbuf(start,end,cp) /* put command in dbuf at start */
int *start, *end;
struct command *cp;
{
	IGNSIG;
	if( !putgeds(start,end,cp) )
		ERRPR0(display buffer possibly corrupted);
	ct.change=TRUE;
	CATCHSIG;
}
