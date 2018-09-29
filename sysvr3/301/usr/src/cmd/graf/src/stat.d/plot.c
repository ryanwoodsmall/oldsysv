/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/stat.d/plot.c	1.4"
/* <:t-5 d:> */
#include "stdio.h"
#include "s.h"
#include "plot.h"
char *nodename;
#define TICKS 7
#define MAXF 10
#define MAXEL 1001
#define MAXLAB MAXLABEL+14

struct options {
	short axes;	/* if FALSE, no axes */
	short bold;	/* if 5-9, plot in bold */
	short conn;	/* if TRUE, connect plot points */
	short frame;	/* if TRUE, frame plot */
	short grid;	/* if FALSE, no background grid */
	short mark;	/* if TRUE, mark plot points */
	short multi;	/* if TRUE, multiline plot */
	short xax;	/* if FALSE, no x-axis */
	short yax;	/* if FALSE, no y-axis */
	short xtit, ytit;	/* if FALSE, no axis title */
	int xn,yn;	/* number of ticks per axis */
	doub xi,xl,xh;	/* x -interval, -low, -high */
	doub yi,yl,yh;	/* y -interval, -low, -high */
	char pc[STYLES];	/* plotting chars */
	int fmax;	/* max index into f array */
	FILE *f[MAXF];	/* array for x-axis files */
}  op = {
		1,0,1,1,1,0,0,1,1,1,1,TICKS,TICKS,
		UNDEF,UNDEF,UNDEF,UNDEF,UNDEF,UNDEF,
		"0123456789",
		-1
	};

main(argc,argv) /* set-up for plot */
int argc;
char *argv[];
{
	FILE *fdi;
	int ac, argf, i;
	char **av, *cp;

	double *filbuf1();
	double xbuf[MAXEL], ybuf[MAXEL], *xep, *yep;
	char xlab[MAXLAB], ylab[MAXLAB];
	int r, fx= -1;

	static struct gslparm gp = { DFLTGP };

	nodename = *argv;
	for( ac=argc-1,av=argv+1; ac>0; ac--,av++ )
		if( *(cp=av[0])=='-' && *++cp!='\0' ) {
			while( *cp!='\0' )
				switch(*cp++) {
				case 'a': op.axes=0; break;
				case 'b': op.bold=5; break;
				case 'd': op.conn=op.bold=0; op.mark=1; break;
				case 'g': op.grid=0; break;
				case 'c': SETSTR(op.pc,cp,10); op.mark=1; break;
				case 'f': op.frame=0; break;
				case 'm': op.mark=1; break;
				case 'F': SETFILE(op.f[++op.fmax],cp); break;
				case 'x':
					switch(*cp++) {
					case 'a': op.xax = 0; break;
					case 'i': SETDOUB(op.xi,cp); break;
					case 'n': SETINT(op.xn,cp); break;
					case 'l': SETDOUB(op.xl,cp); break;
					case 'h': SETDOUB(op.xh,cp); break;
					case 't': op.xtit = 0; break;
					default: cp--; SETDOUB(gp.x0,cp);
					} break;
				case 'y':
					switch(*cp++) {
					case 'a': op.yax = 0; break;
					case 'i': SETDOUB(op.yi,cp); break;
					case 'n': SETINT(op.yn,cp); break;
					case 'l': SETDOUB(op.yl,cp); break;
					case 'h': SETDOUB(op.yh,cp); break;
					case 't': op.ytit = 0; break;
					default: cp--; SETDOUB(gp.y0,cp);
					} break;
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

	if(argc>2) op.multi=1; /* multiline plot */

	if( op.fmax<0 ) { /* generate x-axis */
		double d, low, inc;
		inc = DEF(op.xi) ? op.xi : 1;
		low = DEF(op.xl) ? op.xl : 0;
		for( i=0,d=low; i<MAXEL; d=(++i)*inc+low )
			xbuf[i] = d;
		xep = xbuf+MAXEL-1;
		*xlab = '\0';
	}

	if(argc == 1)  argc=2, argf=0;  /*no args, use stdin*/
	else argf=1;

	while( argc-- > 1 ) {
		if( strcmp(*++argv,"-")==0 || !argf ) fdi=stdin;
		else if( **argv!='-' ) {
			if( (fdi=fopen(*argv,"r"))==NULL ) {
				ERRPR1(cannot open %s, *argv);
				continue;
			}
		} else { argc++; continue; }

		if( (op.fmax-fx)>0 ) { /* get x vector */
			getlabel(op.f[++fx],xlab,MAXLAB);
			xep = filbuf1(op.f[fx],xbuf,MAXEL);
		}

		getlabel(fdi,ylab,MAXLAB);
		yep = filbuf1(fdi,ybuf,MAXEL);
		if( op.fmax<0 ) xep = xbuf+(yep-ybuf);
		plot(&gp,xbuf,xep,ybuf,yep,xlab,ylab);
		fclose(fdi);
	}
	exit(0);
}



#define COLOR DFLTCOLOR
#define FONT DFLTFONT

struct window {
	double xlow;
	double xhigh;
	double ylow;
	double yhigh;
} wn = { 0., 0., 0., 0. };

plot(gp,xp,xep,yp,yep,xlab,ylab) /* plot graph */
double *xp, *xep, *yp, *yep;
char *xlab, *ylab;
struct gslparm *gp;
{
int start=0, end=0;
	double val, cx1, cy1, cx2, cy2;
	double *beg;
	int i, sense, clip1=0, clip2=0;
	char *ctos();
	static double xd, yd, xmin, xmax, xint, ymin, ymax, yint;
	static int pen= -1, maxychs=0,  ls[STLTYPES]={ SOLID, DOTTED, DOTDASH, 
			DASHED, LONGDASH };
	static char *lsid[2][STYLES]={
		{ " (0)", " (1)", " (2)", " (3)", " (4)", " (5)",
		 " (6)", " (7)", " (8)", " (9)" },
		{ " (solid)", " (dotted)", " (dot dashed)", " (dashed)", " (long dashed)",
		" (solid)", " (dotted)", " (dot dashed)", " (dashed)", " (long dashed)" }
	};

	if(++pen == 0) { /* first time only */
		minmax(xp,xep,&xmin,&xmax); /* determine axes scale */
		if(DEF(op.xi)) {
			xint = op.xi;
			if(xint <= 0.0) { ERRPR0(x axis increment must be > 0); return(0); }
			xmin = (DEF(op.xl) ? op.xl : xmin);
			xmax = (DEF(op.xh) ? op.xh : xmax);
		}
		else scale(DEF(op.xl)?op.xl:xmin,DEF(op.xh)?op.xh:xmax,
			op.xn,&xmin,&xmax,&xint);
		if((xd = xmax-xmin) == 0) xd = 1;
		gp->xs = XSIZE/xd;
		wn.xlow = xmin;
		wn.xhigh = xmax;
		minmax(yp,yep,&ymin,&ymax);
		if(DEF(op.yi)) {
			yint = op.yi;
			if(yint <= 0.0) { ERRPR0(y axis increment must be > 0); return(0); }
			ymin = (DEF(op.yl) ? op.yl : ymin);
			ymax = (DEF(op.yh) ? op.yh : ymax);
		}
		else scale(DEF(op.yl)?op.yl:ymin,DEF(op.yh)?op.yh:ymax,
			op.yn,&ymin,&ymax,&yint);
		if((yd = ymax-ymin) == 0) yd = 1;
		gp->ys = YSIZE/yd;
		wn.ylow = ymin;
		wn.yhigh = ymax;
	
		initgsl(gp);
		if(op.axes) { /* build axes */
			char s[20]; /* temp for label conversions */
			if(op.frame)box(0.,0.,xd,yd,0,COLOR,NARROW,SOLID);
			else {
				startlines(xd,0.,0,COLOR,NARROW,SOLID);
				line(0.,0.);
				line(0.,yd);
				endlines();
			}

			if(op.xax) /* generate x-axis labels */
				for(val=xmin,i=0; val<=xmax; val=(++i)*xint+xmin) {
					sprintf(s,"%g",val);
					textcent(val-xmin,-CHEIGHT/gp->ys,s,0,SCALE,COLOR,FONT);
				}
			if(op.yax) /* y labels */
				for(val=ymin,i=0; val<=ymax; val=(++i)*yint+ymin) {
					sprintf(s,"%g",val);
					maxychs=MAX(maxychs,strlen(s));
					textright(-CWIDTH/gp->xs,val-ymin,s,0,SCALE,COLOR,FONT);
				}

			if(!op.conn) /* set plotting chars */
				for(i=0; i<STYLES; i++) lsid[0][i][2] = op.pc[i];
		}
		if( (sense=op.grid)||op.axes ) { /* build background grid */
			int style;
			style = (sense ? DOTTED : SOLID);

			for(val=xint,i=1; val<xd; val=(++i)*xint) {
				startlines(val,0.,0,COLOR,NARROW,style);
				line(val,(sense ? yd : CHEIGHT/gp->ys));
				endlines();
			}
			for(val=yint,i=1; val<yd; val=(++i)*yint) {
				startlines(0.,val,0,COLOR,NARROW,sense);
				line((sense ? xd : CHEIGHT/gp->xs),val);
				endlines();
			}
		}
	}

/* plot points */
	if(op.mark) { /* scatter plot */
		double *txp = xp; double *typ = yp;
		for(; txp<=xep && typ<=yep; txp++,typ++)
			if(*txp>=xmin && *txp<=xmax && *typ>=ymin && *typ<=ymax)
				text(*txp-xmin,*typ-ymin,ctos(op.pc[pen%STYLES]),0,SCALE,COLOR,FONT);
	}
	if(op.conn) { /* connect points */
		beg = xp;
		for (; xp < xep && yp < yep ; xp++, yp++) /* ignore preliminary invisible pts */
			if(clip(*xp,*yp,*(xp+1),*(yp+1),&cx1,&cy1,&cx2,&cy2,&clip1,&clip2)) break;
		if(xp < xep) {
			for ( ; xp < xep && yp < yep ; xp++,yp++,clip1=0,clip2=0) { /* clip pts */
				if (clip(*xp,*yp,*(xp+1),*(yp+1),&cx1,&cy1,&cx2,&cy2,&clip1,&clip2)) {
					if (clip1 && !clip2) { /* pt1 clipped only */
						startlines(cx1-xmin,cy1-ymin,0,COLOR,((op.bold==0)?MEDIUM:BOLD),
							ls[(pen+op.bold)%STLTYPES]);
						line(cx2-xmin,cy2-ymin);
					}
					else if (!clip1 && !clip2) { /* both pts not clipped */
						if(beg == xp) 
							startlines(cx1-xmin,cy1-ymin,0,COLOR,
							((op.bold==0)?MEDIUM:BOLD),ls[(pen+op.bold)%STLTYPES]);
						line(cx2-xmin,cy2-ymin);
					}
					else if (!clip1 && clip2) {  /* pt2 clipped only */
						line(cx2-xmin,cy2-ymin); 
						endlines();
					}
					else { /* both pts clipped */
						startlines(cx1-xmin,cy1-ymin,0,COLOR,((op.bold==0)?MEDIUM:BOLD),
							ls[(pen+op.bold)%STLTYPES]);
						line(cx2-xmin,cy2-ymin);
						endlines(); 
					}
				}
			}
			if (!clip2) endlines();
		}
	}

	if( op.axes ) { /* output axes titles */
		double x;
		if( op.xax && op.xtit && (pen<=op.fmax) ) {
			x = -(3+pen)*CHEIGHT;
			textcent(xd/2,x/gp->ys,xlab,0,SCALE,COLOR,FONT);
		}
		if( op.yax && op.ytit ) {
			x = -((maxychs+4)*CWIDTH + pen*CHEIGHT);
			if(op.multi) strcat(ylab,lsid[op.conn][(pen+op.bold)%(STYLES)]);
			textcent(x/gp->xs,yd/2,ylab,90,SCALE,COLOR,FONT);
		}
	}
}









#define FAILURE 0
#define LEFT 01
#define RIGHT 02
#define BOTTOM 04
#define TOP 8

clip(x1,y1,x2,y2,cx1,cy1,cx2,cy2,clip1,clip2)
double x1, y1, x2, y2;
double *cx1, *cy1, *cx2, *cy2;
unsigned int *clip1, *clip2;
{
	int c1, c2, c;
	double x, y;

	c1 = code(x1,y1);
	c2 = code(x2,y2);

	if(c1 != 0) *clip1 = TRUE;
	if(c2 != 0) *clip2 = TRUE;

	while(c1 != 0 || c2 != 0) {
		if(c1 & c2)return(FAILURE);
		c = c1;
		if(c == 0) c = c2;
		if(c & LEFT) { /* push toward left edge */
			y = y1 + (y2 - y1)*(wn.xlow - x1)/(x2 - x1);
			x = wn.xlow;
		} else if(c & RIGHT) { /*  push toward right edge */
			y = y1 + (y2 - y1)*(wn.xhigh - x1)/(x2 - x1);
			x = wn.xhigh;
		} else if(c & BOTTOM) { /*  push toward bottom edge */
			x = x1 + (x2 - x1)*(wn.ylow - y1)/(y2 - y1);
			y = wn.ylow;
		} else if(c & TOP) { /*  push toward top edge */
			x = x1 + (x2 - x1)*(wn.yhigh - y1) / (y2 - y1);
			y = wn.yhigh;
		}
		if(c == c1) {
			x1 = x;
			y1 = y;
			c1 = code(x,y);
		}
		else {
			x2 = x;
			y2 = y;
			c2 = code(x,y);
		}
	}



	*cx1 = x1;
	*cy1 = y1;
	*cx2 = x2;
	*cy2 = y2;
	return(SUCCESS);
}
code(x,y)
double x, y;
{
	unsigned int kode = 0;

	if(x < wn.xlow) kode = LEFT;
	else if(x > wn.xhigh) kode = RIGHT;
	else kode = 0;

	if(y < wn.ylow) kode |= BOTTOM;
	else if(y > wn.yhigh) { kode |= TOP;
	}

	return(kode);
}
