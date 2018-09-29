/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/glib.d/getgedf.c	1.1"
/* <: t-5 d :> */
#include <stdio.h>
#include "gpl.h"
#include "util.h"
#include "debug.h"
static union cmdword cw;

getgedf(fpi,cdptr)
FILE *fpi;
struct command *cdptr;
{
/*
 *  getgedf gets the next ged command  from a file
 *  and places values in variables of struct command
 */
	short ch;

	if ((ch=getc(fpi)) == EOF) return(EOF);
	cw.cmb.cmbyte1 = ch;
	cw.cmb.cmbyte2=getc(fpi);
	cdptr->cnt=cw.cmc.cn;   cdptr->cmd=cw.cmc.cm;  
	switch(cw.cmc.cm){
		case ARCS:
		case LINES:
			getfwds(fpi,(cdptr->aptr=cdptr->array),cdptr->cnt-2);
			getfsw(fpi,cdptr);
			break;
		case TEXT:
			getfwds(fpi,(cdptr->aptr=cdptr->array),2);
			getfsw(fpi,cdptr);
			getfso(fpi,cdptr);
			getftext(fpi,(cdptr->tptr=cdptr->text),cdptr->cnt-5);
			break;
		case ALPHA:
			getfwds(fpi,(cdptr->aptr=cdptr->array),2);
			getftext(fpi,(cdptr->tptr=cdptr->text),cdptr->cnt-3);
			break;
		case COMMENT:
			getfwds(fpi,(cdptr->aptr=cdptr->array),cdptr->cnt-1);
			break;
	}
	return(SUCCESS);
}
getfwds(fpi,lptr,words)
FILE *fpi;
int words;
int *lptr;
{
	while(words-- > 0){
		cw.cma.pntw = 0;
		cw.cmb.cmbyte1=getc(fpi);
		cw.cmb.cmbyte2=getc(fpi);
		if ((*lptr = cw.cma.pntw) >= LIMIT)
			*lptr |= ~0177777;
		lptr++;
	}
	return;
}
getfsw(fpi,cdptr)
FILE *fpi;
struct command *cdptr;
{	union styleword sw;
	sw.sta.stbyte1=getc(fpi);
	sw.sta.stbyte2=getc(fpi);
	cdptr->color=sw.stb.col;
	if(cdptr->cmd==TEXT) cdptr->font=sw.stc.fon;
	else{
		cdptr->weight=sw.stb.wt;  cdptr->style=sw.stb.st;
	}
	return;
}
getfso(fpi,cdptr)
FILE *fpi;
struct command *cdptr;
{	union szorient so;
	so.sza.szbyte1=getc(fpi);
	so.sza.szbyte2=getc(fpi);
	cdptr->tsiz=so.szb.ts * 5;
	cdptr->trot= so.szb.tr * 360./256. + .5;
	return;
}
getftext(fpi,ptr,words)
FILE *fpi;
char *ptr;
int words;
{
	while(words-- > 0){
		*ptr++=getc(fpi);
		*ptr++=getc(fpi);
	}
	return;
}
