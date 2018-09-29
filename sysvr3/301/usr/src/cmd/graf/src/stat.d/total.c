/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/stat.d/total.c	1.3"
/* <:t-5 d:> */
#include "stdio.h"
#include "s.h"
char *nodename;
main(argc,argv) /* calculate gross total*/
int argc;
char *argv[];
{
	FILE *fdi, *fopen();
	double x, y;
	char *cp;

	while(argc>1 && argv[1][0]=='-') {
		for(cp=argv[1]+1; *cp!='\0'; )
			switch(*cp++) {
			case ' ': case ',': break;
			default: ERRPR1(%c?,*(cp-1));
			}
		--argc; ++argv;
	}
	if(argc == 1) {/*no args, use stdin*/
		for(x=0; getdoub(stdin,&y)!=EOF; x += y) ;
		PRINT(x,1,1);
	}
	else
		while(--argc > 0)
			if((fdi = fopen(*++argv,"r")) != NULL) {
				for(x=0; getdoub(fdi,&y)!=EOF; x += y) ;
				PRINT(x,1,1);
				fclose(fdi);
			}
			else
				ERRPR1(cannot open %s,*argv);
	exit(0);
}
