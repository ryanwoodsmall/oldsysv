/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:m32/program.c	1.21"
#include <stdio.h>
#include <filehdr.h>
#include <sgs.h>
#include <storclass.h>
#include <systems.h>
#include <ctype.h>
#include "symbols.h"
#include "gendefs.h"
#include "as_ctype.h"
#include "instab.h"
#include "program.h"
#include "ind.out"
#include "typ.out"
#include "scnhdr.h"
#include "syms.h"

int txtsec = -1;
int datsec = -1;
static int firstinstr = 1;

#ifdef	CHIPFIX
static char * SWA_CHIP = "@(#) CHIPFIX";
#endif

#if	M32RSTFIX
static char * SWA_RST = "@(#) M32RSTFIX";
#endif

#if	ER10ERR
static char * SWA_EXP = "@(#) ER10ERR";
#endif

#if	ER13WAR
static char * SWA_BLDV = "@(#) ER13WAR";
#endif

#if	ER16FIX
static char * SWA_16 = "@(#) ER16FIX";
#endif

#if	ER21WAR
static char * SWA_21 = "@(#) ER21WAR";
#endif

#if	ER40FIX
static char * SWA_40 = "@(#) ER40FIX";
#endif

#if	ER43ERR
static char * SWA_43 = "@(#) ER43ERR";
#endif


/* mnemonics[] is a table of machine instruction mnemonics used by yyparse().      */
/* These are accessed thru hashtab[].  NOTE: the hashing algorithm for the         */
/* mnemonics occurs in TWO places (mk_hashtabs() and find_mnemonic())              */
/* if changed, THE HASH ALGORITHM MUST BE UPDATED IN BOTH PLACES                   */
static struct mnemonic mnemonics[] = {
#include "mnm.out"
0
};

#define HASHSIZ 1007
static short	hashtab[HASHSIZ];

/* initializes the hash table for mnemonics */
static
mk_hashtab()
{
	register struct mnemonic	*curptr;
	register char			*cptr;
	register unsigned long		hval;
	register			i;

	for (i=0; i<HASHSIZ; i++) hashtab[i] = -1;
	for (curptr=mnemonics; curptr->name != 0; curptr++) {
		for (hval=0,cptr=curptr->name; *cptr; cptr++) {
			hval <<= 2;
			hval += *cptr;
		}
		hval += *--cptr << 5;
		hval %= HASHSIZ;
		curptr->next = hashtab[hval];
		hashtab[hval] = curptr - mnemonics;
	}
}


/* for use by routines other than yyparse(), typically on gencode.c */
struct mnemonic *
find_mnemonic(name)
char	*name;
{
	register char *tokptr;
	register hval, instr_index;

	tokptr = name;
	hval = *tokptr++;
	while (*tokptr) {
		hval <<= 2;
		hval += *tokptr++;
	}
	hval += *(tokptr-1) << 5;
	for (instr_index = hashtab[hval%HASHSIZ]; instr_index >= 0;
			instr_index = mnemonics[instr_index].next)
		if (!strcmp(mnemonics[instr_index].name, name))
			return(mnemonics+instr_index);
	return(NULL);
}

char dbg = 0;	/* -# flag */
static unsigned short action; /* action routine to be called during second pass */
extern unsigned short line;   /* current line number */
extern unsigned short cline;  /* line number from `.ln' */
extern short bitpos;
extern short alignflg;	/* force alignment of `save' and `ret' */
extern short opt;	/* optimize flag */
extern long newdot; /* up-to-date value of "." */
extern int previous; /* previous section number */
extern symbol *dot;
extern FILE *fdin;
static long opcd;
extern char cfile[];
extern struct scnhdr sectab[];
extern int mksect();
extern cgsect();
extern upsymins *lookup(); /* for looking up labels */
static char as_version[] = "02.01";
static short
	expand,		/* expand byte to be generated */
	nbits,		/* number of bits of code to be generated */
	spctype,	/* distinguishes between .byte, .half, .word, .float, .double */
	width;		/* bit field width */
static short memtoreg = 0;	/* Indicator of first instruction in the
				 * 2 instruction sequence that manifests
				 * the Interrupt After TSTW chip bug.
				 */
static struct mnemonic *newins; /* for looking up a new instruction name */
extern short workaround, /* software work around disable flag */
	pswopt;	/* PSW optimization */


int need_mau = NO;		/* Assume mau is not required */
int warlevel = ALLWAR;		/* Assume we are generating all workarounds */

#if	M32RSTFIX
extern short rstflag;
#endif

#if FLOAT
unsigned long fpval[3];
#endif
#if !ONEPROC
extern FILE *fdstring;
#endif

extern int deflab();

char	*oplexptr;	/* set up by yyparse() for use in oplex() */

#define OPMAX 50
/* operstak[] is used to make operand list (operstak[0] is the first operand, */
/* operstak[1] the second, etc.). operstak[] is also used as an expression    */
/* stack by expr().                                                           */
OPERAND operstak[OPMAX];
OPERAND *opertop;	/* points to first unused element in operstak[] */

/* yyparse() token types */
#define ENDOFLINE	0
#define LABEL		1
#define MNEMONIC	2
#define OPNDID		3
#define OPNDSTR		5
#define OPND		6

/* for yyparse level tokens, i.e. LABEL's, MNEMONIC's, and OPERAND's */
static struct token {
	char	*ptr;
	char	type;
	short	numopnds;
	} tokens[BUFSIZ/2];

/*
yyparse() is the high level parser for assembly source it calls operand() for operand
parsing.  yyparse consists of a main loop (one time thru for each assembly line) 
containing two phases:

lexing - chopping the line up into labels, mnemonics, and operands.
	 identifier and string operands are recognized directly;
	 all operands have their whitespace removed.

processing - the labels are processed.  mnemonics are looked up.
	     a switch on the instruction index permits tailored
	     parsing and processing of the statement.  otherwise
	     operand() is called in a loop to parse the operands.
	     the operand types are checked.  a switch executes the 
	     statement semantics provided for the mnemonic on "ops.in".
*/
yyparse()
{
	char linebuf[BUFSIZ];		 /* contains line being parsed */

	mk_hashtab();
	if (dbg) fdin = stdin;
	line = 0;
newline:
	while (fgets(linebuf,BUFSIZ,fdin)) {	/* line loop */
		line++;

{ /* lex the line */
	register struct token *nexttoken;/* points to free location for next token */
	register char *start, *end, *ahead = linebuf;
	register struct token *mnemtoken;

	nexttoken = tokens;
anotherstmt: 
	while (1) { /* parse labels until we find something else */
		while (WHITESPACE(*ahead)) ahead++;
		if (IDBEGIN(*ahead)) {
			start = ahead;
			while (IDFOLLOW(*++ahead)) ;
			end = ahead;
			while (WHITESPACE(*ahead)) ahead++;
			if (*ahead == ':') {	/* label */
				nexttoken->type = LABEL;
				nexttoken++->ptr  = start;
				ahead++;
				*end = 0;
			} else if (WHITESPACE(*end) || !OPRCHAR(*ahead)) {
				/* mnemonic */
				mnemtoken = nexttoken;
				nexttoken->type = MNEMONIC;
				nexttoken++->ptr  = start;
				break;
			} else {
				yyerror("no white space between mnemonic & operand");
				goto newline;
			}
		} else
			switch (*ahead) {
			case '#' : case '\0': case '\n':
				nexttoken->type = ENDOFLINE;
				goto process;
			case ';' :
				ahead++;
				break;
			default:
				yyerror("statement syntax");
				goto newline;
			}
	}
	/*scan operands; recognize id's and strings directly; take out white space*/
	mnemtoken->numopnds = 0;
	if (OPRCHAR(*ahead)) while (1) {
		mnemtoken->numopnds++;
		*end = 0;	/* close preceding mnemonic or operand */
		start = ahead;
		if (IDBEGIN(*ahead)) {	/* try for an id */
			while (IDFOLLOW(*++ahead)) ;
			end = ahead;
			while (WHITESPACE(*ahead)) ahead++;
			if (!OPRCHAR(*ahead)) {
				nexttoken->type = OPNDID;
				goto got_opnd;
			}
		} else if (*ahead == '"') {	/* try for a string */
			while (*++ahead)
				if (*ahead == '"') {
					end = ++ahead;
					while (WHITESPACE(*ahead)) ahead++;
					if (!OPRCHAR(*ahead)) {
						nexttoken->type = OPNDSTR;
						goto got_opnd;
					}
				}
			if (*ahead == '\0') {
				yyerror("bad string");
				goto newline;
			}
		} else {
			while (HOPRCHAR(*ahead)) ahead++;
			end = ahead;
		}
		while (OPRCHAR(*ahead))
		/* remove any whitespace but be careful not to combine   */
		/* tokens previously separated by white space. we assume */
		/* that this is a sytax error.                           */
			if (WHITESPACE(*ahead))
				ahead++;
			else if (end == ahead) {
				end++; ahead++;
			} else if (HOPRCHAR(*(ahead-1)) ||
				/* no combining problem since previous
				   character was not a white space character */
				   end == start ||
				/* no combining problem since this is first
				   character to be copied */
				   SINGCHAR(*(end-1)) || SINGCHAR(*ahead))
				/* no combining problem since at least one
				   of the characters always stands alone */
				*end++ = *ahead++;
			else {
				yyerror("operand syntax");
				goto newline;
			}
		nexttoken->type = OPND;
got_opnd:
		nexttoken++->ptr  = start;
		if (*ahead != ',')
			break;
		while (WHITESPACE(*++ahead)) ;
	}
		
	switch (*ahead) {
	case '#' : case '\n': case '\0':
		*end = 0;
		nexttoken->type = ENDOFLINE;
		goto process;
	case ';' :
		*end = 0;
		ahead++;
		goto anotherstmt;
	default:
		yyerror("statement syntax");
		goto newline;
	}
} /* end lexing */

process:
{ /* process the line */
register struct token *curtoken = tokens;

while(curtoken->type != ENDOFLINE) {
	if (curtoken->type == LABEL) {
		register upsymins	symptr;

		if (dbg) printf("LAB %s\n",curtoken->ptr);
		symptr = *lookup(curtoken->ptr, INSTALL, USRNAME);
		if ((symptr.stp->styp & TYPE) != UNDEF) {
			yyerror("error: multiply defined label");
			fprintf(stderr, "\t\t... \"%s\"\n", curtoken->ptr);
		}
		symptr.stp->value = newdot;
		symptr.stp->styp |= dot->styp;
		symptr.stp->sectnum = dot->sectnum;
		if ((dot->styp == TXT))
			deflab(symptr);
		curtoken++;
	} else { /* MNEMONIC */
		register struct mnemonic *instrptr;
		register curopnd, numopnds = curtoken->numopnds; 

		if (dbg) {
			int i; 

			printf("MNM %s",curtoken->ptr);
			for (i=1; i<=curtoken->numopnds; i++)
				printf(" %s",curtoken[i].ptr);
			printf("\n");
		}
		if ((instrptr = find_mnemonic(curtoken->ptr)) == NULL) {
			yyerror("invalid instruction name");
			fprintf(stderr, "\t\t... \"%s\"\n", curtoken->ptr);
			goto instr_done;
		}
		else if (firstinstr)
		{
			firstinstr = 0;
			/*
			 * set default UNIX section
			 */
	cgsect(mksect(lookup(_TEXT, INSTALL, USRNAME)->stp, STYP_TEXT));
		}

		/* processing operands */
		opertop = operstak;
		/* switch for statements having parsing intermingled with semantics */
		switch(instrptr->index) {

#define CHEKID(n)	if (curtoken[n].type != OPNDID) {\
				yyerror("expecting identifier");\
				fprintf(stderr,"\t\t... not \"%s\"\n",\
					curtoken[n].ptr);\
				goto instr_done;\
			}

#define CHEKONEID	if (numopnds != 1) {\
				yyerror("expecting 1 operand");\
				fprintf(stderr,"\t\t... not %d\n",numopnds);\
				goto instr_done;\
			}\
			CHEKID(1)

#define GETID(n)	(curtoken[n].ptr)

#define GETEXPR(n)	oplexptr = curtoken[n].ptr;\
			switch (expr()) { \
			case 0: break; \
			default: yyerror("expression syntax error");\
				 fprintf(stderr,"\t\t... in \"%s\"\n",\
						curtoken[n].ptr);\
			case -1: goto instr_done;\
			}

#define GETFP(n)	oplexptr = curtoken[n].ptr;\
			switch (fp_expr()) { \
			case 0: break; \
			default: yyerror("floating point syntax error");\
				 fprintf(stderr,"\t\t... in \"%s\"\n",\
						curtoken[n].ptr);\
			case -1: goto instr_done;\
			}


case PSSECTION :	/* .section <id>[,<string>] */
	{ register char *s;
	  register int att = 0;
	CHEKID(1)
	if (numopnds == 2) {
		if (curtoken[2].type != OPNDSTR) {
			yyerror("expecting a string");
			fprintf(stderr,"\t\t... not \"%s\"\n",curtoken[2].ptr);
			goto instr_done;
		}
		s = (curtoken[2].ptr)++;
		s[strlen(s)-1] = '\0';
		s++;
		while (*s) switch(*s++) {

			case 'b':
				/* zero initialized block */
				att |= STYP_BSS;
				break;
			case 'c':
				/* copy */
				att |= STYP_COPY;
				break;
			case 'i':
				/* info */
				att |= STYP_INFO;
				break;
			case 'd':
				/* dummy */
				att |= STYP_DSECT;
				break;
			case 'x':
				/* executable */
				att |= STYP_TEXT;
				break;
			case 'n':
				/* noload */
				att |= STYP_NOLOAD;
				break;
			case 'o':
				/* overlay */
				att |= STYP_OVER;
				break;
			case 'l':
				/* lib */
				att |= STYP_LIB;
				break;
			case 'w':
				/* writable */
				att |= STYP_DATA;
				break;
			default:
				yyerror("invalid section attribute");
				break;
			}
		}
	else if (numopnds > 2) {
		yyerror("'.section' takes one or two operands");
		fprintf(stderr,"\t\t... not %d\n",numopnds);
		goto instr_done;
		}
	cgsect(mksect(lookup(GETID(1),INSTALL,USRNAME)->stp,att));
	}
	goto instr_done;
case PSBSS :	/*   .bss <id>,<expr>,<expr>   */
	CHEKID(1)
	GETEXPR(2) opertop++;
	GETEXPR(3)
	if (operstak[1].exptype != ABS)
		aerror("'.align' field not absolute");
	bss(lookup(GETID(1),INSTALL,USRNAME)->stp,
	    operstak[0].expval,operstak[1].expval,
	    operstak[0].exptype);
	goto instr_done;
case PSDEF :	/*   .def <id>   */
	CHEKONEID
	generate(0,DEFINE,NULLVAL, lookup(GETID(1),INSTALL,USRNAME)->stp);
	goto instr_done;
case PSDIM :	/*   .dim <expr>[,<expr>]*   */
	if (numopnds == 0) {
		yyerror("too few operands");
		goto instr_done;
	}
	GETEXPR(1)
	generate(0,SETDIM1,operstak[0].expval,operstak[0].symptr);
	for (curopnd = 2; curopnd <= numopnds; curopnd++) {
		GETEXPR(curopnd)
		generate(0,SETDIM2,operstak[0].expval,operstak[0].symptr);
	}
	goto instr_done;
case PSGLOBL :	/*   .globl <id>   */
	CHEKONEID
	lookup(GETID(1),INSTALL,USRNAME)->stp->styp |= EXTERN;
	goto instr_done;
case PSFILE :	/*   .file <string>   */
case PSIDENT :	/*   .ident <string>   */
case PSVERS :	/*   .version <string> */
	if (numopnds != 1) {
		yyerror("expecting 1 operand");
		fprintf(stderr,"\t\t... not %d\n",numopnds);
		goto instr_done;
	}
	if (curtoken[1].type != OPNDSTR) {
		yyerror("expecting a string");
		fprintf(stderr,"\t\t... not \"%s\"\n",curtoken[1].ptr);
		goto instr_done;
	}
	(curtoken[1].ptr)++;
	(curtoken[1].ptr)[strlen(curtoken[1].ptr)-1] = '\0';
	if (instrptr->index == PSIDENT) {
		comment(curtoken[1].ptr);
		goto instr_done;
	}
	if (instrptr->index == PSVERS) {
		if (strcmp(curtoken[1].ptr,as_version) > 0) {
			yyerror("inappropriate assembler version");
			fprintf(stderr,
				"\t\tcompiler expected %s or greater, have %s\n",
				curtoken[1].ptr,as_version);
		}
		goto instr_done;
	}
	/* DFILE */
	if (cfile[0] != '\0')
		yyerror("only 1 '.file' allowed");
	if (strlen(curtoken[1].ptr) > 14)
		yyerror("'.file' name > 14 characters");
	else
		strcpy(cfile, curtoken[1].ptr);
	generate(0,SETFILE,NULLVAL,NULLSYM);
	goto instr_done;
case PSLN :	/*   .ln <expr>[,<expr>]   */
	if (numopnds == 1) {
		GETEXPR(1)
		generate(0,LLINENO,operstak[0].expval, operstak[0].symptr);
		cline = operstak[0].expval;
	} else if (numopnds == 2) {
		GETEXPR(1) opertop++;
		GETEXPR(2)
		generate(0,LLINENUM,operstak[0].expval, operstak[0].symptr);
		generate(0,LLINEVAL,operstak[1].expval, operstak[1].symptr);
		cline = operstak[0].expval;
	} else {
		yyerror("expecting 1 or 2 operands");
		fprintf(stderr,"\t\t... not %d\n",numopnds);
	}
	goto instr_done;
case PSCOMM :	/*   .comm <id>,<expr>   */
case PSSET :	/*   .set <id>,<expr>   */
	{ register symbol *symptr;
	CHEKID(1)
	GETEXPR(2)
	symptr = lookup(GETID(1),INSTALL,USRNAME)->stp;
	if (instrptr->index == PSCOMM) {
		if ((symptr->styp & TYPE) != UNDEF) {
			yyerror("tried to redefine symbol");
			fprintf(stderr,"\t\t... \"%s\"\n",GETID(1));
		} else {
			symptr->styp |= (EXTERN | UNDEF);
			symptr->value = operstak[0].expval;
		}
		goto instr_done;
	}
	/* DSET */
	if (symptr == NULL)
		aerror("Unable to define identifier");
	/* reset all but EXTERN bit */
	if (symptr == dot) {
		if ((symptr->styp != operstak[0].exptype))
			yyerror("Incompatible types");
		else if ((operstak[0].symptr != NULL) &&
			 (symptr->sectnum != operstak[0].symptr->sectnum))
			yyerror("Incompatible section numbers");
		else {
			long incr;

			if ((incr = operstak[0].symptr->value + operstak[0].expval - symptr->value) < 0)
				yyerror("Cannot decrement '.'");
			else
				fill(incr);
		}
	} else {
		symptr->styp &= EXTERN;
		if ((symptr->styp |= operstak[0].exptype & TYPE) == UNDEF)
			yyerror("Illegal `.set' expression");
		symptr->value = operstak[0].expval;
		if (operstak[0].symptr != NULL) {
			symptr->value += operstak[0].symptr->value;
			symptr->maxval = operstak[0].symptr->maxval;
			symptr->sectnum = operstak[0].symptr->sectnum;
		}
		else
			symptr->sectnum = N_ABS;
	}
	goto instr_done; }
case PSTAG :	/*   .tag <id>   */
	CHEKONEID
	generate(0,SETTAG,NULLVAL, lookup(GETID(1),INSTALL,USRNAME)->stp);
	goto instr_done;
#if	FLOAT
case PSDECINT :
	{ int spctype; int i; long fpval[3];
	spctype = 3 * NBPW;
	ckalign(4L);
	for (curopnd = 1; curopnd <= numopnds; curopnd++) {
		GETEXPR(curopnd)
		if (operstak[0].fasciip == NULL) {
			yyerror("expecting decimal constant");
			goto instr_done;
		}
		atodi(operstak[0].fasciip, fpval);
		for (i=0; i<=2; i++) generate(32, RELDAT32, fpval[i], NULLSYM);
		dot->value = newdot; /* synchronize */
		generate(0,NEWSTMT,(long)line,NULLSYM);
	}
	ckalign(4L);
	}
	goto instr_done;
case PSDECFP :
	{ int spctype;
	spctype = 3 * NBPW;
	ckalign(4L);
	for (curopnd = 1; curopnd <= numopnds; curopnd++) {
		GETFP(curopnd)
		dot->value = newdot; /* synchronize */
		generate(0,NEWSTMT,(long)line,NULLSYM);
	}
	ckalign(4L);
	}
	goto instr_done;
case PSDOUBLX :
case PSDOUBLE :
case PSFLOAT :
	{ register width; int spctype;
	switch (instrptr->index) {
	case PSDOUBLX : spctype = 3*NBPW; ckalign(4L); break;
	case PSDOUBLE : spctype = 2*NBPW; ckalign(4L); break;
	case PSFLOAT : spctype = NBPW; ckalign(4L); break;
	}
	for (curopnd = 1; curopnd <= numopnds; curopnd++) {
		GETFP(curopnd)
		width = spctype;
		if (bitpos + width > spctype)
			yyerror("Expression crosses field boundary");
		switch (spctype) {
		case 32:	/* single fp number */
		   if (atofs(operstak[0].fasciip,fpval))
		      yyerror("fp conversion error");
		   generate(32, RELDAT32, fpval[0], NULLSYM);
		   break;
		case 64:	/* double fp number */
		   if (atofd(operstak[0].fasciip,fpval))
		      yyerror("fp conversion error");
		   generate(32, RELDAT32, fpval[0], NULLSYM);
		   generate(32, RELDAT32, fpval[1], NULLSYM);
		   break;
		case 96:	/* double extended fp number */
		   if (atofx(operstak[0].fasciip,fpval))
		      yyerror("fp conversion error");
		   generate(32, RELDAT32, fpval[0], NULLSYM);
		   generate(32, RELDAT32, fpval[1], NULLSYM);
		   generate(32, RELDAT32, fpval[2], NULLSYM);
		   break;
		}
		dot->value = newdot; /* syncronize */
		generate(0,NEWSTMT,line,NULLSYM);
	}
	ckalign(4L);
	}
	goto instr_done;
#endif
case PSBYTE :	/*   .byte [<num>:]<expr>[,[<num>:]<expr>]*   */
case PSHALF :	/*   .half [<num>:]<expr>[,[<num>:]<expr>]*   */
case PSWORD :	/*   .word [<num>:]<expr>[,[<num>:]<expr>]*   */
	{ register width; int spctype;
	switch (instrptr->index) {
	case PSBYTE : spctype = NBPW/4; break;
	case PSHALF : spctype = NBPW/2; ckalign(2L); break;
	case PSWORD : spctype = NBPW; ckalign(4L); break;
	}
	for (curopnd = 1; curopnd <= numopnds; curopnd++) {
		oplexptr = curtoken[curopnd].ptr;
		switch (expr()) {
		case ':':
			oplexptr++;
			opertop++;
			if (expr()) {
				yyerror("expression error");
				fprintf(stderr,"\t\t... in \"%s\"\n",
					curtoken[curopnd].ptr);
				goto instr_done;
			}
			if (operstak[0].symptr != NULL || 
			    operstak[0].expval < 0)
				yyerror("bad field width specifier");
			width = operstak[0].expval;
			if (bitpos + width > spctype)
				yyerror("expression crosses field boundary");
			action = (width == NBPW) ? RELDAT32 : RESABS;
			generate(width,(operstak[1].symptr!=NULLSYM)*action,
				operstak[1].expval,operstak[1].symptr);
			opertop = operstak;
			break;
		case '\0':
			width = spctype;
			if (bitpos + width > spctype)
				yyerror("expression crosses field boundary");
			/* Figure out which action routine to use   */
			/* in case there is an unresolved symbol.   */
			/* If a full word is being used, then       */
			/* a relocatable may be specified.          */
			/* Otherwise it is restricted to being an   */
			/* absolute (forward reference).            */
			action = (width == NBPW) ? RELDAT32 : RESABS;
			generate(width,(operstak[0].symptr!=NULLSYM)*action,
				operstak[0].expval,operstak[0].symptr);
			break;
		default:
			yyerror("expression error");
			goto instr_done;
		}
		dot->value = newdot; /* syncronize */
		generate(0,NEWSTMT,line,NULLSYM);
	}
	switch (instrptr->index) {
	case PSBYTE : fillbyte(); break;
	case PSHALF : fillbyte(); ckalign(2L); break;
	case PSWORD : fillbyte(); ckalign(4L); break;
	}
	goto instr_done;
	}
case TESTOP1 :
	if (numopnds == 0) {
		operstak[0].type = IMMD;
		operstak[0].newtype = NOTYPE;
		operstak[0].expval = 0;
	} else if (numopnds != 1) {
		yyerror("expecting 0 or 1 operands");
		goto instr_done;
	} else if (operand()) {
		yyerror("operand error");
		goto instr_done;
	}
	if (!(1<<operstak[0].type & TIMMD)) {
		yyerror("operand type mismatch");
		goto instr_done;
	}
	testgen(instrptr, &(operstak[0]));
	memtoreg = 0;
	goto instr_done;

		}
		/* parse operands & check their types */
		{ register unsigned long *opndtype = instrptr->opndtypes;

		for (curopnd = 1; curopnd <= numopnds; curopnd++) {
			oplexptr = curtoken[curopnd].ptr;
			if (operand()) { /* operand error */
				yyerror("operand error");
				fprintf(stderr,"\t\t... operand #%d of \"%s\", \"%s\"\n", curopnd, curtoken->ptr, curtoken[curopnd].ptr);
				goto instr_done;
			}
			if (*opndtype == 0) {
				yyerror("too many operands");
				fprintf(stderr,"\t\t... \"%s\" takes %d; %d were present\n",
					curtoken->ptr, curopnd-1, numopnds);
				goto instr_done;
			}

/* 32000 specific kludge - type DECIMMD's look just like some IMMD's.  If the  */
/* instruction wants a DECIMMD and the operand was an IMMD, we promote it here */
			if (*opndtype & (1<<DECIMMD) && opertop->type == IMMD) {
				if (opertop->fasciip != NULL) /* decimal constant */
					opertop->type = DECIMMD;
			}

			if (!(*opndtype & (1<<opertop->type))) {
				yyerror("operand type mismatch");
				fprintf(stderr,"\t\t... operand #%d of \"%s\": \"%s\"\n", curopnd, curtoken->ptr, curtoken[curopnd].ptr);
				goto instr_done;
			}
			opertop++;
			opndtype++;
			if (dbg) opnd_dmp(operstak+curopnd-1);
		}
		if (*opndtype != 0) {
			yyerror("too few operands");
			fprintf(stderr,"\t\t... only %d present for \"%s\"\n",
					numopnds, curtoken->ptr);
			goto instr_done;
		}
		}

		/* semantic switch */
		switch(instrptr->index) {
#include "sem.out"
		}
instr_done:
		/* end of statement semantics */
		generate(0,NEWSTMT,line,NULLSYM);
		dot->value = newdot;	/* synchronize */

		curtoken += numopnds + 1;
	}
}
} /* end line processing */

	}
	/* end of program semantics */
}


fill(nbytes)
long nbytes;
{
	long fillval;

	fillval = (dot->styp == TXT)	? ((TXTFILL<<8)|TXTFILL)
					: ((FILL<<8)|FILL);
	while (nbytes >= 2) {
		generate(2*BITSPBY,0,fillval,NULLSYM);
		nbytes -= 2;
	}
	if (nbytes)
		generate(BITSPBY,0,fillval,NULLSYM);
} /* fill */

ckalign(size)
long size;
{
	long mod;
	if ((mod = newdot % size) != 0) {
		fill(size - mod);
	}
} /* ckalign */

fillbyte()
{
	if (bitpos)
		generate(BITSPBY-bitpos,0,FILL,NULLSYM);
} /* fillbyte */

bss(sym,val,alignment,valtype)
register symbol *sym;
long val,alignment;
short valtype;
{
	long mod;
	static int bsssec = -1;

	if (valtype != ABS)
		yyerror("`.bss' size not absolute");
	if (val <= 0)
		yyerror("Invalid `.bss' size");
	if ((sym->styp & TYPE) != UNDEF)
		yyerror("multiply define label in `.bss'");
	if (bsssec < 0) bsssec = mksect(
		lookup(_BSS, INSTALL, USRNAME)->stp, STYP_BSS);
	if (mod = sectab[bsssec].s_size % alignment)
		sectab[bsssec].s_size += alignment - mod;
	sym->value = sectab[bsssec].s_size;
	sectab[bsssec].s_size += val;
	sym->sectnum = bsssec;
	sym->styp |= BSS;
} /* bss */
