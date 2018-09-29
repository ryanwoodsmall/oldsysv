/*	@(#)w3.c	1.3	*/
#include	"optim.h"
#include	"extdec.h"

char	*w3opts[] =
{
/* 0 */	"added acbX",
/* 1 */	"added aob",
	0
};
int	w3stats[sizeof w3opts / sizeof w3opts[0]];
int	w3size = sizeof w3opts / sizeof w3opts[0];
static	char	*acbs[] =
{
	NULL, "acbb", "acbw", "acbl", NULL, "acbf", "acbd", NULL
};

boolean	w3optim(pf, pl)
register struct	node	*pf;
register struct	node	*pl;
{
	register struct	node	*pm = pf->forw;
	register int	T = argtype(pf, 1);

	/*
	**	addl2	A0,A1	\	acbX	A2,A0,A1,L
	**	cmpl	A1,A2	 >
	**	jleq	L	/
	*/
	if (pf->op == ADD && pm->op == CMP && pl->op == JLE &&
		T == argtype(pm, 1) && noside(pf->ops[2]) &&
		!strcmp(pf->ops[2], pm->ops[1]))
	{
		register char	*s;

		switch (T)
		{
		case TB: case TW: case TL: case TF: case TD:
			s = acbs[T >> 4];/* sensitive to type encoding! */
			break;

		default:
			goto try_1;
		}
		wchange();
		pf->ops[2] = pf->ops[1];
		pf->ops[1] = pm->ops[2];
		pf->ops[3] = pm->ops[1];
		pf->ops[4] = pl->ops[1];
		chgop(pf, ACB, s);
		DELNODE(pm); ndisc++;
		DELNODE(pl); ndisc++;
		w3stats[0]++;
		return (wwchanged = true);
	}
	if (loopflag == false)
		return (false);
	/*
	**	incl	A0	\	aobleq	A1,A0,L
	**	cmpl	A0,A1	 >
	**	jleq	L	/
	**
	**	incl	A0	\	aoblss	A1,A0,L
	**	cmpl	A0,A1	 >
	**	jlss	L	/
	*/
	if (T == TL && pf->op == INCR && pm->op == CMP &&
		(pl->op == JLE || pl->op == JLT) && noside(pf->ops[1]) &&
		!strcmp(pf->ops[1], pm->ops[1]))
	{
		wchange();
		pf->ops[2] = pf->ops[1];
		pf->ops[1] = pm->ops[2];
		pf->ops[3] = pl->ops[1];
		chgop(pf, pl->op == JLE ? AOBLEQ : AOBLSS,
			pl->op == JLE ? "aobleq" : "aoblss");
		++w3stats[1];
		return (wwchanged = true);
	}
try_1:
	return (false);
}
