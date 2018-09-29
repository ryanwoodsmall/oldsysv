/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
/*	@(#)main.c	1.2	*/
/*
 *  Main calling routine
 */

#include "defs.h"

SCCSID(@(#)main.c	1.2);

static jmp_buf env;
static int exitflg;
static int fault();
static int nocore();

struct termio usrtermio ;	/* Terminial state from user process */
struct termio adbtermio ;	/* Sane adb terminal state during breakpoint */

int usrfcntl ;
int adbfcntl ;

/* Set up files and initial address mappings */

main(argc, argv)
    STRING  *argv;
    int     argc;
{
/*	Initialize usrtermio and save the terminal state for use during	*/
/*	breakpoints.							*/

    ioctl(0,TCGETA,&usrtermio) ;
    ioctl(0,TCGETA,&adbtermio) ;
    adbfcntl = usrfcntl = fcntl(0,F_GETFL,0) ;

    while (argc > 1) {
        if (eqstr("-w", argv[1])) {
            wtflag = O_RDWR;
            argc--;
            argv++;
        }
        else
            break;
    }

    if (argc > 1)
        symfil = argv[1];
    if (argc > 2)
        corfil = argv[2];
    argcount = argc;

    setsym(); 
    setcor();

    /* Set up variables for user */
    maxoff = MAXOFF;
    maxpos = MAXPOS;
    var[VARB] = slshmap.b1 ? slshmap.b1 : qstmap.b1;
    var[VARD] = datsiz;
    var[VARE] = entrypt;
	var[VARF] = qstmap.f1;
    var[VARM] = magic;
    var[VARS] = stksiz;
    var[VART] = txtsiz;

    if ((sigint.siFUN = signal(SIGINT, SIG_IGN)) != SIG_IGN) { 
        sigint.siFUN = fault;
        if (-1 == (int) signal(SIGINT, fault))
			perror("main");
    }
    sigqit.siFUN = signal(SIGQUIT, SIG_IGN);
	if (-1 == (int) signal(SIGILL, nocore))
		perror("main");
	if (-1 == (int) signal(SIGIOT, nocore))
		perror("main");
	if (-1 == (int) signal(SIGEMT, nocore))
		perror("main");
	if (-1 == (int) signal(SIGFPE, nocore))
		perror("main");
	if (-1 == (int) signal(SIGBUS, nocore))
		perror("main");
	if (-1 == (int) signal(SIGSEGV, nocore))
		perror("main");
	if (-1 == (int) signal(SIGSYS, nocore))
		perror("main");

    (void) setjmp(env);

    if (executing && pid != 0)
        delbp();
    executing = FALSE;

    for ( ; ; ) {
        TRPRINT("* ");
        flushbuf();
        if (errflg != NULL) { 
            adbpr("%s\n", errflg);
            exitflg = (errflg != NULL); 
            errflg = NULL;
        }
        if (mkfault) {
            mkfault = FALSE; 
            printc(EOR); 
            prints(DBNAME);
        }
        lp = 0; 
        (void) rdc(); 
        lp--;
        if (eof) {
            if (infile) {
                iclose(); 
                eof = 0; 
                (void) longjmp(env, 0);
            }
            else
                done();
        }
        else
            exitflg = 0;

        command((STRING) NULL, lastcom);
        if (lp && lastc != EOR)
            error(NOEOR);
    }
}

int round(a, b)
    int a;
    int b;
{
    int w;

    w = (a / b) * b;
    w += (a != w) ? b : 0;
    return(w);
}

/* error handling */

void chkerr()
{
    if (errflg != NULL || mkfault)
        error(errflg);
}

void error(n)
    STRING n;
{
    errflg = n;
    iclose();
    oclose();
    (void) longjmp(env, 0);
}

static int fault(a)
{
    if (-1 == (int) signal(a, fault))
		perror("fault");
    if (-1 == lseek(infile, 0L, 2))
        perror("fault");
    mkfault = TRUE;
}

static int nocore(s)
{
    fprintf(stderr, "adb system failure (signal %d) -- exiting\n",s);
    (void) exit(1);
}

void done()
{
    endpcs();
    (void) exit(exitflg);
}
