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
/*	@(#)supp.c	1.3	*/
#include	"optim.h"
#include	"extdec.h"

void	prinst(p)					/* print instruction */
register struct	node	*p;
{
	register int	i;

	switch (p->op)
	{
	case LABEL:
	case HLABEL:
		printf("%s:", p->ops[0]);
		break;

	case CASE:
		printf("\t%s\t%s\n", p->ops[0], p->ops[1]);
		printf("%s\n%s\n", p->ops[2], p->ops[3]);
		break;
	default:
		printf("\t%s", p->ops[0]);
		for (i = 1; i < MAXOPS; i++)
			if (p->ops[i])
				printf("%s%s", (i == 1? "\t": ","), p->ops[i]);
			else
				break;
	}
	printf("\n");
}

boolean ishlp(p)		/* return true if a fixed label present */
register struct	node	*p;
{
	for (; islabel(p); p = p->forw)
	{
		if (ishl(p))
			return(true);
	}
	return(false);
}

char	*getp(p)				/* return pointer to operand */
register struct	node	*p;
{
	switch (p->op)
	{
	default:
		printf("%cBOGUS GETP: op := %d.; ops[0] := [%s]\n",
			CC, p->op, p->ops[0]);
		/* FALL THRU */

	/* Unknown or multiple destinations return NULL */
	case RXP:
	case RETI:
	case RETT:
	case RET:
	case CASE:
		return (NULL);

	/* Simple branches have only destination */
	case BR:
	case JUMP:
	case BEQ:
	case BNE:
	case BLE:
	case BGE:
	case BLT:
	case BGT:
	case BLS:
	case BLO:
	case BHS:
	case BHI:
        case BCC:
        case BCS:
        case BFC:
        case BFS:
        case JSR:
        case BSR:
		return (p->ops[1]);

	case ACB:
		return (p->ops[3]);

	/* Hard stuff: last operand is the destination */
	case HBRAN:
		return (p->ops[p->userdata->nargs]);
	}
}

void	putp(p, c)				/* insert pointer into node */
register struct	node	*p;
register char	*c;
{
	if (c == NULL)	/* change a BR to a ret */
		chgop(p, RET, "ret");
	switch (p->op)
	{
	default:
		p->ops[1] = c;
		break;

	case ACB:
 		p->ops[3] = c;
		break;

	case CASE:
		printf("BOGUS setp of case instruction!\n");
		break;
	}
}

void	revbr(p)				/* reverse branch in node p */
register struct	node	*p;
{
	switch (p->op)
	{
	case BEQ:	chgop(p, BNE, "bne"); break;
	case BNE:	chgop(p, BEQ, "beq"); break;
	case BLE:	chgop(p, BGT, "bgt"); break;
	case BGE:	chgop(p, BLT, "blt"); break;
	case BLT:	chgop(p, BGE, "bge"); break;
	case BGT:	chgop(p, BLE, "ble"); break;
	case BLO: 	chgop(p, BHS, "bhs"); break;
	case BHI: 	chgop(p, BLS, "bls"); break;
	case BLS: 	chgop(p, BHI, "bhi"); break;
	case BHS: 	chgop(p, BLO, "blo"); break;
	case BCS:	chgop(p, BCC, "bcc"); break;
	case BCC:	chgop(p, BCS, "bcs"); break;
	case BFS: 	chgop(p, BFC, "bfc"); break;
	case BFC: 	chgop(p, BFS, "bfs"); break;

	default:
		printf("revbr of irreversible branch!\n");
	}
}

static	int	labseed = 0;			/* available to dstats() */

char	*newlab()				/* generate a new label */
{
	register char	*c;

	c = getspace(9);
	sprintf(c, ".I%X", labseed);
	labseed++;
	return(c);
}

void	dstats()			/* print machine dependent statistics */
{
	extern	int	fnum;
	extern	void	windstats();	/* window(s) statistics */

	fprintf(stderr, "%d labels made\n", labseed);
	windstats();
}

wrapup()
{
	/* clean out last function, if any */
	endfunc();
	log_stats();
}
