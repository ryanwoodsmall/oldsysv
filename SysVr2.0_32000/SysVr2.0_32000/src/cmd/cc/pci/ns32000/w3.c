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
/*	@(#)w3.c	1.3	*/
#include	"optim.h"
#include	"extdec.h"

char *logbit();

char	*w3opts[] =
{
/* 0 */ "aded acbx",
/* 1 */ "added tbit",
	0
};
int	w3stats[sizeof w3opts / sizeof w3opts[0]];
int	w3size = sizeof w3opts / sizeof w3opts[0];
#ifdef PEEPHOLE
static	char	*acbs[] =
{
	NULL, "acbb", "acbw", "acbd", NULL, NULL, NULL, NULL
};

boolean	w3optim(pf, pl)
register struct	node	*pf;
register struct	node	*pl;
{
	register struct	node	*pm = pf->forw;
	register int	T = argtype(pf, 1);

	/*
	**	addqx	A0,A1	\	
	**	cmpqx	$0,A1	 >      acbX	A0,A1,L
	**	jneq	L	/
	*/
	if (pf->op == ADD && pm->op == CMP && pl->op == BNE &&
		T == argtype(pm, 1) && noside(pf->ops[2]) &&
		pf->ops[0][3] == 'q' && !strcmp(pf->ops[2], pm->ops[2]) &&
		pm->ops[0][3] == 'q' && !strcmp(pm->ops[1],"0") )
	{
		register char	*s;

		switch (T)
		{
		case TB: case TW: case TL:
			s = acbs[T >> 4];/* sensitive to type encoding! */
			break;

		default:
			goto try_1;
		}
		wchange();
		pf->ops[3] = pl->ops[1];
		chgop(pf, ACB, s);
		DELNODE(pm); ndisc++;
		DELNODE(pl); ndisc++;
		w3stats[0]++;
		return (wwchanged = true);
	}
	/*
	**	andx	$X, R0	\
	**	cmpx	R0, $0	 >	tbitx	$(log X), R0
	**	beq	L	/	bfc	L
	**
	**	andx	$X, R0	\
	**	cmpx	R0, $X	 >	tbitx	$(log X), R0
	**	beq	L	/	bfs	L
	**
	**	andx	$X, R0	\
	**	cmpx	R0, $0	 >	tbitx	$(log X), R0
	**	bne	L	/	bfs	L
	**
	**	andx	$X, R0	\
	**	cmpx	R0, $X	 >	tbitx	$(log X), R0
	**	bne	L	/	bfc	L
	*/
	if (pf->op == AND && pm->op == CMP && (pl->op == BEQ || pl->op == BNE)
		&& T == argtype(pm,1) && isdead(pf->ops[2],pl) 
		&& isconst(pf->ops[1]) && oneset(constval,T) && ( (
			isquick(pm,CMP) && (! strcmp(pf->ops[2],pm->ops[2]))
			&& ((!strcmp(pm->ops[1],"0"))||(!strcmp(pm->ops[1],pf->ops[1]+1))))
		|| ( (!strcmp(pf->ops[2],pm->ops[1])) && ((!strcmp(pm->ops[2],"$0")) || (!strcmp(pm->ops[2],pf->ops[1]))))))
	{
		wchange();
		++w3stats[1];
		switch( T )
		{
		case TB:
			chgop(pf, TBIT, "tbitb");
			break;
		case TW:
			chgop(pf, TBIT, "tbitw");
			break;
		case TL:
			chgop(pf, TBIT, "tbitd");
		}
		pf->ops[1] = logbit(constval);
		if ( isquick(pm,CMP) )
		{
			if ((pl->op == BEQ && !strcmp(pm->ops[1],"0")) ||
			    (pl->op == BNE && strcmp(pm->ops[1],"0")))
				chgop(pl, BFC, "bfc");
			else
				chgop(pl, BFS, "bfs");
		}
		else
		{
			if ((pl->op == BEQ && !strcmp(pm->ops[1],"$0")) ||
			    (pl->op == BNE && strcmp(pm->ops[1],"$0")))
				chgop(pl, BFC, "bfc");
			else
				chgop(pl, BFS, "bfs");
		}
		DELNODE(pm); ndisc++;
		return (wwchanged = true);
		}
	return (false);
try_1:
	return (false);
}
#endif
