/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcc2:common/xdefs.c	10.2"

# include "mfile1.h"

/*	communication between lexical routines	*/

int	lineno;		/* line number of the input file */

FILE * outfile = stdout;	/* place to write all output */
#ifdef	CG
FILE * textfile = stdout;	/* user-requested output file */
#endif

CONSZ lastcon;  /* the last constant read by the lexical analyzer */
FP_DOUBLE dcon;   /* the last double read by the lexical analyzer */


/*	symbol table maintainence */

int	curftn;  /* "current" function */
int	strftn;  /* 1 if current function returns struct or union */
int	ftnno;  /* "current" function number */
int	curloc;		  /* current location counter value */

#ifndef	CG			/* CG has no symbol table */
struct symtab stab[SYMTSZ+1];  /* one extra slot for scratch */

int	curclass,	  /* current storage class */
instruct,	/* "in structure" flag */
stwart,		/* for accessing names which are structure members or names */
blevel,		/* block level: 0 for extern, 1 for ftn args, >=2 inside function */
curdim;		/* current offset into the dimension table */

int	dimtab[ DIMTABSZ ];

int	paramstk[ PARAMSZ ];  /* holds symtab indices of function parameters */
int	paramno;	  /* the number of parameters */
int	argsoff[ ARGSZ ];  /* the offsets for the arguments */
int	argno;		/* the number of arguments */
int	argstk[ ARGSZ ];	/* the symtab indices  of arguments */
TWORD	argty[ ARGSZ ];		/* the types of the arguments */
int	autooff,	/* the next unused automatic offset */
argoff,	/* the next unused argument offset */
strucoff;	/*  the next structure offset position */
#endif	/* def CG */

#ifdef	REGSET
RST	regvar;		/* currently busy register variable bitmap */
#else
int	regvar;		/* the next free register for register variables */
#endif
int	nextrvar;	/* the next allocated reg (set by cisreg) */
OFFSZ	inoff;		/* offset of external element being initialized */
int	brkflag = 0;	/* complain about break statements not reached */

struct sw swtab[SWITSZ];  /* table for cases within a switch */
struct sw *swp;  /* pointer to next free entry in swtab */
int swx;  /* index of beginning of cases for current switch */

/* debugging flag */
int xdebug = 0;
int idebug = 0;

int strflg;  /* if on, strings are to be treated as lists */

int reached;	/* true if statement can be reached... */

int idname;	/* tunnel to buildtree for name id's */


extern NODE node[];

int cflag = 0;  /* do we check for funny casts */
int hflag = 0;  /* do we check for various heuristics which may indicate errors */
int pflag = 0;  /* do we check for portable constructions */

int brklab;
int contlab;
#ifdef M32B
	int swregno;
#endif
int flostat;
int retlab = NOLAB;
int retstat;

/* save array for break, continue labels, and flostat */

/* beware!!  If REGSET turned on, register vector bitmaps get
** stored here.  An "int" had better be large enough!
*/

int asavbc[BCSZ];
int *psavbc = asavbc ;

/* stack of scope chains */

struct symtab *scopestack[MAXNEST];

static char *ccnames[] = 
{
	 /* names of storage classes */
	"SNULL",
	"AUTO",
	"EXTERN",
	"STATIC",
	"REGISTER",
	"EXTDEF",
	"LABEL",
	"ULABEL",
	"MOS",
	"PARAM",
	"STNAME",
	"MOU",
	"UNAME",
	"TYPEDEF",
	"FORTRAN",
	"ENAME",
	"MOE",
	"UFORTRAN",
	"USTATIC",
#ifdef IN_LINE
	"INLINE",
#endif
};

char *
scnames( c )
register c; 
{
	/* return the name for storage class c */
	static char buf[12];
	if( c&FIELD )
	{
		sprintf( buf, "FIELD[%d]", c&FLDSIZ );
		return( buf );
	}
	return( ccnames[c] );
}

#ifdef M32B
	/* for register allocation optimizations */

	int fordepth;	/* nest depth of 'for' loops */
	int whdepth;	/* nest depth of 'while' and 'do-while' loops */
	int brdepth;	/* nest depth of 'if', 'if-else', and 'switch' */
#endif

/* default settings for loop test positioning */
int wloop_level = LL_BOT;	/* place "while" test at loop end */
int floop_level = LL_DUP;	/* place "for" test at top and bot */

#ifdef	CG
char costing = 0;	/* 1 if we are costing an expression */
int str_spot;		/* place for structure return */
#endif
