/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)search:syn.c	1.4"
#include "msearch.h"
extern char err_label[STRSIZE];

getsyn(fp, wd, wdfile)
FILE *fp;
char *wd;
char *wdfile;
{
     char linebuf[BUFSIZ];
     char leftword[STRSIZE], rhtword[STRSIZE];
     FILE *fpword;
     int matchstate = FALSE;
     int wordsize = strlen(wd);
     while( fgets(linebuf, BUFSIZ, fp) != NULL) {
        sscanf(linebuf,"%s %s",  leftword, rhtword);	
        if(strncmp(wd,leftword,wordsize)==0) {
             if(matchstate == FALSE) {
                 matchstate = TRUE;
                 fpword = fopen(wdfile,"w");
                 if(fpword == NULL) {
                      sprintf(linebuf,"%s: getsyn: %s",err_label, wdfile);
                      perror(linebuf);
                      exit(1);
                 }
                 fprintf(fpword," %s\n", wd);
                 printf(" %s", wd);
              }
              fprintf(fpword," %s\n",rhtword);
              printf(" %s", rhtword);
        }
     }
     if (matchstate == TRUE) fclose(fpword);
     rewind(fp);
     return(matchstate);
}
