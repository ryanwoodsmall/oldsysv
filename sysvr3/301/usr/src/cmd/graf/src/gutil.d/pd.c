/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/gutil.d/pd.c	1.4"
/* <: t-5 d :> */
#include <stdio.h>
#include <values.h>
#include "util.h";
#include "debug.h";
#include "errpr.h";

#define STRLEN		100
#define PATLEN		256

char *nodename;

main(argc,argv)
	int argc;
	char *argv[]; 
{
	FILE *fpi;
	int ac,argf;
	char **av,*cp;

	nodename = *argv;
	for( ac=argc-1,av=argv+1; ac>0; ac--,av++ )
		if( *(cp=av[0])=='-' && *++cp!=EOS ) {
			for(; *cp!=EOS; --argc )
				switch(*cp++) {
				default: ERRPR1(%c?,*(cp-1));
				}
		}

	if(argc <= 1) { argc=2; argf=0; } /* no args, use stdin */
	else argf=1;

	while(argc-- > 1) {
		if( strcmp(*++argv,"-")==0 || !argf ) fpi=stdin;
		else if( **argv!='-' ) {
			if((fpi =fopen(*argv,"r")) == NULL) continue;
		} else { argc++; continue; }

		pd(fpi,stdout);
		fclose(fpi);
	}
}

pd(fpi,fpo)
	FILE *fpi,*fpo; {
	int c;
	char string[STRLEN];
	int xi,yi,x0,y0,x1,y1,r,dx,n,i,pat[PATLEN];
	while((c=getc(fpi)) != EOF){
		switch(c){
		case 'm':
			xi = get2b(fpi);	yi = get2b(fpi);
			move(fpo,xi,yi);
			break;
		case 'l':
			x0 = get2b(fpi);	y0 = get2b(fpi);
			x1 = get2b(fpi);	y1 = get2b(fpi);
			line(fpo,x0,y0,x1,y1);
			break;
		case 't':
			getstr(fpi,string);
			label(fpo,string);
			break;
		case 'e':
			erase(fpo);
			break;
		case 'p':
			xi = get2b(fpi);	yi = get2b(fpi);
			point(fpo,xi,yi);
			break;
		case 'n':
			xi = get2b(fpi);	yi = get2b(fpi);
			cont(fpo,xi,yi);
			break;
		case 's':
			x0 = get2b(fpi);	y0 = get2b(fpi);
			x1 = get2b(fpi);	y1 = get2b(fpi);
			space(fpo,x0,y0,x1,y1);
			break;
		case 'a':
			xi = get2b(fpi);	yi = get2b(fpi);
			x0 = get2b(fpi);	y0 = get2b(fpi);
			x1 = get2b(fpi);	y1 = get2b(fpi);
			arc(fpo,xi,yi,x0,y0,x1,y1);
			break;
		case 'c':
			xi = get2b(fpi);	yi = get2b(fpi);
			r = get2b(fpi);
			circle(fpo,xi,yi,r);
			break;
		case 'f':
			getstr(fpi,string);
			linemod(fpo,string);
			break;
		case 'd':
			xi = get2b(fpi);	yi = get2b(fpi);
			dx = get2b(fpi);
			n = get2b(fpi);
			for(i=0; i<n; i++)pat[i] = get2b(fpi);
			dot(fpo,xi,yi,dx,n,pat);
			break;
		}
	}
}

get2b(fptr)		/* get an integer stored in 2 ascii bytes. */
FILE *fptr;
{
	short a, b;
	if((b = getc(fptr)) == EOF) {
		ERRPR0(Format error);
		return(EOF);
	}
	if((a = getc(fptr)) == EOF) {
		ERRPR0(Format error);
		return(EOF);
	}
	a <<= 8;
	return(a|b);
}

getstr(fpi,str)
	FILE *fpi; 
	char *str; {
	for( ; *str = getc(fpi); str++){
		if(*str == '\n')
			break;
	}
	*str = '\0';
	return;
}


/* pd print routines */

move(fpo,xi,yi)
	FILE *fpo;
	int xi,yi; {
	fprintf(fpo,"jump     %5d   %5d\n",xi,yi);
}
line(fpo,x0,y0,x1,y1)
	FILE *fpo;
	int x0,x1,y0,y1; {
	fprintf(fpo,"line     %5d   %5d   %5d   %5d\n",x0,y0,x1,y1);
}
label(fpo,ptr)
	FILE *fpo;
	char *ptr; {
	fprintf(fpo,"text                             %s\n",ptr);
}
point(fpo,xi,yi)
	FILE *fpo;
	int xi,yi; {
	fprintf(fpo,"point   %5d   %5d\n",xi,yi);
}
cont(fpo,xi,yi)
	FILE *fpo;
	int xi,yi; {
	fprintf(fpo,"cont     %5d   %5d\n",xi,yi);
}
space(fpo,x0,y0,x1,y1)
	FILE *fpo;
	int x0,y0,x1,y1; {
	fprintf(fpo,"space    %5d   %5d   %5d   %5d\n",x0,y0,x1,y1);
}
arc(fpo,xi,yi,x0,y0,x1,y1)
	FILE *fpo;
	int xi,yi,x0,y0,x1,y1; {
	fprintf(fpo,"arc      %5d   %5d   %5d   %5d   %5d   %5d\n"
	,xi,yi,x0,y0,x1,y1);
}
circle(fpo,xi,yi,r)
	FILE *fpo;
	int xi,yi,r; {
	fprintf(fpo,"circ     %5d   %5d   %5d\n",xi,yi,r);
}
linemod(fpo,ptr)
	FILE *fpo;
	char *ptr; {
	fprintf(fpo,"linemode                 %s\n",ptr);
}
dot(fpo,xi,yi,dx,n,pat)
	FILE *fpo;
	int xi,yi,dx,n;
	char *pat; {
	int i;
	fprintf(fpo,"dot      %5d   %5d   %5d   \n",xi,yi,dx);
	for(i=0;i<n;i++,pat++){
		fprintf(fpo,"pattern in octal		%o\n",*pat);
	}
}
erase(fpo)
	FILE *fpo; {
	fprintf(fpo,"erase\n");
}
