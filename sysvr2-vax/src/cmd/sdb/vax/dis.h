	/*	@(#)dis.h	1.2	*/

/*
 *	This is the header file for the disassemblers (n3b and VAX).
 *	The information contained in the first part of the file
 *	is common to each version, while the last part is dependent
 *	on the particular machine architecture being used.
 */

#define		NCPS	8	/* number of chars per symbol	*/
#define		NHEX	80	/* max # chars in object per line	*/
#define		NLINE	80	/* max # chars in mnemonic per line	*/
#define		FAIL	0
#define		TRUE	1
#define		FALSE	0

#ifdef u3b
#define		LEAD	1
#define		NOLEAD	0
#define		TERM	0	/* used in _tbls.c to indicate		*/
				/* that the 'indirect' field of the	*/
				/* 'instable' terminates - no pointer.	*/
				/* Is also checked in 'dis_text()' in	*/
				/* _bits.c.				*/

#define		STRNGEQ 0	/* used in string compare operation	*/
				/* in 'disassembly' routine		*/
/*
 *	This is the structure that will be used for storing all the
 *	op code information.  The structure values themselves are
 *	in '_tbls.c'.
 */

struct	instable {
	char		name[NCPS];
	struct instable *indirect;	/* for decode op codes */
	unsigned	adr_mode;
};
/*	NOTE:	the following information in this file must be changed
 *		between the different versions of the n3b disassembler.
 *
 *	These are the instruction formats as they appear in
 *	'n3bdis_tbls.c'.  Here they are given numerical values
 *	for use in the actual disassembly of an object file.
 */

#define UNKNOWN		0
#define DYA		1	/* dyadic				*/
#define LNGIO3R		2	/* long I/O with 3 regs			*/
#define LNGIO4R		3	/* long I/O with 4 regs			*/
#define LNGIO2RL	4	/* long I/O, 2 regs, one in last 4 bits	*/
#define LNGIO2S		5	/* long I/O, 2 regs, one special(rmchr)	*/
#define LNGIO2R		6	/* long I/O with 2 regs			*/
#define LNGIOS		7	/* special long I/O--append i,r or blank*/
#define LNGIO4S		8	/* same as above but 4 regs		*/
#define LNG2R		9	/* long inst with 2 regs		*/
#define LNG3R		10	/* long inst with 3 regs		*/
#define SHTR		11	/* short inst, 1 reg, no decode		*/
#define TRI		12	/* triadic				*/
#define JMB		13	/* jump on bit clear/set		*/
#define GEND		14	/* single general mode operand, decode field */
#define BR		15	/* branch				*/
#define RR		16	/* dyadic opt, reg to reg		*/
#define NR		17	/* dyadic opt, nib to reg		*/
#define RN		18	/* dyadic opt, reg to nib		*/
#define MON1		19	/* monadic, type 1			*/
#define MON2		20	/* monadic, type 2			*/
#define LCI4		21	/* loop control, 4 operands		*/
#define LCI3		22	/* loop control, 3 operands		*/
#define I4		23	/* 4 bit immediate operand		*/
#define I8		24	/* 8 bit immediate operand		*/
#define MARC		25	/* move address/read & clear		*/
#define BITF1		26	/* bit field				*/
#define GO_ON		27	/* go on -- no operands			*/
#define D2G		28	/* 2 gen addr mode ops w/ decode	*/
#define CALL		29	/* call					*/
#define RSR		30	/* read special register		*/
#define WSR		31	/* write special register		*/
#define	SDYA		32	/* special dyadic case			*/
#define	BITF2		33	/* second bit field case		*/
#define	TRI2		34	/* second type of triadic		*/
#define	CALL1		35	/* second type of call			*/
#define	CALL2		36	/* third type of call			*/
#define	BP		37	/* for breakpointable hald		*/
#define	GENDW		38	/* single gen mode operand, decode	*/
				/* field, word sized operand assumed	*/
#define	CALE		39	/* call to emulation instruction	*/
#define	RETE		40	/* return to emulation instruction	*/
#define	RDSER		41	/* read store error register instruction */
#define TUSRET		42	/* special return for TUS trap		*/
#define CALL3		43	/* tv call				*/
#define FPD2G		44	/* floating point dyadic instructions	*/
#define FPTRI		45	/* floating point triadic instructions	*/
/*
 *	The following four items indicate in the n3bds_tbls.c  and
 *	n3bds_bits.c files what bits from the byte following the
 *	op code are necessary for decoding.
 */

#define	H4	1	/* high four bits   xxxx----	*/
#define	B54	3	/* bits 5 & 4       --xx----	*/
#define	B654	4	/* bits 6, 5 & 4    -xxx----	*/
#define L4	5	/* low 4 bits       ----xxxx	*/


/*
 *	A 4 bit descriptor to indicate the addressing modes may indicate
 *	a register, a nibble or one of the general addressing modes.
 *	These values are used to initialize tables in 'n3bds_tbls.c' and in
 *	case statements in 'n3bds_bits.c'.
 */

#define	REG	0
#define NIB	1
#define	GEN	2


/*
 *	An operand may be a byte, halfword, fullword or double in length.
 *	Again, these values are used in 'n3bds_tbls.c' and 'n3bds_bits.c'.
 */

#define	BYTE	0
#define	HALF	1
#define	WORD	2
#define DOUBLE  3	/* used for (64 bit) floating point immediate operands */
#else

#define VARNO 36	/* Max number of operands saved in VAX instruction */
#define PCNO 15		/* Number of PC register			   */
/*
 * Argument access types
 */
#define ACCA	(8<<3)	/* address only */
#define ACCR	(1<<3)	/* read */
#define ACCW	(2<<3)	/* write */
#define ACCM	(3<<3)	/* modify */
#define ACCB	(4<<3)	/* branch displacement */
#define ACCI	(5<<3)	/* XFC code */

/*
 * Argument data types
 */
#define TYPB	0	/* byte */
#define TYPW	1	/* word */
#define TYPL	2	/* long */
#define TYPQ	3	/* quad */
#define TYPF	4	/* floating */
#define TYPD	5	/* double floating */


struct optab {
	char *iname;
	char val;
	char nargs;
	unsigned char argtype[6];
};

extern char * 	regname[];
extern char *	fltimm[];

#define LOBYTE	0xff

#endif
