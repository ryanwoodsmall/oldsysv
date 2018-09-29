/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/stat.d/hilo.c	1.4"
/* <:t-5 d:> */
#include "stdio.h"
#include "s.h"
char *nodename;

struct options {
	char ch;	/* char prepended to option keyletter */
	int oflag;	/* option form */
	int lflag;	/* low value */
	int hflag;	/* high value */
} op = { 0, 0, 0, 0, };

main(argc,argv) /*list elements */
int argc;
char *argv[];
{
	FILE *fdi;
	FILE *fopen();
	int ac, argf, i;
	char **av, *cp;

	double high=NEGINFIN, low=POSINFIN;

	nodename = *argv;
	for( ac=argc-1,av=argv+1; ac>0; ac--,av++ )
		if( *(cp=av[0])=='-' && *++cp!='\0' ) {
			while(*cp != '\0')
				switch(*cp++) {
				case 'l': op.lflag++; break;
				case 'h': op.hflag++; break;
				case 'o': op.oflag++;
				  switch(*cp++) {
				  case 'x': op.ch='x'; break;
				  case 'y': op.ch='y'; break;
				  default: cp--; break;
				  }; break;
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

		hilo(fdi,&high,&low);
		fclose(fdi);
	}
	pr(high,low);
	exit(0);
}



hilo(fdi,high,low)
FILE *fdi;
double *high, *low;
{
	double tmp;
	int i;

	while(getdoub(fdi,&tmp) != EOF) {
		if (tmp > *high) *high = tmp;
		if (tmp < *low) *low = tmp;
	}
}



pr(high,low)
double high, low;
{
	if(op.lflag || op.hflag) {
		if(op.lflag)
			if(op.oflag)
				op.ch ? printf("%cl%g,",op.ch,low)
				: printf("l%g,",low);
			else printf("%g\t",low);
		if(op.hflag)
			if(op.oflag)
				op.ch ? printf("%ch%g,",op.ch,high)
				: printf("h%g,",high);
			else printf("%g\t",high);
	}
	else if(op.oflag)
		op.ch ? printf("%cl%g,%ch%g",op.ch,low,op.ch,high)
		: printf("l%g,h%g,",low,high);
      else printf("low=%g\thigh=%g",low,high);
	putchar('\n');
}
