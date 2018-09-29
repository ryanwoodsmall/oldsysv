/*	@(#)w2.c	1.3	*/
#include	"optim.h"
#include	"extdec.h"

char	*w2opts[] =
{
/* 0 */	"added sob",
/* 1 */	"2op -> 3op",
/* 2 */	"added movab",
/* 3 */	"added pushab",
/* 4 */	"deleted mov after 3op",
/* 5 */	"2op+push -> 3op,-(sp)",
/* 6 */	"mova+push -> pusha",
/* 7 */	"deleted movX after cvtYX",
/* 8 */	"deleted movX before caseX",
/* 9 */	"deleted redundant tstX",
/*10 */	"added branch on bit",
/*11 */	"",
/*12 */	"reduced adjacent ashl",
/*13 */	"added index mode",
/*14 */	"deleted neg. short literals",
/*15 */	"deleted redundant temp load",
/*16 */	"deleted movX after movaY",
/*17 */	"created auto-inc",
/*18 */	"created auto-dec",
/*19 */	"created disp. deffered",
/*20 */	"mneg+mov -> mneg",
	0
};
int	w2stats[sizeof w2opts / sizeof w2opts[0]];
int	w2size = sizeof w2opts / sizeof w2opts[0];

struct	X2to3	/* how to transform 2op to 3op and back again */
{
	int	xop;
	char	*xinstr2;
	char	*xinstr3;
};
static	struct	X2to3	X[] =
{
	{	ADD,	"addl2",	"addl3"},
	{	SUB,	"subl2",	"subl3"},
	{	MUL,	"mull2",	"mull3"},
	{	DIV,	"divl2",	"divl3"},
	{	BIS,	"bisl2",	"bisl3"},
	{	BIC,	"bicl2",	"bicl3"},
	{	XOR,	"xorl2",	"xorl3"},
	{	0,	0}
};

boolean	w2optim(p, pf)
register struct	node	*p;
register struct	node	*pf;
{
	register int	n = p->userdata->nargs;
	register int	Tp = argtype(p, 1);
	register int	Tpf = argtype(pf, 1);
	register int	i;			/* random quick integer */
	register int	shcnt;			/* shift count used far below */

	if (loopflag && Tp == TL)
	{
		/*
		**	decl	A0	\	sobgeq	A0,L
		**	jgeq	L	/
		*/
		if (p->op == DECR && (pf->op == JGT || pf->op == JGE))
		{
			wchange();
			chgop(p, pf->op == JGT ? SOBGTR : SOBGEQ,
				pf->op == JGT ? "sobgtr" : "sobgeq");
			p->ops[2] = pf->ops[1];
			w2stats[0]++;
			DELNODE(pf); ndisc++;
			return (wwchanged = true);
		}
	}
	/*
	**	movX	(R0),A0		\	movX	(R0)+,A0
	**	addl2	$C,R0		/
	*/
	if (p->op == MOV && pf->op == ADD && pf->userdata->nargs == 2 &&
		Tpf == TL && isdead(pf->ops[2], pf) && isconst(pf->ops[1]) &&
		constval == typesize(Tp) && strcmp(p->ops[2], pf->ops[2]) &&
		p->ops[1][0] == '(' &&
		!strncmp(&p->ops[1][1], pf->ops[2], i = strlen(pf->ops[2])) &&
		p->ops[1][i += 2] == ')' && p->ops[1][++i] == '\0')
	{
		wchange();
		pf->ops[1] = getspace(++i);
		strcpy(pf->ops[1], p->ops[1]);
		strcat(pf->ops[1], "+");
		p->ops[1] = pf->ops[1];
		++w2stats[17];
		DELNODE(pf); ++ndisc;
		return (wwchanged = true);
	}
	/*
	**	subl2	$C,R0		\	movX	-(R0),A0
	**	movX	(R0),A0		/
	*/
	if (p->op == SUB && pf->op == MOV && p->userdata->nargs == 2 &&
		Tp == TL && isdead(p->ops[2], pf) && isconst(p->ops[1]) &&
		constval == typesize(Tpf) && pf->ops[1][0] == '(' &&
		!strncmp(&pf->ops[1][1], p->ops[2], i = strlen(p->ops[2])) &&
		pf->ops[1][i += 2] == ')' && pf->ops[1][++i] == '\0')
	{
		wchange();
		p->ops[1] = getspace(++i);
		strcpy(p->ops[1], "-");
		strcat(p->ops[1], pf->ops[1]);
		pf->ops[1] = p->ops[1];
		++w2stats[18];
		DELNODE(p); ++ndisc;
		return (wwchanged = true);
	}
	/*
	**	addl2	$S0,R0	\	movab	S0[R0],A0
	**	movl	R0,A0	/
	*/
	if (p->op == ADD && n == 2 && pf->op == MOV && Tp == TL && Tpf == TL &&
		p->ops[1][0] == '$' && isstatic(p->ops[1]) &&
		isdead(p->ops[2], pf) && !strcmp(p->ops[2], pf->ops[1]))
	{
		wchange();
		chgop(pf, MOVA, "movab");
		pf->ops[1] = getspace(strlen(p->ops[1]) + strlen(p->ops[2])+2);
		sprintf(pf->ops[1], "%s[%s]",
			&p->ops[1][(p->ops[1][0] == '$')?1:0],
			p->ops[2]);
		w2stats[2]++;
		DELNODE(p); ndisc++;
		return (wwchanged = true);
	}
	/*
	**	mnegX	A0,R	\	mnegX	A0,A1
	**	movX	R,A1	/
	*/
	if (p->op == MNEG && pf->op == MOV && Tp == Tpf &&
		isdead(p->ops[2], pf) && !strcmp(p->ops[2], pf->ops[1]))
	{
		wchange();
		p->ops[2] = pf->ops[2];
		DELNODE(pf); ++ndisc;
		++w2stats[20];
		return (wwchanged = true);
	}
	/*
	**	movX	R,A	\	movX	R,A
	**	movX	A,R	/
	*/
	if (pf->op == MOV && p->op == MOV && Tp == Tpf && isdead(p->ops[1], p)
		&& !strcmp(p->ops[2], pf->ops[1]) && noside(p->ops[2]) &&
		!strcmp(p->ops[1], pf->ops[2]))
	{
		wchange();
		++w2stats[15];
		DELNODE(pf); ndisc++;
		return (wwchanged = true);
	}
	/*
	**	Aopl2	A0,R0	\	Aopl3	A0,R0,A1
	**	movl	R0,A1	/
	*/
	if (pf->op == MOV && Tp == TL && Tpf == TL && n == 2 &&
		isdead(p->ops[2], pf) && !strcmp(p->ops[2], pf->ops[1]))
	{
		register int	i;

		for (i = 0; X[i].xinstr2; i++)	/* linear search thru X */
			if (p->op == X[i].xop)
				break;
		if (X[i].xinstr2)
		{
			wchange();
			chgop(p, p->op, X[i].xinstr3);
			p->ops[3] = pf->ops[2];
			w2stats[1]++;	/* 2op -> 3op, redundant move */
			DELNODE(pf); ndisc++;
			return (wwchanged = true);
		}
	}
	/*
	**	Aopl2	A0,R0	\	Aopl3	A0,R0,-(sp)
	**	pushl	R0	/
	*/
	if (pf->op == PUSH && Tp == TL && Tpf == TL && n == 2 &&
		isdead(p->ops[2], pf) && !strcmp(p->ops[2], pf->ops[1]))
	{
		register int	i;

		for (i = 0; X[i].xinstr2; i++)
			if (p->op == X[i].xop)
				break;
		if (X[i].xinstr2)
		{
			wchange();
			chgop(p, p->op, X[i].xinstr3);
			p->ops[3] = getspace(sizeof ("-(sp)") + 1);
			strcpy(p->ops[3], "-(sp)");
			w2stats[1]++;	/* 2op+push -> 3op,-(sp) */
			DELNODE(pf); ndisc++;
			return (wwchanged = true);
		}
	}
	/*
	** Use the type of the destination here (argtype(p,n)) to catch ashl,
	**	bic, bis.
	*/
	/*
	**	Aopl3	A0,A1,R0	\	Aopl3	A0,A1,-(sp)
	**	pushl	R0		/
	*/
	if (isaop(p) && pf->op == PUSH && n == 3 && argtype(p, 3) == TL &&
		Tpf == TL && isdead(p->ops[3], pf) &&
		!strcmp(p->ops[3], pf->ops[1]))
	{
		wchange();
		p->ops[3] = getspace(sizeof ("-(sp)") + 1);
		strcpy(p->ops[3], "-(sp)");
		w2stats[1]++;	/* 3op+push -> 3op,-(sp) */
		DELNODE(pf); ndisc++;
		return (wwchanged = true);
	}
	/*
	**	moval	A0,R0	\	pushal	A0
	**	push	R0	/
	*/
	if (p->op == MOVA && pf->op == PUSH && Tp >= TB && Tp <= TD &&
		isdead(p->ops[2], pf) && !strcmp(p->ops[2], pf->ops[1]))
	{
		wchange();
		switch (Tp)
		{
		case TB:	chgop(pf, PUSHA, "pushab"); break;
		case TW:	chgop(pf, PUSHA, "pushaw"); break;
		case TL:	chgop(pf, PUSHA, "pushal"); break;
		case TQ:	chgop(pf, PUSHA, "pushaq"); break;
		case TF:	chgop(pf, PUSHA, "pushaf"); break;
		case TD:	chgop(pf, PUSHA, "pushad"); break;
		}
		pf->ops[1] = p->ops[1];
		w2stats[6]++;	/* mova+push -> pusha */
		DELNODE(p); ndisc++;
		return (wwchanged = true);
	}
	/*
	**	addl3	$C0,R0,R1	\	pushab	C0(R0)
	**	pushl	R1		/
	*/
	if (p->op == ADD && Tp == TL && pf->op == PUSH && Tpf == TL && n == 3 &&
		!strcmp(p->ops[3], pf->ops[1]) && isdead(pf->ops[1], pf) &&
		*p->ops[1] == '$' && isreg(p->ops[2]))
	{
		wchange();
		chgop(pf, PUSHA, "pushab");
		pf->ops[1] = getspace(strlen(p->ops[1]) + strlen(p->ops[2]) +2);
		sprintf(pf->ops[1], "%s(%s)", &p->ops[1][1], p->ops[2]);
		w2stats[3]++;
		DELNODE(p); ndisc++;
		return (wwchanged = true);
	}
	/*
	**	AopX3	A0,A1,R0	\	AopX3	A0,A1,A2
	**	movX	R0,A2		/
	*/
	if (n == 3 && pf->op == MOV && Tp == Tpf &&
		!strcmp(p->ops[3], pf->ops[1]) && isdead(p->ops[3], pf))
	{
		switch (p->op)
		{
		case ADD:
		case SUB:
		case MUL:
		case DIV:
		case BIS:
		case BIC:
		case XOR:
		case ASH:
			wchange();
			p->ops[3] = pf->ops[2];
			w2stats[4]++;
			DELNODE(pf); ndisc++;
			return (wwchanged = true);
		}
	}
	/*
	**	cvtXY	A0,R0	\	cvtXY	A0,A1
	**	movY	R0,A1	/
	*/
	if (p->op == CVT && pf->op == MOV && Tpf == argtype(p, 2) &&
		!strcmp(p->ops[2], pf->ops[1]) && isdead(pf->ops[1], pf))
	{
		wchange();
		p->ops[2] = pf->ops[2];
		w2stats[7]++;
		DELNODE(pf); ndisc++;
		return (wwchanged = true);
	}
	/*
	**	movaX	A0,R	\	movaX	A0,A1
	**	movl	R,A1	/
	*/
	if (p->op == MOVA && pf->op == MOV && Tpf == TL &&
		isdead(pf->ops[1], pf) && !strcmp(pf->ops[1], p->ops[2]))
	{
		wchange();
		p->ops[2] = pf->ops[2];
		++w2stats[16];
		DELNODE(pf); ndisc++;
		return (wwchanged = true);
	}
	/*
	**	movX	A0,R0		\	caseX	A0,A1,A2
	**	caseX	R0,A1,A2	/
	*/
	if (p->op == MOV && pf->op == CASE && Tp == Tpf &&
		!strcmp(p->ops[2], pf->ops[1]) && isdead(p->ops[2], pf))
	{
		wchange();
		pf->ops[1] = p->ops[1];
		w2stats[8]++;
		DELNODE(p); ndisc++;
		return (wwchanged = true);
	}
	/*
	**	redundant tst of previous instruction's destination
	*/
	if (pf->op == TEST && !strcmp(destarg(p), pf->ops[1]) &&
		noside(pf->ops[1]) && Tp == Tpf)
	{
		wchange();
		w2stats[9]++;
		DELNODE(pf); ndisc++;
		return (wwchanged = true);
	}
	/*
	**	bitl	$C0,A0	\	jbc	$C1,A0,L0	# C1=log2(C0)
	**	jeql	L0	/
	**
	**	bitl	$C0,A0	\	jbs	$C1,A0,L0	# C1=log2(C0)
	**	jneq	L0	/
	**
	**	bitl	$0,A0	\	jlbc	A0,L0
	**	jeql	L0	/
	**
	**	bitl	$0,A0	\	jlbs	A0,L0
	**	jneq	L0	/
	*/
	if (p->op == BIT && (pf->op == JEQ || pf->op == JNE) &&
		isconst(p->ops[1]))
	{
		register int	i;

		for (i = 0; i < 32; i++)
		{
			if ((1 << i) == constval)
				break;
		}
		if (i == 0)
		{
			if (argtype(p, 2) == TL || !strchr(p->ops[2], '['))
			{
				wchange();
				chgop(pf, (pf->op == JEQ) ? JLBC : JLBS,
					(pf->op == JEQ) ? "jlbc" : "jlbs");
				pf->ops[2] = pf->ops[1];
				pf->ops[1] = p->ops[2];
				w2stats[10]++;
				DELNODE(p); ndisc++;
				return (wwchanged = true);
			}
		}
		if ((bitbflag || isreg(p->ops[2])) && i != 32)
		{
			if (argtype(p, 2) == TB || !strchr(p->ops[2], '['))
			{
				wchange();
				chgop(pf, (pf->op == JEQ) ? JBC : JBS,
					(pf->op == JEQ) ? "jbc" : "jbs");
				pf->ops[3] = pf->ops[1];
				pf->ops[2] = p->ops[2];
				pf->ops[1] = getspace(4);
				sprintf(pf->ops[1], "$%d", i);
				w2stats[10]++;
				DELNODE(p); ndisc++;
				return (wwchanged = true);
			}
		}
	}
	/*
	**	ashl	$C0,A0,R	\	ashl	$C0+C1,A0,R
	**	ashl	$C1,R,R		/
	*/
	if (p->op == ASH && pf->op == ASH && Tp == Tpf && isreg(p->ops[3]) &&
		!strcmp(p->ops[3], pf->ops[2]) &&
		!strcmp(p->ops[3], pf->ops[3]))
	{
#ifdef	sign	/* sanity clause */
#define	sign shouldn't be defined here!!
#endif
#define	sign(x) (((x)<0)?-1:(((x)==0)?0:1))
		register int	c01;

		if (isconst(p->ops[1]) &&
			(c01 = constval, isconst(pf->ops[1])) &&
			(sign(c01) == sign(constval)) &&
			(c01 += constval, c01 >= -31 && c01 <= 31))
		{
			wchange();
			p->ops[1] = getspace(5);	/* worst: "$-31\0" */
			sprintf(p->ops[1], "$%d", c01);
			w2stats[12]++;
			DELNODE(pf); ndisc++;
			return (wwchanged = true);
		}
#undef	sign
	}
	/*
	**	ashl	$1,R0,R1	\	movaw	S[R0],A0
	**	movab	S[R1],A0	/
	*/
	if (p->op == ASH && isconst(p->ops[1]) &&
		(pf->op == MOVA || pf->op == PUSHA) &&
		Tpf == TB && ((shcnt = constval) >= 0) && shcnt <= 3 &&
		isreg(p->ops[2]) && isdead(p->ops[3], pf) &&
		isindexd(pf->ops[1], p->ops[3]))
	{
		static	char	*table[4][2] =
		{
			{ "movab", "pushab" }, { "movaw", "pushaw" },
			{ "moval", "pushal" }, { "movaq", "pushaq" },
		};
		register int	push;
		register int	i;

		wchange();
		push = (pf->op == PUSHA) || (strcmp("-(sp)", pf->ops[2]) == 0);
		i = (char *) strchr(pf->ops[1], '[') - pf->ops[1];
		sprintf(p->ops[1] = getspace(i + strlen(p->ops[2]) + 2),
			"%.*s[%s]", i, pf->ops[1], p->ops[2]);
		p->ops[2] = pf->ops[2];
		p->ops[3] = NULL;
		chgop(p, push? PUSHA: MOVA, table[shcnt][push]);
		w2stats[13]++;
		DELNODE(pf); ndisc++;
		return (wwchanged = true);
	}
	/*
	**	cmpl	R0,$-1	\	incl	R0
	**	CBR		/	CBR
	**
	**	cmpl	R0,$-SL	\	addl2	$SL,R0
	**	CBR		/	CBR
	*/
	if (p->op == CMP && isdead(p->ops[1], p) && isnshort(p->ops[2]))
	{
		wchange();
		if (constval == -1)
		{
			chgop(p, INCR, "incl");
			p->ops[2] = NULL;
		}
		else
		{
			chgop(p, ADD, "addl2");
			p->ops[2] = p->ops[1];
			p->ops[1] = getspace(4);	/* worst: "$63\0" */
			sprintf(p->ops[1], "$%d", -constval);
		}
		++w2opts[14];
		return (wwchanged = true);
	}
	/*LATER:
	**	cmpl	$-1,R0	\	incl	R0
	**	CBR		/	-CBR
	**
	**	cmpl	$-SL,R0	\	addl2	$SL,R0
	**	CBR		/	-CBR
	*/
	if (p->op == CMP && isrev(pf) && isdead(p->ops[2], p) && isnshort(p->ops[1]))
	{
		wchange();
		if (constval == -1)
		{
			chgop(p, INCR, "incl");
			p->ops[1] = p->ops[2];
			p->ops[2] = NULL;
		}
		else
		{
			chgop(p, ADD, "addl2");
			/* ops[1] contains "$-63\0"; sprintf is "$63\0" */
			sprintf(p->ops[1], "$%d", -constval);
		}
		revbr(pf);
		++w2opts[14];
		return (wwchanged = true);
	}
	return (false);
}

