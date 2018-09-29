/*	@(#)w1.c	1.3	*/
#include	<stdio.h>
#include	"optim.h"
#include	"extdec.h"

char	*w1opts[] =
{
/* 0 */	"3op -> 2op",
/* 1 */	"add/sub -> inc/dec",
/* 2 */	"arg/auto addr comp -> moval",
/* 3 */	"ext[z]v -> cvt/movz",
/* 4 */	"cmpl X,0 -> tstl X",
/* 5 */	"pushl $0 -> clrl -(sp)",
/* 6 */	"extv -> ashl",
/* 7 */	"cvtlX $C -> movX $C",
/* 8 */	"add $S,R -> movab S[R]",
/* 9 */	"add R,$S -> add $S,R",
/*10 */	"mov[df] $0 -> clr[df]",
	0
};
int	w1stats[sizeof w1opts / sizeof w1opts[0]];
int	w1size = sizeof w1opts / sizeof w1opts[0];

boolean	w1optim(p, pf)
register struct	node	*p;
struct	node	*pf;
{
	register int	Tp;	/* Type of 1st operand */
	register int	shcnt;	/* used in case 4 for extv */
	register int	i;	/* random integer */

	Tp = (p->userdata->nargs > 0) ? argtype(p, 1) : 0;
	switch (p->userdata->nargs)
	{
	case 1:
		/*
		**	pushl	$0	>	clrl	-(sp)
		*/
		if (p->op == PUSH && Tp == TL && isconst(p->ops[1]) &&
			constval == 0)
		{
			wchange();
			chgop(p, CLR, "clrl");
			p->ops[1] = COPY("-(sp)", 6);
			++w1stats[5];
			return (wwchanged = true);
		}
		break;

	case 2:
		/*
		**	cmpX	A,$0	>	tstX	A
		*/
		if (p->op == CMP && (isconst(p->ops[2]) && constval == 0) ||
			(isfconst(p->ops[2]) && fconstval == 0.0))
		{
			wchange();
			switch (Tp)
			{
			case TB:	chgop(p, TEST, "tstb"); break;
			case TW:	chgop(p, TEST, "tstw"); break;
			case TL:	chgop(p, TEST, "tstl"); break;
			case TF:	chgop(p, TEST, "tstf"); break;
			case TD:	chgop(p, TEST, "tstd"); break;
			default:	abort("cmp->tst type"); /*NOTREACHED*/
			}
			p->ops[2] = NULL;
			++w1stats[4];
			return (wwchanged = true);
		}
		if (Tp != TL)
			break;
		/*
		**	cvtlX	$C,A0	>	movX	$C,A0
		*/
		if (p->op == CVT && isconst(p->ops[1]) && ((i = argtype(p, 2)) == TB || i == TW))
		{
			wchange();
			switch (i)
			{
			case TB:
				constval &= 0xFF;
				chgop(p, MOV, "movb");
				break;

			case TW:
				constval &= 0xFFFF;
				chgop(p, MOV, "movw");
				break;
			}
			p->ops[1] = getspace(7);	/* worst: "0xXXXX\0" */
			sprintf(p->ops[1], "$0x%X", constval);
			++w1stats[7];
			return (wwchanged = true);
		}
		/*
		**	addl2	$1,A	>	incl	A
		**	addl2	$-1,A	>	decl	A
		**	subl2	$1,A	>	decl	A
		**	subl2	$-1,A	>	incl	A
		*/
		if ((p->op == ADD || p->op == SUB) && isconst(p->ops[1]))
		{
			if ((p->op == ADD && constval == -1) ||
				(p->op == SUB && constval == 1))
			{
				wchange();
				chgop(p, DECR, "decl");
				p->ops[1] = p->ops[2];
				p->ops[2] = NULL;
				++w1stats[1];
				return (wwchanged = true);
			}
			if ((p->op == ADD && constval == 1) ||
				(p->op == SUB && constval == -1))
			{
				wchange();
				chgop(p, INCR, "incl");
				p->ops[1] = p->ops[2];
				p->ops[2] = NULL;
				++w1stats[1];
				return (wwchanged = true);
			}
		}
		break;

	case 3:
		/*
		**	AopX3	A0,A1,A1	>	AopX2	A0,A1
		**	CopX3	A1,A0,A1	>	CopX2	A0,A1
		*/
		if (noside(p->ops[3]))	/* 3op -> 2op */
		{
			register int	i = strlen(p->ops[0]);
			register char	*newcode = COPY(p->ops[0], i + 1);

			if (newcode[--i] == '3')
				newcode[i] = '2';
			switch (p->op)
			{
			case ADD:
			case MUL:
			case BIS:
			case XOR:
				/* commutative ops */
				if (!strcmp(p->ops[1], p->ops[3]))
				{
					wchange();
					chgop(p, p->op, newcode);
					p->ops[1] = p->ops[2];
					p->ops[2] = p->ops[3];
					p->ops[3] = NULL;
					++w1stats[0];
					return (wwchanged = true);
				}
				/* fall thru ... */
			case SUB:
			case DIV:
			case BIC:
				if (!strcmp(p->ops[2], p->ops[3]))
				{
					wchange();
					chgop(p, p->op, newcode);
					p->ops[3] = NULL;
					++w1stats[0];
					return (wwchanged = true);
				}
				/* fall thru ... */
			default:
				break;
			}
		}
		/*
		**	mov[df]	$0d0.0e+00,A	>	clr[fd]	A
		*/
		if (p->op == MOV && (Tp == TF || Tp == TD) &&
			isfconst(p->ops[1]) && fconstval == 0.0)
		{
			wchange();
			p->ops[1] = p->ops[2];
			p->ops[2] = NULL;
			chgop(p, CLR, Tp == TF ? "clrf" : "clrd");
			++w1stats[10];
			return (wwchanged = true);
		}
		/*
		**	cmp[fd]	A,$0d0.0e+00	>	tst[fd]	A
		*/
		if (p->op == CMP && (Tp == TF || Tp == TD) &&
			isfconst(p->ops[2]) && fconstval == 0.0)
		{
			wchange();
			p->ops[2] = NULL;
			chgop(p, TEST, Tp == TF ? "tstf" : "tstd");
			++w1stats[11];
			return (wwchanged = true);
		}
		if (Tp != TL)
			break;
		/*
		**	addl3	$C,R,A		>	movab	C(R),A
		**	subl3	$C,R,-(sp)	>	pushab	C(R)
		*/
		if ((p->op == ADD || p->op == SUB) && isconst(p->ops[1]) &&
			isreg(p->ops[2]))
		{
			register char *t0 = p->ops[1] + 1;	/* offset */
			register int sign = 1;			/* direction */

			wchange();
			if (p->op == ADD && *t0 == '-')
				sign = -1, ++t0;	/* past '-' */
			else
			if (p->op == SUB)
			{
				if (*t0 != '-')
					sign = -1;
				else
					++t0;		/* past '-' */
			}
			p->ops[1] = getspace(6 + strlen(t0));
			sprintf(p->ops[1], "%s%s(%s)", (sign<0)?"-":"",
				t0, p->ops[2]);
			p->ops[2] = p->ops[3];
			p->ops[3] = NULL;
			if (strcmp(p->ops[2], "-(sp)") == 0)
			{
				chgop(p, PUSHA, "pushab");
				p->ops[2] = NULL;
			}
			else
				chgop(p, MOVA, "movab");
			++w1stats[2];
			return (wwchanged = true);
		}
		/*
		**	addl3	$S,R,A	>	movab	S[R],A
		*/
		if (p->op == ADD && p->ops[1][0] == '$' &&
			isstatic(p->ops[1]) && isreg(p->ops[2]))
		{
			register char *	t;

			wchange();
			t = getspace(strlen(p->ops[1]) + strlen(p->ops[2]) + 2);
			sprintf(t, "%s[%s]", &p->ops[1][1], p->ops[2]);
			p->ops[1] = t;
			p->ops[2] = p->ops[3];
			p->ops[3] = NULL;
			if (strcmp(p->ops[2], "-(sp)") == 0)
			{
				chgop(p, PUSHA, "pushab");
				p->ops[2] = NULL;
			}
			else
				chgop(p, MOVA, "movab");
			++w1stats[8];
			return (wwchanged = true);
		}
		/*
		**	addl3	R,$S,A	>	addl3	$S,R,A
		*/
		if (p->op == ADD && isreg(p->ops[1]) && p->ops[2][0] == '$' &&
			isstatic(p->ops[2]))
		{
			register char *	t;

			wchange();
			t = p->ops[1];
			p->ops[1] = p->ops[2];
			p->ops[2] = t;
			++w1stats[9];
			return (wwchanged = true);
		}
		break;

	case 4:
		/*
		**	extv	$n*8,$8,A,B	>	cvtbl	n+A,B
		**	extv	$n*8,$16,A,B	>	cvtwl	n+A,B
		**	extzv	$n*8,$8,A,B	>	movzbl	n+A,B
		**	extzv	$n*8,$16,A,B	>	movzwl	n+A,B
		*/
		if (p->op == EXTV || p->op == EXTZV)
		{
			register int	flen;
			register int	boff;
			register char	*opname;

			if (isconst(p->ops[1]) && !isreg(p->ops[3]) &&
				((flen = constval) == 8 || flen == 16) &&
				isconst(p->ops[2]) &&
				((boff = constval) % 8 == 0))
			{
				boff /= 8; /* cvt bit offset to byte offset */
				sprintf(opname = getspace(7), "%s%sl",
					(p->op == EXTV) ? "cvt" : "movz",
					(flen == 8) ? "b" : "w");
				wchange();
				chgop(p, (p->op == EXTV) ? CVT : MOVU, opname);
				sprintf(p->ops[1]=getspace(6+strlen(p->ops[3])),
					"%d%s%s", boff,
					p->ops[3][0] == '(' ? "" : "+",
					p->ops[3]);
				p->ops[2] = p->ops[4];
				p->ops[3] = 0;
				p->ops[4] = 0;
				++w1stats[3];
				return (wwchanged = true);
			}
		}
		/*
		**	extv	$C,$32-C,A0,A1	>	ashl	$-N,A0,A1
		*/
		if (p->op == EXTV && (isconst(p->ops[1]) &&
			((shcnt = constval) > 0 && shcnt < 32)) &&
			(isconst(p->ops[2]) && (32 - shcnt) == constval))
		{
			wchange();
			chgop(p, ASH, "ashl");
			p->ops[1] = getspace(5);	/* max: "$-31\0" */
			sprintf(p->ops[1], "$%d", -shcnt);
			p->ops[2] = p->ops[3];
			p->ops[3] = p->ops[4];
			p->ops[4] = NULL;
			++w1stats[6];
			return (wwchanged = true);
		}
		break;
	}
	return (false);
}
