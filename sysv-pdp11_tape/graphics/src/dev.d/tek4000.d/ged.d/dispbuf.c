static char SCCSID[]="@(#)dispbuf.c	1.1";
/* <: t-5 d :> */
#include <stdio.h>
#include <signal.h>
#include "ged.h"
#include "../include/termodes.h"

extern struct control ct;
extern int interr();

static int quit;

dispbuf(bptr,eptr,chtable)
int *bptr,*eptr;
char *chtable[];
{
	struct command cd;
	int *getgeds(), locint();

/*
 *  dispbuf displays buf delim by dbuf & eptr
 */
	setmode(GRAPH);
	quit=0; /* reset quit flag */
	signal(SIGINT,locint); signal(SIGQUIT,locint);

	while( (bptr=getgeds(bptr,eptr,&cd))!=NULL && !quit ) {
		switch(cd.cmd) {
		case LINES: teklines(&cd); 
			 break;
		case TEXT:
			if( !ct.textf ) cd.tsiz *= cd.tsiz>0 ? -1 : 1;
			tektext(&cd,chtable); 
			break;
		case ARCS: tekarc(&cd); 
			break;
		case ALPHA: tekalpha(&cd); 
			break;
		default: ;
		}
	}
	restoremode();
	CATCHSIG; /* restore standard interrupt rtn */
	return;
}
outbuf() /* flush output buffer */
{
	IGNSIG;
	tekflush();
	restoremode();
	CATCHSIG;
}
static
locint() /* local interrupt routine */
{
	IGNSIG;
	quit=1;
}
