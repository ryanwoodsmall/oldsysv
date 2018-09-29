/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/fs/see_if_SVR3.c	1.2"

#include "setjmp.h"

#include "fslib.h"

int			have_SVR3	= SVR3_STARTUP_CHOICE;

static jmp_buf		env;

/**
 ** see_if_SVR3() - SEE IF WE'RE RUNNING ON SVR3 OR LATER
 **/

static void		bad_sys_call (sig)
	int			sig;
{
	signal (sig, SIG_IGN);
	longjmp (env, 1);
	/*NOTREACHED*/
}

void			see_if_SVR3 ()
{
	void			(*old_trap)();

	/*
	 * Here's where we try to see if we're running
	 * on UNIX System V Release 3.0 or later. We set a
	 * trap for an illegal system call (saving the old
	 * trap, of course), arrange for a "longjmp()" return,
	 * and issue a "getdents()" call.
	 *
	 * The "longjmp()" is needed so that we never ``return''
	 * from the trap (we do come back here, of course).
	 * It seems that (at least on the 3B2) returning from
	 * the trap causes a core dump with SIGSYS anyway, even
	 * though we though we trapped it.
	 *
	 * Note that the "getdents()" call has bogus arguments,
	 * so it should fail; but we don't care.
	 */

	old_trap = signal(SIGSYS, bad_sys_call);
	switch (setjmp(env)) {
	case 0:
		have_SVR3 = 1;	/* maybe; let's try it and see */
		getdents (-1, (char *)0, 0);
		break;
	case 1:
		have_SVR3 = 0;	/* definitely */
		break;
	}
	signal (SIGSYS, old_trap);

	return;
}
