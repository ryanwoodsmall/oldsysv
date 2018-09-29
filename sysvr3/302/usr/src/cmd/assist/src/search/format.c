/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)search:format.c	1.2"
#include "msearch.h"
extern int popflag;
extern int non_inter;
extern int nlines;

format(sbuf, pg_size)
char *sbuf;
int pg_size;
{
    int i, j;
    int nline = 0;
    char *ptrin, *ptrout;
    char outbuf[BUFSIZ];
    ptrin = sbuf;
    for(i=0; i < BUFSIZ; i++) {
        outbuf[i] = '\0';
    }
    if( (non_inter == TRUE) && (popflag == FALSE) ) nlines = 0;
    while (*ptrin != '\0') {
        
        if(nline == 0) {
            j = 0;
            ptrout = outbuf;
            while (*ptrin != '\002') {  /* ^B */
                 ptrout[j] = *ptrin;
                 j++;
                 ptrin++;
            }
            ptrin++;
            if (j >= MINCHARS) {
                ptrout[j++] = '-';
                ptrout[j++] = '-';
            }
            else {
                while(j < MINCHARS) {
                    ptrout[j] = '-';
                    j++;
                }
            }
            while ( j < MAXCHARS ) {
                   ptrout[j] = *ptrin++;
                   if (ptrout[j] == '\0' )  {
                       fputs(outbuf,stdout);
                       if(++nlines >= pg_size) newpage();
                       return(0);
                   }
                   j++;
            }
            nline++;
            while (ptrout[j] != ' ') {
               ptrout[j] = '\0';
               j--;
               ptrin--;
            }
            ptrout[j] = '\n';
            fputs(outbuf,stdout);
            if(++nlines >= pg_size) newpage();
        }
        else {
           j=0;
           ptrout = outbuf;
           while(j < INDENT) {
             ptrout[j] = ' ';
             j++;
           }
           while ( j < MAXCHARS ) {
                  ptrout[j] = *ptrin++;
                  if (ptrout[j] == '\0') {
                      fputs(outbuf,stdout);
                      if(++nlines >= pg_size) newpage();
                      return(0);
                  }
                  j++;
           }
           nline++;
           while (ptrout[j] != ' ') {
              ptrout[j] = '\0';
              j--;
              ptrin--;
           }
           ptrout[j] = '\n';
           fputs(outbuf,stdout);
           if(++nlines >= pg_size) newpage();
        }
     }
}

newpage()
{
              fflush(stdout);
              if (popflag == TRUE) printf("[Strike RETURN for more; q to quit search]\n");
              else printf("[Strike RETURN for more; q to quit]\n");
              ttyin();
              nlines = 0;
}

ttyin() 
{
     char buf[BUFSIZ];
     static FILE *tty = NULL;

     if ( tty == NULL) {
         tty = fopen("/dev/tty", "r");
     }
     if( fgets(buf,BUFSIZ,tty) == NULL || buf[0] == 'q') {
	 exit(0);
     }
     else
	 return buf[0];
}
