/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)search:msearch.c	1.9"
#include "msearch.h"
#include <sys/utsname.h>

int popflag = FALSE;
int non_inter = FALSE;
int foundmatch = FALSE;

FILE *fplist;
char *word1file, *word2file;
char searchlib[STRSIZE];  /* $ASSISTLIB/search */
char subdir[STRSIZE];     /* directory for specific subject area */
                          /* current version of assist only has unix */
char err_label[STRSIZE];
char  *getenv();
int done();

int nlines = 0;


char tempdir[] = "/tmp";
char prefix1[] = "word1";
char prefix2[] = "word2";

main(argc, argv)
char **argv;
{
        int i, scan_ret, tmpstate;
        char *p, tmpstr[STRSIZE];
        char linebuf[BUFSIZ];
        char refname[MINCHARS];
        char word1[STRSIZE], word2[STRSIZE], extrawords[STRSIZE];
        char *tmpdir, *pfx1, *pfx2;
        FILE *fpmatch;
        int localflag = FALSE;
        int unixflag = FALSE;
        int match1 = FALSE;
        int match2 = FALSE;
        int nwords = 0;
        int pagesize = 22;  /* 24 line screen */

        if((p=getenv("ASSISTLIB"))!=NULL && p[0]!='\0') sprintf(searchlib,"%s/search", p);
        else sprintf(searchlib, "/usr/lib/assist/lib/search");
        
        for(i=1; argv[i][0]=='-'; i++) {
             switch(argv[i][1]) {
                 case 'd':
                       localflag = TRUE;
                       i++;
                       if(i < argc ) {
                          sprintf(subdir,"%s", argv[i]);
                       }
                       else {
                          fprintf(stderr, "Usage: msearch -d DIRNAME\n");
                          fprintf(stderr, "Specify directory containing search-file\n");
                          exit(2);
                       }
                       break;

                 case 'n':  /* set lines on page; default is 24 */
                       pagesize = atoi(argv[i] + 2) - 2;
                       break;


                 case 'p':  /* produce version for assist pop-up menu*/
                       popflag = TRUE;
                       break;

                 case 's':
                       non_inter = TRUE;
                       if(argv[i][2] != '\0') {
                          p = argv[i] + 2 ;
                          switch(nwords++) {
                               case 0: 
                                    sprintf(word1,"%s", p);
                                    break;
                               case 1:
                                    sprintf(word2, "%s", p);
                                    break;
                               default:
                                    break;
                           }
                       }
                       else {
                          i++;
                          switch(nwords++) {
                               case 0: 
                                    sprintf(word1,"%s", argv[i]);
                                    break;
                               case 1:
                                    sprintf(word2, "%s", argv[i]);
                                    break;
                               default:
                                    break;
                           }
                        }
                        break;

                 case 'u':  /** use to get UNIX command options fast */
                       unixflag = TRUE;
                       nwords = 1;
                       if(argv[i][2] != '\0') {
                          p = argv[i] + 2 ;
                          sprintf(word1,"%s", p);
                       }
                       else {
                          i++;
                          sprintf(word1,"%s", argv[i]);
                        }
                        break;

                 default:
                       fprintf(stderr,"msearch: %s is unknown option\n", argv[i]);
                       exit(2);
              }
        }

       if(popflag==TRUE) sprintf(err_label,"assist: msearch");
       else sprintf(err_label, "ref");


       if (unixflag == FALSE) {
         if (access("/usr/bin/fgrep", EXECUTE) != 0) {
              fprintf(stderr, "%s uses the UNIX system command fgrep;", err_label);
              fprintf(stderr, "but cannot execute /usr/bin/fgrep\non your system.\n");
              done(1);
         }
       }
       else {
         if (access("/bin/grep", EXECUTE) != 0) {
              fprintf(stderr, "%s with the -u option uses the UNIX system command grep;", err_label);
              fprintf(stderr, "\nbut cannot execute /bin/grep on your system.\n");
              done(1);
         }
       }


/*
 *   On interrupt, clean up the temporary files
 */

        signal(SIGINT, done );  



/*
 *  Find out what subject area is wanted and where it is
 */

         if (localflag == FALSE) {
          
             sprintf(tmpstr,"%s/searchlist", searchlib);
             fplist = fopen(tmpstr, "r");
             if(fplist == NULL) {
                sprintf(linebuf,"%s: %s/searchlist", err_label, searchlib);
                perror(linebuf);
                exit(1);
             }

             if( ( (i) == argc) && (unixflag == FALSE) ) {
                fprintf(stderr, "%s ", err_label );
                availmess();
                exit(1);
             }
             
             if(unixflag==TRUE) {
                non_inter = TRUE;
                word2[0] ='\0';
             }


             for(tmpstate=FALSE; fgets(linebuf,BUFSIZ, fplist) != NULL &&
                    tmpstate == FALSE; ) {
                  sscanf(linebuf,"%s %s", refname, subdir);
                  if(unixflag == FALSE) {
                      if(strcmp(refname,argv[argc-1])==0) tmpstate=TRUE;
                  }
                  else {
                      if(strcmp(refname,"unix")==0) tmpstate=TRUE;
                  }
             }
             if(tmpstate == FALSE) {
                 fprintf(stderr, "%s: ", err_label);
                 if(unixflag==FALSE) {
                     fprintf(stderr,"Couldn't find subject %s\n", argv[argc-1]);
                 }
                 else fprintf(stderr,"Couldn't find subject unix system\n");
                 rewind(fplist);
                 availmess();
                 exit(1);
             }
             if(popflag==FALSE) {
                 if(strcmp("unix", refname) ==0) printf("                 UNIX System Commands Reference\n\n");
                 else printf("                     %s Reference\n\n", refname);
             }
       }

/*  
 *  Check that search-file in subdir exists and is readable 
 *  If no search-file in sub-dir, check $ASSISTLIB/search/refname/search-file
 */
         sprintf(tmpstr,"%s/search-file",subdir);
         if(  (access(tmpstr, EXIST) != 0) || (access(tmpstr,READ) != 0) ) {
              sprintf(subdir,"%s/%s", searchlib, refname);
              sprintf(tmpstr,"%s/search-file",subdir);
              if(  (access(tmpstr, EXIST) != 0) || (access(tmpstr,READ) != 0) ) {
                  sprintf(linebuf, "%s: %s", err_label, tmpstr);
                  perror(linebuf);
                  exit(1);
              }
         }


/*
 *  Open file of synonyms
 */
        sprintf(tmpstr, "%s/matchpairs", subdir);
        fpmatch = fopen(tmpstr, "r");
        if(fpmatch == NULL) {
            sprintf(linebuf,"%s: %s/matchpairs", err_label, subdir);
            perror(linebuf);
            exit(1);
        }

/*
 *  Paging stuff
 */
        if (  (popflag == FALSE) && (non_inter == TRUE) ) pagesize = 10000 ; 
                      /* any number bigger than 0 */


/*
 *   Get two temporary file names in /tmp
 */
     tmpdir = tempdir;
     pfx1 = prefix1;
     pfx2 = prefix2;
     word1file = tempnam(tmpdir, pfx1);
     word2file = tempnam(tmpdir, pfx2);




   if(non_inter == FALSE) {
/*
 * Prompt user for first search word(s)
 */
        printf("Enter search word: ");
        fflush(stdout);

        if (fgets(linebuf, BUFSIZ, stdin) != NULL) {
            scan_ret = sscanf(linebuf, "%s %s %s",word1,word2, extrawords);
        }
        else done(0);

/*
 * Keep taking input until user enters q or CR or BREAK
 */

        while (strcmp(word1,"q") != 0) {

            if( scan_ret < 1 ) {      /* No search words entered */
                 done(0);
            }
            else if (scan_ret == 1) {  /* one search word entered */
                 printf("\n Beginning search for");
                 match1 = getsyn(fpmatch, word1, word1file);
                 find_one(word1, word1file, unixflag, match1, pagesize);
            }
            else {               /* More than one search word */
                 if(scan_ret > 2) printf("Using first two search words only\n");
                 printf("Beginning search for lines containing both ");
                 printf("a WORD1 and a WORD2.");
                 printf("\n   WORD1: ");
                 match1 = getsyn(fpmatch, word1, word1file);
                 if(match1==FALSE) printf("%s",word1);
                 printf("\n   WORD2: ");
                 match2 = getsyn(fpmatch, word2, word2file);
                 if(match2 == FALSE) printf("%s",word2);
                 printf("\n\n");
                 find_two(word1, word1file, word2, word2file, match1, match2, pagesize);
            }

            printf("\nNext search word (q to quit): ");
            fflush(stdout);
/*
 *  Zero lines for paging of next round of output
 */
            nlines = 0;

/*
 * Get next set of search words
 */
            for(i=0;linebuf[i] != '\0'; i++) {
                linebuf[i] = '\0';
            }
            if(fgets(linebuf, BUFSIZ, stdin) != NULL) {
                scan_ret = sscanf(linebuf,"%s %s %s",word1,word2, extrawords );
            }
            else done(0);
        }


    }    /***** end of interactive version of ref ****/


    else {    /**** non-interactive version of ref  ********/

         if(nwords==1) {
             if(unixflag==FALSE) printf("Beginning search for");
             match1 = getsyn(fpmatch,word1, word1file);
             find_one(word1, word1file, unixflag, match1, pagesize);
         }
         else {
             printf("Beginning search for lines containing both ");
             printf("a WORD1 and a WORD2.");
             printf("\n   WORD1: ");
             match1 = getsyn(fpmatch,word1, word1file);
             if(match1==FALSE) printf("%s",word1);
             printf("\n   WORD2: ");
             match2 = getsyn(fpmatch, word2, word2file);
             if(match2==FALSE) printf("%s",word2);
             printf("\n\n");
             find_two(word1, word1file, word2, word2file, match1, match2, pagesize);
         }


    }




/*
 * Finish up after q or CR
 */

        if(foundmatch == TRUE) done(0);
        else done(1);
}

/*
 *  Remove temporary files and exit
 */
done(exitcode)
int exitcode;
{

        unlink(word1file);
        unlink(word2file);
        exit(exitcode);
}

/*
 *  Routine to search and invoke printing for one search word
 *  and synonyms
 */

find_one(wd1, wd1file, uflag, m1, psize)
char *wd1, *wd1file;
int uflag;
int m1;
int psize;
{
     FILE *fpgrep;
     char tmpstr[STRSIZE];
     char sysbuf[BUFSIZ];
     if(uflag == TRUE) {
        sprintf(tmpstr,"/bin/grep \'^ %s \' %s/search-file", wd1, subdir);
     }
     else if (m1 == FALSE) {
         printf(" %s\n", wd1);
         sprintf(tmpstr, "/usr/bin/fgrep -i \" %s\" %s/search-file", wd1, subdir);
     }
     else {
         printf("\n");
         sprintf(tmpstr, "/usr/bin/fgrep -i -f %s %s/search-file", wd1file, subdir );
     }

     fpgrep = popen(tmpstr, "r");
     if(fpgrep == NULL) {
          fprintf(stderr, "%s: cannot read popen of /usr/bin/fgrep\n",err_label );
          done(1);
     }

     if( ( non_inter == TRUE) && (popflag == FALSE) ){
          while( fgets(sysbuf, BUFSIZ, fpgrep) != NULL) {
               line_end(sysbuf);
               format(sysbuf, psize);
          }
     }
     else {
          if (fgets(sysbuf, BUFSIZ, fpgrep) != NULL) {
                  line_end(sysbuf);
                  format(sysbuf, psize);
              while (fgets(sysbuf, BUFSIZ, fpgrep) != NULL) {
                  line_end(sysbuf);
                  format(sysbuf, psize);
              }
              foundmatch = TRUE;
          }
          else if (popflag == TRUE) printf("No match found.\n");
          else {
              printf("\nSearch word or synonyms not found.\n");
              printf("Try a different search word.\n");
          }
      }
     pclose(fpgrep);
}

/*
 *  Routine to search for two search words and synonyms
 */

find_two(wd1, wd1file, wd2, wd2file, m1, m2, psize)
char *wd1, *wd1file, *wd2, *wd2file;
int m1, m2;
int psize;
{
     char tmpstr[STRSIZE];
     char sysbuf[BUFSIZ];
     FILE *fpgrep;
     if (m1 == FALSE) {
         if(m2 == FALSE) {
            sprintf(tmpstr, "/usr/bin/fgrep -i \" %s\" %s/search-file | /usr/bin/fgrep -i \" %s\"",
                          wd1, subdir, wd2);
         }
         else { /*no word1 syn; at least one word2 syn */
            sprintf(tmpstr,
                "/usr/bin/fgrep -i \" %s\" %s/search-file | /usr/bin/fgrep -i -f %s", wd1, subdir, wd2file);
         }
     }
     else {   /* there is a word1 synonym */
         if(m2 == FALSE) {
            sprintf(tmpstr,
               "/usr/bin/fgrep -i -f %s %s/search-file | /usr/bin/fgrep -i \" %s\"",wd1file, subdir, wd2);
         }
         else {
            sprintf(tmpstr,
             "/usr/bin/fgrep -i -f %s %s/search-file | /usr/bin/fgrep -i -f %s", wd1file, subdir, wd2file );
         }
     }

     fpgrep = popen(tmpstr, "r");
     if(fpgrep == NULL) {
          fprintf(stderr,"%s uses the UNIX system command fgrep.\n", err_label);
          fprintf(stderr,"%s: cannot read pipe fgrep on your system.\n",err_label);
          done(1);
     }

     if(non_inter == TRUE) {
          while(fgets(sysbuf, BUFSIZ, fpgrep) != NULL) {
               line_end(sysbuf);
               format(sysbuf, psize);
          }
     }
     else {
         if (fgets(sysbuf, BUFSIZ, fpgrep) != NULL) {
             line_end(sysbuf);
             format(sysbuf, psize);
             while (fgets(sysbuf, BUFSIZ, fpgrep) != NULL) {
                 line_end(sysbuf);
                 format(sysbuf, psize);
             }
             foundmatch=TRUE;
         }
         else {
             printf("\nNo pairs matched.  ");
             printf("Try one search word at a time.\n");
         }
     }
     pclose(fpgrep);
}

line_end(string)
char *string;
{
    char *q, ending[MAXCHARS];
    for(q = string; *q != '\0'; q++) ;
    if( *(q-5) == '*') {
        *(q-5) = '\0';
        *(q-1) ='\0';
        if (strcmp( q-4 , "DWB") == 0) {
            sprintf(string, "%s  [Documenter's Workbench2.0]\n", string);
        }
        else {
            sprintf(ending,"%s", q-4);
            sprintf(string,"%s [New with SystemV, Release%s]\n", string, ending);
        }
    }
}


availmess()
{
    char linebuf[BUFSIZ];
    char refname[MINCHARS];
    fprintf(stderr, "Available references are ");
    while(fgets(linebuf, BUFSIZ, fplist) != NULL) {
        sscanf(linebuf, "%s", refname);
        fprintf(stderr, " %s", refname);
    }
    fprintf(stderr, "\n");
}

