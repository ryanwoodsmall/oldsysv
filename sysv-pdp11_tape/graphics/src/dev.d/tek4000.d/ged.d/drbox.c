static char SCCSID[]="@(#)drbox.c	1.1";
/* <: t-5 d :> */
#include <stdio.h>
#include "../include/termodes.h"
#include "ged.h"

drbox(arptr,color,lw,ls)
int color,lw,ls;
struct area *arptr;
{	struct command cd;
	int *ptr;
/*
 *  drbox draws rectangle of given color, weight, and style from
 *  two points which define a diagonal
 */
	cd.color=color; cd.weight = lw;  cd.style = ls;
	cd.cmd = LINES;
	ptr=cd.aptr=cd.array;
	*ptr++ = arptr->lx;  *ptr++ = arptr->ly;
	*ptr++ = arptr->hx;  *ptr++ = arptr->ly;
	*ptr++ = arptr->hx;  *ptr++ = arptr->hy;
	*ptr++ = arptr->lx;  *ptr++ = arptr->hy;
	*ptr++ = arptr->lx;  *ptr++ = arptr->ly;
	cd.cnt=ptr-cd.aptr+2;
	setmode(GRAPH); teklines(&cd); /* output lines */
	restoremode();
	return;
}

drx(arptr,color,lw,ls)
int color,lw,ls;
struct area *arptr;
{	struct command cd;
	int *ptr;
/*
 *  drx draws diagonals between four points in arptr
 *  with given color, weight, and style
 */
	cd.color=color; cd.weight = lw; cd.style = ls;
	cd.cmd=LINES;
	ptr=cd.aptr=cd.array;
	*ptr++ = arptr->lx;  *ptr++ = arptr->ly;
	*ptr++ = arptr->hx;  *ptr++ = arptr->hy;
	cd.cnt=ptr-cd.aptr+2;
	setmode(GRAPH); teklines(&cd);
	ptr=cd.array;
	*ptr++ = arptr->lx;  *ptr++ = arptr->hy;
	*ptr++ = arptr->hx;  *ptr++ = arptr->ly;
	cd.cnt=ptr-cd.aptr+2;
	teklines(&cd); restoremode();
	return;
}
