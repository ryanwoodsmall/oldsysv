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
		printf("\t%s", p->ops[0]);
		for (i = 1; i < 4; i++)
			printf("%s%s", (i == 1? "\t": ","), p->ops[i]);
		printf("\n%s\n%s", p->ops[4], p->ops[5]);
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
	case RET:
	case RET0:
	case RET1:
	case RET2:
	case CASE:
		return (NULL);

	/* Simple branches have only destination */
	case JBR:
	case JMP:
	case JEQ:
	case JNE:
	case JLE:
	case JGE:
	case JLT:
	case JGT:
	case JLTU:
	case JGTU:
	case JLEU:
	case JGEU:
		return (p->ops[1]);

	case CALLS:
	case SOBGEQ:
	case SOBGTR:
	case JLBS:
	case JLBC:
		return (p->ops[2]);

	case AOBLEQ:
	case AOBLSS:
	case JBS:
	case JBC:
		return (p->ops[3]);

	case ACB:
		return (p->ops[4]);

	/* Hard stuff: last operand is the destination */
	case HBRAN:
		return (p->ops[p->userdata->nargs]);
	}
}

void	putp(p, c)				/* insert pointer into node */
register struct	node	*p;
register char	*c;
{
	if (c == NULL)	/* change a jbr to a ret */
		chgop(p, RET, "ret");
	switch (p->op)
	{
	default:
		p->ops[1] = c;
		break;

	case CALLS:
	case SOBGEQ:
	case SOBGTR:
	case JLBS:
	case JLBC:
		p->ops[2] = c;
		break;

	case AOBLEQ:
	case AOBLSS:
	case JBS:
	case JBC:
		p->ops[3] = c;
		break;

	case ACB:
		p->ops[4] = c;
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
	case JEQ:	chgop(p, JNE, "jneq"); break;
	case JNE:	chgop(p, JEQ, "jeql"); break;
	case JLE:	chgop(p, JGT, "jgtr"); break;
	case JGE:	chgop(p, JLT, "jlss"); break;
	case JLT:	chgop(p, JGE, "jgeq"); break;
	case JGT:	chgop(p, JLE, "jleq"); break;
	case JLTU:	chgop(p, JGEU, "jgequ"); break;
	case JGTU:	chgop(p, JLEU, "jlequ"); break;
	case JLEU:	chgop(p, JGTU, "jgtru"); break;
	case JGEU:	chgop(p, JLTU, "jlssu"); break;
	case JBS:	chgop(p, JBC, "jbc"); break;
	case JBC:	chgop(p, JBS, "jbs"); break;
	case JLBS:	chgop(p, JLBC, "jlbc"); break;
	case JLBC:	chgop(p, JLBS, "jlbs"); break;

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
