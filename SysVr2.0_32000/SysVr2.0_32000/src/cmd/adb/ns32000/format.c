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
/*	@(#)format.c	1.2	*/
#include "defs.h"

SCCSID(@(#)format.c	1.2);

static STRING fphack;
static void   printesc();

static char rdfp()
{
    return(lastc = *fphack++);
}

void scanform(icount, ifp, itype, ptype)
    int    icount;
    STRING ifp;
    int    itype;
    int    ptype;
{
    STRING fp;
    int    fcount;
    int    init = TRUE;
    int    savdot;
    BOOL   exact;

    while (icount) {  
        exact = (findsym(dot, ptype) == 0);
        if ((! init) && exact && maxoff) 
            adbpr("\n%s:%16t", symbol.n_nptr);

        fp = ifp;
        savdot = dot; 
        init = FALSE;

        /*now loop over format*/
        while (*fp && errflg == NULL) {  
            fphack = fp;
            (void) rdfp();
            if (getnum(rdfp) && lastc == '*') { 
                fcount = expv;
                fp = fphack;
                peekc = 0;
            }
            else
                fcount = 1;

            if (*fp == 0) 
                break;
            fp = exform(fcount, fp, itype, ptype);
        }
        dotinc = dot - savdot;
        dot = savdot;

        if (errflg != NULL) { 
            if (icount < 0) { 
                errflg = NULL;
                break;
            }
            else
                error(errflg);
        }
        if (--icount)
            dot = inkdot(dotinc);
        if (mkfault) 
            error((STRING) NULL);
    }
}

STRING exform(fcount, ifp, itype, ptype)
	int    fcount;
	STRING ifp;
	int    itype;
	int    ptype;
{
    /* execute single format item `fcount' times
     * sets `dotinc' and moves `dot'
     * returns address of next format item
     */
    unsigned w;
    int      savdot;
    int      wx;
    STRING   fp;
    char     c;
    char     modifier;
    char     longpr;
    struct {
        int   sa;
        short sb;
        short sc;
    } fw;

    while (fcount > 0) {
	fp = ifp;
	c = *fp;
        longpr = (c >= 'A') & (c <= 'Z') | (c == 'f') | (c == '4') | (c == 'p');
        if (itype == NSP || *fp == 'a')
		wx = w = dot;
        else
		wx = w = get(dot, itype);
        if (c == 'F') {
		fw.sb = get(inkdot(4), itype);
		fw.sc = get(inkdot(6), itype);
        }
        if (errflg != NULL)
		return(fp);
        if (mkfault)
		error((STRING) NULL);
        var[0] = wx;
        modifier = *fp++;
        dotinc = (longpr ? 4 : 2);

        if (charpos() == 0 && modifier != 'a' )
		adbpr("%16m");

        switch(modifier) {

            case SPACE:
			case TB:
				break;

            case 't':
			case 'T':
				adbpr("%T",fcount);
				return(fp);

            case 'r':
			case 'R':
				adbpr("%M", fcount);
				return(fp);

            case 'a':
				psymoff(dot, ptype, ":%16t");
				dotinc = 0;
				break;

            case 'p':
				psymoff(var[0], ptype, "%16t");
				break;

            case 'u':
				adbpr("%-8u", w);
				break;

            case 'U':
				adbpr("%-16U", wx);
				break;

            case 'c':
			case 'C':
				if (modifier == 'C')
					printesc(w);
				else
					printc((char) (w & LOBYTE));
				dotinc = 1;
				break;

            case 'b':
			case 'B':
				adbpr("%-8o", w&LOBYTE);
				dotinc = 1;
				break;

            case '1':
				adbpr("%-8r", w&LOBYTE);
				dotinc = 1;
				break;

            case '2':
            case 'w':
				adbpr("%-8r", w);
				break;

            case '4':
            case 'W':
				adbpr("%-16R", wx);
				break;

            case 's':
			case 'S':
				savdot = dot;
				dotinc = 1;
				while ((c = get(dot, itype)&LOBYTE) && errflg == NULL) {
					dot = inkdot(1);
				    if (modifier == 'S') 
						printesc((unsigned) c);
				    else
						printc(c);
				    endline();
				}
				dotinc = dot - savdot + 1;
				dot = savdot;
				break;

	    case 'x':
				adbpr("%-8x", w);
				break;

            case 'X':
				adbpr("%-16X", wx);
				break;

            case 'Y':
				adbpr("%-24Y", wx);
				break;

            case 'q':
				adbpr("%-8q", w);
				break;

            case 'Q':
				adbpr("%-16Q", wx);
				break;

            case 'o':
				adbpr("%-8o", w);
				break;

            case 'O':
				adbpr("%-16O", wx);
				break;

            case 'i':
				printins(itype, (int) w, 1);
				printc(EOR);
				break;

            case 'd':
				adbpr("%-8d", w);
				break;

            case 'D':
				adbpr("%-16D", wx);
				break;

            case 'f':
				fw.sa = wx;
				fw.sb = fw.sc = 0;
				if ((fw.sa & 0x7f800000) == 0x7f800000)
					adbpr("INFINITY %X", fw.sa);
				else if (((fw.sa & 0x7f800000) == 0) &&
					      (fw.sa & 0x7fffff))
					adbpr("DENORMALIZED %X", fw.sa);
				else
					adbpr("%-16.9f", fw);
				dotinc = 4;
				break;

            case 'F':
				fw.sa = wx;
				if ((fw.sc & 0x7ff0) == 0x7ff0)
					adbpr("INFINITY %X %x %x", fw.sc, fw.sb,
						fw.sa);
				else if (((fw.sc & 0x7ff0) == 0) &&
					      ((fw.sc & 0xf) || fw.sb || fw.sa))
					adbpr("DENORMALIZED %X %x %x", fw.sc, 
						fw.sb, fw.sa);
				else
					adbpr("%-32.18F", fw);
				dotinc = 8;
				break;

            case 'n':
			case 'N':
				printc('\n');
				dotinc = 0;
				break;

            case '"':
				dotinc = 0;
				while (*fp != '"' && *fp)
					printc(*fp++);
				if (*fp)
					fp++;
				break;

            case '^':
				dot = inkdot(-dotinc * fcount);
				return(fp);

            case '+':
				dot = inkdot(fcount);
				return(fp);

            case '-':
				dot = inkdot(-fcount);
				return(fp);

            default: 
				error(BADMOD);
        }
        if (itype != NSP)
			dot = inkdot(dotinc);
        fcount--;
        endline();
    }

    return(fp);
}

void shell()
{
    int    rc;
    int    status;
    int    unixpid;
    STRING argp = lp;

    while (lastc != EOR)
		(void) rdc();
    if ((unixpid = fork()) == 0) {
		if (-1 == (int) signal(SIGINT, sigint.siFUN))
			perror("shell");
        if (-1 == (int) signal(SIGQUIT, sigqit.siFUN))
			perror("shell");
        *lp=0;
		if (-1 == execl("/bin/sh", "sh", "-c", argp, 0))
			perror("shell");
		(void) exit(16);
    }
    else if (unixpid == -1)
		error(NOFORK);
    else {
		if (-1 == (int) signal(SIGINT, SIG_IGN))
			perror("shell");
        while ((rc = wait(&status)) != unixpid && rc != -1 )
			;
        if (-1 == (int) signal(SIGINT, sigint.siFUN))
			perror("shell");
        prints("!");
		lp--;
    }
}

static void printesc(c)
    unsigned c;
{
    c &= STRIP;
    if (c < SPACE || c > '~' || c == '@') 
        adbpr("@%c", (c == '@' ? '@' : c ^ 0140));
    else 
        printc((char) c);
}

int inkdot(incr)
{
    int newdot;

    newdot = dot + incr;
    if ((dot ^ newdot) >> 24 )
		error(ADWRAP);
    return(newdot);
}
