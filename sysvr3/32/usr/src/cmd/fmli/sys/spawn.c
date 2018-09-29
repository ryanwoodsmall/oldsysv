/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.sys:src/sys/spawn.c	1.1"

#include	<stdio.h>
#include	<signal.h>
#include	<errno.h>
#include	<varargs.h>

#define bytswap(X)	(0xff & ((X) >> 8))

int
spawnv(file, arglist)
char	*file;
char	*arglist[];
{
	register int	pid;
	register int	fd;

	alarm(0);
	switch (pid = fork()) {
	case -1:
		fprintf(stderr, "Can't create another process\r\n");
		break;
	case 0:
		signal(SIGHUP, SIG_IGN);
		signal(SIGINT, SIG_IGN);
		signal(SIGQUIT, SIG_IGN);
		signal(SIGUSR1, SIG_IGN);
		signal(SIGTERM, SIG_DFL);
		for (fd = 0; fd < _NFILE; fd++)
			close(fd);
		dup(dup(open("/dev/null", 2)));
		execvp(file, arglist);
		_exit(127);
	default:
		break;
	}
	return pid;
}

/*VARARGS1*/
spawn(file, va_alist)
char	*file;
va_dcl
{
	char	*arglist[20];
	register char	**p;
	register int	i;
	va_list	ap;

	va_start(ap);
	for (p = arglist; p < arglist + sizeof(arglist)/sizeof(*arglist); p++)
		if ((*p = va_arg(ap, char *)) == NULL)
			break;
	va_end(ap);
	return	spawnv(file, arglist);
}

sysspawn(s)
char	*s;
{
	char	*arglist[4];

	arglist[0] = "sh";
	arglist[1] = "-c";
	arglist[2] = s;
	arglist[3] = NULL;
	return spawnv("/bin/sh", arglist);
}

waitspawn(pid)
register int	pid;
{
	int	status;
	register int waitcode;

	while ((waitcode = wait(&status)) != pid)
		if (waitcode == -1 && errno != EINTR)
			break;
	return bytswap(status);
}
