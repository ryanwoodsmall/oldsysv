/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/stat.d/log.c	1.3"
/* <:t-5 d:> */
#include <stdio.h>
#include "s.h"
char *nodename;

struct options {
	double base;	/* logarithm base */
	int col;	/* columns per output line */
} op = { UNDEF, DFLTCOL };

main(argc,argv) /* calculate log any base */
int argc;
char *argv[];
{
	FILE *fdi;
	int ac, argf, i;
	char **av, *cp;

	double base, x, fabs(), log();
	char lab[MAXLABEL];

	nodename = *argv;
	for( ac=argc-1,av=argv+1; ac>0; ac--,av++ )
		if( *(cp=av[0])=='-' && *++cp!='\0' ) {
			while( *cp!='\0' )
				switch(*cp++) {
				case 'b': SETDOUB(op.base,cp); break;
				case 'c': SETINT(op.col,cp); break;
				case ' ': case ',': break;
				default: ERRPR1(%c?,*(cp-1));
				}
		--argc;
		}
	if(DEF(op.base))
		if( op.base>0 ) base = log(fabs(op.base));
		else {
			ERRPR0(base <= zero undefined\, no output);
			exit(1);
		}
	else base = 1;

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

		if(DEF(op.base)) TITOUT2(fdi,lab,MAXLABEL,log,op.base);
		else TITOUT1(fdi,lab,MAXLABEL,log);
		for(i=1; getdoub(fdi,&x)!=EOF; i++)
			PRINT(log(fabs(x))/base,i,op.col);
		if(i%op.col != 1) putchar('\n');
		fclose(fdi);
	}
	exit(0);
}
