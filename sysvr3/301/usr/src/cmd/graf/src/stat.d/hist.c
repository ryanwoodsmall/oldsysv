/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/stat.d/hist.c	1.3"
/* <:t-5 d:> */
#include "stdio.h"
#include "s.h"
#include "plot.h"
char *nodename;
#define MAXLIMS 200

struct options {
	short axes;	/* if FALSE, no axes */
	short bold;	/* if TRUE, plot in bold */
	short frame;	/* if TRUE, frame plot */
	short grid;	/* if TRUE, draw background grid */
	short xax;	/* if FALSE, no x-axis */
	short yax;	/* if FALSE, no y-axis */
	double ylow;	/* use user's low value */
	double yhigh;	/* use user's high value */
} op = { 1,0,1,1,1,1,UNDEF,UNDEF };

main(argc,argv) /*set-up for hist*/
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
				case ' ': case ',': break;
				default: ERRPR1(%c?,*(cp-1));
				}
			--argc;
		}

	if(argc <= 1)  argc=2, argf=0;  /* no args, use stdin */
	else argf=1;

	while(argc-- > 1) {
		if( strcmp(*++argv,"-")==0 || !argf ) fdi=stdin;
		else if( **argv!='-' ) {
			if( (fdi=fopen(*argv,"r"))==NULL ) {
				ERRPR1(cannot open %s, *argv);
				continue;
			}
		} else { argc++; continue; }

		hist(fdi,&gp);
		fclose(fdi);
	}
	exit(0);
}

#define TICKS 10
#define FONT DFLTFONT
#define COLOR DFLTCOLOR
#define HIGH (maxcnt+(int)YMAR)
#define LOW (mincnt)
#define DPU 2.
#define XMAR 1.
#define YMAR 2.
#define MAXPLIM 15
#define MAXPCNT 15
#define RX(offset) (gp->x+offset)
#define RY(offset) (gp->y+offset)

hist(fdi,gp) /*build virtual histogram*/
FILE *fdi;
struct gslparm *gp;
{
	int i, nlims, cnt[MAXLIMS], weight, maxcnt=YMAR, mincnt=0;
	int j, inc, y0, y1, sense;
	double dmax, dmin, dinc;
	double limit[MAXLIMS], xd, yd;
	char label[MAXLABEL];

	getlabel(fdi,label,MAXLABEL);
	getdoub(fdi,&limit[0]);
	for(nlims=1; getint(fdi,&cnt[nlims])!=EOF && (nlims<=MAXLIMS); nlims++) {
		maxcnt = MAX(maxcnt,cnt[nlims]);
		getdoub(fdi,&limit[nlims]);
	}
	nlims--;

	if((maxcnt - (DEF(op.ylow)?(int)(op.ylow+.5):0)) > TICKS &&
	!DEF(op.ylow) && !DEF(op.yhigh)) {
		scale(DEF(op.ylow) ? op.ylow+.5 : 0.,
			DEF(op.yhigh) ? op.yhigh+.5 : (doub) maxcnt, TICKS,
			&dmin, &dmax, &dinc);
		maxcnt = (int) (dmax+.5);
		mincnt = (int) (dmin+.5);
		inc = (int) (dinc+.5);
	}
	else {
		if(DEF(op.yhigh))
			maxcnt = (i= op.yhigh-YMAR+.5)>0 ? i : 0;
		if(DEF(op.ylow))	
			mincnt = (op.ylow>0) ? op.ylow+.5 : 0;
		inc = ((i=HIGH-YMAR - LOW) > 0 ? i : 0)/MAXPCNT + 1;
	}

	xd = DPU*(nlims+XMAR);
	gp->xs = XSIZE/xd;
	yd = DPU*(HIGH-LOW);
	gp->ys = YSIZE/yd;

	initgsl(gp);
	if(op.axes) { /* generate axes */
		char s[20]; /* temp for label conversion */
		if(op.frame) box(0.,0.,xd,yd,0,COLOR,NARROW,SOLID);
		else { startlines(xd,0.,0,COLOR,NARROW,SOLID); 
			line(0.,0.);
			line(0.,yd);
			endlines();
		}

		if(op.xax) { /* generate x labels */
			j = nlims/MAXPLIM + 1;
			for(i=0; i<=nlims; i+=j) {
				sprintf(s,"%.3g",limit[i]);
				textcent(DPU*i+1,-CHEIGHT/gp->ys,s,0,SCALE,COLOR,FONT);
			}
		}
		if(op.yax) { /* generate y labels */
			for(i=LOW; i<=HIGH; i+=inc) {	
				sprintf(s,"%d",i);
				textright(-CWIDTH/gp->xs,DPU*(i-LOW),s,0,SCALE,COLOR,FONT);
			}
		}
	}
	if( (sense=op.grid)||op.axes ) { /* generate background grid */
		int style;
		style = (sense ? DOTTED : SOLID);

		for( i=inc; i<HIGH-LOW; i+=inc ) {
			startlines(0.,DPU*i,0,COLOR,NARROW,style);
			line((sense ? RX(xd) : CHEIGHT/gp->xs),RY(0.));
			endlines();
		}
	}
	if(strlen(label) > 0)
		textcent(xd/2,-4*CHEIGHT/gp->ys,label,0,SCALE,COLOR,FONT);

	weight = (op.bold ? BOLD : MEDIUM);
	move(XMAR,0.,0); /* plot histogram */
	y0 = cnt[0] = LOW;
	for( i=1; i<=nlims; i++,y0=y1 ) {

		y1 = clip(cnt[i],HIGH,LOW); /* determine start y-value */
		if( y1>y0 ) {
			startlines(RX(0),RY(DPU*(y0-LOW)),0,COLOR,weight,SOLID);
			line(RX(0),RY(DPU*(y1-y0)));
		} else	startlines(RX(0),RY(DPU*(y1-LOW)),0,COLOR,weight,SOLID);

		if( cnt[i]<=HIGH && cnt[i]>=LOW ) /* draw horizontal */
			line(RX(DPU),RY(0));
		else {
			endlines();
			startlines(RX(DPU),RY(0),0,COLOR,weight,SOLID);
		}

		if(cnt[i] >= LOW)  line(RX(0),0.); /* draw descender */
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
