/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/glib.d/putgeds.c	1.1"
/* <: t-5 d :> */
#include <stdio.h>
#include "gpl.h"
#include "debug.h"
#include "util.h"
#define STDANG(ang)   ang= ang % 360; if(ang<0) ang +=360
#define PUTB(a,ptr,eptr)  if(ptr>=eptr)return(NULL);else *ptr++ = a
static union cmdword cw;

int *
putgeds(iptr,ieptr,cdptr)
int *iptr,*ieptr;
struct command *cdptr;
{
	char *putswds(),*putssw(),*putsso(),*putstext();
	char *ptr,*eptr;
	ptr=(char *)iptr;
	eptr=(char *)ieptr;
/*
 *  putgeds puts next command from variables in struct command into
 *  buf delim by iptr and ieptr.  Returned pointer points to buf
 *  location after inserted command and is NULL if ieptr is passed
 */
	cw.cmc.cm=cdptr->cmd;  cw.cmc.cn = cdptr->cnt;
	PUTB(cw.cmb.cmbyte1,ptr,eptr);
	PUTB(cw.cmb.cmbyte2,ptr,eptr);
	switch(cw.cmc.cm){
		case ARCS:
		case LINES:
			ptr=putswds(ptr,eptr,cdptr->aptr,cdptr->cnt-2);
			ptr=putssw(ptr,eptr,cdptr);
			break;
		case TEXT:
			ptr=putswds(ptr,eptr,cdptr->aptr,2);
			ptr=putssw(ptr,eptr,cdptr);
			ptr=putsso(ptr,eptr,cdptr);
			ptr=putstext(ptr,eptr,cdptr->tptr,cdptr->cnt-5);
			break;
		case ALPHA:
			ptr=putswds(ptr,eptr,cdptr->aptr,2);
			ptr=putstext(ptr,eptr,cdptr->tptr,cdptr->cnt-3);
			break;
		case COMMENT:
			ptr=putswds(ptr,eptr,cdptr->aptr,cdptr->cnt-1);
			break;
	}
	return((int *)ptr);
}
char *
putswds(ptr,eptr,lptr,words)
char *ptr,*eptr;
int words;
int *lptr;
{
	while(words-- >0){
		cw.cma.pntw = *lptr++;
		PUTB(cw.cmb.cmbyte1,ptr,eptr);
		PUTB(cw.cmb.cmbyte2,ptr,eptr);
	}
	return(ptr);
}
char *
putssw(ptr,eptr,cdptr)
char *ptr,*eptr;
struct command *cdptr;
{	union styleword sw;
	sw.stb.col = cdptr->color;
	if(cdptr->cmd==TEXT) sw.stc.fon=cdptr->font;
	else{
		sw.stb.wt=cdptr->weight; sw.stb.st=cdptr->style;
	}
	PUTB(sw.sta.stbyte1,ptr,eptr);
	PUTB(sw.sta.stbyte2,ptr,eptr);
	return(ptr);
}
char *
putsso(ptr,eptr,cdptr)
char *ptr,*eptr;
struct command *cdptr;
{	union szorient so;
	int ang, i;
	so.szb.ts = (cdptr->tsiz>1279 ? 1279 : cdptr->tsiz) / 5; /* 1279=256*5-1 */
	ang = cdptr->trot;  STDANG(ang);
	i = ang * 256./360. + .5;
	so.szb.tr = i;
	PUTB(so.sza.szbyte1,ptr,eptr);
	PUTB(so.sza.szbyte2,ptr,eptr);
	return(ptr);
}
char *
putstext(ptr,eptr,tptr,words)
char *ptr,*eptr;
int words;
char *tptr;
{
	while(words-- > 0) {
		PUTB(*tptr++,ptr,eptr);
		PUTB(*tptr++,ptr,eptr);
	}
	return(ptr);
}
