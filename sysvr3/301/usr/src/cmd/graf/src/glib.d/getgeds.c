/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/glib.d/getgeds.c	1.1"
/* <: t-5 d :> */
#include <stdio.h>
#include "gpl.h"
#include "util.h"
#include "debug.h"
#define GETB(a,ptr,eptr)  if(ptr>=eptr)return(NULL);else a = *ptr++
static union cmdword cw;

int *
getgeds(iptr,ieptr,cdptr)
int *iptr,*ieptr;
struct command *cdptr;
{
	char *getswds(),*getssw(),*getsso(),*getstext();
	char *ptr,*eptr;
	ptr=(char *)iptr;
	eptr=(char *)ieptr;
/*
 *  getgeds gets next command from the string delimited by ptr 
 *  and eptr and places values in variables in struct command
 *  returned pointer points to character after command
 */
	cdptr->aptr=cdptr->array;
	GETB(cw.cmb.cmbyte1,ptr,eptr);
	GETB(cw.cmb.cmbyte2,ptr,eptr);
	cdptr->cnt=cw.cmc.cn;   cdptr->cmd=cw.cmc.cm;  
	switch(cw.cmc.cm){
		case ARCS:
		case LINES:
			ptr=getswds(ptr,eptr,(cdptr->aptr=cdptr->array),cdptr->cnt-2);
			ptr=getssw(ptr,eptr,cdptr);
			break;
		case TEXT:
			ptr=getswds(ptr,eptr,(cdptr->aptr=cdptr->array),2);
			ptr=getssw(ptr,eptr,cdptr);
			ptr=getsso(ptr,eptr,cdptr);
			ptr=getstext(ptr,eptr,(cdptr->tptr=cdptr->text),cdptr->cnt-5);
			break;
		case ALPHA:
			ptr=getswds(ptr,eptr,(cdptr->aptr=cdptr->array),2);
			ptr=getstext(ptr,eptr,(cdptr->tptr=cdptr->text),cdptr->cnt-3);
			break;
		case COMMENT:
			ptr=getswds(ptr,eptr,(cdptr->aptr=cdptr->array),cdptr->cnt-1);
			break;
	}
	return((int *)ptr);
}
char *
getswds(ptr,eptr,lptr,words)
char *ptr,*eptr;
int words;
int *lptr;
{
	while(words-->0){
		cw.cma.pntw = 0;
		GETB(cw.cmb.cmbyte1,ptr,eptr);
		GETB(cw.cmb.cmbyte2,ptr,eptr);
		if ((*lptr = cw.cma.pntw) >= LIMIT)
			*lptr |= ~0177777;
		lptr++;
	}
	return(ptr);
}
char *
getssw(ptr,eptr,cdptr)
char *ptr,*eptr;
struct command *cdptr;
{	union styleword sw;
	GETB(sw.sta.stbyte1,ptr,eptr);
	GETB(sw.sta.stbyte2,ptr,eptr);
	cdptr->color=sw.stb.col;
	if(cdptr->cmd==TEXT) cdptr->font=sw.stc.fon;
	else{
		cdptr->weight=sw.stb.wt;  cdptr->style=sw.stb.st;
	}
	return(ptr);
}
char *
getsso(ptr,eptr,cdptr)
char *ptr,*eptr;
struct command *cdptr;
{	union szorient so;
	GETB(so.sza.szbyte1,ptr,eptr);
	GETB(so.sza.szbyte2,ptr,eptr);
	cdptr->tsiz=so.szb.ts * 5;
	cdptr->trot= so.szb.tr * 360./256. + .5;
	return(ptr);
}
char *
getstext(ptr,eptr,tptr,words)
char *ptr,*eptr;
char *tptr;
int words;
{
	while(words-- > 0) {
		GETB(*tptr++,ptr,eptr);
		GETB(*tptr++,ptr,eptr);
	}
	return(ptr);
}
