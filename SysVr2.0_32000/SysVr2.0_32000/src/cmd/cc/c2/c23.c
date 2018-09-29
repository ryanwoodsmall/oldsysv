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
/*	@(#)c23.c	1.4	*/

#include "c2.h"

/*
* ``real'' space allocation for header
*/

char	line[512];
struct	node	first;
char	*curlp;
int	nbrbr;
int	nsaddr;
int	redunm;
int	iaftbr;
int	njp1;
int	nrlab;
int	nxjump;
int	ncmot;
int	nrevbr;
int	loopiv;
int	nredunj;
int	nskip;
int	ncomj;
int	nsob;
int	nrtst;
int	nbj;
int	nfield;

int	nchange;
int	debug;
char	*lasta;
char	*lastr;
char	*firstr;
char	*regs[NREG+5]; /* 0-11, 4 for operands, 1 for running off end */
char	conloc[RANDLEN];
char	conval[RANDLEN];
char	ccloc[RANDLEN];


/*
* These routines are never called, but are useful if called
* via sdb or other debuggers
*/

dumpnode(p)
register struct	node	*p;
{
	printf("op:	%d,%d\n", (unsigned char) p->op,
				(unsigned char) p->subop);
	if (p->code)	printf("code:	%s\n", p->code);
	if (p->refc)	printf("refc:	%d\n", p->refc);
	if (p->labno)	printf("labno:	%d\n", p->labno);
	if (p->seq)	printf("seq:	%d\n", p->seq);
}

dumprands()
{
	register int	i;

	printf("Rands:\n");
	for (i = NREG; i < NREG + 5; i++)
		if (regs[i][0])
			printf("%d: %s\n", i - NREG, regs[i]);
	printf("-\n");
}
