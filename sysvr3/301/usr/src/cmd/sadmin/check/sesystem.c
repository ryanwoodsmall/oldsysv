/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sadmin:check/sesystem.c	1.3"
/*
	system(3S) with standard out redirected to standard error
*/

#include <signal.h>


sesystem(s)
char	*s;
{
	extern int fork(), execl(), wait();
	register int	pid, w;
	int	status;
	register int (*istat)(), (*qstat)();

	if((pid = fork()) == 0) {
		close(1);	/* redirect standard out to standard err */
		dup(2);
		(void) execl("/bin/sh", "sh", "-c", s, 0);
		_exit(127);
	}
	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);
	while((w = wait(&status)) != pid && w != -1)
		;
	(void) signal(SIGINT, istat);
	(void) signal(SIGQUIT, qstat);
	return((w == -1)? w: status);
}
