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
	/* @(#) prvar.c: 1.11 10/14/83 */

#include "head.h"
#include "coff.h"

/* 
 * outvar():
 * Prints named variable, recursing once for each structure member or
 *  subscript.
 * procp: pointer to proc table entry (for local symbol) or NULL pointer (0)
 * var: variable name
 * fmt: print format
 * metaflag: set iff var contains metacharacters * or ?
 * addr: partial address of variable, initally 0
 * class: storage class of variable
 * type: type of variable (dereferenced on each recursive call)
 * subflag: number of array subscripts parsed (pointer dereferences do NOT
 *		count), to be used by scaleindx (symt.c) which must
 *		know which array is being indexed, and at what level
 * prnamep: pointer to end of partially formed print name of variable
 * stroff: offset in symbol table for last element found in structure
 *		tag listing (-1 if not printing a structure)
 * prvar: as in findvar
 *
 * Here and elsewhere we assume that -1 is an invalid address,
 * and it is used to indicate error.	(ERROR for -1 sometimes)
 */
outvar(procp, var, fmt, metaflag, addr, class, type, subflag, prnamep, stroff, prvar)
struct proct *procp;
char *var;
char *fmt;
int metaflag;
ADDR addr;
char class;
short type;
int subflag;
char *prnamep;
long stroff;
int prvar;
{
	char *p, *r, *oldpr;
	static long soffset, goffset;	/* made static, so when recurse */
	static long toffset;		/*  for static variables */
	register ADDR newaddr = -1, arrowaddr;
	enum {INIT, ARROW, DOT} typeflag;
	extern long tagoff;	/* offset for structure tag, set by all
				 * functions which might locate a structure
				 * in the symbol table (slooknext, strlookup,
				 * and sglookup).
				 */

#if DEBUG > 1
	if(debugflag) {
		fprintf(FPRT2,
			"outvar(proc=%s, var=%s, fmt=%s, ",
			procp ? procp->pname : "", var, fmt);
		fprintf(FPRT2,
			"metaflag=%d, addr=%#x, class=%d,",metaflag,addr,class);
		fprintf(FPRT2,
			"\n	type=%#x, subflag=%d, prnamep=%s,",
			type, subflag, prnamep);
		fprintf(FPRT2,
			" stroff=%#lx, prvar=%d);\n", stroff, prvar);
	}
#endif
	switch (var[0]) {
	case '\0':
		/* if only want value of address, return it immediately,
		 * even if it is the address of an array or structure.
		 */
		if (prvar == 0) {
			sl_stype = type;	/* correct, but dangerous code.
						 * sl_stype set ONLY when
						 * getting address.
						 */
			return(addr);
		}

		/* if printout of address desired, go to printout
		 * (print address, even if structure or array name)
		 */
		if (prvar == 2) goto printout;

		/* check if structure, union, or enum, and user left
		 * out member name (i.e. use all)
		 */
		if (type == T_STRUCT || type == T_UNION) {
			*prnamep++ = '.';
			var = "*";
			metaflag |= STREXP;
			goto nextmember;
		}

		/* check if array, and user left out last indices */
		/* (if character array, and format is 's', print)  */
		if (type == ((DT_ARY<<N_BTSHFT) | T_CHAR) &&
			(fmt[0] == '\0' || eqany('s', fmt)));
		else if (ISARY(type)) {
			/* type == sl_type iff no subscripts parsed */
			if (type==sl_type || !procp ||
			    procp->sfptr->f_type == OTHERTYPE)
				*prnamep++ = '[';
			metaflag |= ARRAYEXP;
			var = "*";
			goto nextindx;
		}


		/*  changed proc to procp, and don't use comblk
		 * if (metaflag) {
		 * 	if (comblk[0] && !(eqstr(comblk, "*")))
		 * 		printf("%.8s:%s", comblk, prname);
		 * 	else if (proc[0])
		 * 		printf("%.8s:%s", proc, prname);
		 * 	else
		 * 		printf("%s", prname);
		 * }
		 */

		/* if assignment is to be made, just do that, and do
		 * not print the name of the variable assigned
		 * (that is an option which could be added here,
		 *  perhaps with (metaflag & METACHAR) ||
		 *		(metaflag && prvar != 3) ).
		 */
printout:	if (metaflag && prvar != 3) {
			if (procp && (metaflag & METACHAR)) {
#if u3b || u3b5
				printf("%s:%s", procp->pname, prname);
#else
#if vax || ns32000
				if (procp->pname[0] == '_')
				{
					printf("%s:%s", (procp->pname)+1,
						prname);
				}
				else
				{
					printf("%s:%s",(procp->pname),prname);
				}
#endif
#endif
			}
			else
			{
				printf("%s", prname);
			}
		}
		printit(metaflag, prvar, addr, fmt, class, type,
			sl_size, DSP);
		return(addr);

	case '[':
		*prnamep++ = '[';
		/* fall through to case ',' */
	case ',':
		var++;
	nextindx:
		{
			int i, lbnd, ubnd;
			int rowsize;
			int aryaddr;
			int ftype;
			char *thisnamep;   /* end of current print name */

			ftype = procp ? procp->sfptr->f_type : OTHERTYPE;

			/* F77 multidimensional arrays cannot be handled
			 * if they are arguments (insufficient size
			 * information).  ISARY(DECREF(type)) =>
			 * multidimensional array; ISARGV(class) => arg.
			 */
			if (ftype == F77 &&
				ISARGV(class) && ISARY(DECREF(type)))
			{
			  error("Cannot access multidimensional arguments");
			  return(-1);
			}

			/* dereference in case of pointer */
			newaddr = getindir(class, addr, type);

			/* get size by which to multiply index */
			rowsize = scaleindx(type, subflag, ftype);
			/* '*' or [] mean the whole dimension */
			if (*var == '*' || *var == ']') {
				/* Cannot have ranges for pointers */
				if (ISPTR(type))
				{
				  error("Pointer must have range specified");
				  return(-1);
				}
				metaflag |= ARRAYEXP;
				getbnd( &lbnd, &ubnd, subflag );
				/* kludge for F77 until true lower bound
				 * appears in symbol table
				 * (see getbnd() in symt.c)
				 */
				if (ftype == F77) {
				  lbnd++;
				  ubnd++;
				}
				if (*var == '*') var++; /* skip to ']' */
			}
			else {
				lbnd = readint(&var);
				/* parse lbnd; ubnd */
				if (*var == ';') {
					var++;
					ubnd = readint(&var);
					if (ubnd != lbnd)
						metaflag |= ARRAYEXP;
				}
				else ubnd = lbnd;
			}
			for (i = lbnd; i <= ubnd; i++) {
				thisnamep = prnamep +
					sprintf(prnamep, "%d", i);
				if (*var == ']' || *var == ',') {
				    *thisnamep++ = *var;
				}
				else { /* add delimeter as needed */
				    if (ftype == OTHERTYPE ||
					!ISARY(DECREF(type)))
						*thisnamep++ = ']';
				    else *thisnamep++ = ',';
				}
				*thisnamep = '\0';	/* in case end */

				/* leave ',' as delimeter for next level,
				 * but ']' must be skipped (*var)
				 */
				aryaddr = outvar(procp,
					*var==']' ? var+1 : var, fmt,
					metaflag, newaddr + rowsize * i,
					C_EXT, DECREF(type),
					ISARY(type) ? subflag+1 : subflag,
				        thisnamep, stroff, prvar);
				/* in case of error, do not keep looping,
				 * and repeating the same error.
				 */
				if (aryaddr < 0) break;
			}
		return(aryaddr);
		}
		
	case '-':
	case '>':
		typeflag = ARROW;
		while (eqany(*var, "->"))
			*prnamep++ = *var++;
		arrowaddr = getindir(class, addr, type);
		if (errflg) {
			fprintf(FPRT1, "%s\n", errflg);
			errflg = 0;
			return(0);
		}
		class = C_EXT;	/*  getindir thinks address not reg */
		if (var[0] == '\0') {
		/*	p = var;
			newaddr = arrowaddr;
			subflag++;
			goto recurse;
		 */
			error("-> must point to something");
			return(0);
		}
		break;

	case '.':
		*prnamep++ = *var++;
	nextmember:
		typeflag = DOT;
		if (ISREGV(class))
		{
			error("Not with a register variable");
			return(0);
		}

		break;
	default:
		typeflag = INIT;
		break;
	}

	if (typeflag == INIT) {
		if (procp) {		/*  local variable */
			/* adtostoffset has side effect of setting "level"
			 * (static variable in symt.c) to the block level
			 * of the address.  This is used in slooknext.
			 * procp is needed to know where the symbols
			 * are for the desired fcn.
			 */
			soffset = adtostoffset(callpc-(signo?NOBACKUP:0),procp);
			goffset = -1;
			toffset = -1;	/*  view statics as global */
		}
		else {		/*  global variable */
			soffset = -1;
			goffset = extstart;
			/* dot, not callpc is used, because callpc may be
			 * way back on the stack, while dot should be the
			 * pc of the breakpoint
			 */
			toffset = (adrtoprocp(dot-(signo?NOBACKUP:0)))->
						sfptr->f_statics;
		}
	}
	/* else structure elements.  addr may be a user supplied address,
	 *	in which case the last remembered structure tag is used.
	 */

#if DEBUG > 1
	if(debugflag)
		fprintf(FPRT2,
			" : typeflag=%d; t,g,soffset=%#o,%#o,%#o; var=%s;\n",
			typeflag, toffset, goffset,soffset, var);
#endif

	/* copy variable name to prnamep (if matched, actual string used) */
	p = var;
	oldpr = prnamep;
	while (!eqany(*p, "->.[") && *p != '\0')
		*prnamep++ = *p++;
	*prnamep = '\0';

#if DEBUG > 1
	if(debugflag)
		fprintf(FPRT2,"s,g,toffset = %#lo, %#lo, %#lo;\n",
			soffset, goffset, toffset);
#endif
     /*	if (typeflag == INIT) slookinit();	*/

	/* infinite loop (for metacharacters) until var is no longer matched */
	for (;;) {
		if (typeflag != INIT) {
			if ((stroff = strlookup(var, stroff)) != -1) goto found;
			else return(newaddr);
		}
		if (soffset != -1) {
			if((soffset=slooknext(var,soffset,procp)) != -1)
				goto found;
		}
		if (toffset != -1) {
			if ((toffset = sglookup(var, toffset)) != -1)
				goto found;
		}
		if (goffset != -1)
			if ((goffset = sglookup(var, goffset)) != -1)
				goto found;
		return(newaddr);

	found:
	/*	fprintf(FPRT2, "FOUND: %s, %d;\n", sl_name, typeflag);	*/
#if u3b || u3b5
		r = sl_name;
#else
#if vax || ns32000
		r = *sl_name == '_' ? sl_name+1 : sl_name;
#endif
#endif
		prnamep = oldpr;
		while (*r) *prnamep++ = *r++;
		*prnamep = '\0';

		switch(typeflag) {
		case INIT:
			class = sl_class;
			if (!varclass(class,sl_type) || ISTELT(class))
				continue;	/*goto l*/
			newaddr = formaddr(class, sl_addr);
			break;

		case ARROW:
			class = sl_class;
			if (!varclass(class,sl_type) || !ISTELT(class))
				continue;	/*goto l;*/
			newaddr = arrowaddr;
			if (class != C_FIELD)	
				newaddr += sl_addr;
			break;

		case DOT:
			class = sl_class;
			if (!varclass(class,sl_type) || !ISTELT(class))
				continue;	/*goto l;*/
			newaddr = addr;
			if (class != C_FIELD)
				newaddr += sl_addr;
			break;
		}

	recurse:
		/* if printing a structure, the local variable stroff
		 * remembers which structure element is being displayed,
		 * while outvar recurses with the offset of the tag for
		 * the substructure.
		 */
		newaddr = outvar(procp, p, fmt, metaflag, newaddr, class,
				sl_type, 0, prnamep, tagoff, prvar);
		
		if (!(metaflag & (METACHAR | STREXP)) )
			return(newaddr);
l:;	}
}

/* Output external variables.  Arguments as in outvar() */
/*  extoutvar used to be called by findvar (prvar.c).
 * 	It is no longer used, and has been commented out.
 * extoutvar(var, fmt, metaflag, prvar)
 * char *var, *fmt; {
 * 	long offset;
 * 	ADDR addr = -1;
 * 
 * #if DEBUG > 1
 * 	if(mjmdflag)
 * 		fprintf(FPRT2,
 * 			"extoutvar(var=%s, fmt=%s, metaflag=%d, prvar=%d);\n",
 * 					var, fmt, metaflag, prvar);
 * #endif
 * 	offset = extstart;
 * 	sl_addr = -1;
 * 
 * 	for (;;) {
 * 		offset = extlookup(var, offset);
 * 		addr = sl_addr;
 * 		if (offset == -1)
 * 			return(addr);
 * 		if (metaflag) {
 * #if u3b || u3b5
 * 			printf("%.7s", sl_name);
 * #else
 * if vax
 * 			printf("%.7s", *sl_name=='_' ? sl_name+1 : sl_name);
 * #endif
 * #endif
 * 		}
 * 		printit(metaflag, prvar, addr, fmt[0] ? fmt : "", 
 * 			sl_class, sl_type, 0, DSP);
 * 		if (!metaflag)
 * 			return(addr);
 * 	}
 * }
 */

prdebug() {
	register struct proct *procp;
	register struct filet *filep;
	extern int wtflag;
	extern int magic;
	
	printf("wtflag=%d; badproc=%#x; badfile=%#x; filework=%s; magic=%#o\n",
		wtflag, badproc, badfile, filework, magic);
	printf("dot=%ld;\t", dot);
	printf("extstart = %ld;\t", extstart);
	printf("firstdata = %ld;\n", firstdata);
#ifdef ns32000
	/* this fix isn't just for the ns32000 !!?	*/
	for(filep=files;filep<badfile;filep++)
#else
	for(filep=files;filep->sfilename[0];filep++)
#endif
		printf("%.14s offs %#lo @ %#x flag %d addr %#lx\n",
			filep->sfilename, filep->stf_offset, filep,
				filep->lineflag, filep->faddr);
	for(procp=procs;procp->pname[0];procp++) {
		printf("%s addr %#lx; offs %#lo; sfptr %#x; line %d",
			procp->pname, procp->paddress, procp->st_offset,
			procp->sfptr, procp->lineno);
		if (procp->entrypt) printf(" entrypoint");
		if (procp->notstab) printf(" notstab");
		printf("\n");
	}
}

/*
 * display addr in data space using format desc or class s
 *  type == -1 => use addr for value to print
 */
dispf(addr, desc, class, type, size)
char *desc; short type; ADDR addr; {
	register i;
	i = dispx(addr, desc, class, type, size, DSP);
	printf("\n");
	return(i);
}

/* display addr in instruction space using format desc or class s */
/*  returns -1 if bad address */
dispi(addr, desc, class, type, size)
char *desc; short type; ADDR addr; {
	register i;
	i = dispx(addr, desc, class, type, size, ISP);
	printf("\n");
	return(i);
}

static char pd[] = "%lx";

/* dispx() -
 *	Print the contents at address addr (unless type == -1, in which case
 *		the address itself is printed).  A section of memory is
 *		displayed starting at the address indicated if the user
 *		provides a repeat count in the output format description desc
 *		(q.v.).
 *
 *	desc: the user supplied output format (or default) is in the form - 
 *		[repeat count] [length] [display format]
 *	class: storage class of variable to be displayed.  Only used in
 *		test ISREGV(class) and in checking for C_FIELD.
 *		In the case of dereferenced register variables, class
 *		should be set so that ISREGV(class) fails.  By convention,
 *		C_EXT is used.
 *	type: type of variable, dereferenced (-1 means print address)
 *	size: size of the symbol in bytes.  Note that sizeof(pointer) ==
 *		WORDSIZE, and sizeof(physical address) == WORDSIZE.
 *		Size is not used.
 *
 *	This function has the side effect of setting otype, oclass,
 *	and oaddr, which remember respectively the type, class, and
 *	address of the last variable displayed.  The variable oincr
 *	address of the last variable displayed.  Also, oincr is set
 *	to the number of bytes displayed, or in the case of displaying
 *	an address, the size in bytes of the variable.  Thus
 *	oaddr + oincr is the next byte to be displayed, and this
 *	is how these variables are used by docomm() (docomm.c).
 */

dispx(addr, desc, class, type, size, space)
ADDR addr;
char *desc;
int class;
ushort type;	/* must cast -1 as ushort before comparing */
int space;
{
	int repeat = 0;	/* nonzero if user gave nonzero repeat count */
	long value;	/* buffer for fetched value */
	ADDR savaddr;	/* save initial value of addr (for use with 'i') */
	int for_cntr;
	char *p;
	char *getfmt;	/* "d", "h", "b", "d", or typetodesc(type) -
			 * used to call getval */

	char dfmt;	/* 'a', 's', 'c', 'p', 'f', 'g', 'i', 'I',
			 * 'u', 'x', 'o', or 'd' (default) - output format */

	char dlen;	/* 'l', 'h', 'b', or 'd' (default) - output length */

	int delta;	/* size of fetched value, in bytes.  Used when
			 * displaying characters (to know how many to
			 * display from fetched value), and when dumping
			 * ranges of memory (to know delta to next region).
			 */
	union dbl
	{
		struct
		{
			int w1;
			int w2;
		} ww;
		double d;
	} dbl;

	union word word;

	/* initialize getfmt to typetodesc(type).  Will override if
	 * user specified a repeat count.
	 */
	if (type != (ushort)(-1)) 	/* don't need to fetch address */
		getfmt = typetodesc(type);
	else	getfmt = "d";		/* getfmt is tested below */

	/* decode repeat count, if any */
	p = desc;
	if (number(*p))
	{
		repeat = readint(&p);
	}

	/* save format descriptor, variable type, class, and address
	 * in odesc, otype, oclass, and oaddr respectively, for possible
	 * later reuse.
	 *
	 * oincr (old size increment) is set to zero in case of error,
	 * and is later set to the correct output length.
	 */
	cpstr(odesc, desc);	/* copies original desc; perhaps wrong */
	otype = type;
	oclass = class;
	oaddr = addr;
	oincr = 0;

#if DEBUG
	if (debug)
	{
		fprintf(FPRT2,
			"dispx(addr=%#x,desc=%s,class=%d,", addr, desc, class);
		fprintf(FPRT2,
			"type=%d,size=%d,space=%d)\n", type, size, space);
	}
#endif


	/* decode length */
	switch (*p)
	{
	case 'l':
		if (repeat) {		/* override natural fetch length */
			getfmt = "l";
			delta = 4;
		}
		dlen = 'l';
		p++;
		break;
	case 'h':
		if (repeat) {		/* override natural fetch length */
			if (p[1] == 'd')
			     getfmt = "h";	/* sign extend */
			else getfmt = "hu";	/* don't extend hex, octal */
			delta = 2;
		}
		dlen = 'h';
		p++;
		break;
	case 'b':
		if (repeat) {		/* override natural fetch length */
			if (p[1] == 'd')
			     getfmt = "b";	/* sign extend */
			else getfmt = "bu";	/* don't extend hex, octal */
			delta = 1;
		}
		dlen = 'b';
		p++;
		break;
	default:
		/* getfmt already has natural format */
		dlen = 'd';	/* default okay; dlen only important
				 * if user wants to truncate fectched val */
		if (type != (ushort)(-1))
		    delta = typetosize(type, 0); /* natural value for oincr */
		else delta = 0;
		break;
	}

	/* decode format */
	switch(*p)
	{
	case 'a':
		getfmt = "b";	/* regardless of address type, view as
				 * address of character string
				 */
		/* fall through */
	case 'f':
	case 'g':
	case 'i':
	case 'I':
	case 's':
	case 'p':
		if (class == C_FIELD)
		{
		    fprintf(FPRT1, "%d format not allowed with bit fields\n",
				*p);
		    return(-1);
		}
		/* else fall through */
	case 'c':
	case 'u':
	case 'x':
	case 'd':
	case 'o':
		pd[2] = dfmt = *p;
		break;
	case '\0':	/* user gave no format, so use default */
		if (type == (ushort)-1)		/* print address */
			dfmt = 'x';
		else {
			char *natfmt;
			natfmt = typetodesc(type);
			if (eqany(*natfmt, "lhb")) {
				if (natfmt[1]) dfmt = natfmt[1];
				else dfmt = 'd';
			}
			else dfmt = natfmt[0];
		}
		pd[2] = dfmt;
		break;
	default:
		fprintf(FPRT1, "Illegal descriptor: %c\n", *p);
		return(-1);
	}
	/* if character pointer is to be displayed numerically, don't
	 * fetch character
	 */
	if (dfmt != 's' && *getfmt == 's') getfmt = "d";

	/* sign extension logic:
	 * if getfmt[1] != 'u', sign extend on fetch and output, otherwise
	 * do not.
	 *
	 * unsigned short var/d - fetch in natural format ("hu"), and
	 *			print as %d;  thus it is necessary for
	 *			getval to know that it is not sign
	 *			extending (getval lengthens values while
	 *			dispx can truncate on output).
	 *
	 * long var/2hd -	fetch in "h" format, and print halfword
	 *			value, sign extended.  Thus it is necessary
	 *			for dispx to cast the value as (short), to
	 *			sign extend on output.  Note that getfmt
	 *			has the sign extension information here, too.
	 *
	 * long var/2bu -	fetch in "bu" format, and print byte value,
	 *			without sign extending.  Here, getfmt == "bu".
	 */

	errflg = 0;
	if (*getfmt == 'g') getfmt = "d";  /* can only fetch one word */

	/* if address, check error inputs, take address as value,
	 * and goto section of code that handles values that are
	 * completely retrieved
	 */
	if (type == (ushort)-1)
	{
		if (class == C_FIELD)
		{
			fprintf(FPRT1, "Bit fields have no address.\n");
			return(-1);
		}
		/* repeat count meaningless for address */
		if (repeat)
		{
			fprintf(FPRT1,
			    "Repeat count ignored when displaying address\n\t\t");
			repeat = 0;
		}
		/* registers have no address.  If a register has been
		 * dereferenced, then ISREGV(class) should fail.
		 */
		if (ISREGV(class))
		{
			fprintf(FPRT1, "Registers have no address.  ");
			if (ISREGN(addr))	/* register not on stack */
			{
				fprintf(FPRT1,
				    "(register number) ");
			}
			else	fprintf(FPRT1,
				    "(address of variable on stack)");
		}


		value = addr;

		/* Only numeric or character ('c') formats allowed */
		if (!eqany(dfmt, "xduoc"))
		{
			fprintf(FPRT1,
			    "%c format not allowed with address.\n",
			    dfmt);
			return(-1);
		}
	}

	/* if bit bield, extract value, check for valid (numeric or 'c')
	 * format, check for null repeat count, and process
	 */
	else if (class == C_FIELD)
	{
		int w = sl_addr/(WORDSIZE*8);	/* which word (off of zeroth)*/
		int b = sl_addr - 8*WORDSIZE*w;	/* bit offset in word - from
						 * left (3B) or right (VAX) */

#if u3b || u3b5
		/* all shifts and bit fields are unsigned, so can
		 * get sloppy and use value (a signed int) */
		
		/* get full word, and mask (thus length is 'd') */
		value = getval(addr+w*WORDSIZE, "d", space);

		/* sl_size has number of bits.  Shift left to clear
		 * bits to left of bit field, then right justify result */
		value = value << b >> (WORDSIZE*8 - sl_size);
#else
#if vax || ns32000
		/* VAX has signed and unsigned bit fields and shifts */
		unsigned uval;
		ushort btype = BTYPE(type);	/* basic type */

		/* if bit field unsigned, shift as unsigned int */
		if (btype == T_UCHAR || btype == T_USHORT ||
			btype == T_UINT || btype == T_ULONG)
		{
			uval = getval(addr+w*WORDSIZE, "d", space);
			uval = uval << (WORDSIZE*8 - sl_size - b) >>
					(WORDSIZE*8 - sl_size);
			value = uval;
		}
		else	/* signed bit field */
		{
			value = getval(addr+w*WORDSIZE, "d", space);
			value = value << (WORDSIZE*8 - sl_size - b) >>
					(WORDSIZE*8 - sl_size);
		}
#endif
#endif
		if (repeat)
		{
			fprintf(FPRT1,
				"Repeat count ignored for bit fields\n\t\t");
			repeat = 0;
		}
		if (!eqany(dfmt, "xudo"))
		{
			if (dfmt == 'c')
			{
				delta = (sl_size + 7)/8;
			}
			else {
				fprintf( FPRT1,
				    "%c format not allowed with bit fields.\n",
				    dfmt );
				return(-1);
			}
		}
		else
		{
			delta = 0;	/* don't set oincr past bit field */
		}
	}

	/* if register variable, check for dump range commands, and
	 * invalid repeat counts (except for 's') */
	else if (ISREGV(class))
	{
		/* commands which ask for a dump of a range of memory
		 * starting at the address of the variable are not
		if (dfmt == 'a' || dfmt == 'I' || dfmt == 'i')
		{
			fprintf(FPRT1,
			    "%c format not allowed with registers\n", dfmt);
			return(-1);
		}

		/* Repeat counts imply range of memory, and are not allowed.
		 * 's' format means dereference, and is okay
		 */
		if (repeat && dfmt != 's')
		{
			fprintf(FPRT1,
			    "Repeat count ignored with register variables\n");
			repeat = 0;
		}
		if (dfmt == 's') addr = getreg(addr, "d");
		else value = getreg(addr, getfmt);
	}

	else {
		if (dfmt == 's') addr = getindir(class, addr, type);
		else value = getval(addr, getfmt, space);
	}

	if (dfmt == 's') 
	{
		getfmt = "c";
		value = getval(addr, getfmt, space);
		delta = 1;
	}

	/* "value" now has the first value of the range to be dumped.
	 * If "repeat" <= 1, then merely dump the value in the format
	 * specified.  If "repeat" > 1, then use getfmt to retrieve
	 * more data, and continue dumping.  oincr is initially zero,
	 * and is incremented by delta each time a dump is performed.
	 */

#if DEBUG > 1
	if (debugflag)
	{
		fprintf(FPRT2,
		    "addr=%#x; value=%#x; repeat=%d; delta=%d;\n",
		    addr, value, repeat, delta);
		fprintf(FPRT2,
		    "dlen=%c; dfmt=%c; getfmt=%s\n", dlen, dfmt, getfmt);
	}
#endif

	for_cntr = 0;	/* to keep track of tabs vs. new-lines */
	for(;;)
	{
		switch(dlen)
		{
		case 'h':
			if (getfmt[1]) {	/* unsigned short */
				value = (unsigned short)value;
			}
			else	value = (short)value;
			break;
		case 'b':
			if (getfmt[1]) {	/* unsigned char */
				value = (unsigned char)value;
			}
			else	value = (char)value;
			break;
		case 'l':
			value = (long)value;
			break;
		default:
#ifdef mc68000
#endif
			break;
		}

		switch(dfmt)
		{
		case 'u':
		case 'x':
		case 'd':
		case 'o':
			if (dfmt == 'x' && (value > 9 || value < 0))
				printf("0x");
			else if (dfmt == 'o' && (value > 7 || value < 0))
				printf("0");
			printf(pd, value);
			oincr += delta;
			addr += delta;
			break;

		case 'f':
			word.w = value;
			printf("%g", word.f);
			oincr += delta;
			addr += delta;
			break;

		case 'g':
			dbl.ww.w1 = value;
			if (ISREGV(class)) {
				if (ISREGN(addr))
					dbl.ww.w2 = getreg(addr+1,"d");
				else dbl.ww.w2 = getreg(addr+REGSIZE,"d");
			}
			else	dbl.ww.w2 = getval(addr+WORDSIZE, "d", space);
			printf("%.13g", dbl.d);
			oincr += delta;
			addr += delta;
			break;

		case 'p':
		{
			struct proct *procp;
			char *name;
			if ((procp = adrtoprocp((ADDR)value)) == badproc)
			{
				fprintf(FPRT1, "not a text address\n");
			}
			else {
				name = procp->pname;
#if u3b || u3b5
				fprintf(FPRT1, "%s", name);
#else
#if vax || ns32000
				if (*name == '_')
				{
					fprintf(FPRT1,"%s",name+1);
				}
				else
				{
					fprintf(FPRT1,"%s",name);
				}
#endif
#endif
			}
			oincr += delta;
			addr += delta;
		}
			break;

		case 'c':		/* dump delta bytes in byte order,
					 * assuming value is right justified
					 */
		{
			char *byteptr;
			int loopcnt;
			if (delta > sizeof(value)) {	/* double word */
				delta = sizeof(value);	/* could do by
					* brute force (fetch first word, dump,
					* fetch second word, and fall through),
					* but why bother;  nonsense anyway.
					*/
			}
#if u3b || u3b5
			byteptr = (char *)(&value) + sizeof(value)-delta;
#else
#if vax || ns32000
			byteptr = (char *)(&value);
#endif
#endif
			for (loopcnt = 0; loopcnt < delta; loopcnt++) {
				printchar(*byteptr++);
				fprintf(FPRT1, " ");
			}
			oincr += delta;
			addr += delta;
		}
			break;

		case 'a':
		case 's':
		{
			int loopcnt;
			if (repeat) {	/* user gave width of string */
				oincr = repeat;
				loopcnt = repeat;
			} else {
				loopcnt = 128;	/* max width of string */
						/* count oincr while printing */
			}
			for(;;) {
				/* if the user did not supply a width...*/
				if(!repeat)
				{
					/* count character */
					oincr++;
					/* if null character, return */
					if ((char)value == '\0')
						return(1);
				}
				printchar(value);
				if (--loopcnt <= 0) return(1);
				value = getval(++addr, "b", space);
			}
		}
			break;

		case 'i':
		case 'I':
		{
			int savincr;	/* printins sets oincr */
			savincr = oincr;
			if (!for_cntr) savaddr = addr; /* reset dot at end */
			dot = addr;	/* printins disassembles instruction
					 * at dot */
			printins(dfmt, space);
			if (errflg) {
				fprintf(FPRT1, "%s", errflg);
				errflg = '\0';
				return(-1);
			}
			addr += oincr;
			oincr += savincr;
		}
			break;

		}	/* end of switch statement */
		if (--repeat <= 0) break;
		value = getval(addr, getfmt, space);
		if (dfmt != 'i' && dfmt != 'I') {
			if (++for_cntr % 6 == 0) fprintf(FPRT1, "\n");
			else fprintf(FPRT1, "\t");
		}
		else {
			fprintf(FPRT2, "\n");
			dot = addr;
			prisploc(2);	/* print text address symbolically */
			for_cntr++;
		}
	}
	if (dfmt == 'i' || dfmt == 'I') dot = savaddr;
	return(1);
}

/* print variable as in prvar */
printit(metaflag, prvar, addr, desc, class, type, size, space) 
char *desc; short type; ADDR addr; {
#if DEBUG > 1
	if(debugflag) {
		fprintf(FPRT2,"printit(metaflag=%d, prvar=%d", metaflag, prvar);
		fprintf(FPRT2," addr=%#lx, desc=%s, class=%d,",addr,desc,class);
		fprintf(FPRT2, " type=%d, size=%d, space=%d);\n",
				type, size, space);
	}
#endif
	if (prvar == 0)
		return;
	if (prvar == 3) {	/* assign rhs_value to variable */
		enum valtype lhs_type;
		union dbl dblbuf;
		char *desc;
		desc = typetodesc(type);
		lhs_type = (*desc == 'g') ? DBL : (*desc == 'f') ? FLOAT :
					INTEG;
		if (ISREGV(class)) {
			switch(TYPETYPE(lhs_type, rhs_value.tag)) {
			case 0:		/* INTEG, INTEG */
				putreg(addr, desc, rhs_value.val.i);
				break;

			case 1:		/* INTEG, FLOAT */
				putreg(addr, desc, (int)(rhs_value.val.f));
				break;

			case 2:		/* INTEG, DBL */
				putreg(addr, desc, (int)(rhs_value.val.dd));
				break;

			case 3:		/* FLOAT, INTEG */
				putreg(addr,desc,(float)(rhs_value.val.i));
				break;

			case 4:		/* FLOAT, FLOAT */
				putreg(addr, desc, rhs_value.val.f);
				break;

			case 5:		/* FLOAT, DBL */
				putreg(addr, desc, (float)(rhs_value.val.dd));
				break;

			case 6:		/* DBL, INTEG */
			case 7:		/* DBL, FLOAT */
			case 8:		/* DBL, DBL */
				switch(rhs_value.tag) {
				case 0:		/* INTEG */
				    dblbuf.dd = (double)(rhs_value.val.i);
				    break;
				case 1:		/* FLOAT */
				    dblbuf.dd = (double)(rhs_value.val.f);
				    break;
				case 2:		/* DBL */
				    dblbuf.dd = rhs_value.val.dd;
				    break;
				}

				putreg(addr, "d", dblbuf.ww.w1);
				if (ISREGN(addr))
				    putreg(addr+1, "d", dblbuf.ww.w2);
				else
				    putreg(addr+REGSIZE, "d", dblbuf.ww.w2);

				break;
			}
		}

		else if (class == C_FIELD) {
			unsigned mask = ~0;
			int w = sl_addr/(WORDSIZE*8);	/* which word
							 * (off zeroth) */
			int br;		/* bit offset in word, from right */
			int bl;		/* bit offset in word, from left */
			int wordval;	/* full word to be reinserted */
			int bitval;	/* numeric value of new bit field */
			addr += w*WORDSIZE;

#if u3b || u3b5
			bl = sl_addr - w*8*WORDSIZE;  /* bit offset from left */
			br = 8*WORDSIZE - bl - sl_size;  /* offset from right */
#else
#if vax || ns32000
			br = sl_addr -w*8*WORDSIZE;  /* bit offset from right */
			bl = 8*WORDSIZE - br -sl_size;  /* offset from left */
#endif
#endif
			/* ((unsigned)1 << sl_size) - 1 will not work
			 * to create a right justified mask because it fails
			 * if sl_size == 32.
			 */

			mask = mask << bl >> (8*WORDSIZE - sl_size) << br;
			switch(rhs_value.tag) {
			case INTEG:
				bitval = rhs_value.val.i;
				break;
			case FLOAT:
				bitval = (int)(rhs_value.val.f);
				break;
			case DBL:
				bitval = (int)(rhs_value.val.dd);
				break;
			}
			wordval = (getval(addr, "d", space) & ~mask) |
				((bitval << br) & mask);
			putval(addr, "d", wordval);
		}

		else {
			union word thisword;

			switch(TYPETYPE(lhs_type, rhs_value.tag)) {
			case 0:		/* INTEG, INTEG */
				putval(addr, desc, rhs_value.val.i);
				break;

			case 1:		/* INTEG, FLOAT */
				putval(addr, desc, (int)(rhs_value.val.f));
				break;

			case 2:		/* INTEG, DBL */
				putval(addr, desc, (int)(rhs_value.val.dd));
				break;

			case 3:		/* FLOAT, INTEG */
				putval(addr,desc,(float)(rhs_value.val.i));
				break;

			case 4:		/* FLOAT, FLOAT */
				putval(addr, desc, rhs_value.val.f);
				break;

			case 5:		/* FLOAT, DBL */
				thisword.f= (float)(rhs_value.val.dd);
				putval(addr, desc, thisword.l);
				break;

			case 6:		/* DBL, INTEG */
			case 7:		/* DBL, FLOAT */
			case 8:		/* DBL, DBL */
				switch(rhs_value.tag) {
				case 0:		/* INTEG */
				    dblbuf.dd = (double)(rhs_value.val.i);
				    break;
				case 1:		/* FLOAT */
				    dblbuf.dd = (double)(rhs_value.val.f);
				    break;
				case 2:		/* DBL */
				    dblbuf.dd = rhs_value.val.dd;
				    break;
				}

				putval(addr, "d", dblbuf.ww.w1);
				putval(addr+WORDSIZE, "d", dblbuf.ww.w2);

				break;
			}
		}
		return;
	}
	if (metaflag) {
		if (prvar == 1)
			printf("/ ");
		else
			printf("= ");
	}
	if (prvar == 1)
		dispx(addr, desc, class, (short) type, size, space);
	else
		dispx(addr, desc, class, (short) -1, 0, DSP);
	printf("\n");
}

printchar(c)
char c; {
	if ((c & 0177) < ' ') 
		printf("^%c", c + ('A' - 1));
	else if ((c & 0177) == 0177)
		printf("^?");
	else
		printf("%c", c);
}

printmap(s,amap)
STRING	s; MAP *amap;
{
	int file;
	file=amap->ufd;
	printf("%s\t`%s'\n",s,(file<0 ? "-" : (file==fcor ? corfil : symfil)));
	printf("b1 = %#-16lx",amap->b1);
	printf("e1 = %#-16lx",amap->e1);
	printf("f1 = %#-lx",amap->f1);
	printf("\nb2 = %#-16lx",amap->b2);
	printf("e2 = %#-16lx",amap->e2);
	printf("f2 = %#-lx",amap->f2);
	printf("\n");
}

printregs()
{
#if vax || ns32000 || u3b
	REG REGPTR	p;
#else
#if u3b5
	REG i;
#endif
#endif

	if (pid == 0 && (fcor < 0 || fakecor))	{  /* no registers */
		errflg = "No process and no core file.";
		chkerr();
	}

#if vax || ns32000 || u3b
	for (p=reglist; p < &reglist[(NUMREGLS+1)/2]; p++) {
		printf("%4.4s/  ", p->rname);
		prhex12(SDBREG(p->roffs));
		if (p + NUMREGLS/2 < &reglist[NUMREGLS]) {
			printf("\t%7.7s/  ",(p+NUMREGLS/2)->rname);
			prhex12(SDBREG((p+NUMREGLS/2)->roffs));
		}
		printf("\n");
	}
#else
#if u3b5
	for (i = 0; i < 9; i += 3)
	{
		printf("%4.4s/ ",reglist[i].rname);
		prhex12(regvals[i]);
		printf("\t%4.4s/ ",reglist[i+1].rname);
		prhex12(regvals[i+1]);
		printf("\t%4.4s/ ",reglist[i+2].rname);
		prhex12(regvals[i+2]);
		printf("\n");
	}
	printf("%4.4s/ ",reglist[10].rname);	/* ap */
	prhex12(regvals[10]);
	printf("\t%4.4s/ ",reglist[9].rname);	/* fp */
	prhex12(regvals[9]);
	printf("\t%4.4s/ ",reglist[12].rname);	/* sp */
	prhex12(regvals[12]);
	printf("\n");
	printf("%4.4s/ ",reglist[11].rname);	/* psw */
	prhex12(regvals[11]);
	printf("\t%4.4s/ ",reglist[15].rname);	/* pc */
	prhex12(regvals[15]);
	printf("\n");
#endif
#endif
	printpc();
}

printpc()
{
	if (pid == 0 && (fcor < 0 || fakecor))	{  /* no pc value */
		errflg = "No process and no core file.";
		chkerr();
	}
	dot= USERPC;
	prisploc(2);
	printins('i',ISP);
#ifndef	ns32000
	printf("\n");
#endif
}
	
/* print register -- or just return value if prvar == 0 */
regout(name, prvar, fmt) 
char *name, *fmt;
{
	REG REGPTR p;
	for (p=reglist; p< &reglist[NUMREGLS]; p++) {
		if (eqstr(name, p->rname)) {
#if u3b5
			if (p->roffs == -1)
				break;		/* unsaved register */
#endif
			printit(0, prvar, p-reglist, fmt[0] ? fmt : "d", C_REG,
				prvar == 2 ? -1 : 0, 0, DSP);
			return(p - reglist);
		}
	}
	error("Unknown register variable");
	return(ERROR);
}
/* Print symbolic location of dot */
prisploc(mode)
int mode;	/*  added arg; old way is mode = 2 */
{
	register struct proct *procp;
	register long diff;

	printf("%#lx", dot);
	procp = adrtoprocp(dot);
#if DEBUG > 1
	if(debugflag)
		fprintf(FPRT2, "prisploc(mode=%d); pname=%s;\n", mode,
				procp == badproc ? "badproc" : procp->pname);
#endif
	if (procp != badproc) {
		printf(" (");
		prlnoff(procp, dot);
		if(mode > 1)
			printf("):  \t");
		else
			printf(")\n");
	} else {	/*  added extra */
		diff = adrtoext(dot);
		if(diff >= 0 && sl_scnum > 0) {
#if u3b || u3b5
			printf(" %s", sl_name);
#else
#if vax || ns32000
			if (*sl_name == '_')
			{
				printf(" %s", sl_name+1);
			}
			else
			{
				printf( " %s", sl_name );
			}
#endif
#endif
			if(diff > 0) {
				printf("+");
				prhex((long)diff);
			}
		}
		if(mode > 1)
			printf(":  \t");
		else
			printf("\n");
	}
}
