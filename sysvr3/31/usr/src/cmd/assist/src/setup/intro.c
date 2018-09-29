/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)setup:setup/intro.c	1.6"
#include "msetup.h"
#define CONTROL_CHARS 10
#define ASSIST_SUMMARY 3
#define ASSIST_WALK_INTRO 4
#define EXIT_ASSIST_WALK 5
#define HOW_TO_ACCESS_ASSIST_WALK 6
#define CTRL_C_NOTATION 11
#define REV_1_CTRL_C 12
#define REV_2_CTRL_C 13
int rawdone();
extern char assistbin[MAXCHAR];  /* $ASSISTBIN-dir with assist executables */
extern char clear[NCHAR];

intro()
{
    FILE *pp;
    char *p, tmpbuf[BUFSIZ];
    char err_line[MAXCHAR];

    if (access("/usr/bin/tput", EXECUTE) == 0) {
        if ( (pp = popen("/usr/bin/tput clear" , "r")) != NULL) 
            fgets(clear, NCHAR, pp);
    }

    p = tmpbuf;
    printf("\n%s\n", clear);
    message(CONTROL_CHARS);

    signal(SIGINT, rawdone);
    signal(SIGQUIT, rawdone);

    /* Teach user about control ccharacters */
    control();

    printf("\n%s\n", clear);
    message(ASSIST_SUMMARY);
    getreturn();
    message(ASSIST_WALK_INTRO);
    *p  = WRONG_ANS;
    while ( tolower(*p) != 'y' && tolower(*p)!= 'n') {
       printf("        Would you like the introduction to ASSIST? (y/n): ");
       fgets(tmpbuf, BUFSIZ, stdin);
       if(tolower(*p) != 'y' && tolower(*p) != 'n') printf("        Please answer y or n \n");
    }
    if(tolower(*p) == 'y') {
         /* Show the intro to assist walkthru */
         sprintf(tmpbuf,"%s/mscript", assistbin);
         if(access(tmpbuf, EXECUTE) != 0) {
            sprintf(err_line,"assist: setup: %s", tmpbuf);
            perror(err_line);
            exit(1);
         }
         sprintf(tmpbuf, "%s/mscript -s as.intro", assistbin);
         printf("%s\n", clear);
         /*
          * do not exit on break from this system call; instead assume
          * user wants to enter assist without finishing intro to
          * assist walkthru
          */
         system(tmpbuf);
         printf("%s\n", clear);
         message(EXIT_ASSIST_WALK);
    }
    else  {
         printf("%s", clear);
         fflush(stdout);
         message(HOW_TO_ACCESS_ASSIST_WALK);
    }
    printf("\n      --> Strike the RETURN key to enter ASSIST's menu system ");
    fgets(tmpbuf, BUFSIZ, stdin);
    printf("\n%s", clear);
    fflush(stdout);
}


/*
 *  Routine to teach control characters; processing done in raw mode
 */
control()
{
    int c;

    setbuf(stdin, NULL);

    system("stty raw");
    system("stty -echo");
    system("stty brkint");

    printf("\n\n\r          --> Enter ^A  (or ? for help): ");

    c = (getchar() & MASK);
    switch(c) {
       case CONTROL_A : 
           printf("\n\n\r        Good; ^A entered.\n\r");
           rawreturn();
           break;
       case '^':
           c = (getchar() & MASK);
           if (c == 'A') {
               printf("\n\r%s\r", clear);
               fflush(stdout);
               message(CTRL_C_NOTATION);
               printf("\r         --> Enter ^A (or ? for help): ");
               c = (getchar() & MASK);
               }
           else if (c == DELETE) {
               rawdone();
           }
           if (c == CONTROL_A) {
                printf("\n\n\r          Good.  You entered ^A\n\r");
                rawreturn();
           }
           else if ( c == DELETE) rawdone();
           else teachctrl();
           break;
       case DELETE:
           rawdone();
           break;
       default:
           teachctrl();
           break;
    }
    system("stty -raw");
    system("stty echo");
}
teachctrl()
#define MAXTRIES 10
{
    int c, ntries;
    printf("\n\r%s\r", clear);
    fflush(stdout);
    message(REV_1_CTRL_C);
    rawreturn();
    message(REV_2_CTRL_C);
    printf("\n\r        --> Enter the control-d character ");
    c=(getchar() & MASK);
    for (ntries=0; c!=CONTROL_D && c!=DELETE && ntries<MAXTRIES ; ntries++) {
        printf("\n\n\r\007");  /* ring a bell */
        printf("        Depress the CONTROL key and strike lower-case d  ");
        c = getchar() & MASK;
    }
    if (c == DELETE) rawdone();
    if ( ntries == MAXTRIES ) {
         printf("\n\r%s\n\r        Not receiving control-d ", clear);
         rawdone();
    }
    printf("\n\n\r        Good; you have entered control-d \n\r");
    rawreturn();
    message(CTRL_C_NOTATION);
    printf("\n\r         --> Enter ^A ");
    c = getchar() & MASK;
    for (ntries=0;  c!=CONTROL_A && c!=DELETE && ntries<MAXTRIES ; ntries++) {
        printf("\n\n\r\007");
        printf("         Depress the CONTROL key and strike lower-case a  ");
        c = getchar() & MASK;
    }       
    if (ntries == MAXTRIES) {
        printf("\n\r%s\n\r         Not receiving control-a ", clear);
        rawdone();
    }
    if (c == DELETE) rawdone();
    printf("\n\n\r         Good; you have entered ^A \n\r");
    rawreturn();
}

rawreturn()
{
    int c;
    printf("\n\r         --> Strike the RETURN key to continue ");
    c = getchar() & MASK;
    while( c != CONTROL_J && c != CONTROL_M && c != DELETE ) {
       printf("\007");
       c = getchar() & MASK;
    }
    if (c == DELETE) rawdone();
    printf("\n\r%s\r", clear);
    fflush(stdout);
}

rawdone()
{
    system("stty -raw");
    system("stty echo");
    printf("\n");
    exit(0);
}
