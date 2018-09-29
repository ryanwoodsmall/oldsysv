static char SCCSID[]="@(#)power.c	1.2";
/* <:t-5 d:> */
#include "stdio.h"
#include "s.h"
char *nodename;
#define DFLTEXP 2

struct options {
	int col;	/* columns per output line */
	doub exp	/* exponent */
} op = { DFLTCOL, DFLTEXP };

main(argc,argv) /* calculate any power */
int argc;
char *argv[];
{
	FILE *fdi;
	int ac, argf, i;
	char **av, *cp;
	double x, pow1();
	char lab[MAXLABEL];

	nodename = *argv;
	for( ac=argc-1,av=argv+1; ac>0; ac--,av++ )
		if( *(cp=av[0])=='-' && *++cp!='\0' ) {
			while( *cp!='\0' )
				switch(*cp++) {
				case 'c': SETINT(op.col,cp); break;
				case 'p': SETDOUB(op.exp,cp); break;
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

		TITOUT2(fdi,lab,MAXLABEL,power,op.exp);
		for(i=1; getdoub(fdi,&x)!=EOF; i++)
			PRINT(pow1(x,op.exp),i,op.col);
		if(i%op.col != 1) putchar('\n');
		fclose(fdi);
	}
	exit(0);
}
