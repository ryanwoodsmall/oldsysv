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
	/* @(#) pcs.c: 1.8 7/18/83 */

/*
 *	UNIX debugger
 */

#include "head.h"

extern MSG		NOBKPT;
extern MSG		SZBKPT;
extern MSG		EXBKPT;
extern MSG		NOPCS;
extern MSG		BADMOD;

extern L_INT		loopcnt;	/* used here and in runpcs.c */



/* sub process control */

subpcs(modif)
{
	REG INT		check;
	INT		execsig,runmode;
	REG BKPTR	bkptr;
	STRING		comptr;
	extern int errlev;		/* in dis/bits.c */
	execsig=0; loopcnt=cntval;

#if DEBUG > 1
	if(debugflag)
		fprintf(FPRT2, "subpcs(modif=%d('%c'));\n", modif, modif);
#endif
	switch (modif) {

	    /* delete breakpoint */
	    case 'd': case 'D':
		if (bkptr=scanbkpt(dot)) {
			bkptr->flag = 0;
			return(1);
		}
		else {
			error(NOBKPT);
			return(-1);
		}

	    /* set breakpoint */
	    case 'b': case 'B':
		dis_dot(dot,ISP,'\0');	/* sets errlev if can't disassemble */
		if(errlev > 0)	/* warn if not on instruction boundary */
		    fprintf(FPRT1,
			"Warning: Breakpoint at illegal instruction.\n");
		if (bkptr=scanbkpt(dot)) bkptr->flag = 0;
		for (bkptr=bkpthead; bkptr; bkptr=bkptr->nxtbkpt) {
			if (bkptr->flag == 0) break;
		}
		if (bkptr == 0) {
			if ((bkptr=(BKPTR) sbrk(sizeof *bkptr)) == (BKPTR) -1)
			{
				error(SZBKPT);
			}
			else {
				bkptr->nxtbkpt = bkpthead;
				bkpthead=bkptr;
			}
		}
		bkptr->loc = dot;
		bkptr->initcnt = bkptr->count = cntval;
		bkptr->flag = BKPTSET;
		check=MAXCOM-2; comptr=bkptr->comm;	/* rdc(); */
		if (cmd == 'b' || cmd == 'B' || cmd == 'a')
		{
			do
			{
				*comptr++ = readchar();
			} while (check-- && lastc != '\n');
		}
		else
			*comptr++ = '\n';
		*comptr = '\0';
		if (check) return(1);
		else {
			error(EXBKPT);
			return(-1);
		}

	    /* exit */
	    case 'k' :case 'K':
		if (pid) {
			printf("%d: Killed", pid); endpcs(); return(1);
		}
		error(NOPCS);
		return(-1);

	    /* load program */
	    case 'r': case 'R':
		endpcs();
		setup();
		runmode = CONTIN;
#if u3b || u3b5 || ns32000
		return(1);
#else
		/* VAX automatically breaks at start+2; must continue */
		break;
#endif

	    /* single step */
	    case 's': case 'S':
		if (pid) {
			runmode=SINGLE; execsig=getsig(signo);
		}
		else {
			setup();
			loopcnt--;
		}
		break;

	    /* continue with optional signal */
	    case 'c': case 'C': case 0:
		if (pid==0) {
			error(NOPCS);
			return(-1);
		}
		runmode=CONTIN; execsig=getsig(signo);
		break;

	    default: error(BADMOD);
	}

	runpcs(runmode,execsig);
	delbp();
	return(1);
}
