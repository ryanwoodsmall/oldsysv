/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)script:mscript.c	1.14"
/*
 * The mscript program can be invoked with or without the -m option
 * The -m option is designed to be called from a ASSIST menu;
 * without -m the program is used by a developer
 * directly invoking a script.
 */


#include "script.h"

char *getenv();
FILE  *fdtut;    /* fdtut points at script file */
char linebuf[MAXCHAR];
char assistpath[MAXCHAR];
char assistlib[MAXCHAR];

int done1(), done2(), bdone1(), bdone2();

int menuflag = FALSE;  /* if true shows ASSIST commands for previous 
                        * and TOP ASSIST menus when the help window is 
                        * displayed
                        */

int termalt, termso, termfkey;



/*******************************************************************************
         assist commands menu and shell escape
*******************************************************************************/
WINDOW *helpwin;
char syscmd[BUFSIZ];
WINDOW *syswin;

char subject[MAXCHAR], subsub[MAXCHAR];
char *tmpscript;
int buildflag = FALSE;  /* if true, script is built from pieces
                         * specified in the build file for that subject area.
                         * If false, mscript uses the named script
                         */
struct termio termbuf;  /* keep stty settings done inside of program */

char tempfile[MAXCHAR];

main(argc, argv)
char **argv;
{

        int kount, a;
        char *argp;
        char *q;
        char err_line[MAXCHAR];


        signal(SIGINT, bdone1);
        signal(SIGQUIT, bdone1);
        signal(SIGHUP, bdone1);


        if ((q=getenv("ASSISTLIB")) != NULL && q[0]!= '\0') sprintf(assistlib,"%s",q);
        else sprintf(assistlib, "/usr/lib/assist/lib");

        sprintf(assistpath,"%s/scripts", assistlib);

        for (kount=1, a=argc; argv[kount][0] == '-'; kount++, a--) {
            switch(argv[kount][1])  {

                case 'd':   /* use to override assistpath */
                    if(argv[kount][2] == '\0') {  /* space after -d option */
                         kount = kount + 1;
                         argp = argv[kount];
                    }
                    else {
                         argp = argv[kount]+ 2;
                    }
                    sprintf(assistpath,"%s", argp);
                    break;


                case 'm':
                    menuflag = TRUE;
                    break;

                case 's':  /* build script from script chunks*/
                           /* syntax is -s subjectname.subsubjectname */
                           /* e.g. -s vi.delete */
                    
                    a++;
                    if(argv[kount][2] == '\0') {  /* space after -s option */
                         kount = kount + 1;
                         argp = argv[kount];
                    }
                    else {
                         argp = argv[kount]+ 2;
                    }
                    sscanf(argp,"%[a-zA-Z0-9].%s", subject, subsub);
                    buildflag = TRUE;
                    break;



                default:
                    fprintf(stderr,"assist: %s is unknown option to mscript\n",
                           argv[kount]);
                    exit(BADCMD);
                    break;
             }
        }

        if(a < 2) {
            fprintf(stderr, "assist: mscript requires script name\n");
            exit(BADCMD);
        }

        printf("\n\nPlease stand by ...\n");
        fflush(stdout);

        if (buildflag == FALSE) { /*use script name given in command line */
            fdtut = fopen(argv[argc-1], "r");
        }
        else {    /* build and use a temporary script */
            mkscript(subject, subsub, assistpath);
            fdtut = fopen(tmpscript, "r");
        }

        if(fdtut == NULL) {
            sprintf(err_line,"assist: mscript: %s", argv[argc-1]);
            perror(err_line);
            done1();
            exit(BADCMD);
        }

        signal(SIGINT, SIG_IGN); 
        signal(SIGQUIT, SIG_IGN);
        signal(SIGHUP, SIG_IGN);
        signal(SIGTERM, SIG_IGN);

        termalt = termso = termfkey = FALSE;
        if(strcmp("1",getenv("MSTANDOUT")) == 0) termso = TRUE;
        if(strcmp("1",getenv("MFKEYS")) == 0) termfkey = TRUE;
        if(strcmp("1",getenv("MALTCHAR")) == 0) termalt = TRUE;
        
        catchtty(&termbuf);

	initscr();			/* initialize curses */
	noecho();			/* turn off tty echo */
	cbreak();			/* enter cbreak mode */
	nonl();
  	idlok(stdscr, TRUE);  
        keypad(stdscr, TRUE);           /* turn on keypad  */
	scrollok(stdscr, TRUE);         /* turn on scrolling  */
        leaveok(stdscr, FALSE);         /* hardware cursor */
        intrflush(stdscr, TRUE);

        signal(SIGINT, bdone2); 
        signal(SIGQUIT, bdone2);
        signal(SIGHUP, bdone2);
        signal(SIGTERM, bdone2);
        helpwin = newwin(13, 75,11,3); /*window for redrawing and exiting */
        if (termalt == TRUE) {
           wattron(helpwin,A_ALTCHARSET);
           mvwprintw(helpwin, 0 , 0, "lqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqk");
           mvwprintw(helpwin, 12,0,  "mqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqj");
           for (kount = 1; kount < 12; kount++) {
               mvwprintw(helpwin, kount, 0, "x                                                                         x");
           }
           wattroff(helpwin,A_ALTCHARSET);
        }
        else {
           box(helpwin, '|' , '-'); 
           mvwprintw(helpwin, 0 ,0,  " ------------------------------------------------------------------------- ");
           mvwprintw(helpwin, 12, 0, " ------------------------------------------------------------------------- ");
        }

        keypad(helpwin, TRUE);
        intrflush(helpwin, TRUE);

/*
 * Check window size and exit if window smaller than NROWSxNCOLS
 */

        if ( (LINES < NROWS) || (COLS < NCOLS) ) {
            move(0,0);
            clrtobot();
            done2();
            fprintf(stderr,"WARNING: ASSIST assumes that your ");
            fprintf(stderr, "display has at least\n");
            fprintf(stderr,"         %d lines and %d columns.", NROWS, NCOLS);
            fprintf(stderr,"  Since your display has\n", NROWS, NCOLS);
            if (LINES < NROWS && COLS < NCOLS) {
                fprintf(stderr,"         %d lines and %d columns",LINES,COLS);
            }
            else if (LINES < NROWS) {
                fprintf(stderr,"         only %d lines", LINES);
            }
            else fprintf(stderr, "         only %d columns", COLS);
            fprintf(stderr, " ASSIST will not\n");
            fprintf(stderr, "         work correctly.\n");

            fprintf(stderr, "Please use a display with at least ");
            fprintf(stderr, "%d rows and %d columns\n", NROWS, NCOLS);
            exit(BADCMD);
        }
/*
 * A temporary file for script writers; mscript deletes at end
 */
        sprintf(tempfile, "/tmp/twalk.%s", getenv("LOGNAME"));

/*
 * Now we do each line of the script 
 */


        while( fgets(linebuf, MAXCHAR, fdtut) != NULL) {
             do_line();
        }

/*
 * Finish up nicely
 */

        done2();
        exit(0);
}

