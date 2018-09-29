/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/stat.d/cor.c	1.3"
/* <:t-5 d:> */
#include "stdio.h"
#include "s.h"
char *nodename;
#define MAXEL 1000

struct options {
	FILE *fda;	/* file contains base vector */
} op = { NULL };

main(argc,argv) /*set-up for cor*/
int argc;
char *argv[];
{
	FILE *fdi;
	int ac, argf, i;
	char **av, *cp;

	double x, cor(), X[MAXEL], Y[MAXEL];
	int xn, yn;

	nodename = *argv;
	for( ac=argc-1,av=argv+1; ac>0; ac--,av++ )
		if( *(cp=av[0])=='-' && *++cp!='\0' ) {
			while( *cp!='\0' )
				switch(*cp++) {
				case 'F': SETFILE(op.fda,cp); break;
				case ' ': case ',': break;
				default: ERRPR1(%c?,*(cp-1));
				}
			--argc;
		}

	xn = cor1((op.fda?op.fda:stdin),X,MAXEL);

	if(argc <= 1) { argc=2; argf=0; } /*no args, use stdin*/
	else argf=1;

	while( argc-->1 ) {
		if( strcmp(*++argv,"-")==0 || !argf ) fdi=stdin;
		else if( **argv!='-' ) {
			if( (fdi=fopen(*argv,"r"))==NULL ) {
				ERRPR1(cannot open %s, *argv);
				continue;
			}
		} else { argc++; continue; }

		yn = cor1(fdi,Y,MAXEL);
		if(xn == yn) {
			x = cor(X,Y,xn);
			PRINT(x,1,1);
		} else ERRPR2(vector length mismatch\, %d vs %d,xn,yn);
		fclose(fdi);
	}
	exit(0);
}

cor1(fdi,buf,size) /*generate residuals from mean, return length*/
FILE *fdi;
double *buf;
int size;
{
	int count;
	double sum, mean, *p, *ep, *filbuf1();
	ep = filbuf1(fdi,buf,size);
	for(p=buf,count=0,sum=0; p<=ep; p++,count++)
		sum += *p;
	mean = sum/( count>0 ? count : 1 );
	for(p=buf; p<=ep; p++) *p -= mean;
	return(ep-buf+1);
}


double cor(X,Y,n) /*correlation coefficient between X and Y*/
double X[], Y[];
int n;
{
	int i;
	double xys=0, xss=0, yss=0, sqrt(), xyvar;
	for(i=0; i<n; i++) {
		xys += X[i] * Y[i];
		xss += X[i] * X[i];
		yss += Y[i] * Y[i];
	}
	xyvar = sqrt(xss)*sqrt(yss);
	return( xys/(xyvar?xyvar:1) ); /* cor=0. for xyvar of 0 */
}
