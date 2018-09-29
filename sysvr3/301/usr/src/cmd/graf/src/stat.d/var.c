/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/stat.d/var.c	1.3"
/* <:t-5 d:> */
#include "stdio.h"
#include "s.h"
char *nodename;
#define DFLTN .5

main(argc,argv) /*set-up for var*/
int argc;
char *argv[];
{
	FILE *fdi;
	int ac, argf;
	char **av, *cp;
	double x, var();

	nodename = *argv;
	for( ac=argc-1,av=argv+1; ac>0; ac--,av++ )
		if( *(cp=av[0])=='-' && *++cp!='\0' ) {
			while( *cp!='\0' )
				switch(*cp++) {
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

		x = var(fdi);
		PRINT(x,1,1);
		fclose(fdi);
	}
	exit(0);
}

double var(fdi) /* calculate variance */
FILE *fdi;
{
	int count=0, i;
	double sqr=0, sum=0, x;

	while( getdoub(fdi,&x)!=EOF ) {
		count++;
		sqr += x*x;
		sum += x;
	}
	return( (sqr-sum*sum/count)/((i=count-1)>0 ? i : 1));
}
