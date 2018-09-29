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
/*	@(#)runpcs.c	1.2	*/
/* 
 *  Service routines for subprocess control 
 */

#include "defs.h"

SCCSID(@(#)runpcs.c	1.2);

#define BPOUT 0
#define BPIN 1

static int  bpstate = BPOUT;
static int  userpc = 1;
static void doexec();
static void setbp();
static void bpwait();
static void readregs();

int getsig(sig)
    int sig;
{
    return(expr(0) ? expv : sig);
}

int runpcs(runmode,execsig)
    int runmode;
    int execsig;
{
    int   rc;
    BKPTR bkpt;
    extern int usrfcntl ;

    if (adrflg) {
        if (bkpt = scanbkpt(userpc))
			bkpt->flag = BKPTSET; 
        userpc = dot;
    }
    adbpr("%s: running\n", symfil);

    setbp();
    while (--loopcnt >= 0) {
        if ((bkpt = scanbkpt(userpc)) && (bkpt->flag == BKPTEXEC)) {
            dot = bkpt->loc;
            execbkpt(bkpt, execsig);
            execsig = 0;
            if (runmode == CONTIN) { 
		fcntl(0,F_SETFL,usrfcntl) ;
                if (-1 == ptrace(runmode, pid, userpc, execsig))
                    perror("runpcs");
                bpwait();
				chkerr();
				execsig = 0;
				readregs();
            }
        }
		else {
            if (-1 == ptrace(runmode, pid, userpc, execsig))
                perror("runpcs");
            bpwait();
			chkerr();
			execsig = 0;
			readregs();
        }

        if ((signo == 0) && (bkpt = scanbkpt(userpc))) {
            bkpt->flag = BKPTEXEC;
            --bkpt->count;
            command(bkpt->comm, ':');
            if ((bkpt->count == 0)) { 
				bkpt->count = bkpt->initcnt;
				rc = 1;
				loopcnt = 0;
            }
			else
                if (runmode == CONTIN)
					loopcnt++;
        } 
		else { 
			execsig = signo;
			rc = 0;
        }
    }
    return(rc);
}

void endpcs()
{
    BKPTR bkptr;

    if (pid) {
		(void) ptrace(EXIT, pid, 0, 0);
		pid = 0;
		userpc = 1;
        for (bkptr = bkpthead; bkptr; bkptr = bkptr->nxtbkpt)
			if (bkptr->flag) 
				bkptr->flag = BKPTSET;
    }
    bpstate = BPOUT;
}

void setup()
{
    if (-1 == close(fsym))
        perror("setup");
    fsym = -1;
    if ((pid = fork()) == 0) { 
		(void) ptrace(SETTRC, 0, 0, 0);
        if (-1 == (int) signal(SIGINT, sigint.siFUN))
			perror("setup");
		if (-1 == (int) signal(SIGQUIT, sigqit.siFUN))
			perror("setup");
        doexec(); 
		(void) exit(0);
    } 
    else if (pid == -1) 
		error(NOFORK);
    else { 
		bpwait();
		errno = 0;
		if ((-1 == (int) (u.u_ar0 = (int *) ptrace(RUREGS, 
			pid, (int) (((char *) &u.u_ar0) - ((int) &u)), 0))) && 
			(errno != 0))
			perror("setup");
        u.u_ar0 = (int *) (((char *) u.u_ar0) - ADDR_U);
        u.u_ar0 = (int *) (((char *) u.u_ar0) + (int) &u);
		readregs();
		lp[0] = EOR;
		lp[1] = 0;
        if (-1 == (fsym = open(symfil, wtflag)))
            perror("setup");
        if (errflg != NULL) { 
			(void) fprintf(stderr, "%s: cannot execute\n", symfil);
            endpcs();
			error((STRING) NULL);
        }
    }
    bpstate=BPOUT;
}

static execbkpt(bkptr, execsig)
    BKPTR bkptr;
    int execsig;
{
    extern struct termio usrtermio ;
    extern int usrfcntl ;

    delbp();
    ioctl(0,TCSETAF,&usrtermio) ;
    fcntl(0,F_SETFL,usrfcntl) ;
    (void) ptrace(SINGLE, pid, bkptr->loc, execsig);
    bkptr->flag = BKPTSET;
    bpwait(); 
    chkerr(); 
    readregs();
    setbp();
}


static void doexec()
{
    STRING argl[MAXARG];
    char   args[LINSIZ];
    STRING p = args;
    STRING *ap = argl;
    STRING filnam;

    *ap++ = symfil;
    do { 
		if (rdc() == EOR) 
			break;
        *ap = p;
        while (lastc != EOR && lastc != SPACE && lastc != TB) { 
			*p++ = lastc;
			(void) readchar(); 
		}
        *p++ = 0;
		filnam = *ap + 1;
        if (**ap == '<') {
			if (-1 == close(0))
                perror("doexec");
            if (open(filnam, 0) == -1) { 
				perror("doexec");
                (void) exit(0);
            }
        } 
		else if (**ap == '>') {
			if (-1 == close(1))
                perror("doexec - close 2");
            if (creat(filnam, 0666) == -1) { 
				perror("doexec - creat");
                (void) exit(0);
            }
        }
		else 
			ap++;
    } while (lastc != EOR);
    *ap++ = 0;
    if (-1 == exect(symfil, argl, environ))
        perror("doexec - exect");
}

BKPTR scanbkpt(adr)
    int adr;
{
    BKPTR bkptr;

    for (bkptr = bkpthead; bkptr; bkptr = bkptr->nxtbkpt)
		if (bkptr->flag && bkptr->loc == adr)
			break;

    return(bkptr);
}

void delbp()
{
    int   a;
    BKPTR bkptr;
    int   ptrres;

    if (bpstate != BPOUT) {
        for (bkptr = bkpthead; bkptr; bkptr = bkptr->nxtbkpt) {  
			if (bkptr->flag) { 
				a = bkptr->loc;
				errno = 0;
                if ((-1 == (ptrres = ptrace(RIUSER, pid, a, 0))) && 
					(errno != 0))
                    perror("delbp");
				errno = 0;
                if ((-1 == ptrace(WIUSER, pid, a, 
					(bkptr->ins & 0xFF) | (ptrres & ~0xFF))))
                    perror("delbp");
            }
        }
        bpstate = BPOUT;
    }
}

static void setbp()
{
    int   a;
    BKPTR bkptr;

    if (bpstate != BPIN) {
        for (bkptr = bkpthead; bkptr; bkptr = bkptr->nxtbkpt) { 
			if (bkptr->flag) { 
				a = bkptr->loc;
				errno = 0;
                bkptr->ins = ptrace(RIUSER, pid, a, 0);
                if ((errno != 0) && (-1 == bkptr->ins))
                    perror("setbp");
				errno = 0;
                if ((-1 == ptrace(WIUSER, pid, a, BPT | 
					(bkptr->ins&~0xFF)))) { 
					prints("cannot set breakpoint: ");
                    psymoff(bkptr->loc, ISYM, "\n");
                }
            }
        }
		bpstate = BPIN;
    }
}

static void bpwait()
{
    int w;
    int stat;
    extern struct termio adbtermio,usrtermio ;
    extern int adbfcntl,usrfcntl ;

    if (-1 == (int) signal(SIGINT, SIG_IGN))
		perror("bpwait");
    while ((w = wait(&stat)) != pid && w != -1 )
		;
    if (-1 == (int) signal(SIGINT, sigint.siFUN))
		perror("bpwait");

/*	Save the terminal state that the process had and restore a	*/
/*	state that can be used for talking to adb.			*/

    ioctl(0,TCGETA,&usrtermio) ;
    ioctl(0,TCSETAF,&adbtermio) ;
    usrfcntl = fcntl(0,F_GETFL,0) ;
    fcntl(0,F_SETFL,adbfcntl) ;

    if (w == -1) { 
		pid = 0;
        errflg = BADWAIT;
    }
    else if ((stat & 0177) != 0177) { 
		if (signo = stat & 0177)
			sigprint();
        if (stat & 0200) { 
			prints(" - core dumped");
            if (-1 == close(fcor))
                perror("bpwait");
            setcor();
        }
        pid = 0;
        errflg = ENDPCS;
    }
    else { 
		signo = stat >> 8;
        if (signo != SIGTRAP)
			sigprint();
        else 
			signo = 0;
        flushbuf();
    }
}

static void readregs()      /* Get register values from child */
{
    int i;
    int ptrres;

    for (i = 0; i < MAXREGS; i++) { 
	errno = 0;
	if ((-1 == (ptrres = ptrace(RUREGS, pid,
		    SYSREG(reglist[i].roffs), 0))) && (errno != 0))
        	perror("readregs");
        ADBREG(reglist[i].roffs) = ptrres;
    }
    userpc = ADBREG(PC);
    for (i = 0; i < MAXFREGS; i++) { 
	errno = 0;
	if ((-1 == (ptrres = ptrace(RUREGS, pid,
		    SYSREG(freglist[i].roffs), 0))) && (errno != 0))
        	perror("readregs");
        ADBREG(freglist[i].roffs) = ptrres;
    }
}
