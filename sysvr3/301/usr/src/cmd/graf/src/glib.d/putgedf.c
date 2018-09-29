/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/glib.d/putgedf.c	1.1"
/* <: t-5 d :> */
#include <stdio.h>
#include "gpl.h"
#include "debug.h"
#include "util.h"
#define STDANG(ang)   ang= ang % 360; if(ang<0) ang +=360
static union cmdword cw;

putgedf(fpo,cdptr)
FILE *fpo;
struct command *cdptr;
{
/*
 *  putgedf puts a ged command into file designated by fpo
 *  from values in struct command
 */
	cw.cmc.cm=cdptr->cmd;  cw.cmc.cn = cdptr->cnt;
	putc(cw.cmb.cmbyte1,fpo);
	putc(cw.cmb.cmbyte2,fpo);
	switch(cw.cmc.cm){
		case ARCS:
		case LINES:
			putfwds(fpo,cdptr->aptr,cdptr->cnt-2);
			putfsw(fpo,cdptr);
			break;
		case TEXT:
			putfwds(fpo,cdptr->aptr,2);
			putfsw(fpo,cdptr);
			putfso(fpo,cdptr);
			putftext(fpo,cdptr->tptr,cdptr->cnt-5);
			break;
		case ALPHA:
			putfwds(fpo,cdptr->aptr,2);
			putftext(fpo,cdptr->tptr,cdptr->cnt-3);
			break;
		case COMMENT:
			putfwds(fpo,cdptr->aptr,cdptr->cnt-1);
			break;
	}
	return(SUCCESS);
}
putfwds(fpo,lptr,words)
FILE *fpo;
int words;
int *lptr;
{
	while(words-- > 0){
		cw.cma.pntw = *lptr++;
		putc(cw.cmb.cmbyte1,fpo);
		putc(cw.cmb.cmbyte2,fpo);
	}
	return;
}
putfsw(fpo,cdptr)
FILE *fpo;
struct command *cdptr;
{	union styleword sw;
	sw.stb.col = cdptr->color;
	if(cdptr->cmd==TEXT) sw.stc.fon=cdptr->font;
	else{
		sw.stb.wt=cdptr->weight; sw.stb.st=cdptr->style;
	}
	putc(sw.sta.stbyte1,fpo);
	putc(sw.sta.stbyte2,fpo);
	return;
}
putfso(fpo,cdptr)
FILE *fpo;
struct command *cdptr;
{	union szorient so;
	int ang, i;
	so.szb.ts = (cdptr->tsiz>1279 ? 1279 : cdptr->tsiz) / 5; /* 1279=256*5-1 */
	ang = cdptr->trot;  STDANG(ang);
	i = ang * 256./360. + .5;
	so.szb.tr = i;
	putc(so.sza.szbyte1,fpo);
	putc(so.sza.szbyte2,fpo);
	return;
}
putftext(fpo,ptr,words)
FILE *fpo;
int words;
char *ptr;
{
	while(words-- > 0){
		putc(*ptr++,fpo);
#if u370
		if(*(ptr - 1) == EOS) putc(EOS, fpo);
		else putc(*ptr++, fpo);
#else
		putc(*ptr++,fpo);
#endif
	}
	return;
}
