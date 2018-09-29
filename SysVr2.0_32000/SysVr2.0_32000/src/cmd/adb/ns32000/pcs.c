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
/*	@(#)pcs.c	1.2	*/
/* 
 *  Subprocess control 
 */

#include "defs.h"

SCCSID(@(#)pcs.c	1.2);

void subpcs(modif)
    char modif;
{
    int    check;
    int    execsig;
    int    runmode;
    int    oldflag;
    BKPTR  bkptr;
    STRING comptr;
    
    execsig = 0;
    loopcnt = cntval;
    
    switch (modif) {
    
        case 'd':
        case 'D':                     /* delete breakpoint */
            if (bkptr = scanbkpt(dot)) {
                bkptr->flag=0;
                return;
            } 
            else
                error(NOBKPT);
    
        case 'b':
        case 'B':           /* set breakpoint */
            oldflag = 0;
            if (bkptr = scanbkpt(dot)) { 
                if (bkptr->flag == BKPTEXEC) 
                    oldflag = 1;
                    bkptr->flag = 0;
            }
            for (bkptr = bkpthead; bkptr; bkptr = bkptr->nxtbkpt)
                if (bkptr->flag == 0) 
                    break;
            if (bkptr == 0)
                if (((int) (bkptr = (BKPTR) sbrk(sizeof *bkptr))) == -1) 
                    error(SZBKPT);
                else { 
                    bkptr->nxtbkpt = bkpthead;
                    bkpthead = bkptr;
                }
            bkptr->loc = dot;
            bkptr->initcnt = bkptr->count = cntval;
            bkptr->flag = oldflag ? BKPTEXEC : BKPTSET;
            check = MAXCOM - 11;
            comptr = bkptr->comm;
            (void) rdc();
            lp--;
            do { 
                *comptr++ = readchar();
            } while (check-- && lastc != EOR);
                *comptr = 0;
            lp--;
            if (check)
                return;
            else
                error(EXBKPT);
    
        case 'k':
        case 'K':             /* exit */
            if (pid) { 
                adbpr("%d: killed", pid);
                endpcs();
                return;
            }
            else
                error(NOPCS);
    
        case 'r':
        case 'R':                    /* run program */
            endpcs();
            setup();
            runmode = CONTIN;
            break;
    
        case 's': 
        case 'S':                         /* single step */
            if (pid) {
                runmode = SINGLE;
                execsig = getsig(signo);
            }
            else { 
                setup();
                loopcnt--;
            }
            break;
    
        case 'c': 
        case 'C': 
        case 0:          /* continue with optional signal */
            if (pid == 0) 
                error(NOPCS);
            runmode = CONTIN;
            execsig = getsig(signo);
            break;
    
        default:
            error(BADMOD);
    }
    
    if (loopcnt > 0 && runpcs(runmode,execsig)) 
        adbpr("breakpoint%16t");
    else
        adbpr("stopped at%16t");
    delbp();
    printpc();
}
