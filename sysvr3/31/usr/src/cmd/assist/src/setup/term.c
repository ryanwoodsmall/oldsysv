/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)setup:term.c	1.13"
#include "msetup.h"

#define MAXTRIES 4 /* Number of term names user can try before program exits */
#define IDENTIFY_TERM 8
#define TERM_IDENT_PROBLEM 100
#define INTRO_TO_TERM_CHECK 20
#define BASIC_CHECK 30
#define STANDOUT_CHECK 41
#define GRAPH_CHAR_CHECK 46
#define END_TERM_CHECK 60
#define FKEY_CHECK 50
#define BAD_FKEYS 51
extern FILE *fpassistrc;  /* pointer to .assistrc file in user's home directory */
extern char *getenv();
extern char assistrc[MAXCHAR];  /*complete pathname of user's .assistrc file*/
extern char assistbin[MAXCHAR];  /* $ASSISTBIN-dir with assist executables */
extern char assistlib[MAXCHAR];  /* $ASSISTLIB - root dir. assist datafiles */
extern char term[MAXCHAR]; /* array for terminal name */
extern char terminfo[MAXCHAR];
extern int mfkeys, maltchar, mstandout;
extern int sflag; 
extern char clear[NCHAR];

/* Following are variables exported by setup using putenv() */
char sobuf[NCHAR];      /* array for "MSTANDOUT=.." */
char fkeybuf[NCHAR];    /* array for "MFKEYS=..." */
char altbuf[NCHAR];     /* array for "MALTCHAR=..." */
char promptstr[NCHAR];  /* array for "PS1=..."  */

/*
 *   findterm() helps user get the name of his/her terminal.  findterm() is
 *   called when there is no environmental value of TERM.  If the user asks,
 *   findterm() calls getterm(), which lists all the terminals in
 *   /usr/lib/terminfo/A and $TERMINFO/A where A is a user supplied
 *   alphnumeric character corresponding to the first character of
 *   the name of the user's terminal.
 */

findterm()
{
    char tmpbuf[BUFSIZ];
    char err_line[MAXCHAR];
    char p, *t;
    int kount;
    int foundtname = FALSE;
    int termok = FALSE;

    if( (t=getenv("TERMINFO"))!=NULL && *t!=NULL) sprintf(terminfo, "%s", t);
    else sprintf(terminfo,"/usr/lib/terminfo");

/*
 *  Look for line in .assistrc beginning with "TERM" and the name of the
 *  last used terminal.  If the line is
 *  found, ask if that is name of current terminal.
 */
    rewind(fpassistrc);

    while((fgets(tmpbuf, MAXCHAR,fpassistrc) != NULL)&&(foundtname == FALSE))  {
       if (sscanf(tmpbuf, "TERM %s", term) == 1) foundtname = TRUE;
    }

    if (foundtname == TRUE)  {
        fflush(stdin);
        printf("\n  --> Is your terminal a %s ? (y/n/?) : ", term);
        p = WRONG_ANS;
        charans(&p);
        while( tolower(p) != 'y' && tolower(p) != 'n' && p != '?') {
            fflush(stdin);
            printf("\n  Answer y if your terminal is a %s ",term);
            printf("\n  Answer n if your terminal is not a %s", term);
            printf("\n  Answer ? if you want help with naming your terminal");
            printf("\n\n  --> Is your terminal a %s ? (y/n/?): ", term);
            charans(&p);
        }
        switch(p) {
           case 'y':
           case 'Y':
                break;
           case 'n':
           case 'N':
                foundtname = FALSE;
                fflush(stdin);
                printf("  --> Please enter terminal name: ");
                scanf("%s", term);
                fflush(stdin);
                break;
           case '?':
                foundtname = FALSE;
                term[0] = '?';
                break;
           default:
                fprintf(stderr, "assist: setup: Error in findterm() \n");
                exit(1);
                break;
        }
    }
    if (foundtname == FALSE) {
        while( term[0] == NULL) {
            fflush(stdin);
            printf("  --> Enter terminal name ");
            printf("or ? for help with naming terminal : ");
            fgets(tmpbuf, BUFSIZ, stdin);
            sscanf(tmpbuf, "%s", term);
            fflush(stdin);
        }
        for (kount=0; kount < MAXTRIES && termok == FALSE; kount++) {
           if (term[0] == '?') getterm();

           if (checkterm() == TRUE) termok = TRUE;
           else {
                  printf("\n\n  Do not have information for %s terminal.\n",term);
                  fflush(stdin);
                  printf("\n  --> Enter another name or ? for help: ");

                  fgets(tmpbuf, BUFSIZ, stdin);
                  sscanf(tmpbuf, "%s", term);
                  fflush(stdin);
           }
        }
        if (termok == FALSE)  {
            message(TERM_IDENT_PROBLEM);  /* tell user about assist having problems */
                           /* identifying the user's terminal        */
            exit(1);
        }
/*
 *  Put most recent terminal name in .assistrc.  The next time user enters
 *  assist with a null value of TERM, assist will ask if user's
 *  terminal has this name.
 */
        sprintf(tmpbuf,"TERM %s\n", term);
        update(tmpbuf);
        fclose(fpassistrc);
        if ( (fpassistrc = fopen(assistrc, "r")) == NULL) {
           sprintf(err_line,"assist: setup: %s", assistrc);
           perror(err_line);
           exit(1);
        }
    }
}

checkterm()
{
    char tmpbuf[MAXCHAR], tmpbuf2[MAXCHAR];
    sprintf(tmpbuf,"/usr/lib/terminfo/%c/%s", term[0], term);
    sprintf(tmpbuf2, "%s/%c/%s", terminfo, term[0], term);

    /* check that user can read a terminfo entry */
    if( (access(tmpbuf, READ) == 0) || (access(tmpbuf2, READ) == 0) ) return(TRUE);
    else return(FALSE);
    
}

readrc()
{
    int ret;
    char termname[MAXCHAR], prompt[MAXCHAR];
    char linebuf[MAXCHAR], tmpbuf[MAXCHAR];
    rewind(fpassistrc);
    while( fgets(linebuf, MAXCHAR, fpassistrc) != NULL)  {
         ret = sscanf(linebuf, "%s %d %d %d", 
                   termname, &mstandout, &mfkeys, &maltchar);
         switch(linebuf[0]) {
             case 'P':
                 /*
                  *  Gets line with PS1 and exports it.
                  *  PS1 line must come before terminal definitions
                  */
                 if (strcmp("PS1",termname) == 0) {
                     sscanf(linebuf,"%s %s", termname, prompt);
                     sprintf(promptstr,"PS1=%s ", prompt);
                     putenv(promptstr);   
                 }
                 else if( (strcmp(term,termname) == 0) && (ret == 4) ) {
                     exp_atts();
                     return(SUCCESS);
                 }
                 break;

             default:
                 /* ret == 4 means found values for the 3 assist variables */
                 if( (strcmp(term, termname) == 0) && (ret == 4) ){
                     exp_atts();  /* get attributes and export them */
                     return(SUCCESS);
                 }
                 break;
         }
    }

    if (testterm() == SUCCESS) {
        sprintf(tmpbuf,"%s %d %d %d\n", term, mstandout, mfkeys, maltchar);
        update(tmpbuf);
        exp_atts();
        message(END_TERM_CHECK);
        getreturn();
        return(SUCCESS);
    }

/*
 *  If testing didn't work or terminal doesn't show basic test pattern
 *  then remove the terminal entry from .assistrc file
 */
    sprintf(tmpbuf, "DELETE %s\n", term); 
    update(tmpbuf);
    return(FAILURE);
}

testterm()
{
    FILE *pp;

    if(basictest() == FAILURE) return(FAILURE);

/*
 * Now we know what the terminal is so we can clear the
 * screen so that messages write from the top of the screen down
 * instead of scrolling up from the bottom
 */
    if (access("/usr/bin/tput", EXECUTE) == 0) {
        if ( (pp = popen("/usr/bin/tput clear" , "r")) != NULL) 
            fgets(clear, NCHAR, pp);
    }

    printf("\n%s", clear);
    fflush(stdout);
    if(sotest() == FAILURE) return(FAILURE);
    printf("\n%s", clear);
    fflush(stdout);
    if(alttest() == FAILURE) return(FAILURE);
    if(fkeytest() == FAILURE) return(FAILURE);
    printf("\n%s", clear);
    fflush(stdout);
    return(SUCCESS);
}

alttest()
{
    char c,  tmpbuf[BUFSIZ], tmpcmd[MAXCHAR];
    int sysret;
    sprintf(altbuf,"MALTCHAR=1");
    putenv(altbuf);
    for (c = 'r'; c == 'r';) {
        printf("\n%s", clear);
        fflush(stdout);
        message(GRAPH_CHAR_CHECK);
        fflush(stdin);
        printf("  --> Strike the RETURN key to see the pattern ");
        fgets(tmpbuf,BUFSIZ, stdin);
        printf("\n%s", clear);
        fflush(stdout);
        sprintf(tmpcmd, "%s/mscript %s/setup/t.altchar", assistbin, assistlib);

        /* system returns an integer whose lower 8 bits reflect system's
         * idea of child's exit status (0 for normal termination)
         * Higher 8 bits are taken from the argument of the call to exit
         * or retrun from main that caused termination of the child process
         * (Kernighan and Pike pp 222-223)  Dividing the value returned by
         * system() by 256 yields the higher 8 order bits
         */
        sysret = (system(tmpcmd))/256;
        if( sysret == BREAK ) done();
        if (sysret == NO_MSCRIPT) {
           fprintf(stderr, "assist: setup: error in mscript\n");
           done();
        }
        c = WRONG_ANS;
        while(tolower(c) != 'y' && tolower(c) != 'n' && tolower(c) != 'r') {
            fflush(stdin);
            printf("  --> Is the rectangle made of continuous lines and\n      corners?");
            printf(" [Answer y or n (or r to repeat)] ");
            charans(&c);
        }
    }
    if(tolower(c) == 'y') maltchar = 1;
    else maltchar = 0;
    sprintf(altbuf, "MALTCHAR=%d", maltchar);
    putenv(altbuf);
    printf("\n%s", clear);
    fflush(stdout);
    return(SUCCESS);
}

fkeytest()
{
    char  tmpbuf[BUFSIZ], tmpcmd[MAXCHAR];
    char c = WRONG_ANS;
    int sysret;
    message(FKEY_CHECK);
    fflush(stdin);
    printf("  --> Do you have at least 8 function keys? (y/n): ");
    charans(&c);
    while (tolower(c) != 'n' && tolower(c) != 'y') {
        fflush(stdin);
        printf("  --> Please answer either y or n: ");
        charans(&c);
    }
    if (tolower(c) == 'n') {
        mfkeys = FALSE;
        return(SUCCESS);
    }
    for (c = 'y'; tolower(c) == 'y'; ) {
        fflush(stdin);
        printf("\n  --> Strike the RETURN key to check your function keys ");
        fgets(tmpbuf,BUFSIZ, stdin);
        fflush(stdin);
        printf("\n%s", clear);
        fflush(stdout);
        sprintf(tmpcmd, "%s/mscript %s/setup/t.fkeys", assistbin, assistlib);
        sysret = (system(tmpcmd))/256;
        if ( sysret  == 2 ) { /* returned if both f8 and f1 received */
           mfkeys = TRUE;
        }
        else if (sysret == BREAK) done();
        else if (sysret == NO_MSCRIPT) {
           fprintf(stderr, "assist: setup: error in mscript\n");
           done();
        }
        else mfkeys = FALSE;
        printf("\n%s\n    This check showed that the signals from your ", clear);
        printf("terminal's\n    function keys were ");
        if (mfkeys == TRUE) printf("understood by the system.\n\n");
        else {
            printf("not understood. \n\n");
            message(BAD_FKEYS);
        }
        fflush(stdin);
        if(mfkeys == TRUE) {
            getreturn();
            return(SUCCESS);
        }
        else {
            fflush(stdin);
            printf("     --> Would you like to repeat the function key check? [Answer y or n]: ");
            charans(&c);
            while(tolower(c) != 'y' && tolower(c) != 'n') {
                fflush(stdin);
                printf("\n        Please answer y or n.\n\n");
                printf("      --> Would you like to repeat the function key check? (y/n): ");
                charans(&c);
            }
        }
    }
    return(SUCCESS);
}

sotest()
{
    char tmpbuf[BUFSIZ], tmpcmd[MAXCHAR];
    char c;
    int sysret;
    sprintf(sobuf,"MSTANDOUT=1");
    putenv(sobuf);
    for(c = 'r'; c == 'r';) {
        printf("\n%s", clear);
        fflush(stdout);
        message(STANDOUT_CHECK);
        fflush(stdin);
        printf("  --> Strike the RETURN key to see the pattern ");
        fgets(tmpbuf,BUFSIZ, stdin);
        fflush(stdin);
        printf("\n%s", clear);
        fflush(stdout);
        sprintf(tmpcmd, "%s/mscript %s/setup/t.standout", assistbin, assistlib);
        sysret = (system(tmpcmd))/256;
        if( sysret == BREAK ) done();
        if (sysret == NO_MSCRIPT) {
           fprintf(stderr, "assist: setup: error in mscript\n");
           done();
        }

        c = WRONG_ANS;
        fflush(stdin);
        while(tolower(c) != 'y' && tolower(c) != 'n' && tolower(c) != 'r') {
            printf("  --> Does the second line standout?");
            printf(" [Answer y or n (or r to repeat)] ");
            charans(&c);
        }
    }
    if(tolower(c) == 'y') mstandout = 1;
    else mstandout = 0;
    sprintf(sobuf, "MSTANDOUT=%d", mstandout);
    putenv(sobuf);
    printf("\n%s", clear);
    fflush(stdout);
    return(SUCCESS);
}

basictest()
{
    char tmpbuf[BUFSIZ], tmpcmd[MAXCHAR];
    char c;
    int sysret;
    printf("\n             CHECKING OF TERMINAL NAMED:  %s\n", term);
    message(INTRO_TO_TERM_CHECK);
    for(c = 'r'; tolower(c) == 'r';) {
        message(BASIC_CHECK);
        fflush(stdin);
        printf("  --> Strike the RETURN key to see the pattern ");
        fgets(tmpbuf,BUFSIZ, stdin);
        fflush(stdin);
        sprintf(tmpcmd, "%s/mscript %s/setup/t.basic", assistbin, assistlib);
        sysret = (system(tmpcmd))/256;
        if( sysret == BREAK ) done();
        if (sysret == NO_MSCRIPT) {
           fprintf(stderr, "assist: setup: error in mscript\n");
           done();
        }
        if (sysret == 1) done();
        c = WRONG_ANS;
        fflush(stdin);
        while(tolower(c) != 'y' && tolower(c) != 'n' && tolower(c) != 'r') {
            printf("  --> Do you see the pattern?");
            printf(" [Answer y or n (or r to repeat)] ");
            charans(&c);
        }
    }
    
    if (tolower(c) == 'n') return(FAILURE);
    return(SUCCESS);
}

exp_atts()
{
        sprintf(sobuf,"MSTANDOUT=%d",mstandout);
        putenv(sobuf);
        sprintf(fkeybuf, "MFKEYS=%d", mfkeys);
        putenv(fkeybuf);
        sprintf(altbuf, "MALTCHAR=%d", maltchar);
        putenv(altbuf);
}
getterm()
{
    char b1[MAXCHAR], b2[MAXCHAR], b3[MAXCHAR], b4[MAXCHAR];
    char buff[BUFSIZ];
    char c = WRONG_ANS;
    message(IDENTIFY_TERM);
    fflush(stdin);
    while (term[0] == '?') {
        printf("\n  --> Enter the first character (alphabetic letter or");
        printf("\n      digit) of the possible name of your terminal: ");
        charans(&c);
        while(isalnum(c) == FALSE) {
            printf("\n  --> Please enter only an alphabetic letter or a digit: ");
            charans(&c);
        }

        printf("\n  Checking for terminals beginning with %c ", c);
        printf("that are currently\n  known to this system. \n\n");

/*
 *  getterm() runs a shell script that lists all files that begin with A
 *  and are in $TERMINFO/A or /usr/lib/terminfo/A, where A is the user
 *  supplied first character of the terminal name.
 */
        if(ck_cmd("/bin/sh")==FALSE || ck_cmd("/bin/ls")==FALSE ||
            ck_cmd("/bin/grep")==FALSE || ck_cmd("/bin/pr")== FALSE) {
            exit(1);
        }

        sprintf(b1,

             "T=`if test -d /usr/lib/terminfo/%c;then ls /usr/lib/terminfo/%c",
              c, c);
        sprintf(b2,
             " | grep \"^%c\";fi`;U=`if test -n \"$TERMINFO\" ",
              c);
        sprintf(b3,
             " && test \"$TERMINFO\" != \"/usr/lib/terminfo\" ");
        sprintf(b4,
             "&& test -d $TERMINFO/%c;then ls $TERMINFO/%c | grep \"^%c\";",
              c, c, c);
               
        sprintf(buff,"%s%s%s%s", b1, b2, b3, b4);
    
        sprintf(b1,
             "fi`;if test -n \"$U\" || test -n \"$T\" ;then for i in ");
        sprintf(b2,
             "`echo $U $T`;do echo \"$i\";done | pr -t -5; else ");
        sprintf(b3,
             "echo \"  No terminals beginning with %c\"; fi",
             c);
 
        sprintf(buff, "%s%s%s%s", buff, b1, b2, b3);
        system(buff);
        
        if(sflag == TRUE) exit(0);

        fflush(stdin);
        printf("\n  --> Enter terminal name (or ? for another list of names) : ");
        fgets(buff, BUFSIZ, stdin);
        sscanf(buff, "%s", term);
        fflush(stdin);
    }
}

ck_cmd(cmd_name)
char cmd_name[MAXCHAR];
{
    if(access(cmd_name, EXECUTE) == 0) return(TRUE);
    else {
        fprintf(stderr,"\nassist: setup: cannot execute the command %s\n", cmd_name);
        return(FALSE);
    }
}
