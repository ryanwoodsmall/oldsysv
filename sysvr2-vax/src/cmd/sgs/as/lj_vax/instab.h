#define PSEUDO	1 /* pseudo operation */
#define SPSYM	2 /* special symbol */
#define EXTIN1	3 /* sdi optimizable instr type 1 : 2/5  bytes */
#define EXTIN2	4 /* sdi optimizable instr type 2 : 2/3  bytes */
#define EXTIN3	5 /* sdi optimizable instr type 3 : 7/10 bytes */
#define EXTIN4	6 /* sdi optimizable instr type 4 : 6/9  bytes */
#define INSTX  	7 /* NO operands */
#define INSTB	8 /* all byte displacement operands */
#define INSTW	9 /* all word displacement operands */
#define INSTL	10 /* all long displacement operands */
#define INSTQ	11 /* all quadword displacement operands */
#define INSTF	12 /* all floating operands */
#define INSTD	13 /* all double operands */
#define INSTJB	14 /* BRANCH+BYTE displacement */
#define INSTJW	15 /* BRANCH+WORD displacement */

#define	ILU00	50 /* lookup context in table element 00 */
#define ILU01	51 /* lookup context in table element 01 */
#define ILU02	52 /* lookup context in table element 02 */
#define ILU03	53 /* lookup context in table element 03 */
#define ILU04	54 /* lookup context in table element 04 */
#define ILU05	55 /* lookup context in table element 05 */
#define ILU06	56 /* lookup context in table element 06 */
#define ILU07	57 /* lookup context in table element 07 */
#define ILU08	58 /* lookup context in table element 08 */
#define ILU09	59 /* lookup context in table element 09 */
#define	ILU10	60 /* lookup context in table element 10 */
#define ILU11	61 /* lookup context in table element 11 */
#define ILU12	62 /* lookup context in table element 12 */
#define ILU13	63 /* lookup context in table element 13 */
#define ILU14	64 /* lookup context in table element 14 */
#define ILU15	65 /* lookup context in table element 15 */
#define ILU16	66 /* lookup context in table element 16 */
#define ILU17	67 /* lookup context in table element 17 */
#define ILU18	68 /* lookup context in table element 18 */
#define ILU19	69 /* lookup context in table element 19 */
#define	ILU20	70 /* lookup context in table element 20 */
#define ILU21	71 /* lookup context in table element 21 */
#define ILU22	72 /* lookup context in table element 22 */
#define ILU23	73 /* lookup context in table element 23 */
#define ILU24	74 /* lookup context in table element 24 */
#define ILU25	75 /* lookup context in table element 25 */
#define ILU26	76 /* lookup context in table element 26 */
#define ILU27	77 /* lookup context in table element 27 */
#define ILU28	78 /* lookup context in table element 28 */
#define ILU29	79 /* lookup context in table element 29 */
#define	ILU30	80 /* lookup context in table element 30 */
#define ILU31	81 /* lookup context in table element 31 */
#define ILU32	82 /* lookup context in table element 32 */
#define ILU33	83 /* lookup context in table element 33 */
#define ILU34	84 /* lookup context in table element 34 */
#define ILU35	85 /* lookup context in table element 35 */
#define ILU36	86 /* lookup context in table element 36 */
#define ILU37	87 /* lookup context in table element 37 */
#define ILU38	88 /* lookup context in table element 38 */
#define ILU39	89 /* lookup context in table element 39 */
#define	ILU40	90 /* lookup context in table element 40 */
#define ILU41	91 /* lookup context in table element 41 */
#define ILU42	92 /* lookup context in table element 42 */
#define ILU43	93 /* lookup context in table element 43 */
#define ILU44	94 /* lookup context in table element 44 */
#define ILU45	95 /* lookup context in table element 45 */
#define ILU46	96 /* lookup context in table element 46 */


/*
 * operand syntax types
 */
#define AREG	1	/* %r */
#define ABASE	2	/* (%r) */
#define ADECR	3	/* -(%r) */
#define AINCR	4	/* (%r)+ */
#define ADISP	5	/* expr(%r) */
#define AEXP	6	/* expr */
#define AIMM	7	/* $ expr */
#define ASTAR	8	/* * */
#define AINDX	16	/* [%r] */

/*
 * expression data types
 */
#define TYPB	0	/* byte */
#define TYPW	2	/* word */
#define TYPL	4	/* long */
#define TYPQ	8	/* quad */
#define TYPF	16	/* floating */
#define TYPD	32	/* double floating */

#define TINT	(TYPB|TYPW|TYPL|TYPQ)
#define TFLT	(TYPF|TYPD)

#define TSYM	64	/* symbol + (-)offset which is in expval */

#define TBRNCH	128

#define TDIFF	256	/* symbol - symbol expression, the ptr to the left */
			/* symbol is stored in expval.lngval and the symptr
			   contains the right symbol */

typedef struct {
	short	exptype;
	symbol	*symptr;
	union {
		long	lngval;
		float	fltval;	
		double	dblval;
	      } expval;
} rexpr;

typedef struct {
	BYTE admode;
	BYTE adreg1;
	BYTE adreg2;
	rexpr adexpr;
} addrmode;
