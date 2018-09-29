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
/*	@(#)system.c	1.2	*/
#include	"defs.h"

extern int fork(), execl(), wait();

system(s)
	char	*s;
{
	int	status;
	int 	pid;
	int 	i;

	signal(SIGCLD, SIG_DFL);

	if((pid = fork()) == 0) 
	{
		(void) execl("/bin/ps", "ps", "-f", "-g", s, 0);
		_exit(127);
	}
	
	for (;;)
	{
		i = wait(&status);

		if (i != -1 && i != pid)
			clean_up(i);
		else
		{
			signal(SIGCLD, child_death);
			return;
		}
	}
}
		

