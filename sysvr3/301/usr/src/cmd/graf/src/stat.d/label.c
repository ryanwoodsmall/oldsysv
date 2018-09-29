/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/stat.d/label.c	1.4"
/* <:t-5 d:> */
#include "stdio.h"
#include "s.h"
#include "plot.h"
char *nodename;
#define MAXLABS 80
#define MAXBUF 80

struct options {
	int ang;	/* label rotation */
	char axis;	/* axis to be labeled */
	char side;	/* upper, lower, left, right */
	int cflg;	 /* all upper case flag */
	FILE *fda;	/* file source for labels */
	int pdiv;	/* plot divisions: -1 plot, 1 hist and bar */
	int poff;	/* plot offset: 0 plot, 1 hist and bar */
}  op = { 0, 'x', 'l', 1, NULL, -1, 0 };

main(argc,argv) /* generate axis labels */
int argc;
char *argv[];
{
	FILE *fdi;
	int ac, argf, i;
	char **av, *cp;
	char *ep, *labs[MAXLABS], buf[MAXBUF];
	char *getline(), *calloc();

	static struct gslparm gp = {
		stdout, XLTRIM, YLTRIM, 0., -1., 1., 1., 0, 0
	};

	nodename = *argv;
	for( ac=argc-1,av=argv+1; ac>0; ac--,av++ )
		if( *(cp=av[0])=='-' && *++cp!='\0' ) {
			while( *cp!='\0' )
				switch(*cp++) {
				case 'r': SETSINT(op.ang,cp); break;
				case 'c': op.cflg=0; break;
				case 'F': SETFILE(op.fda,cp); break;
				case 'b': case 'h': op.pdiv = 1; op.poff = 1; break;
				case 'p': op.pdiv = -1; op.poff = 0; break;
				case 'x': op.axis='x';
					switch(op.side = *cp++) {
					case 'u': gp.y = (YSIZE/CHEIGHT)+1.; break;
					default: gp.y = -1.; cp--;
					}; break;
				case 'y': op.axis='y';
					switch(op.side = *cp++) {
					case 'r': gp.x = (XSIZE/CWIDTH)+1.; break;
					default: gp.x = -1.; cp--;
					}; break;
				case ' ': case ',': break;
				default: ERRPR1(%c?,*(cp-1));
				}
			--argc;
		}

	for( i=0; (ep=getline(op.fda?op.fda:stdin,buf,MAXBUF))!=(char *) EOF; i++)
		if( i<MAXLABS ) { /* read labels into labs */
			strcpy(labs[i]=calloc(ep-buf+1,1), buf);
			if(op.cflg)
				for(cp=labs[i]; *cp!='\0'; cp++)
					if(*cp>='a' && *cp<='z') *cp=toupper(*cp);
		}
		else {
			ERRPR1(only first %d labels used,MAXLABS);
			break;
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

		if( label(fdi,&gp,labs,i)==1 )
			ERRPR0(input not in GPS format);
		fclose(fdi);
	}
	exit(0);
}

#define FONT DFLTFONT
#define COLOR 0
label(fdi,gp,labs,n) /* label axis */
FILE *fdi;
struct gslparm *gp;
char *labs[];
int n;
{
	int fch, j, wd;
	double klu;

	if( (fch=getc(fdi))!=FCH ) { ungetc(fch,fdi); return(1);}
	else {
		putc(fch,stdout);
		putc(getc(fdi),stdout);
		getx0y0(fdi,&(gp->x0),&(gp->y0));
		putx0y0(stdout,gp->x0,gp->y0);
		while ((wd=getc(fdi)) != EOF)
			putc(wd,stdout); /* copy input to output */
	}

	switch(op.axis) {
	case 'x':
		gp->xs = (doub)XSIZE/((j=n+op.pdiv)>0?j:1);
		gp->ys = CHEIGHT;
		newparm(gp);
		klu = gp->y; /* textcent() was altering gp->y;Temporary kludge to prevent this. */
		while ( --n>=0 )
			if( op.ang ) /* rotated text begins at tick */
				text((doub)n+op.poff,gp->y,labs[n],op.ang,SCALE,COLOR,FONT);
			else {  /* text is centered */
				textcent((doub)n+op.poff,klu,labs[n],0,SCALE,COLOR,FONT);
			}
		break;
	case 'y':
		gp->xs = CWIDTH;
		gp->ys = YSIZE/((j=n+op.pdiv)>0?j:1);
		newparm(gp);
		klu = gp->x;
		while( --n>=0 )
			if(op.side=='r')
				text(gp->x,(doub)n+op.poff,labs[n],op.ang,SCALE,COLOR,FONT);
			else
				textright(klu,(doub)n+op.poff,labs[n],op.ang,SCALE,COLOR,FONT);
		break;
	}
	return(0);
}
