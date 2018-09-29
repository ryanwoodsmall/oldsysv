/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)setup:setup/mset.c	1.14"
#include "msetup.h"
#define ASSIST_USAGE 120
#define TERM_SPEC_PROBLEM 101
char *getenv();
int toolflag = FALSE;
FILE *fpassistrc;  /* pointer to .assistrc file in user's home directory */
char assistrc[MAXCHAR];  /*complete pathname of user's .assistrc file*/
char assistbin[MAXCHAR];  /* $ASSISTBIN-directory with assist executables */
char assistlib[MAXCHAR];  /* $ASSISTLIB - root directory of assist datafiles */
char term[MAXCHAR]; /* array for "TERM=.." */
char termbuf[NCHAR]; /* used with putenv() to export TERM */
char terminfo[MAXCHAR];
int mfkeys, maltchar, mstandout;
int newuser = FALSE;
int cnewuser = FALSE;
int sflag = FALSE;   /* -s option to assist; gives user direct */
                    /* access to assist's setup functions     */
char clear[NCHAR];  /* String for user's terminal that will clear */
                    /* the screen; used to allow text to write from */
                    /* the top of the screen down, rather than having */
                    /* the text scroll up at the bottom of the screen */
int done();

main(argc, argv)
int argc;
char **argv;
{
    char *p, *q;
    int k;
    char *pathptr, pathname[MAXCHAR];
    char err_line[MAXCHAR];
/*
 * Exit if restricted shell
 */
   if( (p=getenv("SHELL")) != NULL) {
      for(q=p; *q != '\0' ; q++) ;
      while( (*q != '/') && (q > p) ) {
          if (*q == 'r') {     /* restricted shell definition */
             fprintf(stderr, "You cannot use ASSIST because you have a restricted shell.\n");
             fprintf(stderr, "Please contact your system administrator if you wish to use ASSIST.\n");
             exit(0);
          }
          q--;
      }
   }

/* Get the values of ASSISTBIN and ASSISTLIB.  These values are exported
 * by the assist.sh program that is kept in /usr/bin .  This design
 * allows the "assist" programs to be stored in places other than
 * /bin and /usr/bin with out having to put that info in  the
 * user's .profile.  Keeping the values in a shell script allows
 * the system administrator to move assistbin and the assistlib directory
 * to other locations withour recompilation (just with binary assist
 * available).  The default value for ASSISTBIN is /usr/lib/assist/bin
 * and for ASSISTLIB is /usr/lib/assist/lib 
 */
    
    if((p=getenv("ASSISTBIN"))!=NULL && *p != NULL) sprintf(assistbin,"%s",p);
    else sprintf(assistbin,"/usr/lib/assist/bin");

    if((p=getenv("ASSISTLIB"))!=NULL && *p != NULL) sprintf(assistlib,"%s",p);
    else sprintf(assistlib,"/usr/lib/assist/lib");

    ck_mscript();

       
/*
 *  Process command line
 */

    for(p = argv[0]; *p != '\0'; p++);
    p = p - 6;  /*match all path names that end in tsetup */
    
    if(strcmp(p,"tsetup") != 0) {   /* not astgen */

        for(k = 1; argv[k][0] == '-'; k++) {
           switch(argv[k][1])  {
               case 's':        /* setup */
                    sflag = TRUE;
                    break;
               case 'c':
                    break;
               default:
                    message(ASSIST_USAGE);
                    exit(1);
           }
        }
        if (k > 2) {
           message(ASSIST_USAGE);
           exit(1);
        }
        if (argc > k + 1) {
           message(ASSIST_USAGE);
           exit(1);
        }
      
        if (sflag == TRUE) {
            if (argc > 2) {
                message(ASSIST_USAGE);
                exit(1);
            }
            else {
                if (choices() == EXIT) exit(0);
                /*  Don't exit from choices if choice c (introinfo)    */
                /*  or choice a when user is new user (no .assistrc file)   */
            }
        }

    }
    else {
        toolflag = TRUE;
        if (argc != 2) {
           fprintf(stderr,"astgen usage: astgen name_of_.fs_file\n");
           exit(1);
        }
        sprintf(err_line,"%s/astgen", assistlib);
        if(access(err_line,EXECUTE) != 0) {
           fprintf(stderr,"astgen: setup: Cannot access %s directory\n",err_line);
           exit(1);
        }
    }


/* openrc decides if user is newuser*/

    openrc();
/*
 * Is there a special terminfo directory?
 */

    if( (p=getenv("TERMINFO")) != NULL && *p != NULL) sprintf(terminfo, "%s", p);
    else sprintf(terminfo,"/usr/lib/terminfo");

/*
 *  Is there a value for TERM in the environment?
 *  If not, do something to get a value for TERM
 *  Export whaterver value of term is found.
 */
    if(  ((p=getenv("TERM")) == NULL) || (*p == NULL)  )   findterm(); 
    else sprintf(term,"%s",p); 
    sprintf(termbuf, "TERM=%s", term);
    putenv(termbuf);

/*
 *
 *  Make sure there is a terminfo entry for TERM
 */
    if (checkterm() == FALSE) {
        if (toolflag == FALSE) fprintf(stderr, "assist: ");
        else fprintf(stderr, "astgen: ");
        fprintf(stderr, "cannot find terminfo information file for ");
        fprintf(stderr, "%s terminal type.\n", term);
        fprintf(stderr,"Use assist -s to identify  known terminfo entries.\n" );
        /*
         * newuser is set to FALSE if there is no $HOME/.assistrc
         * file. We need to remove $HOME/.assistrc if there is no terminfo entry
         * for term because the user is still a new user; the user hasn't
         * gone thru the introduction to assist.
         */
        if(newuser == TRUE) unlink(assistrc);  
        exit(1);
    }

/*
 *  Look at .assistrc.  If there's a value matching term and 3 other
 *  values, then readrc() exports MSTANDOUT, MFKEYS, and MALTCHAR
 *  If not, readrc() invokes initialization for that terminal type.
 */
    if ( readrc() != SUCCESS)  {
        if(newuser == TRUE) unlink(assistrc);
        fprintf(stderr, "\n\n                 TERMINAL SPECIFICATION PROBLEM");
        fprintf(stderr, "\n\n        Terminal name %s does not work with your terminal.\n\n",
                 term);
        message(TERM_SPEC_PROBLEM);
        exit(1);
    }

/*
 *  newuser is set to TRUE by openrc() if there is no $HOME/.assistrc file
 *  cnewuser is set to TRUE if the command used was "assist -s" and the
 *  user chose option c (show introductory information).
 */
    if(newuser == TRUE || cnewuser == TRUE) intro(); 

    if(toolflag==TRUE) sprintf(pathname, "%s/tforms", assistbin);
    else sprintf(pathname, "%s/mforms", assistbin);

    if(access(pathname, EXECUTE) != 0) {
       sprintf(err_line,"assist setup: %s",pathname);
       perror(err_line);
       exit(1);
    }


    pathptr = pathname;
    signal(SIGINT, SIG_IGN);
/*
 *  If the user is new user of assist, he/she is put into assist's TOP
 *  menu even if they have typed assist with an argument.
 *  Also, people who have chosen the "show introductory info" answer to
 *  the -s option are placed in the assist TOP menu at the end of the
 *  introductory info
 */
    if(newuser == TRUE || cnewuser == TRUE) argv[1] = NULL;

    execv(pathptr, argv);
/*
 *  If the exec succeeded, then this point is not reached.
 */
    if(toolflag == TRUE) fprintf(stderr,"astgen: tsetup: ");
    else fprintf(stderr,"assist: msetup: ");
    fprintf(stderr, "exec of %s failed\n", pathname);

    fprintf(stderr,
        "Please contact your system administrator. \n");

    exit(1);
}

/*  
 *  The "messages" file in $ASSISTLIB/setup contains the text for much
 *  of the text shown on the screen by mset.  The text is divided by
 *  lines beginning with "X" followed by a number (eg X 101).
 *  The call message(101) will look thru "messages" until it finds the
 *  line "X 101".  It will then print the lines from "messages" until it reaches
 *  a the line beginning with "X" or "Z".  If the line begins with "X", 
 *  the message is finished;  if the line begins with "Z", messages() waits for
 *  the user to hit the RETURN key before continuing.  The "Z" line is used
 *  for messages that require more than one 24-line page to complete.
 */
message(n)
int n;
{
    int m = 0;
    char tmpbuf[MAXCHAR];
    char err_line[MAXCHAR];
    FILE *fpmessage;   /* pointer to "messages" file in $ASSISTLIB/setup */
    sprintf(tmpbuf,"%s/setup/messages", assistlib);

    if ( (fpmessage = fopen(tmpbuf,"r")) == NULL) {
        sprintf(err_line, "assist setup: %s",tmpbuf);
        perror(err_line);
        exit(1);
    }
	
    while( (m != n) && (fgets(tmpbuf, MAXCHAR, fpmessage) != NULL) ) {
       sscanf(tmpbuf,"X %d", &m);
    }
    if (m != n)  {
        fprintf(stderr, "assist: Could not find message numbered %d in\n", n);
        fprintf(stderr, "%s/setup/messages file.\n", assistlib);
        exit(1);
    }
    while( (fgets(tmpbuf, MAXCHAR, fpmessage) != NULL) && tmpbuf[0] != 'X') {
        if (tmpbuf[0] == 'Z') getreturn();
        else printf("%s\r", tmpbuf);
    }
    fflush(stdout);
    fclose(fpmessage);
}

getreturn()
{
    char tmpbuf[BUFSIZ];
    printf("  --> Strike the RETURN key to continue. ");
    fgets(tmpbuf, BUFSIZ, stdin);
    printf("\n%s", clear);
    fflush(stdout);
    fflush(stdin);
}

openrc()
{
    char *p, tmpbuf[MAXCHAR];
    char linebuf[BUFSIZ];
    char err_line[MAXCHAR];
    FILE *fpa;
/*
 *  Get complete pathname of .assistrc file
 */
    if((p=getenv("HOME")) == NULL || *p == NULL) {
        fprintf(stderr, "assist: setup: cannot identify your login (HOME) directory\n");
        exit(1);
    }
    else sprintf(assistrc, "%s/.assistrc", p);


/*
 *  Is this a new ASSIST user?  (No $HOME/.assistrc file)
 *  If so, copy $ASSISTLIB/setup/assistrc file into $HOME
 */
    if(access (assistrc, EXIST) != 0) {
       newuser = TRUE;
        signal(SIGINT, done);
        sprintf(tmpbuf,"%s/setup/assistrc", assistlib);
        if( (fpa = fopen(tmpbuf, "r") ) == NULL) {
            fprintf(stderr,"assist: setup: Cannot create .assistrc file because");
            fprintf(stderr," assist cannot read %s/setup/assistrc\n", assistlib);
            exit(1);
        }
        if( (fpassistrc = fopen(assistrc,"w")) == NULL) {
            fprintf(stderr, "assist: setup: Cannot write to %s\n", assistrc);
            exit(1);
        }
        while(fgets(linebuf, BUFSIZ, fpa) != NULL) fputs(linebuf, fpassistrc);
        fclose(fpa);
        fclose(fpassistrc);
        if(toolflag == TRUE) {
           printf("\n%s\n", clear);
           printf("astgen: assuming you are an experienced ASSIST user\n");
           printf("        If you haven't used the \"assist\" command before, ");
           printf("please become familiar\n");
           printf("        with ASSIST before using \"astgen\".\n");
           newuser = FALSE;
           printf("\n  --> Strike the RETURN key to continue with astgen. ");
           fgets(tmpbuf, BUFSIZ, stdin);
        }
    }
    if ( (fpassistrc = fopen(assistrc, "r")) == NULL) {
       sprintf(err_line,"assist: setup: %s", assistrc);
       perror(err_line);
       exit(1);
    }
}

/*
 *  This is done when the -s option is used with assist
 */
choices()
{
    char p, *t, tmpbuf[MAXCHAR];
    printf("\n  Choices are:");
    printf("\n    a) Check/recheck terminal");
    printf("\n    b) List terminals (terminfo entries) on this system.");
    printf("\n    c) Show introductory ASSIST information");
    fflush(stdin);
    printf("\n\n  --> Enter choice: ");
    for(p = WRONG_ANS; p == WRONG_ANS;) {
        charans(&p);
        switch(p) {
            case 'a':
                openrc();
                if( ((t = getenv("TERM")) == NULL) || (*t == NULL) ) {
                    findterm();
                }
                else sprintf(term, "%s", t);
                sprintf(termbuf, "TERM=%s", term);
                putenv(termbuf);

                if( (t=getenv("TERMINFO")) != NULL && *t != NULL) sprintf(terminfo, "%s", t);
                else sprintf(terminfo,"/usr/lib/terminfo");

                if (checkterm() == FALSE) {
                    if (toolflag == FALSE) fprintf(stderr, "assist: setup ");
                    else fprintf(stderr, "astgen: setup ");
                    fprintf(stderr,"cannot find terminfo information file for");
                    fprintf(stderr, " %s terminal type.\n", term);
                    fprintf(stderr,"Use assist -s (choice b) to identify  known terminfo entries.\n" );
                    if(newuser == TRUE) unlink(assistrc);  
                    exit(1);
                 }
                if ( testterm() == SUCCESS) {
                     sprintf(tmpbuf, "%s %d %d %d\n", term, mstandout,
                               mfkeys, maltchar);
                     update(tmpbuf);
                     printf("\n%s terminal initialized for ASSIST use\n\n", term);
                }
                else {
                     sprintf(tmpbuf, "DELETE %s\n", term);
                     update(tmpbuf);
                     message(TERM_SPEC_PROBLEM);
                }
                if (newuser == TRUE) return(CONTINUE);
                else return(EXIT);
                break;
            case 'b':
                printf("\n\n");
                term[0] = '?';
                getterm();
                return(EXIT);
                break;
            case 'c':
                cnewuser = TRUE;
                return(CONTINUE);
                break;
            case NULL:
            case 'q':
                return(EXIT);
                break;
            default:
                p = WRONG_ANS;
                fflush(stdin);
                printf("\n  --> Enter letter corresponding to choice or q to quit: ");
                break;
        }
    }
}

done()
{
    if (newuser == TRUE) unlink(assistrc);
    exit(1);
}

/*
 *  get answer and make sure only one char
 */
charans(q)
char *q;
{
    char tmpbuf[BUFSIZ];
    char *ptr;
    ptr = tmpbuf;
    fgets(tmpbuf, BUFSIZ, stdin);
    while(*ptr == ' ' || *ptr == '\t') ptr++;  /* jump over white space */
    fflush(stdin);
    *q = *ptr;
    for(ptr++; *ptr != 10 && *ptr != NULL; ptr++) {
        if(*ptr != ' ' && *ptr != '\t') *q = WRONG_ANS;
    }
}
ck_mscript()
{
   char tmpstr[BUFSIZ];
   sprintf(tmpstr, "%s/mscript", assistbin);
   if(access(tmpstr, EXECUTE) != 0) {
      fprintf(stderr, "assist: setup: Cannot execute %s\n", tmpstr);
      exit(1);
   }
}
