/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/gutil.d/gd.c	1.3"
/* <: t-5 d :> */
#include<stdio.h>
#include "gsl.h"
#include "gpl.h"
#include "util.h"
#include "errpr.h"
#define PRC(str)  fprintf(stdout,"str")
#define PRXY(x,y)   fprintf (stdout,"   %5d %5d",x,y)
struct command cd;
char *nodename;

main(argc,argv) 
int argc;
char *argv[];
{
	FILE *fpi;
	int ac, argf, i;
	char **av, *cp;

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
			if( (fpi=fopen(*argv,"r"))==NULL ) {
				ERRPR1(cannot open %s, *argv);
				continue;
			}
		} else { argc++; continue; }

		if( getc(fpi)==FCH ) {
			getc(fpi);
			gd(fpi);
		} else ERRPR0(input not GPS);
		fclose(fpi);
	}
}
gd(fpi)
FILE *fpi; 
{	int *lptr,cnt;
	char *ptr;
	while(getgedf(fpi,&cd) != EOF){
		PRC(\n);
		switch (cd.cmd) {
		case LINES :
		case ARCS:
			if(cd.cmd==LINES) PRC(lines    );
			else PRC(arc      );
			fprintf(stdout," col%d wt%d st%d ",
			 cd.color,cd.weight,cd.style);
			for(cnt=cd.cnt-2,lptr=cd.aptr; cnt>0; cnt -= 2,lptr += 2){
				PRXY(*lptr,*(lptr+1));
			}
			break;
		case TEXT :
			PRC(text     );
			fprintf(stdout," col%3d fon%3d tsz%3d tro%3d ",
			 cd.color,cd.font,cd.tsiz,cd.trot);
			cnt = cd.cnt-5;
		case ALPHA:
			if(cd.cmd==ALPHA) {PRC(alpha    ); cnt=cd.cnt-3;}
			fprintf(stdout," %d %d ",*cd.aptr,*(cd.aptr+1));
			for(ptr=cd.tptr; cnt>=0; cnt--,ptr +=2){
				if(*ptr != EOS) fprintf(stdout,"%c",*ptr);
				if(*(ptr+1) != EOS) fprintf(stdout,"%c",*(ptr+1));
			}
			break;
		case COMMENT:
			if(cd.cmd==COMMENT) PRC(comment );  
			for(lptr=cd.aptr,cnt=cd.cnt-1; cnt>0; cnt--,lptr++){
				fprintf(stdout,"    %o",*lptr);
			}
		}
	}
	PRC(\n);
}
