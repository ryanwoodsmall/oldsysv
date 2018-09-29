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
/*	@(#)w2.c	1.3	*/
#include	"optim.h"
#include	"extdec.h"

char *w2opts[] =
{
/* 0 */ "movx R, X; cmpx S, X -> movx R, X; cmpx S, R",
/* 1 */ "movqx S, X; movx X, R -> movqx S, X; movqx S, R",
/* 2 */ "rwopx X, R; movx R, Y -> rwopx X, Y",
/* 3 */ "movx X, R; aopx R, Y -> aopx X, Y",
/* 4 */ "movx X, R; tbitx S, R -> tbitx S, X",
	0
};
int	w2stats[sizeof w2opts / sizeof w2opts[0]];
int	w2size = sizeof w2opts / sizeof w2opts[0];

#ifdef PEEPHOLE
boolean	w2optim(p, pf)
register struct	node	*p;
register struct	node	*pf;
{
	register int	n = p->userdata->nargs;
	register int	Tp = argtype(p, 1);
	register int	Tpf = argtype(pf, 1);
	register int	i;			/* random quick integer */
	register int	shcnt;			/* shift count used far below */

	/*
	**	movx	R0, A0	\	movx	R0, A0
	**	cmpx	A1, A0	/	cmpx	A1, R0
	*/
	if ( p->op == MOV && pf->op == CMP && Tp == argtype(pf,2) &&
		isreg(p->ops[1]) && (!isreg(p->ops[2])) && !strcmp(p->ops[2],pf->ops[2]) )
	{
		wchange();
		++w2stats[0];
		pf->ops[2] = p->ops[1];
		return (wwchanged = true);
	}
	if (p->op == MOV && (!isreg(p->ops[2])) && pf->op == CMP && Tp == Tpf && isreg(p->ops[1]) &&
		!strcmp(p->ops[2],pf->ops[1]) )
	{
		wchange();
		++w2stats[0];
		pf->ops[1] = p->ops[1];
		return (wwchanged = true);
	}
	/*
	**	movqx	X, A0	\	movqx	X, A0
	**	movx	A0, R	/	movqx	X, R
	*/
	if (p->op == MOV && p->ops[0][3] == 'q' && pf->op == MOV &&
		( (i = argtype(pf,2)) == TL || i == TW || i == TB) &&
		! strcmp(p->ops[2],pf->ops[1]) )
	{
		wchange();
		switch ( argtype(pf,2) )
		{
		case TB:
			chgop(pf, MOV, "movqb"); break;
		case TW:
			chgop(pf, MOV, "movqw"); break;
		case TL:
			chgop(pf, MOV, "movqd"); break;
		}
		pf->ops[1] = p->ops[1];
		w2stats[1]++;
		if ( isdead(p->ops[2],pf) )
		{
			DELNODE(p); ndisc++;
		}
		return (wwchanged = true);
	}
	/*
	**	rwopx	A0, R0	\
	**	movx	R0, A1	/	rwopx	A0, A1
	*/
	if ( (p->op == ADDR || isrwop(p)) && pf->op == MOV && Tpf == argtype(pf,2) && Tpf == argtype(p,2) &&
		isdead(p->ops[2],pf) && !strcmp(p->ops[2],pf->ops[1]) )
	{
		wchange();
		p->ops[2] = pf->ops[2];
		DELNODE(pf); ndisc++;
		++w2stats[2];
		return (wwchanged = true);
	}
	/*
	**	movx	A0, R0	\
	**	aopx	R0, A1	/	aopx	A0, A1
	*/
	if ( p->op == MOV && Tp == argtype(p,2) && Tp == Tpf &&
		(isaop(pf) || isrwop(pf)) && isdead(p->ops[2],pf) &&
		(! isquick(p,MOV)) && ! strcmp(p->ops[2],pf->ops[1]) )
	{
		wchange();
		++w2stats[3];
		pf->ops[1] = p->ops[1];
		DELNODE(p); ndisc++;
		return (wwchanged = true);
	}
	/*
	**	movx	A0, R0	\
	**	tbitx	$X, R0	/	tbitx	$X, A0
	*/
	if ( p->op == MOV && pf->op == TBIT && Tp == Tpf &&
		isdead(p->ops[2],pf) && Tp == argtype(p,2) &&
                p->ops[1][0] != '$' &&
		! strcmp(p->ops[2],pf->ops[2]))
	{
		wchange();
		++w2stats[4];
		pf->ops[2] = p->ops[1];
		DELNODE(p); ndisc++;
		return (wwchanged = true);
	}
	return false;
}
#endif
