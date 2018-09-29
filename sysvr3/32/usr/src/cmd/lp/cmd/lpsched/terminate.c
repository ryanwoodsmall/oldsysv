/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpsched/terminate.c	1.2"

#include "lpsched.h"

/**
 ** terminate() - STOP A CHILD PROCESS
 **/

void			terminate (ep)
	register EXEC		*ep;
{
	if (ep->pid > 0) {

#if	defined(DEBUG)
		if (debug) {
			FILE			*fp
						= open_logfile("exec");

			char			buffer[BUFSIZ];

			time_t			now;

			extern char		*ctime();


			if (fp) {
				setbuf (fp, buffer);
				(void)time (&now);
				fprintf (
					fp,
					"KILL: %24.24s pid %d%s%s\n",
					ctime(&now),
					ep->pid,
					((ep->flags & EXF_KILLED)?
						  ", second time"
						: ""
					),
					(kill(ep->pid, 0) == -1?
						  ", but child is GONE!"
						: ""
					)
				);
				close_logfile (fp);
			}
		}
#endif

		if (ep->flags & EXF_KILLED)
			return;
		ep->flags |= EXF_KILLED;

		/*
		 * Theoretically, the following "if-then" is not needed,
		 * but there's some bug in the code that occasionally
		 * prevents us from hearing from a finished child.
		 * (Kill -9 on the child would do that, of course, but
		 * the problem has occurred in other cases.)
		 */
		if (kill(-ep->pid, SIGTERM) == -1 && errno == ESRCH) {
			ep->pid = -99;
			ep->status = SIGTERM;
			ep->errno = 0;
			DoneChildren++;
		}
	}
	return;
}
