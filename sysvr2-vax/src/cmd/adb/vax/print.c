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
static void printmap();

void dollar(modif)
int modif;
{
short  narg;
short  i;
BKPTR  bkptr;
int    word;
STRING comptr;
int    argp;
int    frame;
int    oldframe;
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
			frame = adrval;
			callpc = get(frame+16, DSP);
			callpc += get(callpc-4, ISP);
			word = get(adrval + 6, DSP) & 0xFFFF;
			if (word & 0x2000) /* 'calls', can figure out argp */
			{
				argp = adrval + 20 + ((word >> 14) & 3); 
				word &= 0xFFF;
				while (word) 
				{
					if (word & 1)
						    argp += 4;
						word >>= 1; 
				}
			}
			else     /* 'callg', can't tell where argp is */ 
				argp = frame;
		}
		else
		{
			argp = ADBREG(AP);
			frame = ADBREG(FP);
			callpc = ADBREG(PC);
		}
		newest = 1;
		while (cntval--) {
			chkerr();
			adbpr("%s(", findsym(callpc, ISYM) == 
						   0x7fffffffL ? "?" : symbol.n_nptr);
			narg = get(argp, DSP);
			if (narg & ~0xFF)
				narg = 0;
			for ( ; ; ) { 
				if (narg == 0)
					break;
				adbpr("%R", get(argp += 4, DSP));
				if (--narg != 0)
					printc(',');
			}
			prints(")\n");

			if (modif == 'C') 
			{
				j=get((newest?frame:oldframe)+6,DSP)&0xffff;
				if(j&0x2000)
					argp=calcap(newest?frame:oldframe,j);
				else
					argp=0;
				localsym(newest?frame:oldframe,argp,newest);
				newest = 0;
				for(j=0;(j<numlocs)&&argp;j++)
				{
					localval=expv=locadr-(4*j);
					word=get(localval,DSP);
					adbpr("%8t%s.%X% = ", symbol.n_nptr,j);
					if (errflg != NULL) {
						prints("?\n"); 
						errflg = NULL; 
					}
					else 
						adbpr("%R\n", word); 
				}
			}
			callpc = get(frame+16, DSP);
			argp = get(frame+8, DSP);
			oldframe = frame;
			frame = get(frame+12, DSP) & EVEN;
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
