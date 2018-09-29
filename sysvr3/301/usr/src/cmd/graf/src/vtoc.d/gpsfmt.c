/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/vtoc.d/gpsfmt.c	1.1"
extern char *nodename;
#include "vtoc.h"
#include "gsl.h"
extern struct node *nd[];
extern int iopt,kopt,copt,dopt;
extern int lastnode;
extern double xsize,ysize,xspace,yspace;
gpsfmt(fpo) 
	FILE *fpo; {
	int i,j,wt,st,rot;
	double x1,y1,x2,y2,halfx,halfy,halfys;
	struct gslparm gsl;
/*
 *  gpsfmt builds a GPS from data in tree structure
 */
	gsl.fp=fpo;	gsl.x0=XYMID;	gsl.y0=XYMID;
	gsl.x=0.;	gsl.y=0.;
	gsl.xs=XUNIT(TXSZ);	gsl.ys=YUNIT(TXSZ);
	initgsl(&gsl);
	rot = 0;
	halfx = xsize/2.;
	halfy = (ysize)/2.;
	halfys = yspace/2.;
	for(i=0;i<=lastnode;i++) {
		x2 = nd[i]->vx;
		y2 = nd[i]->vy;
		x1 = x2 - (2*halfx);
		y1 = y2 - (2*halfy);
		if (!(copt)) upperstr(nd[i]->text);
		if (!(copt)) upperstr(nd[i]->mark);
		textbox(x1,y1,x2,y2,nd[i]->text,0,DFLTCOLOR,
		nd[i]->weight,nd[i]->style,TXSZ,DFLTCOLOR,DFLTFONT);
								/*  textlines in box  */
		if(iopt){
			text((x1),(y2+.5),nd[i]->id,rot,IDTXSZ,DFLTCOLOR,DFLTFONT);
		}
		if(kopt){
			textright(x2,y2+.5,nd[i]->mark,rot,IDTXSZ,DFLTCOLOR,DFLTFONT);
		}

		wt=NARROW; st=SOLID;		/*  for connectors  */
		if (dopt) {
			for (j=0; j < nd[i]->numkid; j++) {
				startlines(x1 + halfx,y1,rot,DFLTCOLOR,wt,st);
				line(nd[i]->kid[j]->vx - halfx,y1 - 2*halfys);
				endlines();
			}
		}
		else {
		if (nd[i]->seq) {		/*  vert line into box top */
			startlines((x2-halfx),y2,rot,DFLTCOLOR,wt,st);
			line(x2-halfx,y2+halfys);
			endlines();
		}
		if (nd[i]->numkid) {
				/*  vert line from bot & hor line connecting kids  */
			startlines(x1+halfx,y1,rot,DFLTCOLOR,wt,st);
			line(x1+halfx,y1-halfys);
			endlines();
			startlines(nd[i]->kid[0]->vx-halfx,y1-halfys,rot,DFLTCOLOR,wt,st);
			line(x1+halfx,y1-halfys);
			endlines();
			startlines(nd[i]->kid[nd[i]->numkid-1]->vx-halfx,y1-halfys,rot,
				   DFLTCOLOR,wt,st);
			line(x1+halfx,y1-halfys);
			endlines();
		}
		}
	}
}
