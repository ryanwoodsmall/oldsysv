static char SCCSID[]="@(#)mod.c	1.2";
/* <:t-5 d:> */
#include "stdio.h"
#include "s.h"
char *nodename;
#define DFLTMODU 2

struct options {
	int col;	/* columns per output line */
	double modu;
} op = { DFLTCOL,DFLTMODU };

main(argc,argv) /*modulo function */
int argc;
char *argv[];
{
	FILE *fdi;
	int ac, argf, i;
	char **av, *cp;

	double x, fmod();
	char lab[MAXLABEL];

	nodename = *argv;
	for( ac=argc-1,av=argv+1; ac>0; ac--,av++ )
		if( *(cp=av[0])=='-' && *++cp!='\0' ) {
			while( *cp!='\0' )
				switch(*cp++) {
				case 'c': SETINT(op.col,cp); break;
				case 'm': SETDOUB(op.modu,cp); break;
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

		TITOUT2(fdi,lab,MAXLABEL,mod,op.modu);
		for(i=1; getdoub(fdi,&x)!=EOF; i++)
			PRINT( (op.modu ? fmod(x,op.modu) : 0),i,op.col);
		if(i%op.col != 1) putchar('\n');
		fclose(fdi);
	}
	exit(0);
}
