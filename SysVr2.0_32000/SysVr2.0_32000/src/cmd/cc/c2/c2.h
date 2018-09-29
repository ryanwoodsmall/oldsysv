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
/*	@(#)c2.h	1.4	*/
/* Point of last convergence with vax135!jfr: @(#)c2.h 1.18 80/03/14 10:27:01 */
/*
 * Header for object code improver
 */
#define DOTFILE	123

#define	JBR	1
#define	CBR	2
#define	JMP	3
#define	LABEL	4
#define	DLABEL	5
#define	EROU	7
#define	JSW	9
#define	MOV	10
#define	CLR	11
#define	INC	12
#define	DEC	13
#define	TST	14
#define	PUSH	15
#define CVT 16
#define	CMP	17
#define	ADD	18
#define	SUB	19
#define	BIT	20
#define	BIC	21
#define	BIS	22
#define	XOR	23
#define	COM	24
#define	NEG	25
#define	MUL	26
#define	DIV	27
#define	ASH	28
#define EXTV	29
#define EXTZV	30
#define INSV	31
#define	CALLS	32
#define RET	33
#define	CASE	34
#define	SOB	35
#define	TEXT	36
#define	DATA	37
#define	BSS	38
#define	ALIGN	39
#define	END	40
#define MOVZ 41
#define WGEN 42
#define SOBGEQ 43
#define SOBGTR 44
#define AOBLEQ 45
#define AOBLSS 46
#define ACB 47
#define MOVA 48
#define PUSHA 49
#define LGEN 50
#define SET 51
#define MOVC3 52
#define RSB 53
#define JSB 54
#define MFPR 55
#define MTPR 56
#define PROBER 57
#define PROBEW 58
#define	CMPV	59
#define	CMPZV	60
#define	COMM	61
#define	LCOMM	62

#define	JEQ	0
#define	JNE	1
#define	JLE	2
#define	JGE	3
#define	JLT	4
#define	JGT	5
/* rearranged for unsigned branches so that jxxu = jxx + 6 */
#define	JLOS	8
#define	JHIS	9
#define	JLO	10
#define	JHI	11

#define JBC 12
#define JBS 13
#define JLBC 14
#define JLBS 15
#define JBCC 16
#define JBSC 17
#define JBCS 18
#define JBSS 19

#define	BYTE	1
#define	WORD	2
#define LONG	3
#define	FLOAT	4
#define	DOUBLE	5
#define QUAD	6
#define OP2	7
#define OP3	8
#define OPB 9
#define OPX 10

#define T(a,b) (a|((b)<<8))
#define U(a,b) (a|((b)<<4))

extern struct optab {
	char	opstring[7];
	short	opcode;
} optab[];

union	fullop
{
	short	_combop;
	struct
	{
		char	_op;
		char	_subop;
	}
	op_;
};

struct node {
	union	fullop	ops;
	short	refc;
	struct	node	*forw;
	struct	node	*back;
	struct	node	*ref;
	char	*code;
	struct	optab	*pop;
	long	labno;
	short	seq;
};
#define	combop	ops._combop
#define	op	ops.op_._op
#define	subop	ops.op_._subop

extern char	line[512];
extern struct	node	first;
extern char	*curlp;
extern int	nbrbr;
extern int	nsaddr;
extern int	redunm;
extern int	iaftbr;
extern int	njp1;
extern int	nrlab;
extern int	nxjump;
extern int	ncmot;
extern int	nrevbr;
extern int	loopiv;
extern int	nredunj;
extern int	nskip;
extern int	ncomj;
extern int	nsob;
extern int	nrtst;
extern int	nbj;
extern int	nfield;

extern int	nchange;
extern int	isn;
extern int	debug;
extern char	*lasta;
extern char	*lastr;
extern char	*firstr;
extern char	revbr[];
extern char	invbr[];
#define	NREG	12
extern char	*regs[NREG+5]; /* 0-11, 4 for operands, 1 for running off end */
#define	RANDLEN	40	/* max length of an instruction operand */
extern char	conloc[RANDLEN];
extern char	conval[RANDLEN];
extern char	ccloc[RANDLEN];

#define	RT1	12
#define	RT2	13
#define RT3 14
#define RT4 15
#define	LABHS	127

struct { char lbyte; };

extern char *copy();
extern long getnum();
extern struct node *codemove();
extern struct node *insertl();
extern struct node *nonlab();
