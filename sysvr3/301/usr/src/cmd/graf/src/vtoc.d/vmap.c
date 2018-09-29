/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/vtoc.d/vmap.c	1.1"
extern char *nodename;
#include"vtoc.h"
extern struct node *nd[];
extern int lastnode;
extern double ysize,xsize,yspace,xspace;

vmap() {
	int j,maxlev = 0,maxindx = 0, maxwd =0;
	double facx,facy,maxvx,maxvy;
/*
 *  vmap translates node relative hor positions
 *  and levels into virtual unit positions.  These
 *  coordinates allow for box dimensioning and
 *  spacing.  vmap also sets maxvx and maxvy
 *  and determines size by fitting longest textstring
 *  into a golden ratio box.
 #  assert lastnode betw 0,MAXNODE incl
 */
	maxvx = maxvy = 0.0;
	for(j=0;j<=lastnode;j++) {
		maxlev = MAX(maxlev,nd[j]->level);
		maxindx = (strlen(nd[maxindx]->text)  >
			strlen(nd[j]->text) ? maxindx : j);
		maxwd = MAX(maxwordlen(nd[j]->text),maxwd);
	}
	sizefac(maxindx,maxwd,&xsize,&ysize);
	xspace*=xsize;
	facx = (xspace/2 +xsize/2);	/* halve to compact x dir  */
	yspace*=ysize;
	facy = yspace +ysize;
	for(j=0; j <= lastnode; j++) {
		nd[j]->vx = (nd[j]->rx)*facx;
		maxvx = MAX(maxvx,nd[j]->vx);
		nd[j]->vy = (double)(maxlev-((nd[j]->level)*facy));
		maxvy = MAX(maxvy,nd[j]->level);
	}
	for(j=0;j<=lastnode;j++){	/*  center in virtual space  */
		nd[j]->vx -= maxvx/2;
		nd[j]->vy -= maxvy/2;
	}
}
