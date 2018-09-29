/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/gutil.d/remcom.c	1.4"
/* <:t-5 d:> */
#include "stdio.h"
#include "errpr.h"
#define  escape(fpi,c)  if(c=='\\') putchar(getc(fpi)); else  /*ignore escaped char*/

char *nodename;

main(argc,argv)  /*remove comments from C program*/
int argc;
char *argv[];
{
	FILE *fpi;

	nodename = *argv;
	if( argc==1 ) { /* use stdin */
		remcom(stdin);
	}
	else
		while( --argc>0 )
			if( (fpi=fopen(*++argv,"r"))!=NULL ) {
				remcom(fpi);
			}
	exit(0);
}

remcom(fpi) /* remove comments */
FILE *fpi;
{
	int c;

	while((c=getc(fpi)) != EOF) {
		if(c=='/' && next(fpi)=='*') {  /*comment*/
			getc(fpi);
			while((c=getc(fpi)) != EOF)
				if(c=='*' && next(fpi)=='/') {
					getc(fpi);
					break;
				}
			if( c==EOF ) ERRPR0(missing */ assumed);
		}
		else {  /*noncomment*/
			putchar(c);
			if(c=='\'') { /*char constant*/
				while((c=getc(fpi))!='\'' && c!=EOF) {
					putchar(c); escape(fpi,c);
				}
				putchar('\'');
			}
			else if(c=='"') { /*string*/
				while((c=getc(fpi))!='"' && c!=EOF) {
					putchar(c); escape(fpi,c);
				}
				putchar('"');
			}
		}
	}
}

next(fpi) /*one character lookahead*/
FILE *fpi;
{
	int c;
	c = getc(fpi);
	if(c != EOF) ungetc(c,fpi);
	return(c);
}
