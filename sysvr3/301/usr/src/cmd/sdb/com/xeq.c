/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:com/xeq.c	1.13"

/*
 *		MACHINE and OPERATING SYSTEM DEPENDENT
 */

#include "head.h"
#include "coff.h"
#include <fcntl.h>

static struct filet *cfp = (struct filet *)0;	/* current file pointer */
static struct proct *cpp = (struct proct *)0;	/* current proc pointer */

extern MSG		NOPCS;

/* service routines for sub process control */

/*
 * Single step until loc with descriptor format d is modified;
 * return its new value.	made it work!
 */
/*	sl_class must be set before calling monex.  It calls getlocv,
 *	which uses sl_class to determine whether loc is an image of
 *	a register (important on the 3B, where register chars and
 *	shorts are right justified, while nonregister data is left
 *	justified).
 */
monex(loc, d, cnt)
ADDR loc; char *d; int cnt;
{
	register int oldval;
	register int newval;
	int count, nc;
	struct proct *procp, *procn;
	BOOL inlocal, outsub = FALSE;
	char savclass;			/* save sl_class */	
#if DEBUG
	if (debugflag ==1)
	{
		enter1("monex");
	}
	else if (debugflag == 2)
	{
		enter2("monex");
		arg("loc");
		printf("0x%x",loc);
		comma();
		arg("d");
		printf("\"%s\"",d);
		comma();
		arg("cnt");
		printf("0x%x",cnt);
		closeparen();
	}
#endif
	newval = oldval = 0;
	nc = 0;
	cfp = (struct filet *)0;
	cpp = (struct proct *)0;
	savclass = sl_class;
	if(loc != -1) {

#if u3b || u3b5 || u3b15 || u3b2
		/* in 3B, stack grows up; test if on stack or in reg */
		inlocal = (loc >= USRSTACK) || ISREGN(loc);
#else
#if vax
		/* in VAX, stack grows down; test on stack or in reg */
		inlocal = (loc >= 0x70000000) || ISREGN(loc);
#endif
#endif
		newval = oldval = getlocv(loc, d, DSP);
		procp = adrtoprocp(dot);
	}
	count = (cnt == 0 ? 1 : cnt);
	do {
		singstep(1,'s');	/* was subpcs('s'); */
		sl_class = savclass;	/* in case bkpt had command */
		if (signo) {
			printf(" at\n");
			break;
		}
		if(loc == -1)	/* just single step through prog */
			continue;
		/* if a variable name and on the stack, must be in same proc */
		procn = adrtoprocp(dot);
		if(inlocal && cnt == 0 && procn != procp) {
			outsub = TRUE;
			continue;
		}
		if(outsub && procn == procp) {	/* re-entering subr; restart */
			outsub = FALSE;
			newval = oldval = getlocv(loc, d, DSP);
		}
		newval = getlocv(loc, d, DSP);
	} while (oldval == newval && ++nc && (cnt == 0 || --count > 0));
	cntval = nc;
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("monex");
		printf("0x%x",newval);
		endofline();
	}
#endif
	return(newval);
}

/* Single step count source stmts (cm is 's' or 'S') */
singstep(count, cm)
char cm; {
	struct stmt thisstmt, curstmt;
	register struct proct *thisproc, *curproc;
	struct filet *fp;
	
#if DEBUG
	if (debugflag ==1)
	{
		enter1("singstep");
	}
	else if (debugflag == 2)
	{
		enter2("singstep");
		arg("count");
		printf("0x%x",count);
		comma();
		arg("cm");
		printf("0x%x",cm);
		closeparen();
	}
#endif
	usrtty();	/* restore user tty modes */
	if (count == 0) count = 1;
	for( ; count; count--) {
		setcur(0);	/*  sets dot */
		thisproc = adrtoprocp(dot-(signo ? NOBACKUP : 0));
		thisstmt = adtostmt(dot-(signo ? NOBACKUP : 0), thisproc);
		do {
			if(vmode) {	/* +1 in docommand() */
			    if( (fp = thisproc->sfptr) != cfp && fp != badfile)
				printf("[%.14s]\n", (cfp=fp)->sfilename);
			    if( thisproc != cpp && thisproc != badproc) {
				cpp = thisproc;
				if(vmode == 1) {
#if u3b || u3b5 || u3b15 || u3b2
					printf("[%s()]\n", cpp->pname);
#else
#if vax
					if (cpp->pname[0] == '_')
					{
						printf("[%s()]\n",
							cpp->pname+1);
					}
					else
					{
						printf("[%s()]\n",
							cpp->pname);
					}
#endif
#endif
				}
			    }
			    if(vmode > 1)
			    	if(exactaddr == -1) {	/* at begin of a line */
#if u3b || u3b5 || u3b15 || u3b2
					printf("%s:", thisproc->pname);
#else
#if vax
					if (thisproc->pname[0] == '_')
					{
					    printf("%s:",
						thisproc->pname+1);
					}
					else
					{
					    printf("%s:", thisproc->pname);
					}
#endif
#endif
					fprint();
			    	}
			    if(vmode > 2) {
				printf("    ");
				printpc();
			    }
			}
			if(singsub(cm) == 1)
			{
#if DEBUG
				if (debugflag == 1)
				{
					exit1();
				}
				else if (debugflag == 2)
				{
					exit2("singstep");
					endofline();
				}
#endif
			    return;		/* at a breakpoint */
			}
			if( pid == 0 && PC < 0 )
			{
#if DEBUG
				if (debugflag == 1)
				{
					exit1();
				}
				else if (debugflag == 2)
				{
					exit2("singstep");
					printf("0x%x",-1);
					endofline();
				}
#endif
				return( -1 );
			}
			else
			{
				dot = USERPC;
			}
			curstmt = adtostmt(dot-(signo ? NOBACKUP : 0),
				(curproc=adrtoprocp(dot-(signo?NOBACKUP:0))) );
		} while (!signo && 
			((thisproc == curproc && thisstmt.lnno == curstmt.lnno
				&& thisstmt.stno == curstmt.stno) ||
				curstmt.lnno <= -1 ||
				curstmt.stno <= 0));
		if (signo) break;
	}
	sdbtty();	/* save user tty modes, restore sdb tty modes */
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("singstep");
		endofline();
	}
#endif
}

/* added new subroutine singsub()	[only singstep() uses]
 *	Tests if at a subroutine call and if 'S' 
 *	or if 's' and no lineno stuff, "single step"
 *	thru the entire subroutine; else just do a single step
 */
singsub(cm)
char cm;
{
	ADDR retaddr;
	struct proct *procp;
				/* MACHINE DEPENDENT */
#if DEBUG
	if (debugflag ==1)
	{
		enter1("singsub");
	}
	else if (debugflag == 2)
	{
		enter2("singsub");
		arg("cm");
		printf("0x%x",cm);
		closeparen();
	}
#endif
	if( pid == 0 && PC < 0 )
	{
		error( NOPCS );
#if DEBUG
		if (debugflag == 1)
		{
			exit1();
		}
		else if (debugflag == 2)
		{
			exit2("singsub");
			printf("0x%x",-1);
			endofline();
		}
#endif
		return( -1 );
	}
	else
	{
		dot = USERPC;
	}
	if (SUBCALL) {		/* SUBCALL depends on dot */
		subpcs('s');
		retaddr = SUBRET;
		/* if subr doesn't have lineno stuff, treat 's' as 'S' */
		if( pid == 0 && PC < 0 )
		{
			error( NOPCS );
#if DEBUG
			if (debugflag == 1)
			{
				exit1();
			}
			else if (debugflag == 2)
			{
				exit2("singsub");
				printf("0x%x",-1);
				endofline();
			}
#endif
			return( -1 );
		}
		else
		{
			dot = USERPC;
		}
		procp = adrtoprocp(dot);
		if(cm == 's' && (procp != badproc && procp->lineno > 0))
		{
#if DEBUG
			if (debugflag == 1)
			{
				exit1();
			}
			else if (debugflag == 2)
			{
				exit2("singsub");
				printf("0x%x",0);
				endofline();
			}
#endif
			return(0);	/* single step thru subroutine */
		}
		dot = retaddr;		/* do entire subroutine at once */
		subpcs('b');
		subpcs('c');
		dot = retaddr;
		subpcs('d');
		if( pid == 0 && PC < 0 )
		{
			error( NOPCS );
#if DEBUG
			if (debugflag == 1)
			{
				exit1();
			}
			else if (debugflag == 2)
			{
				exit2("singsub");
				printf("0x%x",-1);
				endofline();
			}
#endif
			return( -1 );
		}
		else
		{
			dot = USERPC;
		}
		if (retaddr != dot && signo == 0) {
			/* save user, restore sdb tty */
			sdbtty();
			printf("Breakpoint at \n");
#if DEBUG
			if (debugflag == 1)
			{
				exit1();
			}
			else if (debugflag == 2)
			{
				exit2("singsub");
				printf("0x%x",1);
				endofline();
			}
#endif
			return(1);
		}
	} else
		subpcs('s');
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("singsub");
		printf("0x%x",0);
		endofline();
	}
#endif
	return(0);
}

doscall()
{
/*    Needs assembly language interface, libg.s (libg.a) in USER prog */
	int subargs[NUMARGS];	 
		/* subargs[0]    = address, 
		 * subargs[1]    = number of arguments
		 * subargs[2:NUMARGS] = actual arguments
		 */
	union {
		int w[128-NUMARGS];
		char c[4*(128-NUMARGS)];
	}substr;
	
	register int i, numchars, *subargp;
	register char ch;
	ADDR adr, praddr;
	ADDR j;
	union word word;
	
#if DEBUG
	if (debugflag ==1)
	{
		enter1("doscall");
	}
	else if (debugflag == 2)
	{
		enter2("doscall");
		closeparen();
	}
#endif
	praddr = extaddr(proc);
	if (praddr == -1) {
		fprintf(FPRT1, "Cannot find %s\n", proc);
#if DEBUG
		if (debugflag == 1)
		{
			exit1();
		}
		else if (debugflag == 2)
		{
			exit2("doscall");
			endofline();
		}
#endif
		return;
	}
	if (adargs == -1) {
		error("Program not loaded with -lg");
#if DEBUG
		if (debugflag == 1)
		{
			exit1();
		}
		else if (debugflag == 2)
		{
			exit2("doscall");
			endofline();
		}
#endif
		return;
	}
	
	numchars = 0;
	subargp = subargs;
	do
	{
		ch = *++argsp;	/* argsp --> first real char after '(' */
	} while( ch == ' ' || ch == '(' );
	*subargp++ = praddr;
	subargp++;
	
	for (i=0; i<NUMARGS - 2; i++) {  /* process an argument */
		ch = *argsp;
		if (ch == '\'') {
			*subargp++ = *(argsp+1);
			argsp += 2;
		} else if (ch == '"') {
			*subargp++ = adargs + sizeof subargs + numchars;
			argsp++;
			for (;;) {
				ch = *argsp++;
				if (ch == '\\')
					ch = bsconv(*argsp++);
				substr.c[numchars++] = ch;
				if (ch == '"') {
					substr.c[numchars-1] = '\0';
					break;
				}
				if (ch == '\0') {
					error("Unterminated string constant");
#if DEBUG
					if (debugflag == 1)
					{
						exit1();
					}
					else if (debugflag == 2)
					{
						exit2("doscall");
						endofline();
					}
#endif
					return;
				}
				if (numchars > sizeof substr.c) {
					error("Too many string constants");
#if DEBUG
					if (debugflag == 1)
					{
						exit1();
					}
					else if (debugflag == 2)
					{
						exit2("doscall");
						endofline();
					}
#endif
					return;
				}
			}
		} else if ((ch >= '0' && ch <= '9') || ch == '-') {
			*subargp++ = readint(&argsp);
		} else if ((ch >= 'a' && ch <= 'z') ||
				(ch >= 'A' && ch <= 'Z') || ch == '_') {
			char *desc;
			cpname(var, argsp);
			j = varaddr("", var); /* curproc()->pname --> "" */
			if (j == -1)
			{
#if DEBUG
				if (debugflag == 1)
				{
					exit1();
				}
				else if (debugflag == 2)
				{
					exit2("doscall");
					endofline();
				}
#endif
				return;
			}
			if (ISARY(sl_stype))
				*subargp++ = j;
			else if (ISTRTYP(sl_stype)) {
				int numwords = (sl_size+WORDSIZE-1)/WORDSIZE;
				while (numwords-- > 0) {
					*subargp++ = getval(j, "d", DSP);
					j += WORDSIZE;
				}
			}

			else {
				/* pushing a character pointer means getting
				 * the value at the address of the pointer
				 *
				 * Note that this is different than a character
				 * array (handled above), where it is the
				 * address of the array which must be pushed.
				 */
				if ((desc = typetodesc(sl_stype))[0] == 's')
					desc = "x";

				if (ISREGV(sl_class)) {
				    if (*desc == 'g') {
					*subargp++ = getreg(j, "d");
					if (ISREGN(j))
					    *subargp++ = getreg(j+1, "d");
					else *subargp++ = getreg(j+REGSIZE,"d");
				    }
				    else  *subargp = getreg(j, desc);
				}
				else {
				    if (*desc == 'g') {
					*subargp++ = getval(j, "d", DSP);
					*subargp++ = getval(j, "d", DSP);
				    } else *subargp++ = getval(j, desc, DSP);
				}
			}
			do {
				argsp++;
			} while (varchar(*argsp) || number(*argsp));
		} else if (ch != ')') {
			fprintf(FPRT1, "Unexpected character %c\n", ch);
#if DEBUG
			if (debugflag == 1)
			{
				exit1();
			}
			else if (debugflag == 2)
			{
				exit2("doscall");
				endofline();
			}
#endif
			return;
		}
	
		do {
			ch = *argsp++;
		} while(ch == ' ');
		if (ch == ')') {
			if (scallx == 0) {
				scallx = 1;
				if (pid) {
#if vax || u3b
					pcs = SDBREG(PC);
 					fps = SDBREG(FP);
 					aps = SDBREG(AP);
#else
#if u3b5 || u3b15 || u3b2
					pcs = regvals[15];
					fps = regvals[9];
					aps = regvals[10];
#endif
#endif
				}
				else {
					pcs = fps = aps = -1;
				}
				if (bkpts = scanbkpt(userpc)) {
					if (flagss = bkpts->flag) {
						bkpts->flag = BKPTSET;
					}
				}
			}
			/* set dot below, instead of here */
			adrflg = 1;
			cntval = 1;
			if (pid == 0 || signo) subpcs('r');
#if vax || u3b
			dot = SDBREG(PC) = adsubc;
#else
#if u3b5 || u3b15 || u3b2
			dot = regvals[15] = adsubc;
#endif
#endif
			if (dot == -1) {	/* moved from above */
				error("Internal error - cannot find __dbsubc");
#if DEBUG
				if (debugflag == 1)
				{
					exit1();
				}
				else if (debugflag == 2)
				{
					exit2("doscall");
					endofline();
				}
#endif
				return;
			}
			subargs[1] = (subargp - subargs) - 2;	/* num args */
			adr = adargs;
			for (j=0; j<(subargp-subargs); j++) {	/* fix */
				put(adr, DSP, subargs[j]);
				adr += WORDSIZE;
			}
			adr = adargs + sizeof subargs;
			for (j=0; j<(numchars+WORDSIZE-1)/WORDSIZE; j++) {
				put(adr, DSP, substr.w[j]);
				adr += WORDSIZE;
			}

#if u3b
			/* 3B libg.a assembler call must be
			 *  overwritten with the right number of args
			 */
			adr = adsubn - 2;		/* in call instr */
			word.w = get(adr,ISP);		/* see libg.s(a) */
			word.s[0] = subargs[1];		/* num of args */
			put(adr,ISP,word.w);		/* write user I! */
#endif
			dschar = *argsp++;
			errflg = 0;
			dopcs('c');
			if (!signo) printf("Breakpoint");
			printf(" at\n");
#if DEBUG
			if (debugflag == 1)
			{
				exit1();
			}
			else if (debugflag == 2)
			{
				exit2("doscall");
				endofline();
			}
#endif
			return;
		}
		while (*argsp == ' ' || *argsp == ',')
			argsp++;
	}
	
	error ("Too many arguments");
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("doscall");
		endofline();
	}
#endif
}


/* get arguments from core file, place them in args */
/* removed if vaxsdb tests and made (major) changes for u3B */
getargs() {
	register struct proct *procp;
	register char **av;
	register int ac, i;
	register long pa;
	char *argsp = args;
	char *argsp1 = args;
#if u3b || u3b5 || u3b15 || u3b2
	union word word;
#else
#if vax
	union {
		char c[WORDSIZE];
		int w;
	} word;
#endif
#endif

#if DEBUG
	if (debugflag ==1)
	{
		enter1("getargs");
	}
	else if (debugflag == 2)
	{
		enter2("getargs");
		closeparen();
	}
#endif
	procp = 0;
	if (pid != 0 || datmap.ufd > 0)
	    for( procp = initframe(); procp != badproc; procp = nextframe() ) {
		if (eqstr("main", procp->pname))
			break;
	    }
	if (procp == 0 || procp == badproc || errflg )
		cpstr(args, oldargs);
	else if ( (ac = get(argp, DSP)) == 0)	/* 3B:int argc;VAX:#args*/
		cpstr(args, oldargs);
#if vax
	else if (ac & ~0xff) cpstr(args, oldargs); /* #args in low byte */
	else {
		ac = get(argp+4, DSP);	/* 1'st arg is int argc; VAX */
		av = (char **) get(argp+8, DSP);	/* char *argv[]; */
#else
#if u3b || u3b5 || u3b15 || u3b2
	else {
		av = (char **) get(argp+4, DSP);	/* char *argv[]; */
#endif
#endif
		av++;			/* 0'th is file name -- skip */
		ac--;			/* arg count */
		pa = get(av++, DSP);
		while (ac) {
#if u3b || u3b5 || u3b15 || u3b2
			i = (pa & 03);	/* 3B: must be 4 byte boundary */
			pa &= ~03;	/* now get() fixed, no longer need */
#else
#if vax
			i = 0;		/* VAX: no boundary problems */
#endif
#endif
			word.w = get(pa, DSP);
			pa += WORDSIZE;
			for ( ; i < WORDSIZE; i++) {
				if (word.c[i] == '\0') {
					if (argsp == argsp1) {	/*  "" */
						*argsp++ = '"';
						*argsp++ = '"';
					}
					*argsp++ = ' ';
					argsp1 = argsp;	/*  arg begins */
					ac--;
					pa = get(av++, DSP);
					break;
				}
				else {
					if (word.c[i]==' ' || word.c[i]=='\t')
						*argsp++ == '\\';
					*argsp++ = word.c[i];
				}
			}
		}
		*argsp = '\0';
	}

	printf("%s %s\n", symfil, args);
	errflg = 0;
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("getargs");
		endofline();
	}
#endif
	return;
}

dopcs(c) 
char c; {
#if DEBUG
	if (debugflag ==1)
	{
		enter1("dopcs");
	}
	else if (debugflag == 2)
	{
		enter2("dopcs");
		arg("c");
		printf("0x%x",c);
		closeparen();
	}
#endif
	if (c != 'r' && c != 'R')
		usrtty();
	if (subpcs(c) == -1)
	{
#if DEBUG
		if (debugflag == 1)
		{
			exit1();
		}
		else if (debugflag == 2)
		{
			exit2("dopcs");
			printf("0x%x",-1);
			endofline();
		}
#endif
		return(-1);
	}
	sdbtty();
 	
	if (eqany(c, "cCiIsS") && userpc == adsubn) {
		if (dschar == '/') {
			dispf((ADDR) RETREG, *argsp ? argsp : "d", C_REG,
				(short)0, 0);
		}
		else
			printf("Procedure returned normally\n");
#if vax || u3b
 		userpc = dot = SDBREG(PC) = pcs;
		SDBREG(FP) = fps;
		SDBREG(AP) = aps;
#else
#if u3b5 || u3b15 || u3b2
		userpc = dot = regvals[15] = pcs;
		regvals[9] = fps;
		regvals[10] = aps;
#endif
#endif
		if (bkpts)
			bkpts->flag = flagss;
		scallx = 0;
		longjmp(env, 0);
	}
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("dopcs");
		printf("0x%x",1);
		endofline();
	}
#endif
	return(1);
}

/*
 * execute commands from a breakpoint:
 * 	returns 1 iff executions should continue from breakpoint.
 */
acommand(cmds)
char *cmds; {
	char buf[256];
	register char *p, *q;
	int endflg = 0;

#if DEBUG
	if (debugflag ==1)
	{
		enter1("acommand");
	}
	else if (debugflag == 2)
	{
		enter2("acommand");
		arg("cmds");
		printf("\"%s\"",cmds);
		closeparen();
	}
#endif
	setcur(0);
	for (p=buf,q=cmds; *q!='\n'; q++)
		if (*q != ' ')		/* squish out spaces */
			*p++ = *q;
	*p = *q;
	q = p = buf;
	do { 		/* process a command */
		for (;;) {
			if (*p == ';') {
				*p = '\n';
				break;
			}
			if (*p == '\n') {
				endflg++;
				break;
			}
			p++;
		}
		if (q[0] == 'k' && eqany(q[1], ";\n"))
		{
#if DEBUG
			if (debugflag == 1)
			{
				exit1();
			}
			else if (debugflag == 2)
			{
				exit2("acommand");
				printf("0x%x",0);
				endofline();
			}
#endif
			return(0);
		}
		if (decode(q) == 1) {
			fprintf(FPRT1, "Bad command: ");
			do {
				fprintf(FPRT1, "%c", *q);
			} while (*q++ != '\n');
#if DEBUG
			if (debugflag == 1)
			{
				exit1();
			}
			else if (debugflag == 2)
			{
				exit2("acommand");
				printf("0x%x",0);
				endofline();
			}
#endif
			return(0);
		}
		docommand();
		p = q = p + 1;
	} while (!endflg);
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("acommand");
		printf("0x%x",1);
		endofline();
	}
#endif
	return(1);
}

/* added sdbtty() and also ifndef SGTTY
 * Save current user tty modes
 * and restore original sdb tty modes
 */

sdbtty()
{
	int arg;	/* unused ? */
#if DEBUG
	if (debugflag ==1)
	{
		enter1("sdbtty");
	}
	else if (debugflag == 2)
	{
		enter2("sdbtty");
		closeparen();
	}
#endif
#ifndef SGTTY
	usrttyf = fcntl(FDIN, F_GETFL, 0);
	ioctl(FDTTY, TCGETA, &usrttym);
	ioctl(FDTTY, TCSETAW, &sdbttym);
	fcntl(FDIN, F_SETFL, sdbttyf);
#else
	gtty(FDTTY, &usrttym);
	if (sdbttym.sg_flags != usrttym.sg_flags) {
		/* Bug in USG tty driver, put out a DEL as a patch. */
		if (sdbttym.sg_ospeed >= B1200)
			write(FDTTY, "\377", 1);
		stty(FDTTY, &sdbttym);
	}
#endif
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("sdbtty");
		endofline();
	}
#endif
}

/* added usrtty() and also ifndef SGTTY
 * Restore user tty modes
 */
usrtty()
{
#if DEBUG
	if (debugflag ==1)
	{
		enter1("usrtty");
	}
	else if (debugflag == 2)
	{
		enter2("usrtty");
		closeparen();
	}
#endif
#ifndef SGTTY
	ioctl(FDTTY, TCSETAW, &usrttym);
	fcntl(FDIN, F_SETFL, usrttyf);
#else
	if (sdbttym.sg_flags != usrttym.sg_flags) {
		/* Bug in USG tty driver, put out a DEL as a patch. */
		if (usrttym.sg_ospeed >= B1200)	/* from ex-vi */
			write(FDTTY, "\377", 1);
		stty(FDTTY, &usrttym);
	}
#endif
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("usrtty");
		endofline();
	}
#endif
}
