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
/*	@(#)print.c	1.2	*/
/* 
 *  General printing routines
 */

#include "defs.h"

SCCSID(@(#)print.c	1.2);

static STRING signals[] = {
				"",
				"hangup",
				"interrupt",
				"quit",
				"illegal instruction",
				"trace/BPT",
				"IOT",
				"EMT",
				"floating exception",
				"killed",
				"bus error",
				"memory fault",
				"bad system call",
				"broken pipe",
				"alarm call",
				"terminated",
				"user defined - 1",
				"user defined - 2",
				"death of a child",
				"power fail",
			};

static void printregs();
static void printfregs();
static void printmap();

void dollar(modif)
int modif;
{
unsigned narg;
short  i;
BKPTR  bkptr;
int    word;
STRING comptr;
int    argp;
int    frame;
int    oldframe, newframe;
SYMPTR symp;
int newest;
int j;

if (cntflg == FALSE)
	cntval = -1;

switch (modif) {

	case '<':
	case '>': {
		char  userfile[64];
		short index;

		index = 0;
		if (modif == '<')
			iclose();
		else
			oclose();
		if (rdc() != EOR) {
			do { 
				userfile[index++] = lastc;
				if (index >= 63)
						error(LONGFIL);
			} while (readchar() != EOR);
			userfile[index] = 0;
			if (modif == '<') { 
				infile = open(userfile, 0);
				if (infile < 0) {
						infile = 0; 
						error(NOTOPEN);
				}
			}
			else { 
				outfile = open(userfile, 1);
				if (outfile < 0)
						outfile = creat(userfile, 0644);
				else 
						if (-1L == lseek(outfile, 0L, 2))
						    perror("dollar");
			}
		}
		lp--;
		}
		break;

	case 'd':
		if (adrflg) {
			if (adrval < 2 || adrval > 16)
				error(BADRAD);
			radix = adrval;
		}
			else
				radix = DEFRADIX;
		break;

	case 'o':
	   radix = 8;
	   break;

	case 'q': 
	case 'Q': 
	case '%':
		done();

	case 'w':
	case 'W':
		maxpos = (adrflg ? adrval : MAXPOS);
		break;

	case 's': 
	case 'S':
		maxoff = (adrflg ? adrval : MAXOFF);
		break;

	case 'v': 
	case 'V':
		prints("variables\n");
		for (i = 0; i < VARNO; i++)
			if (var[i]) {
				printc((i <= 9 ? '0' : 'a' - 10) + i);
				adbpr(" = %R\n",var[i]);
			}
		break;

	case 'm': 
	case 'M':
		printmap("? map", &qstmap);
		printmap("/ map", &slshmap);
		break;

	case   0: 
	case '?':
		if (pid)
			adbpr("pcs id = %d\n", pid);
		else
			prints("no process\n");
		sigprint();
		flushbuf();

	case 'r':
	case 'R':
		printregs();
		return;

	case 'c':
	case 'C':
		if (adrflg)
		{
 			callpc = get(adrval+4, DSP);
 			frame = get(adrval, DSP);
 			argp = frame + 8;
 			newframe = 0;
		}
		else
		{
			callpc = ADBREG(PC);
 			if (findsym(callpc, ISYM) == 0 || 
 			    (get(callpc, ISP) & 0xff) == 0x82) {
 				/* at entry - fp not saved yet */
 				frame = ADBREG(SP) - 4;
 				newframe = ADBREG(FP);
 			} else {
 				frame = ADBREG(FP);
 				newframe = 0;
 			}
 			argp = frame + 8;
		}
		newest = 1;
		while (cntval--) {
			chkerr();
			adbpr("%s(", findsym(callpc, ISYM) == 
					   0x7fffffffL ? "?" : symbol.n_nptr);
 			callpc = get(frame+4, DSP);
 			narg = get(callpc, ISP);
 			if ((narg & 0xFFFF) == 0xA57C) { /* adjspb ? */
 				narg >>= 16;
 				narg = (0x100 - (narg & 0xff)) / 4;
 			} else
				narg = 0;
			for ( ; ; ) { 
				if (narg == 0)
					break;
				adbpr("%R", get(argp, DSP));
				argp += 4;
				if (--narg != 0)
					printc(',');
			}
			prints(")\n");

			oldframe = frame;
 			if (newframe) {
 				frame = newframe;
 				newframe = 0;
 			} else
 				frame = get(frame, DSP);
 			argp = frame + 8;
			if (frame == 0 || frame <= oldframe)
				break;
		}
		break;

	case 'e': 
	case 'E':            /* Print externals */
		symset();
		while (symp = symget()) {
			chkerr();
			if (((symp->n_scnum == 3 || symp->n_scnum == 2) &&
				symp->n_sclass == C_EXT) ||
				(symp->n_scnum == 0 &&
				symp->n_sclass == C_EXT && symp->n_value != 0))
			{
				adbpr("%s:%12t%R\n", symp->n_nptr,
						get((int) symp->n_value, DSP));
			}
			skip_aux(symp);
		}
		break;

	case 'b':           /* print breakpoints */
	case 'B':           /* set default c frame */
		adbpr("breakpoints\ncount%8tbkpt%24tcommand\n");
		for (bkptr = bkpthead; bkptr; bkptr = bkptr->nxtbkpt) {
			if (bkptr->flag) {
				adbpr("%-8.8d", bkptr->count);
				psymoff(bkptr->loc, ISYM, "%24t");
				comptr = bkptr->comm;
				while (*comptr)
						printc(*comptr++);
			}
			}
		break;

	case 't':
		trace = ! trace;
		break;

	case 'f':
	case 'F':
		printfregs();
		return;

	default: 
		error(BADMOD);
}
}

static void printmap(s, amap)
STRING s;
MAP    *amap;
{
	int fid;

	fid = amap->ufd;
	adbpr("%s%12t`%s'\n", s, (fid < 0 ? "-" : 
		(fid == fcor ? corfil : symfil)));
	adbpr("b1 = %-16R", amap->b1);
	adbpr("e1 = %-16R", amap->e1);
	adbpr("f1 = %-16R", amap->f1);
	adbpr("\nb2 = %-16R", amap->b2);
	adbpr("e2 = %-16R", amap->e2);
	adbpr("f2 = %-16R", amap->f2);
	printc(EOR);
}
   
static void printregs()
{
REGPTR p;
int    v;

	for (p = reglist; p < &reglist[MAXREGS]; p++) { 
		v = ADBREG(p->roffs);
		adbpr("%s%6t%R %16t", p->rname, v);
		valpr(v, (p->roffs == PC ? ISYM : DSYM));
		printc(EOR);
	}
	printpc();
}
   
static void 
printfregs()
{
	REGPTR	p;
	long	v;
	union {
		double df;
		struct {
			unsigned u_low;
			unsigned u_high;
		} sf;
	} uf;

	p = freglist;
	v = ADBREG(p->roffs);
	adbpr("%s%6t%x %16t", p->rname, v);
	printc(EOR);

	for (p++; p < &freglist[MAXFREGS]; p++) {
		uf.sf.u_high = ADBREG(p++->roffs);
		uf.sf.u_low = ADBREG(p->roffs);
		if ((uf.sf.u_high & 0x7ff00000) == 0x7ff00000)
			adbpr("%s%6tINFINITY %x %x%16t", p->rname,
				uf.sf.u_high, uf.sf.u_low);
		else if (((uf.sf.u_high & 0x7ff00000) == 0) &&
				((uf.sf.u_high & 0xfffff) || uf.sf.u_low))
			adbpr("%s%6tDENORMALIZED %x %x%16t", p->rname,
				uf.sf.u_high, uf.sf.u_low);
		else
			adbpr("%s%6t%-32.18F %16t", p->rname, uf.df);
		printc(EOR);
	}
}

struct grres getreg(regnam)
int regnam;
{
	REGPTR p;
	STRING regptr;
	char   *olp;
	struct grres result;

	result.sf = 0;
	result.roff = 0;

	olp = lp;
	for (p = reglist; p < &reglist[MAXREGS]; p++) { 
		regptr = p->rname;
		if (regnam == *regptr++) {
			while (*regptr) { 
				if (readchar() != *regptr++) { 
					--regptr;
					break;
				}
			}
			if (*regptr)
				lp = olp;
			else {
				result.sf = 1;
				result.roff = p->roffs;
				return(result);
			}
		}
	}
	lp = olp;
	return(result);
}

void printpc()
{
	dot = ADBREG(PC);
	psymoff(dot, ISYM, ":%16t"); 
	printins(ISP, (int) chkget(dot, ISP), 1);
	printc(EOR);
}

void sigprint()
{
if ((signo >= 0) && (signo <= NSIG))
prints(signals[signo]);
}
