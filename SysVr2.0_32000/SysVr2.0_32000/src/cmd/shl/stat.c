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
/*	@(#)stat.c	1.2	*/
#include	"defs.h"

static struct sxtblock status;

all_layers(flag)
	int flag;
{
	int i;

	status.input = 0;
	status.output = 0;
	ioctl(cntl_chan_fd, SXTIOCSTAT, &status);

	for (i = 1; i <= max_index; ++i)
		if (layers[i])
			stat_layer(i, flag);
}


one_layer(name, flag)
	char *name;
	int flag;
{
	int i;

	if (i = lookup_layer(name))
		stat_layer(i, flag);
}


stat_layer(i, flag)
	int i;
	int flag;
{
	printf("%s (%d)", layers[i]->name, layers[i]->p_grp);

	if (status.input & (1 << i))
		printf(" blocked on input\n");
	else if (status.output & (1 << i))
		printf(" blocked on output\n");
	else
		printf(" executing or awaiting input\n");

	fflush(stdout);

	if (flag)
	{
		char buff[50];

		sprintf(buff, "%d", layers[i]->p_grp);
		system(buff);
		printf("\n");
	}
}

