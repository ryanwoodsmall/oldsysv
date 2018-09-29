/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/stat.d/pair.c	1.3"
/* <:t-5 d:> */
#include "stdio.h"
#include "s.h"
char *nodename;
#define DFLTXG 1

struct options {
	int col;	/* columns per output line */
	FILE *fda;	/* file contains base vector */
	int xg;	/* base vector elements per paired vector elements */
} op = { DFLTCOL, NULL, DFLTXG };

main(argc,argv) /*set-up for pair*/
int argc;
char *argv[];
{
	FILE *fdi;
	int ac, argf, i;
	char **av, *cp;

	int j;
	double xbuf[MAXELEM], *xep, *p, y, *filbuf1();

	nodename = *argv;
	for( ac=argc-1,av=argv+1; ac>0; ac--,av++ )
		if( *(cp=av[0])=='-' && *++cp!='\0' ) {
			while( *cp!='\0' )
				switch(*cp++) {
				case 'c': SETINT(op.col,cp); break;
				case 'F': SETFILE(op.fda,cp); break;
				case 'x': SETINT(op.xg,cp); break;
				case ' ': case ',': break;
				default: ERRPR1(%c?,*(cp-1));
				}
			--argc;
		}
	xep = filbuf1(op.fda?op.fda:stdin, xbuf, MAXELEM);
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

		for(p=xbuf,i=1; p+op.xg<=xep+1 && getdoub(fdi,&y)!=EOF; ) {
			for(j=0; j<op.xg; j++,p++) PRINT(*p,i++,op.col);
			PRINT(y,i++,op.col);
		}
		if(i%op.col != 1) putchar('\n');
		fclose(fdi);
	}
	exit(0);
}
