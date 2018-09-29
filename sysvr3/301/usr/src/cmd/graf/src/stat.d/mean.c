/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/stat.d/mean.c	1.3"
/* <:t-5 d:> */
#include "stdio.h"
#include "s.h"
char *nodename;

struct	options{
	double n;	/* if != 0 then trimmed mean */
} op = { 0 };

main(argc,argv) /*set-up for mean*/
int argc;
char *argv[];
{
	FILE *fdi;
	int argf, ac, i;
	char **av, *cp;

	double  x, mean(), tmean();

	nodename = *argv;
	for( ac=argc-1,av=argv+1; ac > 0; ac--,av++ )
		if( *(cp=av[0])=='-' && *++cp!='\0' ) {
			while( *cp!='\0' )
				switch(*cp++) {
				case 'f': SETDOUB(x,cp);
					op.n = ( x!=0 ? fabs(1/x) : 0 );
					break;
				case 'n': SETINT(x,cp);
					op.n = fabs(x); break;
				case 'p': SETDOUB(x,cp);
					op.n = ( x<.5 ? fabs(x) : 0 );
					break;
				case ' ': case ',': break;
				default: ERRPR1(%c?,*(cp-1));
				}
			--argc;
		}

	if( argc<=1 ) { argc=2, argf=0; /*no args, use stdin*/
	} else argf=1;

	while( argc-- > 1 ) {
		if( strcmp(*++argv,"-") == 0 || !argf ) fdi=stdin;
		else if( **argv!='-' ) {
			if( (fdi=fopen(*argv,"r")) == NULL ) {
				ERRPR1(cannot open %s,*argv);
				continue;
			}
		} else { argc++; continue; }

		if(op.n) x = tmean(fdi,op.n);
		else x = mean(fdi);
		PRINT(x,1,1);
		fclose(fdi);
	}
	exit(0);
}

double mean(fdi) /*calculate mean*/
FILE *fdi;
{
	double x, sum=0;
	int count=0;
	while( getdoub(fdi,&x)!=EOF ) {
		sum += x; count++;
	}
	return(VALUE(x,(count > 0 ? sum/count : 0)));
}

double sortbuf[MAXELEM];
double tmean(fdi,n)
FILE *fdi;
double n;
{
	int count=0, i;
	double sum=0, *sep, *p, floor(), *filbuf1();
	sep = filbuf1(fdi,sortbuf,MAXELEM);
	if(n < 1) i = floor(n*(sep-sortbuf+1));
	else i = (int)n;
	for(p=sortbuf+i; p<=sep-i; p++,count++) sum += *p;
	return(VALUE(n,(count>0 ? sum/count : 0)));
}
