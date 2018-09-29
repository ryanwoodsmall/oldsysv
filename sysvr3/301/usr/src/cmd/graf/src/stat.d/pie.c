/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/stat.d/pie.c	1.12"
/* <: t-5 d :> */
#include <stdio.h>
#include "s.h"
#include "plot.h"
#include <math.h>
char *nodename;
#define MAXSECT 40
 
struct options  {
	int perpie;	/* percent of pie to be drawn, default=100 */
	int pernum;	/* total for all percentages, 0 means just print values */
	short bold;	/* if TRUE, lines drawn in bold */
	short noval;	/* if TRUE; no values printed in pie */
	short out;	/* if TRUE, values printed outside pie */
	}  op = {100,0,0,0,0};

main(argc,argv)		/* set-up for pie */
int argc;
char *argv[];
{
	FILE *fdi;
	int ac, argf, r;
	char **av, *cp;

	static struct gslparm gp = {
		 stdout,0.,0.,0.,0.,280.,280.,0.,0. };  /* 280 = CHEIGHT in plot.h */

	nodename = *argv;
	for (ac=argc-1,av=argv+1; ac > 0; ac--,av++)
		if (*(cp=av[0]) == '-' && *++cp != '\0')  {
			while (*cp != '\0')
				switch (*cp++)  {
					case 'p':
							switch (*cp++)  {
								case 'p':	SETINT(op.perpie,cp);
										if(op.perpie <= 0) op.perpie = 100;	/*prevent div by 0 below*/
										 break;
								case 'n':	SETINT(op.pernum,cp); break;
								default:	cp--; op.pernum=100;  break;
							}  break;
					case 'b':	op.bold = 1; break;
					case 'v':	op.noval = 1; break;
					case 'o':	op.out = 1; break;
					case 'r':	SETINT(r,cp);
							region(r,&gp.x0,&gp.y0);
							gp.x0 += XDIM/2; gp.y0 += YDIM/2;
							break;
					case 'x':	SETDOUB(gp.x0,cp); break;
					case 'y':	SETDOUB(gp.y0,cp); break;
					case ',': case ' ': break;
					default:	ERRPR1(%c?,*(cp-1));
				}
			--argc;
		}

	if (argc <= 1) argc=2, argf=0;  /* no args, use stdin */
	else argf=1;

	while (argc-- > 1)  {
		if (strcmp(*++argv,"=")==0 || !argf)  fdi=stdin;
		else if (**argv!='-')  {
			if ( (fdi=fopen(*argv,"r")) == NULL )  {
				ERRPR1(cannot open %s,*argv);
				continue;
			}
		}  else {argc++; continue;}
		pie(fdi,&gp);
		fclose(fdi);
	}
	exit(0);
}


#define PI 3.1415926535897932384626433
#define MINSLI .03		/* below MINSLI, value is printed outside pie */
#define LABPOS (11./10)		/* LABPOS is ratio of radius, label is from center */
#define VALPOS (3./4)		/* VALPOS is ratio of radius, value is from center */
#define FONT DFLTFONT
#define COLOR DFLTCOLOR
#define RADIUS 15
#define RAD(x,y) (y >= 0 ? SUBRAD(x) : PI + SUBRAD(-x))
#define SUBRAD(x) (x >= 0 ? acos(TR(x)) : PI/2 + asin(TR(fabs(x))))
#define TR(x)  (x > 1 ? 1. : x)	/* truncate so acos won't return 0 for number rounded to above 1 */

struct key {
		short i; /* visibility flag */
		short e; /* explosion flag, moves slice out from pie */
		short f; /* mask for style field for filling */
		short hue; /* color of slice  */
	};

pie(fdi,gp)		/* build a pie chart */
FILE *fdi;
struct gslparm *gp;
{
	int nsect, weight, j;
	int bigslicej = -1, half;
	double x1,x2,x3,y1,y2,y3,slice[MAXSECT],total,pertotal,x0,y0,xoff,yoff,tmp;
	char label[MAXLABEL], name[MAXSECT][MAXLABEL];
	struct key keys[MAXSECT];

	getlabel(fdi,label,MAXLABEL);

	/* retrieve and store keys, values, and labels */

	for (nsect=0,total=0; !feof(fdi); nsect++)
		if (nsect < MAXSECT)  {
			if (getkey(fdi,&keys[nsect]) == EOF)  break;
			if (getdoub(fdi,&slice[nsect]) != EOF)
				getline(fdi,name[nsect],MAXLABEL);
			else break;		/* getdoub reached EOF */
			total += slice[nsect];
		}
		else  {
			if (getdoub(fdi,&tmp) != EOF)	/* if over MAXSECT values, error print */
				ERRPR1(too many values\, only first %d used, MAXSECT);
			break;
		}

	if (op.pernum != 0)		/* op.pernum format flag for value */
		pertotal = total * 100.0/op.pernum;	/* pertotal basis for percentage calculation */
	total *= 100.0/op.perpie;
	half = total/2;
	for (j=0; j < nsect; j++) if (slice[j] > half && keys[j].f == FILL) {
		bigslicej = j;
		break;
		}

	initgsl(gp);

	if (strlen(label) > 0)
		textcent(0,RADIUS+4,label,0,SCALE,COLOR,FONT);


	/* output pie as a series of slices */

	for (j=0,x0=0,y0=RADIUS; j < nsect; j++)  {
		weight = (op.bold ? BOLD : MEDIUM);
		x1=x0; y1=y0;
		x2 = RADIUS * cos(PI * slice[j]/total + RAD(x1/RADIUS,y1/RADIUS));
		y2 = RADIUS * sin(PI * slice[j]/total + RAD(x1/RADIUS,y1/RADIUS));
		x3 = RADIUS * cos(PI * slice[j]/total + RAD(x2/RADIUS,y2/RADIUS));
		x0 = x3;
		y3 = RADIUS * sin(PI * slice[j]/total + RAD(x2/RADIUS,y2/RADIUS));
		y0 = y3;
		if ( keys[j].e )  {
			xoff = x2/5;  yoff = y2/5;
			x1 += xoff;  y1 += yoff;
			x2 += xoff;  y2 += yoff;
			x3 += xoff;  y3 += yoff;
		}
		else  {
			xoff = 0;  yoff = 0;
		}
		if ( !keys[j].i )  {
			char s[MAXLABEL];
			short style;
			if (!op.noval)  {
				if (op.pernum > 0)
						sprintf(s,"%.2g%%", (slice[j]/pertotal)*100);
				else sprintf(s,"%.3g",slice[j]);
				if (slice[j]/total > MINSLI && !op.out)
					textcent((x2-xoff)*VALPOS +xoff,(y2-yoff)*VALPOS +yoff,s,0,SCALE,COLOR,FONT);
				else if (*name[j] != EOS)
					strcat(name[j]," : "), strcat(name[j],s);
				else strcpy(name[j],s);
			}
			if ( x2 > 0)
				text((x2-xoff)*LABPOS + xoff,(y2-yoff)*LABPOS + yoff,name[j],0,SCALE,COLOR,FONT);
			else textright((x2-xoff)*LABPOS + xoff,(y2-yoff)*LABPOS + yoff,name[j],0,SCALE,COLOR,FONT);
			if (bigslicej == j) style = SOLID;
			else style = SOLID + keys[j].f;
			startlines(x1,y1,0,keys[j].hue,weight,style);
			line(xoff,yoff);
			line(x3,y3);
			endlines();
			startarcs(x1,y1,0,keys[j].hue,weight,style);
			arc(x2,y2);
			arc(x3,y3);
			endarcs();
			if (bigslicej == j) {	/* more than 50%, produce 2 more INVISIBLE filled slices */
				startarcs(x1,y1,0,keys[j].hue,INVISIBLE,FILL);	/* half circle */
				arc(-y1,x1);
				arc(-x1,-y1);
				endarcs();
				x1 = -x1;
				y1 = -y1;
				x2 = RADIUS * cos(PI * (slice[j] - half)/total + RAD(x1/RADIUS,y1/RADIUS));
				y2 = RADIUS * sin(PI * (slice[j] - half)/total + RAD(x1/RADIUS,y1/RADIUS));
				startlines(x1,y1,0,keys[j].hue,INVISIBLE,FILL);	/* rest of section */
				line(xoff,yoff);
				line(x3,y3);
				endlines();
				startarcs(x1,y1,0,keys[j].hue,INVISIBLE,FILL);
				arc(x2,y2);
				arc(x3,y3);
				endarcs();
			}
		}
	}
}


int getkey(fdi,pkey)		/* get key delimited by < > */
FILE *fdi;
struct key *pkey;
{
#if u370 | u3b | u3b5 | u3b2
	int c;
#else
	char c;
#endif

	pkey->i=0; pkey->e=0; pkey->f=0; pkey->hue=0;
	while (isspace(c=getc(fdi)));
	if (c == '<')  {
		while ((c=getc(fdi)) != '>' && c != '\n')
			switch (c)  {
				case 'i':	pkey->i=1; break;
				case 'e':	pkey->e=1; break;
				case 'f': pkey->f=FILL; break;
				case 'c':	switch (c=getc(fdi))  {
							case 'b':	pkey->hue=BLACK; break;
							case 'r':	pkey->hue=RED; break;
							case 'g':	pkey->hue=GREEN; break;
							case 'u': pkey->hue=BLUE; break;
							default:	if (c!=EOF) ungetc(c,fdi);
									else return(EOF);
						}
						break;
				case ' ': case ',': break;
			}
		return(1);
	}
	else
		if ( !feof(fdi) )  {
#if u370 | u3b | u3b5 | u3b2
			ungetc((char) c,fdi);
#else
			ungetc(c,fdi);
#endif
			return(1);
		}
		else return(EOF);
}
