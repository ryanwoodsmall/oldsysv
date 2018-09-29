/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.proc:src/proc/pcurrent.c	1.4"

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include "wish.h"
#include "token.h"
#include "slk.h"
#include "actrec.h"
#include "proc.h"
#include "procdefs.h"
#include "terror.h"

extern struct proc_rec PR_all[];
extern bool Suspend_interupt;
extern char *Suspend_window;

int
proc_current(rec)
struct actrec *rec;
{
	int p = rec->id;
	int pid, w, status;
	extern int errno;
	extern int _Debug;
	void sigcatch();
	int (*oldquit) (), (*oldint) ();

	if (PR_all[p].status == ST_DEAD)
		return;

	/* if process is not already forked, fork it else resume it */

	vt_before_fork();
	fork_clrscr();
	if (PR_all[p].pid == NOPID) {
#ifdef _DEBUG
		_debug(stderr, "NEW PROCESS FORKING\n");
#endif
		switch (pid = fork()) {
		case FAIL:
#ifdef _DEBUG
			_debug(stderr, "Fork failed with errno=%d\n", errno);
#endif
			error(MISSING, "process fork failed");
			return(FAIL);
		case 0:
#ifdef _DEBUG
			if (_Debug)
				(void) freopen("/dev/tty", "w+", stderr);
#endif
			execvp(PR_all[p].name, PR_all[p].argv);
			error(NOEXEC, PR_all[p].name);
			_exit(255);
		default:
			oldquit = signal(SIGQUIT, SIG_IGN);
			oldint = signal(SIGINT, SIG_IGN);
			PR_all[p].pid = PR_all[p].respid = pid;
			break;
		}
	} else {	/* resume */
		pid = PR_all[p].pid;
#ifdef _DEBUG
		_debug(stderr, "resuming pid %d by signaling %d\n", pid, PR_all[p].respid);
#endif
		if (PR_all[p].flags & PR_CLOSING) {
			fflush(stdout);
			fflush(stderr);
			printf("You are returning to a suspended activity.  This activity\r\n");
			printf("must be ended before you can complete logging out.\r\n");
			printf("Please take whatever steps are necessary to end this\r\n");
			printf("activity.\r\n\n");
			fflush(stdout);
			sleep(3);
		} else {
			fflush(stdout);
			fflush(stderr);
			printf("You are returning to a suspended activity. \r\n");
			fflush(stdout);
			sleep(3);
		}

		if (kill(PR_all[p].respid, SIGUSR1) == FAIL) {
#ifdef _DEBUG
			_debug(stderr, "RESUME SIGNAL FAILED WITH ERRNO=%d\n", errno);
#endif
			return(FAIL);
		}
	}
	PR_all[p].status = ST_RUNNING;

#ifdef _DEBUG
	_debug(stderr, "Waiting for pid %d\n", pid);
#endif
	status = 0;
	Suspend_interupt = FALSE;

	while ((w = wait(&status)) != pid) {
		if ((w == FAIL && errno != EINTR) || Suspend_interupt) {
#ifdef _DEBUG
			_debug(stderr, "Woken while waiting for %d\n", pid);
#endif
			break;
		}
	}
	if (Suspend_interupt) {
#ifdef _DEBUG
		_debug(stderr, "Process %d suspended, making non-current\n", pid);
#endif
		if (Suspend_window == NULL)
			ar_backup();	/* go back to previous activation record */
		else {
			objop("OPEN", NULL, Suspend_window, NULL);
			free(Suspend_window);
			Suspend_window = NULL;
		}
	} else {
#ifdef _DEBUG
		_debug(stderr, "Process terminated, closing actrec\n");
#endif
		PR_all[p].pid = PR_all[p].respid = NOPID;
		if ((PR_all[p].flags & PR_CLOSING) == 0)
			ar_close(rec);
		if ((PR_all[p].flags == PR_ERRPROMPT && status>>8 != 0) ||
				(PR_all[p].flags & ~PR_CLOSING) == 0) {
			char buf[PATHSIZ];

			printf("\r\nPress RETURN to continue");
			fflush(stdout);
			fgets(buf, PATHSIZ, stdin);
		}
	}
	signal(SIGINT, oldint);
	signal(SIGQUIT, oldquit);
	vt_after_fork();
	return(SUCCESS);
}

void sigcatch(sig)
int sig;
{
	signal(sig, SIG_IGN);
	signal(sig, sigcatch);

}
