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
	/* @(#) sub.c: 1.9 7/18/83 */

#include "head.h"
#include "coff.h"

char *
readline(f)
FILE *f; {
	static char buff[BUFSIZ];
	
	register char *p;
	register int i;
	
	p = buff;
	do {
		if ((i = getc(f)) == EOF) {
			if (f != stdin) {
				fclose(f);
				inputfile = stdin;
				metabase = 0;
				return(readline(stdin));
			}
			else {
				clearerr(f);
				*p++ = '\004';
				*p = '\n';
			}
		}
		/* i == ^D iff ^D read from input script */
		else if ((char)i == '\004')
		{
			*p++ = '\004';
			*p = '\n';
		}
		else if(i == sdbttym.c_cc[VEOL])
		{
			if(p == buff)
				*p++ = '\004';
			*p = '\n';
		}
		else *p = i;
	} while (*p++ != '\n');
	
	return(buff);
}

char *
cpname(p, q)
char *p, *q; {
	for (;;)
	{
		while (smplchar(*q) || number(*q)) *p++ = *q++;
		if (*q == '[')
		{
			*p++ = *q++;
			while (indxchar(*q)) *p++ = *q++;
			if (*q != ']')
			{
				error("unmatched '[';  ']' provided");
				*p++ = ']';
			}
			else	*p++ = *q++;
			continue;
		}
		break;
	}
	*p = '\0';
	return(q);
}

char *
cpall(p, q)
char *p, *q; {
	while (*q != '\n') 
		*p++ = *q++;
	*p = '\0';
	return(q);
}

eqany(c, s)
char c, *s; {
	while(*s != '\0')
		if (c == *s++) return(1);
	return(0);
}

/* eqanyvar checks if the character c appears in the variable name *s.
 * It does not check within subscripts ([]), since that is not part of
 * the variable name.
 */
eqanyvar(c, s)
char c;
char *s;
{
	while(*s != '\0') {
		if (*s == '[') {
			do s++; while (*s != ']' && *s != '\0');
			continue;
		}
		if (c == *s++) return(1);
	}
	return(0);
}

error(s)
char *s; {
	if(*s)
		fprintf(FPRT1, "%s\n", s);
}

char *
cpstr(p,q)
register char *p, *q; {
/*  BAD cpstr(); no good for null strings !
	do {
		*p++ = *q++;
	} while (*q != '\0');
	*p = '\0';
*/
	do {
		*p++ = *q;
	} while (*q++ != '\0');
}

L_INT
round(a,b)
REG L_INT a, b;
{
	REG L_INT w;
	w = (a/b)*b;
	if (a!=w) w += b;
	return(w);
}

/* error handling */

chkerr()
{
	extern int errno;
	char str[80];

	if (errflg || mkfault) {
		error(errflg);
		errflg = (char *)0;
		mkfault = 0;
		longjmp(env, 0);
	}
	if(errno)
	{
		sprintf(str, "sdb chkerr() [errno=%d]", errno);
		perror(str);
		errno = 0;
		longjmp(env, 0);
	}
}

eqstr(s1, s2)
char	*s1, *s2;
{
	return( !strcmp( s1, s2 ) );
}

/* descriptor format to length */
dtol(d)
char d; {
	switch(d) {
	
	case 'a':
	case 's':
		return(0);	/* ?? */
		
	case 'b':
	case 'c':
		return(1);
		
	case 'h':
		return(2);
		
	case 'l':
	case 'f':
		return(4);

	case 'g':
		return(8);

	default:
		return(WORDSIZE);
	}
}

/*
 * checks equality of pattern pat with str,
 * assuming str is truncated at length 8
 */
eqpat( pat, str )
char *pat, *str;
{
#ifdef	FLEXNAMES
	return( eqpatr( pat, str ) );
#else
	char	local[ 9 ];
	strncpy( local, pat, 8 );
	local[ 8 ] = '\0';
	return( eqpatr( local, str ) );
#endif
}

eqpatr( pat, str )
char *pat, *str;
{
	register int i;
	register char p, s;
	
	p = pat[0];
	s = str[0];
	if (p == '?') {
		if (s == '\0') return(0);
		return(eqpatr(pat+1, str+1));
	}
	if (p == '*') {
		if (pat[1] == '\0') return(1);
		for( i = 1 ;; i++ )
		{
			if ( eqpatr( pat + 1, str + i ) ) return(1);
			if (str[i] == '\0') return(0);
		}
	}
	if ((eqany(p, ".[->") || p == '\0') && s == '\0') return(1);
	if (p != s) return(0);
	return(eqpatr(pat+1, str+1));
}

/* checks eqpat, eliminating leading `_' if any */
/*  changed so user may specify leading '_' */
/*	than rechanged so no special treeatment for leading '_' */
eqpatu( pat, str )
char *pat, *str;
{
#ifdef	FLEXNAMES
	if( eqpatr( pat, str ) )
	{
		return( 1 );
	}
#else
	{
		char	local[ 9 ];
		strncpy( local, pat, 8 );
		local[ 8 ] = '\0';
		if( eqpatr( local, str ) )
		{
			return( 1 );
		}
	}
#endif

/* VAX symbols may have a leading underscore, 3B symbols do not */
#ifdef FLEXNAMES
#if vax || ns32000
	if ( *str == '_' )
	{
		return( eqpatr( pat, str + 1 ) );
	}
#endif
#else
#if vax || ns32000
	{
		char	local[ 9 ];
		strncpy( local, pat, 7 );
		local[ 7 ] = '\0';
		if ( *str == '_' )
		{
			return( eqpatr( local, str + 1 ) );
		}
	}
#endif
#endif
	return(0);
}

/* gets indirect address for pointers and subscripts */
ADDR
getindir(class, addr, type) 
ADDR addr; {
#if DEBUG > 1
	if(debugflag)
		fprintf(FPRT2, "getindir(class=%d, addr=%#lx, type=%#x);\n",
					class, addr, type);
#endif
	if (ISARY(type)) return(addr);
	if (ISREGV(class))
#if vax || ns32000 || u3b
		return(getreg(addr, ADDRTYPE));
#else
#if u3b5
		return(regvals[addr]);
#endif
#endif
	return(getval(addr, ADDRTYPE, DSP));
}

long
readint(p)
char **p; {
	int sign;

	if (**p == '-') {
		sign = -1;
		(*p)++;
	} else {
		sign = 1;
	}
	if (**p == '0') {
		(*p)++;
		if (**p == 'x' || **p == 'X') {
			(*p)++;
			return(sign * rint(p, 16, hexdigit, hexconv));
		}
		else return(sign * rint(p, 8, octdigit, octconv));
	}
	else return(sign * rint(p, 10, decdigit, decconv));
}

long
rint(p, base, digit, conv)
char **p;
int (*digit)(), (*conv)(); {
	long value;
	
	value = 0;
	while ((*digit)(**p)) value = base*value + (*conv)(*(*p)++);  
	return(value);
}

octdigit(c) 
char c; {
	return(c >= '0' && c <= '7');
}

octconv(c)
char c; {
	return(c - '0');
}

decdigit(c)
char c; {
	return(c >= '0' && c <= '9');
}

decconv(c)
char c; {
	return(c - '0');
}

hexdigit(c)
char c; {
	return((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
		(c >= 'A' && c <= 'F'));
}

hexconv(c)
char c; {
	if (c >= '0' && c <= '9') return(c - '0');
	if (c >= 'a' && c <= 'f') return(c - 'a' + 10);
	if (c >= 'A' && c <= 'F') return(c - 'A' + 10);
	error("hex conversion error");
	return(0);
}

/*  convert char to backslash value */
char 
bsconv(c)
char c;
{
	switch (c) {
	    case 'b':
		return ('\b');
	    case 'f':
		return ('\f');
	    case 'n':
		return ('\n');
	    case 'r':
		return ('\r');
	    case 't':
		return ('\t');
	    case '\\':
		return ('\\');
	}
	return (c);
}

/* argvalue(p) - computes the value associated with the string p,
 *		which may be a variable, a number, or a quoted
 *		character.
 *		The value is stored in the global variable rhs_value.
 *		It is stored as an INTEG, except for floats and doubles.
 *		Chars and shorts have values as ints, and getval
 *		and getreg (which this function calls), properly sign
 *		extends them (unless, of course, they are unsigned).
 *
 *		The value may be used for a '!' (assignment) operation.
 *		In the case of char or shorts, putval and putreg will
 *		truncate properly.
 */

enum valtype
argvalue(p)
char *p;
{
	register char ch;
	register long value;
	register ADDR j;
	char proc[30];
	char var[60];
	int spctflg;
	int svcolonflg;
	register char *typeptr;

#if DEBUG > 1
	if(debugflag)
		fprintf(FPRT2, "argvalue(p=%s);\n", p);
#endif

	ch = *p;

	/* decode numeric value */
	if (number(ch) || eqany(ch, ".-")) {
		char *q;
		double atof();
		/* determine whether number is float or int */
		for (q = ch=='-' ? p : p+1;  *q!='.' && *q!='e';  q++)
		{
			/* if integer... */
			if (!(number(*q))) {
				rhs_value.tag = INTEG;
				rhs_value.val.i = readint(&p);
				break;
			}
		}
		if (*q == '.' || *q == 'e') {	/* floating point number */
			rhs_value.tag = DBL;
			rhs_value.val.dd = atof(p);
		}
		return(rhs_value.tag);
	}

	/* Get ascii value of a character (user said ...!'char) */
	if (ch == '\'')
	{
		rhs_value.val.i = (int) *(p+1); /* sign extend on VAX */
		rhs_value.tag = INTEG;
		return(INTEG);
	}

	/* Get value of symbolic variable */

	spctflg = percentflag;		/*  save and restore */
	percentflag = eqany('%',p);
	svcolonflg = colonflag;		/* save and restore */
	colonflag = eqany(':',p);
	if (colonflag)
		p = cpname(proc, p) + 1;/*  skip colon */
	cpname(var, p);
	j = varaddr(proc, var);		/*  may be a reg% or extern */
	if (j == -1) {
		fprintf(FPRT1, "Unknown variable: %s\n", argsp);
		return((enum valtype)-1);
	}

	/* Note: reglist must be in numeric order, since
	 * for a register variable, j is the number of the
	 * variable, not the index into reglist.
	 * Another solution is to say:
	 *
	 * if (!percentflag && ISREGV(sl_class) && ISREGN(j)) {
	 *	char regstring[] = "r0\0";
	 *	if (j > 9) {
	 *		regstring[1] = '1';
	 *		regstring[2] = '0' + j - 10;
	 *	}
	 *	else {
	 *		regstring[1] = '0' + j;
	 *		regstring[2] = '\0'; /* to clear out last value?? */
	/*	}
	 *	j = regout(regstring,0,"");
	 *
	 */

	if (percentflag) {
#if vax || ns32000 || u3b
		rhs_value.val.i = SDBREG(reglist[j].roffs);
#else
#if u3b5
		rhs_value.val.i = regvals[j];
#endif
#endif
		rhs_value.tag = INTEG;
	}
	else {
		typeptr = typetodesc(sl_stype);
		ch = *typeptr;
		if (ISREGV(sl_class)) {	 /* sl_class only valid for variables */
			if (ch == 'g') {
				rhs_value.val.ww.w1 = getreg(j, "d");
				rhs_value.val.ww.w2 = ISREGN(j) ?
						getreg(j+1, "d") :
						getreg(j+REGSIZE, "d");
			}
			else {
				rhs_value.val.i = getreg(j, typeptr);
			}
		}
		else {
			if (ch == 'g') {
				rhs_value.val.ww.w1 = getval(j, "d");
				rhs_value.val.ww.w2 = getval(j+WORDSIZE, "d");
				rhs_value.tag = DBL;
			}
			else {
				rhs_value.val.i = getval(j, typeptr);
			}
		}
		rhs_value.tag = (ch == 'g') ? DBL :
				(ch == 'f') ? FLOAT : INTEG;
	}

	percentflag = spctflg;
	colonflag = svcolonflg;

#if DEBUG>1
	if (debugflag) {
		fprintf(FPRT2, "argvalue: rhs_value.tag=%d,  rhs_value.val=",
			rhs_value.tag);
		switch(rhs_value.tag) {
		case INTEG:
			fprintf(FPRT2, "%#x\n", rhs_value.val.i);
			break;
		case DBL:
			fprintf(FPRT2, "%g\n", rhs_value.val.dd);
			break;
		case FLOAT:
			fprintf(FPRT2, "%f\n", rhs_value.val.f);
			break;
		}
	}
#endif

	return(rhs_value.tag);
}

prhex(v) 
long v; {
	if (v < 0 && v != 0x80000000)  {	/*  test for 0x80000000 */
		v = -v;
		printf("-");
	}
	if (v <= 9 && v != 0x80000000)		/*  test for 0x80000000 */
		printf("%ld", v);
	else
		printf("%#lx", v);
}

/* print hex number in field of length 12 */
prhex12(v) 
long v; {
	if (v >= -9 && v <= 9)
		printf("%-12ld", v);
	else
		printf("0x%-10lx", v);
}

/* print line number followed by offset */
prlnoff(procp, v)
struct proct *procp; L_INT v; {
	int diff;
	struct stmt stmt;
	char *name;

	if(procp == badproc)
	{
#if DEBUG > 1
		if(debugflag)
			fprintf(FPRT2, "prlnoff('badproc`, v=%#x);\n", v);
#endif
		return;
	}
	name = procp->pname;

	/* VAX symbols may have an underscore prepended, 3B
	 *	symbols do not.
	 */
#if u3b || u3b5
	if (name[0] == '_')
	{
		printf("%s", name);
		stmt.lnno = -1;
	}
	else
	{
		printf("%s", name);
		stmt = adtostmt((ADDR) v, procp);
	}
#else
#if vax || ns32000
	if (name[0] == '_')
	{
		printf( "%s", name + 1 );
	}
	else
	{
		printf("%s", name);
	}
	stmt = adtostmt((ADDR) v, procp);
#endif
#endif
	if (stmt.lnno == -1)
		diff = v - procp->paddress;
	else {
		printf(":");
		printstmt(stmt);
		diff = v - lnfaddr;  /* set by adrtolineno() */
	}
	if (diff) {
		printf("+");
		prhex((long) diff);
	}
}
