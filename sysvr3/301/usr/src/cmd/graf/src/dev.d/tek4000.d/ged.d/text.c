/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/ged.d/text.c	1.3"
#include <stdio.h>
#include "ged.h"
/* #include "errpr.h" done by include ged.h */
extern char *nodename;

gedtext(cdptr)
struct command *cdptr;
{	char *ptr;
	int more, *lptr, len, line;
	double x0,y0;
/*
 *  gedtext installs textcommand in buffer replacing newlines
 *  with multiple text line calls
 */
	lptr=cdptr->aptr;
	x0= *lptr; y0= *(lptr+1);
	for(more=TRUE,line=0; more&&line<MAXTXLINES; cdptr->tptr=ptr){
		for(ptr=cdptr->tptr; *ptr!= '\n' && *ptr != EOS; ptr++);
		if(*ptr=='\n') *ptr++=EOS;
		else more=FALSE;
		if((len=strlen(cdptr->tptr))==0 && *ptr==EOS)break;
		cdptr->cnt=(len/2)+1+5;
		putbufe(cdptr);
		if(more){
			y0 -= 1.2 * cdptr->tsiz;
			if(cdptr->trot) gslswing(x0,y0+1.2*cdptr->tsiz,
			  &x0,&y0,cdptr->trot);
			*lptr=x0; *(lptr+1)=y0;
		}
	}
	return(SUCCESS);
}
textmr(cdptr,type)
struct command *cdptr;
char type;  /* 'm' for centered, 'r' for rt-jus  */
{	char *ptr;
	int more,*lptr,len;
	double xsp,ysp,x0,y0;
/*
 *  textmr makes calls to gedtext to produce centered or right-justified
 *  primitives in buffer for strings in cdptr separated by newlines
 */
	xysp(&xsp,&ysp,cdptr->tsiz,cdptr->trot);
	lptr=cdptr->aptr;
	for(x0= *lptr,y0= *(lptr+1),more=TRUE; more; cdptr->tptr=ptr){
		for(ptr=cdptr->tptr; *ptr!= '\n' && *ptr != EOS; ptr++);
		if(*ptr=='\n')*ptr++=EOS;
		else more=FALSE;
		if(type=='m'){
			*lptr=x0-((gslslen(cdptr->tptr)-1.)/2.*xsp);
			*(lptr+1)=y0-((gslslen(cdptr->tptr)-1.)/2.*ysp);
		}else if(type=='r'){
			*lptr=x0-((gslslen(cdptr->tptr)-1.)*xsp);
			*(lptr+1)=y0-((gslslen(cdptr->tptr)-1.)*ysp);
		}
		if((len=strlen(cdptr->tptr))==0 && *ptr==EOS)break;
		cdptr->cnt=(len/2)+1+5;
		putbufe(cdptr);
		if(more){
			y0 -= 1.2 * cdptr->tsiz;
			if(cdptr->trot) gslswing(x0,y0+1.2*cdptr->tsiz,&x0,&y0,cdptr->trot);
		}
	}
}
