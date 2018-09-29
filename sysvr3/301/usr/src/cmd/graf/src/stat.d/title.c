/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/stat.d/title.c	1.3"
/* <:t-5 d:> */
#include "stdio.h"
#include "s.h"
#include "plot.h"
char *nodename;
#define COLOR DFLTCOLOR

struct options {
	int bold;
	int cflg;
	char llab[MAXLABEL];
	char ulab[MAXLABEL];
} op = { 16, 1, '\0', '\0' };

main(argc,argv) /* title vector */
int argc;
char *argv[];
{
	FILE *fdi;
	int ac, argf;
	char **av, *cp;
	int fch;

	nodename = *argv;
	for( ac=argc-1,av=argv+1; ac>0; ac--,av++ )
		if( *(cp=av[0])=='-' && *++cp!='\0' ) {
			while( *cp!='\0' )
				switch(*cp++) {
				case 'b': op.bold = BOLD<<4;; break;
				case 'c': op.cflg = 0; break;
				case 'v': case 'l': SETSTR(op.llab,cp,MAXLABEL); break;
				case 'u': SETSTR(op.ulab,cp,MAXLABEL); break;
				case ' ': case ',': break;
				default: ERRPR1(%c?,*(cp-1));
				}
			--argc;
		}

	if(op.cflg) {
		for(cp=op.llab; *cp!='\0'; cp++)
			if(*cp>='a' && *cp<='z') *cp = toupper(*cp);
		for(cp=op.ulab; *cp!='\0'; cp++)
			if(*cp>='a' && *cp<='z') *cp = toupper(*cp);
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

		if((fch=getc(fdi)) == FCH) { ungetc(fch,fdi); labged(fdi,op.llab,op.ulab);}
		else { ungetc(fch,fdi); labtext(fdi,op.llab);}
		fclose(fdi);
	}
	exit(0);
}

labtext(fdi,lab) /* llab text file */
FILE *fdi;
char *lab;
{
	int c;

	printf("\01%s\n",lab);
	while((c=getc(fdi)) != EOF) putchar(c);
}

labged(fdi,llab,ulab) /* title GPS file */
FILE *fdi;
char *llab, *ulab;
{
	struct command cd;
	int wd, txsz;
	static struct gslparm gp = { DFLTGP };
	static struct area ar;

	putc(getc(fdi),stdout);
	putc(getc(fdi),stdout);
	gp.x0 = 0.;
	gp.y0 = 0.;
	gp.xs = 1.;
	gp.ys = 1.;
	newparm(&gp);
	txsz = xymaxmin(fdi,stdout,&ar);
	if(llab[0]!='\0')
		textcent(((doub)ar.lx+ar.hx)/2.,((doub)ar.ly-(3.*CHEIGHT)),
			llab,0,(int)(1.5*txsz),COLOR,op.bold);
	if(ulab[0]!='\0')
		textcent(((doub)ar.lx+ar.hx)/2.,((doub)ar.hy+(3.*CHEIGHT)),
			ulab,0,(int)(1.5*txsz),COLOR,op.bold);
}
