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
/*	@(#)aux.c	1.2	*/
#include	"defs.h"

static int	stack[3];
static int	sp;

top()
{
	return(stack[sp]);
}


pop()
{
	stack[sp] = 0;
	sp = (sp + 2) % 3;
}

push(n)
	int n;
{
	if (n != stack[sp])
	{
		sp = (sp + 1) % 3;
		stack[sp] = n;
	}
}


conv(n)
	int n;
{
	int i;
	int j;

	i = n / MAX_LAYERS;
	j = n % MAX_LAYERS;

	return((i * 10) + j);
}


toggle()
{
	int 	i;
	int 	tmp;

	i = (sp + 2) % 3;
	if (stack[i])
	{
		tmp = stack[i];
		stack[i] = stack[sp];
		stack[sp] = tmp;
	}

	resume_current();
}
