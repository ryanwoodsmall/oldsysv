/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/stat.d/list.c	1.7"
/* <:t-5 d:> */
#include "stdio.h"
#include "s.h"
char *nodename;
#define MAXDELIM 128

struct options {
	int col;	/* columns per output line */
	short dflag;
} op = { DFLTCOL, FALSE };

main(argc,argv) /*list elements */
int argc;
char *argv[];
{
	FILE *fdi;
	int ac, argf, i;
	char **av, *cp;

	double x;
	char label[MAXLABEL];
	static char deltab[MAXDELIM]="\n\t ", *p=deltab+3;
						/*p initialized to next empty space in deltab */

	nodename = *argv;
	for( ac=argc-1,av=argv+1; ac>0; ac--,av++ )
		if( *(cp=av[0])=='-' && *++cp!='\0' ) {
			while( *cp!='\0' )
				switch(*cp++) {
				case 'c': SETINT(op.col,cp); break;
				case 'd':	op.dflag=TRUE; SETSTR(p,cp,MAXDELIM); break;
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

		TITOUT0(fdi,label,MAXLABEL);
		if (op.dflag)  for (i=1; dgetdoub(deltab,fdi, &x) != EOF; i++)
					PRINT(x,i,op.col);
		else  for(i=1; getdoub(fdi,&x)!=EOF; i++)
				PRINT(x,i,op.col);
		if(i%op.col != 1) putchar('\n');
		fclose(fdi);
	}
	exit(0);
}



dgetdoub(deltab,fdi, x)  /* returns a number delimited by characters in deltab */
char *deltab;					/* or EOF if no number is found */
FILE *fdi;
double *x;
{
#if u370 | u3b | u3b5 | u3b2
	int c;
#else
	char c;
#endif
	int s;
	while ((s=fscanf(fdi,"%F",x)) != EOF)
#if u370 | u3b | u3b5 | u3b2
		if (s>0 && strchr(deltab, (char) (c=getc(fdi)))!=NULL)
#else
		if (s>0 && strchr(deltab,(c=getc(fdi)))!=NULL)
#endif
			return(1);
		else  {
#if u370 | u3b | u3b5 | u3b2
			for (c=getc(fdi); (strchr(deltab,(char)c)==NULL) 
#else
			for (c=getc(fdi); (strchr(deltab,c)==NULL) 
#endif
							&& c!='\1' && c!=EOF; c=getc(fdi));
			if (c == '\1')  {ungetc('\1',fdi); stplabel(fdi);}
		}
	return(EOF);
}
