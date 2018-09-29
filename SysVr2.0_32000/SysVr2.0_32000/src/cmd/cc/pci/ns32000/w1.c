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
/*	@(#)w1.c	1.3	*/
#include	<stdio.h>
#include	"optim.h"
#include	"extdec.h"

char	*w1opts[] =
{
/* 0 */  "and $S -> cbit $(log ~S)",
/* 1 */  "or $S -> sbit $(log S)",
/* 2 */  "xor $S -> ibit $(log S)",
		  0
};
int	w1stats[sizeof w1opts / sizeof w1opts[0]];
int	w1size = sizeof w1opts / sizeof w1opts[0];

#ifdef PEEPHOLE
char *logbit();

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
	case 2:
		/*
		**	 and $S -> cbit $(log ~S)
		*/
	   	if (p->op == AND && isconst(p->ops[1]) && oneset(~constval,Tp) && p->ops[2][0] != '$' )
		{
			wchange();
			switch (Tp) {
			case TL:
				chgop(p, CBIT, "cbitd");
				break;
			case TW:
				chgop(p, CBIT, "cbitw");
				break;
			case TB:
				chgop(p, CBIT, "cbitb");
				break;
			}
			p->ops[1] = logbit(~ constval);
			++w1stats[0];
			return (wwchanged = true);
		}
		/*
		**	or $S -> sbit $(log S)
		*/
		if (p->op == OR && isconst(p->ops[1]) && oneset(constval,Tp) && p->ops[2][0] != '$')
		{
			wchange();
			switch (Tp) {
			case TL:
				chgop(p, CBIT, "sbitd");
				break;
			case TW:
				chgop(p, CBIT, "sbitw");
				break;
			case TB:
				chgop(p, CBIT, "sbitb");
				break;
			}
			p->ops[1] = logbit(constval);
			++w1stats[1];
			return (wwchanged = true);
		}
		/*
		**	xor $S -> ibit $(log S)
		*/
		if (p->op == XOR && isconst(p->ops[1]) && oneset(constval,Tp) && p->ops[2][0] != '$')
		{
			wchange();
			switch (Tp) {
			case TL:
				chgop(p, CBIT, "ibitd");
				break;
			case TW:
				chgop(p, CBIT, "ibitw");
				break;
			case TB:
				chgop(p, CBIT, "ibitb");
				break;
			}
			p->ops[1] = logbit(constval);
			++w1stats[2];
			return (wwchanged = true);
		}
		break;
	}
	return (false);
}
#endif
