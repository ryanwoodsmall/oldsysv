/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/stat.d/sin.c	1.3"
/* <:t-5 d:> */
#include "stdio.h"
#include "s.h"
char *nodename;

struct options{
	int col;	/* number of columns per line */
} op = { DFLTCOL };

main(argc,argv) /*set-up for sin*/
int argc;
char *argv[];
{
	FILE *fdi;
	int argf, ac, i;
	char **av, *cp, lab[MAXLABEL];

	double x, sin();

	nodename = *argv;
	for( ac=argc-1,av=argv+1; ac > 0; ac--,av++ )
		if( *(cp=av[0])=='-' && *++cp!='\0' ) {
			while( *cp != '\0' )
				switch(*cp++) {
				case 'c': SETINT(op.col,cp); break;
				case ' ': case ',': break;
				default: ERRPR1(%c?,*(cp-1));
				}
			--argc;
		}

	if(argc <= 1)  argc=2, argf=0;  /* no args, use stdin */
	else argf=1;

	while( --argc>0 ) {
		if( strcmp(*++argv,"-") == 0 || !argf ) fdi=stdin;
		else if( **argv != '-' ) {
			if( (fdi = fopen(*argv,"r")) == NULL ) {
				ERRPR1(cannot open %s,*argv);
				continue;
			}
		} else { argc++; continue; }

		TITOUT1(fdi,lab,MAXLABEL,sin);
		for(i=1; getdoub(fdi,&x)!=EOF; i++)
			PRINT(sin(x),i,op.col);
		if(i%op.col != 1) putchar('\n');
		fclose(fdi);
	}
	exit(0);
}
