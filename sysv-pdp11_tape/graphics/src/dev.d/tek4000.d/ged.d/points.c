static char SCCSID[]="@(#)points.c	1.1";
/* <: t-5 d :> */
#include <stdio.h>
#include "ged.h"

points(arptr,dbuf,endbuf)
struct area *arptr;
int *dbuf, *endbuf;
{
	struct command cd;
	int x, y, cnt, *lptr, *getgeds();
/*
 *  points reads buffer and labels any points within area 
 */
	  while( (dbuf=getgeds(dbuf,endbuf,&cd))!=NULL ) {
		if(cd.cmd==COMMENT) continue;
		lptr=cd.aptr;
		if( inarea(x= *lptr, y= *(lptr+1), arptr) ) {
			unscaleoff(&x,&y);
			putcraw('O',x,y);
		}
		if( cd.cmd==ARCS || cd.cmd==LINES ) {
			for( cnt=cd.cnt-4,lptr+=2; cnt>0; cnt-=2,lptr+=2) {
				if( inarea(x= *lptr,y= *(lptr+1),arptr) ) {
					unscaleoff(&x,&y);
					putcraw('P',x,y);
				}
			}
		}
	}
	return;
}
