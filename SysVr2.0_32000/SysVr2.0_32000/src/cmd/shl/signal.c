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
/*	@(#)signal.c	1.2	*/
#include	"defs.h"

int 	(*(sig[NSIG]))() = 
{
	0,
	hangup,
	SIG_IGN,
	SIG_IGN,
	SIG_IGN,
	SIG_IGN,
	SIG_IGN,
	SIG_IGN,
	SIG_IGN,
	SIG_IGN,
	SIG_IGN,
	SIG_IGN,
	SIG_IGN,
	SIG_IGN,
	SIG_IGN,
	SIG_IGN,
	SIG_IGN,
	SIG_IGN,
	child_death,
	SIG_IGN
};

int	(*(save_sig[NSIG]))();
	
hangup()
{

	int i;

	signal(SIGCLD, SIG_IGN);

	for (i = 1; i <= max_index; ++i)
		if (layers[i])
			kill(-layers[i]->p_grp, SIGHUP);

	reset();

	exit(5);
}


child_death()
{
	int pid;
	int status;
	
	pid = wait(&status);

	clean_up(pid);
}	


clean_up(pid)
	int pid;
{
	int i;


	for (i = 1; i <= max_index; ++i)
		if (layers[i])
			if (pid == layers[i]->p_grp)
				break;


	if (i > max_index)
	{
		signal(SIGCLD, child_death);
		return;
	}

	free_layer(i);

	if (i == top())
	{
		if (ioctl(cntl_chan_fd, SXTIOCSWTCH, 0) != SYSERROR)
			pop();
		else
		{
			fprintf(stderr, "switch to channel 0 failed (errno = %d)\n", errno);
			reset();
			exit(4);
		}
	}

	signal(SIGCLD, child_death);
}


setsig()
{
	int i;

	for (i = 1; i < NSIG; ++i)
		save_sig[i] = signal(i, sig[i]);

	save_sig[SIGHUP] = SIG_DFL;
}

restore_sig()
{
	int i;
	
	for (i = 1; i < NSIG; ++i)
		signal(i, save_sig[i]);
}
