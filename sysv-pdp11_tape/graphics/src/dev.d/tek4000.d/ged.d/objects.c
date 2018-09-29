static char SCCSID[]="@(#)objects.c	1.1";
/* <: t-5 d :> */
#include <stdio.h>
#include "ged.h"

objects(arptr,dbuf,endbuf)
struct area *arptr;
int *dbuf, *endbuf;
{
	struct command cd;
	int x, y, *getgeds();
/*
 *  objects reads buffer and labels objects for 
 *  any commands that fall within defined area 
 */
	while( (dbuf=getgeds(dbuf,endbuf,&cd))!=NULL ) {
		if(cd.cmd==COMMENT) continue;
		if( inarea(x=(*cd.aptr), y=(*(cd.aptr+1)), arptr) ) {
			unscaleoff(&x,&y);
			putcraw('O',x,y);
		}
	}
	return;
}
