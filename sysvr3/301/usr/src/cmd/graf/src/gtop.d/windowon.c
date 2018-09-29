/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/gtop.d/windowon.c	1.1"
/* <: t-5 d :> */
#include <stdio.h>
#include "gsl.h"
#include "gpl.h"
#include "errpr.h"
#include "util.h"
#define MARGRAT    (.10)
#define MINMARG    (10)

extern char  string[];
extern char *nodename;

FILE *
windowon(fpi,arptr)
FILE *fpi;
struct area *arptr;
{	FILE *tfpo,*fopen(),*freopen();
	char *tempfile="/tmp/gtop.XXXXXX";
	int marg,cnt,*lptr;
	struct command cd;
/*
 *  windowon sets appropriate window to frame file pointed to by fpi
 */
	mktemp(tempfile);
	if(tfpo = fopen(tempfile,"w")) {
		xymaxmin(fpi,tfpo,arptr);
		if(fpi = freopen(tempfile,"r",tfpo)) {
			marg= MAX((long)arptr->hx-arptr->lx,arptr->hy-arptr->ly)*MARGRAT;
			marg=MAX(marg,MINMARG);
			arptr->lx = clipu(arptr->lx-(long)marg);
			arptr->ly = clipu(arptr->ly-(long)marg);
			arptr->hx = clipu(arptr->hx+(long)marg);
			arptr->hy = clipu(arptr->hy+(long)marg);
		} else {ERRPR1(cannot reopen %s,tempfile);return(NULL);}
	} else {ERRPR1(cannot open %s,tempfile);return(NULL);}
	sprintf(string,"rm %s",tempfile);
	system(string);
	return(fpi);
}
clipu(x) /* clip x to universe */
long x;
{
	if( x>XYMAX ) return(XYMAX);
	else if( x<XYMIN ) return(XYMIN);
	else return(x);
}
