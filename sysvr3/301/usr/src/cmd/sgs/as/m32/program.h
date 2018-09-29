/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:m32/program.h	1.4"
/*
 */

/* format of instruction table, mnemonics[] */
struct mnemonic {
	char		*name;
	unsigned short	tag;
	unsigned short	nbits;
	long		opcode;
	unsigned long	*opndtypes;
	unsigned short	index;
	short		next;
};

/* operand semantic information */
typedef struct {
	/*	variables that may be needed in future fp releases
	 * short fptype;	1=single fp ; 2=double fp constant
	 * double fpexpval;	holds value of floating point expression
	 */

	unsigned char  newtype;
	unsigned char  type;
	unsigned char  reg;
	unsigned char  exptype;
	symbol         *symptr;
	long	expval;		/* holds value of integer expression, coded
				 float or first word of a double constant */
	char           *fasciip;	/* holds second word of a double constant */
	/*
	 * The expression specifier describes the size of the expression
	 * and is kept here.
	 */
	long	expspec;
} OPERAND;

#define	PSEUDO	0x25
#define	IS25	0x40
#define REGMD		0x1	/* register mode */
#define REGDFMD		0x2	/* register defered mode */
#define IMMD		0x3	/* immediate mode */
#define ABSMD		0x4	/* absolute address mode */
#define ABSDFMD		0x5	/* absolute address deferred mode */
#define EXADMD		0x6	/* external address mode */
#define EXADDFMD	0x7	/* external address deferred mode */
#define DSPMD		0x8	/* displacement mode */
#define DSPDFMD		0x9	/* displacement deferred mode */
#define FREGMD		0xa	/* mau register mode */
#define SREGMD		0xb	/* single register mode */
#define DREGMD		0xc	/* double register mode */
#define XREGMD		0xd	/* double extended register mode */

#if	FLOAT
#define	FPIMMD		0x13	/* floating point immediate mode */
#define	DECIMMD		0x14	/* floating point immediate mode */
#define	CDIMMD		0xCF	/* floating point double immediate mode */
#endif

#define TEXADMD (1<<EXADMD)
#define TREGMD	(1<<REGMD)
#define TIMMD	(1<<IMMD)
#define MEM	((1<<ABSDFMD)|(1<<ABSMD)|(1<<EXADDFMD)|(1<<EXADMD)|(1<<DSPDFMD)|(1<<DSPMD)|(1<<REGDFMD))
#define DEST	(MEM|TREGMD)
#define GEN	(DEST|TIMMD)
#define FPDEC	(MEM|(1<<FPIMMD)|(1<<DECIMMD))
#define FPINT	(MEM|TIMMD)
#define FPDEST	((1<<FREGMD)|(1<<SREGMD)|(1<<DREGMD)|(1<<XREGMD)|MEM)
#define FPGEN	(FPDEST|(1<<FPIMMD)|(1<<DECIMMD))
#define FPGEN2	(DEST|(1<<FPIMMD))
