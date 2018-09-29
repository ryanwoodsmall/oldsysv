/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/td.d/td.c	1.1"
/* <: t-5 d :> */
#include <stdio.h>
#include "setop.h"
#include "chtable"
#include "dev.h"
struct options { 
	int eopt;  /*  erase  */
	int win;  /*  flag for window options ( 'u' or 'r')  */
}op = {TRUE,FALSE};
struct device dv = {
	0,0,1023,767,  /*  viewport  */
	FALSE,  /*  hardware text  */
	FALSE,  /*  hardware arcs  */
	FALSE,	/*  hardware fill  */
	FALSE     /*  hardware buffering  */
};
struct window wn = { 0.};  /*  utodrat, window in univ units  */

char *nodename,string[256];

main(argc,argv)
int argc;
char *argv[];
{	FILE *fpi,*tfpo;
	char *cp,**av;
	int ac,argf;

	int r;
	double dx,dy;
	char *tempfile = "/tmp/dev.XXXXXX";

	nodename= *argv;
	for(ac=argc-1,av=argv+1; ac>0 ;ac--,av++){
		if( *(cp=av[0])=='-' && *++cp!=EOS){
			while(*cp != EOS){
				switch(*cp++){
				case 'u':
					wn.w.lx=wn.w.ly=XYMIN;
					wn.w.hx=wn.w.hy=XYMAX;
					op.win=TRUE;
					break;
				case 'r':  SETINT(r,cp);
					region(r,&dx,&dy);
					wn.w.lx=(int)dx; wn.w.ly=(int)dy;
					wn.w.hx=(int)dx+XDIM; wn.w.hy=(int)dy+YDIM;
					op.win=TRUE;
					break;
			case 'e':  op.eopt = FALSE; break;
				case ' ': case',': break;
				default:
					op.eopt=FALSE;
					ERRPR1(%c?,*(cp-1));
					break;
				}
			}
			--argc;
		}
	}
	devinit();
	if(op.win) compwind(&wn,dv);
	else { 
		mktemp(tempfile);
		if(!(tfpo=fopen(tempfile,"a"))){
			ERRPR1(cannot open %s, tempfile);
			exit(1);
		}
		wn.w.lx=wn.w.ly=XYMAX; wn.w.hx=wn.w.hy=XYMIN;
	}
	if(argc<=1 ) argc=2,argf=0; /* no args,use stdin */
	else argf=1;
	while(argc-->1){
		if(strcmp(*++argv,"-")==0 || !argf) fpi=stdin;
		else if( **argv != '-') {
			if((fpi=fopen(*argv,"r"))==NULL) {
				ERRPR1(cannot open %s, *argv);
				continue;
			}
		} else { argc++; continue; }

		if( getc(fpi)==FCH ) { getc(fpi); 
			if(op.win) dispfile(fpi,table); 
			else savemxmn(fpi,tfpo,&wn.w);
		}
		else ERRPR0(input not GPS);
		fclose(fpi);
	}
	if(!op.win){
		if(fpi=freopen(tempfile,"r",tfpo)) {
			compwind(&wn,dv); 
			dispfile(fpi,table);
		}
		else ERRPR1(cannot open %s,tempfile);
		sprintf(string,"rm %s",tempfile); system(string);
	}
	devfinal();
	exit(0);
}

static
savemxmn(fpi,tfpo,wp)
FILE *fpi,*tfpo;
struct area *wp;
{	struct area ar;
	xymaxmin(fpi,tfpo,&ar);
	wp->lx = MIN(ar.lx,wp->lx);
	wp->ly = MIN(ar.ly,wp->ly);
	wp->hx = MAX(ar.hx,wp->hx);
	wp->hy = MAX(ar.hy,wp->hy);
}
