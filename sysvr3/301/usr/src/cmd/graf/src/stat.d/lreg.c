/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/stat.d/lreg.c	1.3"
/* <:t-5 d:> */
#include "stdio.h"
#include "s.h"
char *nodename;
#define MAXEL 1500

struct options {
	short iflg;	/* print intercept */
	short oflg;	/* output usable as option */
	short sflg;	/* print slope */
	FILE *fda;	/* file contains base vector */
}  op={0,0,0,NULL};

main(argc,argv) /* set-up for linear regression */
int argc;
char *argv[];
{
	FILE *fdi;
	int ac, argf, i;
	char **av, *cp;

	double X[MAXEL], Y[MAXEL], *filbuf1();
	double slope, intercept;
	int xn, yn;

	nodename = *argv;
	for( ac=argc-1,av=argv+1; ac>0; ac--,av++ )
		if( *(cp=av[0])=='-' && *++cp!='\0' ) {
			while( *cp!='\0' )
				switch(*cp++) {
				case 'F': SETFILE(op.fda,cp); break;
				case 'i': op.iflg++; break;
				case 'o': op.oflg++; break;
				case 's': op.sflg++; break;
				case ' ': case ',': break;
				default: ERRPR1(%c?,*(cp-1));
				}
			--argc;
		}

	if(op.fda) xn = filbuf1(op.fda,X,MAXEL) - X + 1;
	else { /* use natural numbers */
		for( i=0; i<MAXEL; i++ ) X[i]=i;
		xn = -1;
	}

	if( argc<=1 ) argc=2, argf=0; /* no args, use stdin */
	else argf=1;

	while( --argc>0 ) {
		if( strcmp(*++argv,"-")==0 || !argf ) fdi=stdin;
		else if( **argv!='-' ) {
			if( (fdi=fopen(*argv,"r"))==NULL ) {
				ERRPR1(cannot open %s, *argv);
				continue;
			}
		} else { argc++; continue; }

		yn = filbuf1(fdi,Y,MAXEL) - Y + 1;
		if( (xn<0 && yn>0) || (xn==yn && xn>0) ) {
			reg(X,Y,yn,&slope,&intercept);
			pr(slope,intercept);
		}
		else ERRPR2(vector length mismatch\, %d vs %d,xn,yn);
		fclose(fdi);
	}
	exit(0);
}

reg(x,y,n,slope,intercept) /* linear regression */
double x[], y[], *slope, *intercept;
int n;
{

	int i;
	double xy=0, xs=0, xsum=0, ysum=0, xbar, ybar, xvar;

	for( i=0; i<n; i++ ){
		xy += x[i]*y[i];
		xs += x[i]*x[i];
		xsum += x[i];
		ysum += y[i];
	}
	xbar = xsum/n;
	ybar = ysum/n;
	xvar = (xs/n)-(xbar*xbar);
	*slope = (xy/n - xbar*ybar) / xvar;
	*intercept = ybar - *slope*xbar;
}

pr(slope,intercept) /* print results */
double slope,intercept;
{
	if( op.iflg||op.sflg ) {
		if(op.iflg)
			if(op.oflg) printf("i%g,",intercept);
			else printf("%g\t",intercept);
		if(op.sflg)
			if(op.oflg) printf("s%g",slope);
			else printf("%g,",slope);
	}
	else
		if (op.oflg) printf("i%g,s%g,",intercept,slope);
		else printf("intercept=%g\tslope=%g",intercept,slope);
	putchar('\n');
}
