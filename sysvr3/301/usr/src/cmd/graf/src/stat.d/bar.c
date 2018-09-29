/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/stat.d/bar.c	1.3"
/* <:t-5 d:> */
#include "stdio.h"
#include "s.h"
#include "plot.h"
char *nodename;
#define MAXCNT 200

struct options {
	int axes;	/* if FALSE, no axes */
	int frame;	/* if TRUE, frame plot */
	int grid;	/* if TRUE, background grid */
	int bold;	/* if TRUE, plot in bold */
	int xax;	/* if FALSE, no x-axis */
	int yax;	/* if FALSE, no y-axis */
	double ylow;	/* if TRUE, use user's low value */
	double yhigh;	/* if TRUE, use user's high value */
	int width; 	/* bar width in percent to class interval */
} op = { 1,1,1,0,1,1,UNDEF,UNDEF,50};

main(argc,argv) /*set-up for bar chart*/
int argc;
char *argv[];
{
	FILE *fdi;
	int ac, argf, r;
	char **av, *cp;

	static struct gslparm gp = { DFLTGP };

	nodename = *argv;
	for( ac=argc-1,av=argv+1; ac>0; ac--,av++ )
		if( *(cp=av[0])=='-' && *++cp!='\0' ) {
			while( *cp!='\0' )
				switch(*cp++) {
				case 'a': op.axes=0; break;
				case 'b': op.bold=1; break;
				case 'f': op.frame=0; break;
				case 'g': op.grid=0; break;
				case 'x':
					switch(*cp++) {
					case 'a': op.xax=0; break;
					default: cp--; SETDOUB(gp.x0,cp);
					}; break;
				case 'y':
					switch(*cp++) {
					case 'a': op.yax=0; break;
					case 'l': SETDOUB(op.ylow,cp); break;
					case 'h': SETDOUB(op.yhigh,cp); break;
					default: cp--; SETDOUB(gp.y0,cp);
					}; break;
				case 'r': SETINT(r,cp);
					region(r,&gp.x0,&gp.y0);
					gp.x0 += XLTRIM;
					gp.y0 += YLTRIM;
					break;
				case 'w': SETINT(op.width,cp); break;
				case ' ': case ',': break;
				default: ERRPR1(%c?,*(cp-1));
				}
			--argc;
		}
	if(argc <= 1) { argc=2; argf=0; } /* no args, use stdin */
	else argf=1;

	while(argc-- > 1) {
		if( strcmp(*++argv,"-")==0 || !argf ) fdi=stdin;
		else if( **argv!='-' ) {
			if( (fdi=fopen(*argv,"r"))==NULL ) {
				ERRPR1(cannot open %s, *argv);
				continue;
			}
		} else { argc++; continue; }

		bar(fdi,&gp);
		fclose(fdi);
	}
	exit(0);
}

#define ROUND(x) (int) ((x>0) ? x+.5 : x-.5)
#define TICKS 10
#define FONT DFLTFONT
#define COLOR DFLTCOLOR
#define DPU 100.	/* division per unit */
#define MAXNUM 15	/* max # of cnts on y-axis */
#define XMAR 100.
#define RX(offset) (gp->x+offset)
#define RY(offset) (gp->y+offset)
#define WIDTH (op.width/DPU)	/* bar width */
bar(fdi,gp) /*build virtual bar chart*/
FILE *fdi;
struct gslparm *gp;
{
	int i, inc, ncnts, weight, cnt[MAXCNT], maxcnt= -32678, mincnt=32677;
	int sense, ymar=200, ndigits=1;
	double d, x, y, xd, yd, dmax, dmin, dinc;
	char label[MAXLABEL];

	getlabel(fdi,label,MAXLABEL);
		/* read in cnts to label y-axis */
	for(ncnts=0; ncnts<MAXCNT && !feof(fdi); ncnts++){
		getdoub(fdi, &x);
		cnt[ncnts] = ROUND(x);
		maxcnt = MAX(maxcnt,cnt[ncnts]);
		mincnt = MIN(mincnt,cnt[ncnts]);
		}
 
	if(maxcnt - mincnt > TICKS && !DEF(op.ylow) && !DEF(op.yhigh)) {
		scale(DEF(op.ylow) ? op.ylow+.5 : (doub) mincnt,
			DEF(op.yhigh) ? op.yhigh+.5 : (doub) maxcnt, TICKS,
			&dmin, &dmax, &dinc);
		maxcnt = ROUND(dmax);
		mincnt = ROUND(dmin);
		inc = ROUND(dinc);
	}
	else {
		if(DEF(op.ylow)) mincnt = ROUND(op.ylow);
		if(DEF(op.yhigh))  { ymar=0; maxcnt = ROUND(op.yhigh); }
		inc = (maxcnt - mincnt)/MAXNUM+1;
	}

	xd = DPU*(ncnts-2)+2.*XMAR;
	gp->xs = XSIZE/xd;
	yd = DPU*(maxcnt - mincnt)+ymar;
	gp->ys = YSIZE/yd;

	initgsl(gp);
	if(op.axes) { /* generate axes */
		char s[20]; /* temp for label generation */
		if(op.frame) box(0.,0.,xd,yd,0,COLOR,NARROW,SOLID);
		else {  startlines(xd,0.,0,COLOR,NARROW,SOLID); 
			line(0.,0.);
			line(0.,yd);
			endlines();
		}

		if(op.xax) { /* generate x labels */
			for(i=0; i<ncnts-1; i++) {
				sprintf(s,"%d",i+1);
				textcent(DPU*i+XMAR,-CHEIGHT/gp->ys,s,0,SCALE,COLOR,FONT);
			}
		}
		if(op.yax) { /* generate y labels */
			for(i=mincnt; i<=maxcnt+ymar/DPU; i+=inc) {	
				sprintf(s,"%d",i);
				textright(-CWIDTH/gp->xs,DPU*(i-mincnt),s,0,SCALE,COLOR,FONT);
			}
		}
	}
	if( (sense=op.grid)||op.axes )  { /* generate background grid */
		int style;
		style = (sense ? DOTTED : SOLID);
		for(i=mincnt+inc; i<maxcnt+ymar/DPU; i+=inc) {
			startlines(0.,DPU*(i-mincnt),0,COLOR,NARROW,style);
			line((sense ? RX(xd) : CHEIGHT/gp->xs),RY(0));
			endlines();
		}
	}
	if(strlen(label) > 0) {
		for(i=maxcnt; (i=i/10)>0; ndigits++);
		textcent(-ndigits*CHEIGHT/gp->xs,yd/2,label,90,SCALE,COLOR,FONT);
	}


	weight = (op.bold ? BOLD : MEDIUM);
	for(d=XMAR, i=0; i<ncnts-1; d=d+DPU, i++) {
		y = clip(cnt[i],maxcnt,mincnt);
		startlines((d-DPU*(WIDTH/2.)),0.,0,COLOR,weight,SOLID);
		if(cnt[i] >= mincnt)	line(RX(0),RY(DPU*(y-mincnt)));
		if(cnt[i]<=maxcnt && cnt[i]>=mincnt) /* draw horizontal */
			line(RX(DPU*WIDTH),RY(0));
		else {
			endlines();
			startlines(RX(DPU*WIDTH),RY(0),0,COLOR,weight,SOLID);
		}
		if(cnt[i] >= mincnt)	line(RX(0),0.);
		endlines();
	}
}

clip(val,high,low) /* return val within high to low */
int val, high, low;
{
	if( val>high ) return high;
	else if( val<low) return low;
	else return val;
}
