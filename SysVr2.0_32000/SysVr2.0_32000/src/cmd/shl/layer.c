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
/*	@(#)layer.c	1.3	*/
#include	"defs.h"

char *malloc();

create(name)
	char *name;
{
	int i;
	int n;
	char c;

	if (i = get_new_layer(name))
	{
		layers[i] = (struct layer *)malloc(sizeof(struct layer));

		if (*name)
			strcpy(layers[i]->name, name);
		else
			sprintf(layers[i]->name, "(%d)", i);
				
		if (layers[i]->p_grp = spawn_shell(i))
		{	
			while ((n = read(fildes[0], &c, 1)) < 0)
				;
			if (n > 0)
			{
				if (ioctl(cntl_chan_fd, SXTIOCSWTCH, i) != SYSERROR)
				{
					close(fildes[0]);
					push(i);
					adj_utmp(i);
					wait_for_fg();
					adj_utmp(0);
					return;
				}
				else
				{
					fprintf(stderr, "switch failed (errno = %d)\n", errno);
					kill(-layers[i]->p_grp, SIGKILL);
				}
			}
			close(fildes[0]);
		}		

		free_layer(i);
	}
}


destroy(name)
	char *name;
{
	int i;

	if (i = lookup_layer(name))
	{	
		kill(-layers[i]->p_grp, SIGHUP);
		if (i == top())
			pop();
	}
}


resume(name)
	char *name;
{
	int i;

	if (i = lookup_layer(name))
	{
		printf("resuming %s\n", name);

		if (ioctl(cntl_chan_fd, SXTIOCSWTCH, i) != SYSERROR)
		{
			push(i);
			adj_utmp(i);
			wait_for_fg();
			adj_utmp(0);
		}
		else
			fprintf(stderr, "switch failed (errno = %d)n", errno);
	}
}


resume_current()
{
	int n;

	while (n = top())
	{
		if (layers[n] == 0)
			pop();
		else
			break;
	}

	if (n)
	{
		printf("resuming %s\n", layers[n]->name);

		if (ioctl(cntl_chan_fd, SXTIOCSWTCH, n) == SYSERROR)
		{
			pop();
			fprintf(stderr, "switch failed (errno = %d)n", errno);
		}
		else
		{
			adj_utmp(n);
			wait_for_fg();
			adj_utmp(0);
		}
	}
}


kill_all()
{
	int i;
	int retval;
	int found = 0;

	signal(SIGCLD, SIG_IGN);

	for (i = 1; i <= max_index; ++i)
		if (layers[i])
		{
			kill(-layers[i]->p_grp, SIGHUP);
			found = 1;
		}
	if (found)
		wait(&retval);
}


wait_for_fg()
{

	while(ioctl(cntl_chan_fd, SXTIOCWF, 0) == SYSERROR)
		;

	ioctl(cntl_chan_fd, TCSETA, &ttysave);
}


block(name)
	char *name;
{
	int i;

	if (i = lookup_layer(name))
	{
		if (ioctl(cntl_chan_fd, SXTIOCBLK, i) == SYSERROR)
			fprintf(stderr, "switch failed (errno = %d)n", errno);
	}
}


unblock(name)
	char *name;
{
	int i;

	if (i = lookup_layer(name))
	{
		if (ioctl(cntl_chan_fd, SXTIOCUBLK, i) == SYSERROR)
			fprintf(stderr, "switch failed (errno = %d)n", errno);
	}
}


