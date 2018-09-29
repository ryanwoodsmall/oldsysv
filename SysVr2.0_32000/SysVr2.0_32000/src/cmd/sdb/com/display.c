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
	/* @(#) display.c: 1.9 7/18/83 */

#include "head.h"
#include "coff.h"

/* returns address of proc:var */
ADDR
varaddr(proc, var)
char *proc, *var; {
	return(findvar(proc, var, "", 0));
}

/*
 * displays values of variables matching proc:var, 
 * returns its address
 */
ADDR
dispvar(proc, var, fmt)
char *proc, *var, *fmt; {
	return(findvar(proc, var, fmt, 1));
}

/* assigns rhs_value (a global variable, already set) to all variables
 * matching the pattern proc:var.
 * returns address of last variable which received value
 */
ADDR
varasgn(proc,var)
char *proc;
char *var;
{
	return(findvar(proc, var, "", 3));
}
/*
 * Find and print values of all variables matching proc:var
 *	using specified format.
 * Returns address of last matching variable.
 *
 * prvar==0 => no output,
 * prvar==1 => output value,
 * prvar==2 => output addr,
 * prvar==3 => assign rhs_value to address
 *
 * This function matches the proc name, and then calls outvar to match
 *	the variable name.  Outvar in turn calls dispx to print the value.
 */
ADDR
findvar(proc, var, fmt, prvar)
char *proc, *var, *fmt; {
	ADDR a = -1;		/* address of current variable
				 *(-1 iff not found) */
	ADDR addr = -1;		/* last address found */
	int metaflag=metabase;	/* metabase is 0 or SCRIPTEXP (display
				 * names because test script is being
				 * run).  metaflag is a mask used by
				 * outvar (prvar.c) in determining
				 * whether to print symbolic names,
				 * and whether to continue pattern
				 * matching because of metacharacters.
				 * If proc or var contain "*" or "?",
				 * METACHAR is or'ed into the mask here.
				 */
	int match = 0;	/* 1 iff proc name is matched (see comments in code) */
	int nullflag=0;	/* 1 iff user specified no proc (proc == "") */
	int depthcnt= -1;/* negative value means take first proc on stack
			 * matching name given; non-negative quantity tells
			 * how many more matches of proc must be found on
			 * stack before the correct instance of the function
			 * is found.
			 */
	register struct proct *procp;
	extern long tagoff;/* offset for structure tag, set by previous
			    * reference to a structure
			    */
	static char blnkvar[60] = "_BLNK_";  /* used to construct blank
						* common name.
						*/

#if DEBUG > 1
	if(debugflag)
		fprintf(FPRT2, "findvar(proc=%.8s, var=%s, fmt=%s, prvar=%d);\n",
			proc, var, fmt, prvar);
#endif

	/* percentflag set in parsing user command with register name,
	 * e.g. r0%/
	 */
	if (percentflag) {	/* kludge for register names */
		return(regout(var, prvar, fmt));
	}

	if (var[0] == '\0') {
		error("Unexpected null variable name");
		return(-1);
	}

	if(eqany('*', proc) || eqany('?', proc) ||
		eqanyvar('*', var) || eqany('?', var)) metaflag |= METACHAR;
	
	if (proc[0] == '\0')
	{
		nullflag++;
		/* if :var specified, only look for globals, so proc matched */
		if (colonflag)
		{
			match++;
		}
		else
		{
			proc = curproc()->pname;   /* default to current proc */
		}
	}

	/* Used to look only in blank common if user said :var.
	 *	Now, user says .var for blank common.  A dummy name
	 *	is constructed (see below).
	comblk = colonflag ? "" : "*";
	 */
	if (integ) {
		/* number.var, etc. means take the number as the address
		 * of the desired structure, and get the value that would
		 * be in variable var.  The global sl_* sdb variables
		 * remember the last structure accessed, so this will
		 * only give the right result if the user gives an
		 * address of a structure immediately after accessing
		 * the same type structure.  match is set because the
		 * proc name need not be found, and is thus "matched".
		 */
		if (eqany(var[0], "->.[")) 
			match++;

		/* user command- proc:var,number will get the value of
		 * var in the number'th occurence of the call to proc
		 * on the stack (if number not specified, all occurences
		 * of proc:var will be found on the stack) 
		 */
		else
			depthcnt = integ;
	}

	else if (var[0] == '.') {	/* blank common */
		strcpy(&blnkvar[6], var);  /* copy var after _BLNK_ */
		var = blnkvar;
	}

	/* search for local variables (try to find proc on stack) */
	/* nullflag && colonflag == true -> user said :var (global only) */
	/* eqany(var[0], "->[") == true -> user gave address.var	 */
	if (!match && !eqany(var[0], "->.[") && !(nullflag && colonflag)) {
		for (procp=initframe(); procp != badproc;
					procp = nextframe()) {
#if DEBUG > 1
			if(debugflag) {
				fprintf(FPRT2, "findvar: proc=%s; pname=%s;\n",
						proc, procp->pname);
				fprintf(FPRT2, " : depthcnt=%d; integ=%d;\n",
						depthcnt, integ);
			}
#endif
			if (eqpatu(proc, procp->pname)) {
				match++;
				/* if correct occurence of proc, or need all */
				if (--depthcnt==0 || integ==0) {
				     /*	comblk no longer used
				      * a = outvar(procp->pname, var, fmt,
				      *		metaflag, (ADDR) integ,
				      *		C_EXT, 0, prname, comblk,
				      *		prvar);
				      */
					a = outvar(procp, var, fmt,
						metaflag, (ADDR) integ,
						C_EXT, (short)0, 0, prname,
						-1, prvar);
					if (a != -1)
						addr = a;
					if (depthcnt == 0) /* right level */
						break;
				}
			}
			
		}
	}

	/* search for global variables */
	/* if the user said :var (colonflag and nullflag) to do global search
	 * or used metasymbols (metavar [no proc] might match global, and
	 *	proc=* must match global symbols too),
	 * or the variable wasn't found as a local, and the user allowed
	 * globals (including common)
	 */
	if ((colonflag || (metaflag & METACHAR) || a == -1) && 
			(nullflag || eqpat(proc, ""))) {
#if DEBUG > 1
		if(debugflag)
			fprintf(FPRT2, "findvar: before outvar(..C_EXT..);\n");
#endif
		/* comblk no longer used
	      * a = outvar("", var, fmt, metaflag, (ADDR) integ,
	      * 	C_EXT, 0, prname, comblk, prvar);
		*/
		a = outvar((struct proct *)0, var, fmt, metaflag, (ADDR) integ,
			C_EXT, (short)0, 0, prname, tagoff, prvar);
		if (a != -1) {
			addr = a;
			match++;
		}
	}

	/* If proc not on stack, or only globals sought but not found,
	 *	(and colonflag set), then proc was really common name.
	 *
	 *	common:var no longer used (ambiguous at best)
      * if (match==0 && colonflag) {
      *		set callpc, used by outvar
      * 	for (procp=initframe(); procp != badproc;
      * 				procp = nextframe()) {
      * 		if (eqstr(curproc()->pname, procp->pname))
      * 			break;
      * 	}
#if DEBUG > 1
      * 	if(debugflag)
      * 		fprintf(FPRT2, "findvar: before outvar(curproc);\n");
#endif
      * 	a = outvar(curproc()->pname, var, fmt, metaflag,
      * 		(ADDR) integ, C_EXT, 0, prname, 
      * 		nullflag ? "_BLNK_" : proc, prvar);
      * 	if (a != -1) {
      * 		addr = a;
      * 		match++;
      * 	}
      * }
	*/

	/* if the variable wasn't found, try searching globals again,
	 *	even though they were searched before (unless the user
	 *	specified proc:var).  If the user precluded searching
	 *	through globals, then his wishes should be granted.
	 *	This redundant code is hereby commented out.
	 *
      * if (addr == -1 && match == 0) {
      * 	addr = extoutvar(var, fmt, metaflag, prvar);
      * 	if (addr != -1)
      * 		return(addr);
      * }
	*/

	/* if proc was not found on stack, and variable not found
	 * (finding var sets match also), then proc not active
	 */
	if (match == 0) {
#if u3b || u3b5
		fprintf(FPRT1, "%s not an active procedure\n", proc);
#else
		if (*proc == '_')
		    fprintf(FPRT1, "%s not an active procedure\n", proc+1);
		else
		    fprintf(FPRT1, "%s not an active procedure\n", proc);
#endif
		return(-1);
	}

	/* if proc found on stack, but variable not found */
	if (addr == -1) {
		if (var[0] == '.')
			var++;
		/* if user specified globals only, just print global name */
		if (nullflag && colonflag)
			fprintf(FPRT1, "%s not found\n", var);

		/* if user implied blank common, do not print _BLNK_ */
		else if (var == blnkvar)
			fprintf(FPRT1, "%s not found\n", var+6);
		else {
#if u3b || u3b5
			fprintf(FPRT1, "%s:%s not found\n", proc, var);
#else
			if (proc[0]=='_')
			     fprintf(FPRT1, "%s:%s not found\n", proc+1,
					var);
			else fprintf(FPRT1, "%s:%s not found\n", proc,
					var);
#endif
		}
		return(-1);
	}
	return(addr);
}

/* typetodesc(type) -
 *	Given a type, dereferenced by the number of times outvar()
 *	(in prvar.c) has recursed, return the "natural" descriptor.
 *	For character arrays or pointers, the natural type is 's' (string).
 */
char *
typetodesc(type)
ushort type; {
	register int ptr, ftn, ary;
	register char *desc;

	static char *typedesc[] = {
		"d",	/* undef */
		"d",	/* farg */
		"c",	/* char */
		"h",	/* short */
		"d",	/* int */
		"l",	/* long */
		"f",	/* float */
		"g",	/* double */
		"d",	/* strty */
		"d",	/* unionty */
		"d",	/* enumty */
		"d",	/* moety */
		"bu",	/* uchar */
		"hu",	/* ushort */
		"u",	/* unsigned */
		"lu",	/* ulong */
		"d"	/* ? */
	};
	
	ptr = ftn = ary = 0;
	
	desc = typedesc[type&N_BTMASK];
	for (; type & N_TMASK; type = DECREF(type)) {
		if (ISPTR(type)) ptr++;
		else if (ISFCN(type)) ftn++;
		else if (ISARY(type)) ary++;
	}
	
	/* if outvar has recursed to the point that you have dereferenced
	 * to a pointer or array of chars, treat as a string.
	 */
	if ((ptr+ary == 1) && desc[0] == 'c')
		return("s");
#if DEBUG
	if (debug)
		fprintf(FPRT2,
			"PTR %d; FTN %d; ARY %d; DESC %s\n",ptr,ftn,ary,desc);
#endif
	/* if totally dereferenced, return basic descriptor */
	/* if (ptr + ary == subflag)
	 *	return(desc);
	 */
	if (ptr==1 && ftn==1) return("p");	/* pointer to function */
	if (ptr) return("lx");			/* pointer descriptor */
	return(desc);
}

/* typetosize(type, stsize) -
 *	Return size associated with type.  If the type is a pointer to
 *	something, return sizeof(pointer) = WORDSIZE.  If the type is
 *	an array (i.e. an array of something, including multidimensional
 *	arrays, which are arrays of arrays) return the size of an
 *	array element, which may be a pointer, a structure, a union,
 *	or a more basic type.  The size of a structure or union is
 *	the size stored in the symbol table, and passed in stsize.
 *	The type "type" is assumed to be dereferenced to the right level.
 *	For instance:
 *		short (*x[3])[2]; is an array (3) of pointers to arrays
 *				of two shorts.
 *		Its type is 0x373.  The size is 4, because the first
 *		derived type is array (with three elements) of pointers
 *		to something.  Pointers have size 4 (VAX and 3B).
 *
 *		If x[1][0]/ is specified (the [0] dereferences the pointer),
 *		then the type, after the two dereferences indicated is
 *		0x33 (array of shorts), and the size is sizeof short.
 */
typetosize(type, stsize)
ushort type; {

	register int size;
	static char typesize[] = {
		4,	/* undef */
		4,	/* farg */
		1,	/* char */
		2,	/* short */
		WORDSIZE,	/* int */
		4,	/* long */
		4,	/* float */
		8,	/* double */
		0,	/* strty */
		0,	/* unionty */
		4,	/* enumty */
		4,	/* moety */
		1,	/* uchar */
		2,	/* ushort */
		4,	/* unsigned */
		4,	/* ulong */
		4	/* ? */
	};
	
#if DEBUG
	if (debugflag)
		printf(FPRT2, "typetosize(type=%#x, stsize=%#x);\n",
			type, stsize);
#endif

	for (; ISFCN(type) || ISARY(type); type = DECREF(type));
	size = ISPTR(type) ? WORDSIZE : typesize[type & N_BTMASK];

#if DEBUG
	if (debug)
		fprintf(FPRT2,
			"SIZE %d; STSIZE %d\n", size, stsize);
#endif

	if (size == 0) return(stsize);
	else return(size);
}


/* print breakpoints */
prbkpt() {
	register BKPTR bkptr;
	register int cnt;
	char *cmdp;

	cnt = 0;
	
	for (bkptr = bkpthead; bkptr; bkptr=bkptr->nxtbkpt)
		if (bkptr->flag) {
			cnt++;
			printbkpt("", adrtoprocp(bkptr->loc), bkptr->loc);
			cmdp = bkptr->comm;
			if (*cmdp != '\n') {
				printf("   <");
				while (*cmdp != '\n')
					printf("%c", *cmdp++);
				printf(">\n");
			}
			else
				printf("\n");
		}
	if (cnt == 0) 
		printf("No breakpoints set\n");
}

/* interactively delete breakpoints */

idbkpt() {
	register BKPTR bkptr;
	register int yesflg, cnt;
	register char c;
	
	cnt = 0;

	for (bkptr = bkpthead; bkptr; bkptr=bkptr->nxtbkpt)
		if (bkptr->flag) {
			printbkpt(" ? ", adrtoprocp(bkptr->loc), bkptr->loc);
			yesflg = 0;
			cnt++;
			do {
				c = getchar();
				if (c == 'y' || c == 'd') yesflg++;
			} while (c != '\n');
			if (yesflg)
				bkptr->flag = 0;
		}
	if (cnt == 0)
		printf("No breakpoints set\n");
}

/* delete all breakpoints */

dabkpt() {
	register BKPTR bkptr;
	
	for (bkptr = bkpthead; bkptr; bkptr=bkptr->nxtbkpt)
		bkptr->flag = 0;
}
 
/* 
 * Print name of breakpoint for a, b, d commands:
 */
printbkpt(s, procp, dot)
char *s; struct proct *procp; ADDR dot; {
	adrtolineno(dot, procp);		/* sets lnfaddr */
	if (dot != lnfaddr || procp == badproc)
		printf("%#lx (", dot);
	prlnoff(procp, dot);
	if (dot != lnfaddr || procp == badproc)
		printf(")");
	printf("%s", s);
}

/* print call frame */
prframe() {
	prfrx(0);
}

extern INT		signo;
extern STRING		signals[];
sigprint() {
	printf("%s (sig %d)\n", signals[signo], signo);
}

/* print statement number */
printstmt(stmt)
struct stmt stmt; {
	if(stmt.lnno > 0)
		printf("%d", stmt.lnno);
	if (stmt.stno > 1)
		printf(",%d", stmt.stno);
}
