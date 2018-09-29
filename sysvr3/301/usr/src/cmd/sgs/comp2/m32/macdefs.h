/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcc2:m32/macdefs.h	10.5"
/*	macdefs - machine dependent macros and parameters
 *
 *		Bellmac32 w/Mao/Berenbaum stack frame
 *
 *			R. J. Mascitti
 */

/* initial values for stack offsets in bits */

#define ARGINIT 0
#define AUTOINIT 0

/* sizes of types in bits */

#define SZCHAR 8
#define SZSHORT 16
#define SZINT 32
#define SZPOINT 32

#define SZFLOAT 32
#define SZDOUBLE 64

/* alignments of types in bits */

#define ALCHAR 8
#define ALSHORT 16
#define ALINT 32
#define ALPOINT 32
#define ALSTACK 32
#define ALSTRUCT 32
#define ALINIT 32
#define	ALFIELD 32			/* force bit fields to have this alignment */

#define ALFLOAT 32
#define ALDOUBLE 32

#define NOLONG			/* map longs to ints */

/* format for labels (if anybody cares) */

#define LABFMT ".L%d"

#ifdef REGSET
/* type size in number of regs */
/* all types take 1 regs: doubles/floats 1 MAU reg, others 1 CPU reg*/

#define szty(t) (1)

/* number of scratch registers:
** 3 are integer/pointer
** 1 is MAU
*/

#define NRGS 4

/* total registers
** 9 are integer/pointer (3 scratch, 6 user register)
** 4 are MAU		 (1 scratch, 2 user register, 1 reserved)
*/

#define TOTREGS	13

/* Define symbol so we can allocate odd register pairs; in fact,
** no pairs are allocated
*/

#define ODDPAIR
#else	/* ndef REGSET */
/* type size in number of regs/temps */

#define szty(t) (((t) == TDOUBLE) ? 2 : 1)

/* number of scratch registers - fp r0->r2 (watch out for pairs!) */

#define NRGS 3
/* total number of registers (scratch and variables)	*/

#define TOTREGS	9
/* Define symbol so we can allocate odd register pairs */
/* For function call floating point emulation, it is assumed that doubles
/* will only be in R0,R1,  so ODDPAIR is undefined to ensure this
*/

#undef ODDPAIR
#endif	/* def REGSET */
/* params addressed with positive offsets from ap, args with negative from sp */

#undef BACKPARAM
#define BACKARGS

/* temps and autos addressed with positive offsets from fp */

#undef BACKTEMP
#undef BACKAUTO

/* bytes run left to right */

#undef RTOLBYTES

/* function calls and arguments */
/* don't nest function calls (where do the args go?) */

/* 	#define NONEST			  */

/* evaluate args left to right (3b style) */

#define LTORARGS

/* chars are unsigned */

#undef CHSIGN

/* structures returned in temporary location -- define magic string */

#define TMPSRET	"	movaw	T,%r2\n"

/* optimizer throws away unless %r2 */

#define AUXREG 2


/* comment indicator */

#define COMMENTSTR "#"

/* string containing ident assembler directive for #ident feature */

#define IDENTSTR	".ident"

/* asm markers */

#define	ASM_COMMENT	"#ASM"
#define	ASM_END		"#ASMEND"

/* reflect my high costing in default load/store cost */

#define CLOAD(x) 55
#define CSTORE(x) 55
#define CCTEST(x)  55

#define MYLOCCTR

#define EXIT(x) myexit(x)

/* Register number for debugging */

#ifdef REGSET
extern int regout[]; 	/* only used for the OUTREGNO(p) macro */
#define	OUTREGNO(p) ( regout[p->offset])
#else
#define	OUTREGNO(p) (p->offset)
#endif

/* Turn on debug information */

#define	SDB

/* Enable assembly language comments */

#define	ASSYCOMMENT

/* user-level fix-up at symbol definition time */

#define FIXDEF(p)	fixdef(p)

/* support for structure debug info */

#define FIXSTRUCT(p,q)	strend(p)

#ifdef M32B
#	define IMPSWREG
#	define IMPREGAL  
#endif

/* Commutable operators are symetric anyway */
#define NOCOMMUTE 1

/* To turn on proper IEEE floating point standard comparison of non-trapping NaN's.
/* Two floating point comparisons:  CMPE for exception raising on all NaN's.
/* CMP for no exception raising for non-trapping NaN's, used for fp == and !=
*/
#define IEEE

/* expand string space in sty */

#define	NSTRING	14000

#ifdef	M32B
#define	BCSZ	125		/* expand break/continue label table */
#endif

/* bypass initialization of INTs through templates */

#define	MYINIT	sincode

/* extend DIMTAB and SYMTAB size for kernel and friends */

#define	DIMTABSZ	1700
#define	SYMTSZ		1300

/* floating point routine definitions for cross environment */

#ifdef uts
#define	FP_FLOAT long			/* prevent unfortunate conversions */
#define FP_CMPD(d1,d2)	flcmp(d1,d2)
#define FP_DTOFP(d)	dtofp(d)
#define	FP_DTOF(d)	dtof(d)
#define FP_DTOL(d)	dtol(d)
#define FP_LTOD(d)	ltod(d)
#define FP_ULTOD(d)	ultod(d)
#define FP_PLUS(d1,d2)	fladd(d1,d2)
#define FP_MINUS(d1,d2) fladd(d1,-d2)
#define FP_TIMES(d1,d2) flmul(d1,d2)
#define FP_DIVIDE(d1,d2) fldiv(d1,d2)
#define	FP_ISZERO(d)	fp_iszero(d)

extern double ltod(), ultod(), fladd(), flmul(), fldiv(), dtofp();
extern FP_FLOAT dtof();
extern long dtol();
extern int fp_iszero();
#endif /* uts */

/* Replace atof() for Amdahl and 3B20, but not for in-line MAU version */

#if (defined(uts) || defined(u3b)) && !defined(REGSET)
#define FP_ATOF(s)	atof2(s)
extern double atof2();
#endif
