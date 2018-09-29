/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/hp7220.d/hpd.d/devinit.c	1.2"
#include <stdio.h>
#include "hpd.h"
#include "dev.h"
extern struct device dv;
extern struct options {
	int eopt;  /*  erase  */
	int slant;  /* character slant  */
	int achset ;  /* select alternate char set  */
	int pen;  /*  select pen  */
	int win;  /*  flag for window options ( 'u' or 'r')  */
}op;

devinit()
{	PLOTON;
	if(op.pen){PENSEL; sbn(op.pen);}
	SETGLIM;
	mbp(dv.v.lx,dv.v.ly); mbp(dv.v.hx,dv.v.hy); 
	TERMINATOR;
	dv.v.hx -= dv.v.lx; dv.v.hy -= dv.v.ly;
	dv.v.lx = 0; dv.v.ly = 0;
	SETGRIDSZ;
	mbp(dv.v.hx,dv.v.hy);
	TERMINATOR;
	ROTATE;   /*  init ang for text rotation to 0  */
	if(op.slant) { LABSLANT; mba(op.slant+90.);}
	if(op.achset) { LABCHSET; mbp(op.achset,0);}
}
