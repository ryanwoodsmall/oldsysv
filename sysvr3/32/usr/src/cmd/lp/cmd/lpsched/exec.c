/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpsched/exec.c	1.13"

#include "limits.h"
#include "varargs.h"

#include "lpsched.h"

#define Done(EC,ERRNO)	done(((EC) << 8),ERRNO)

static int		Child,
			Waited_Child,
			slot;

static long		key;

static void		sigtrap(),
			done(),
			cool_heels();

#if	!defined(I_AM_LPSCHED_JR)

static int		do_undial;

static void		addenv(),
			Fork(),
			trap_fault_signals(),
			ignore_fault_signals();

static char		argbuf[ARG_MAX];

#if	defined(LOST_LOCK)
static void		relock ()
{
	struct flock		l;

	l.l_type = F_WRLCK;
	l.l_whence = 1;
	l.l_start = 0;
	l.l_len = 0;
	(void)Fcntl (fileno(LockFp), F_SETLK, &l);
	return;
}
#endif

/**
 ** exec() - FORK AND EXEC CHILD PROCESS
 **/

/*VARARGS0*/
int			exec (va_alist)
	va_dcl
{
	va_list			args;

	int			type,
				i,
				procuid,
				procgid,
				ret;

	char			*cp,
				**listp,
				*infile,
				*outfile,
				*errfile,
				**file_list,
				*cpi,
				*lpi,
				*width,
				*length;

	PSTATUS			*printer;

	RSTATUS			*request;

	FSTATUS			*form;

	EXEC			*ep;

	PWSTATUS		*pwheel;


	va_start(args);

	type = va_arg(args, int);
	switch (type) {

	case EX_INTERF:
		printer = va_arg(args, PSTATUS *);
		request = printer->request;
		ep = printer->exec;
		break;

	case EX_SLOWF:
	case EX_NOTIFY:
		request = va_arg(args, RSTATUS *);
		ep = request->exec;
		break;

	case EX_ALERT:
		printer = va_arg(args, PSTATUS *);
		if (!(printer->printer->fault_alert.shcmd)) {
			errno = EINVAL;
			return(-1);
		}
		ep = printer->alert->exec;
		break;

	case EX_PALERT:
		pwheel = va_arg(args, PWSTATUS *);
		ep = pwheel->alert->exec;
		break;

	case EX_FALERT:
		form = va_arg(args, FSTATUS *);
		ep = form->alert->exec;
		break;

	default:
		errno = EINVAL;
		return(-1);

	}
	va_end(args);

	if (ep->pid > 0) {
		errno = EBUSY;
		return(-1);
	}

	ep->flags = 0;

	key = ep->key = getkey();
	slot = ep - Exec_Table;

	switch ((ep->pid = fork())) {

	case -1:
#if	defined(LOST_LOCK)
		relock ();
#endif
		return(-1);

	case 0:
		break;

	default:
		switch(type) {

		case EX_INTERF:
			request->request->outcome |= RS_PRINTING;
			break;

		case EX_NOTIFY:
			request->request->outcome |= RS_NOTIFYING;
			break;

		case EX_SLOWF:
			request->request->outcome |= RS_FILTERING;
			request->request->outcome &= ~RS_REFILTER;
			break;

		}
		return(0);

	}

	for (i = 0; i < NSIG; i++)
		(void)signal (i, SIG_DFL);
	(void)signal (SIGALRM, SIG_IGN);
	(void)signal (SIGTERM, sigtrap);
	
	for (i = 0; i < OpenMax; i++)
		Close(i);

	setpgrp();

	{				  /* 2 ^ 32 */
		static char	*buf	= "4294967296";

		sprintf (buf, "%ld", key);
		addenv ("SPOOLER_KEY", buf);
#if	defined(LPSCHED_JR)
		sprintf (buf, "%ld", slot);
		addenv ("SPOOLER_SLOT", buf);
#endif
	}

#if	defined(DEBUG)
	addenv ("LPDEBUG", (debug? "1" : "0"));
#endif

	/*
	 * Open the standard input, standard output, and standard error.
	 */
	switch (type) {

	case EX_SLOWF:
	case EX_INTERF:
		/*
		 * stdin:  /dev/null
		 * stdout: /dev/null (EX_SLOWF), printer port (EX_INTERF)
		 * stderr: req#
		 */

		infile = 0;
		outfile = 0;

		if (!(errfile = makereqerr(request->secure->req_id)))
			Done (EXEC_EXIT_NOMEM, ENOMEM);

		break;

	case EX_NOTIFY:
		/*
		 * stdin:  req#
		 * stdout: /dev/null
		 * stderr: /dev/null
		 */
		if (!(infile = makereqerr(request->secure->req_id)))
			Done (EXEC_EXIT_NOMEM, ENOMEM);

		outfile = 0;
		errfile = 0;

		break;

	case EX_ALERT:
	case EX_FALERT:
	case EX_PALERT:
		/*
		 * stdin:  /dev/null
		 * stdout: /dev/null
		 * stderr: /dev/null
		 */
		infile = 0;
		outfile = 0;
		errfile = 0;
		break;

	}

	if (infile) {
		if (Open(infile, O_RDONLY) == -1)
			Done (EXEC_EXIT_NOPEN, errno);
	} else {
		if (Open("/dev/null", O_RDONLY) == -1)
			Done (EXEC_EXIT_NOPEN, errno);
	}

	if (outfile) {
		if (Open(outfile, O_CREAT|O_TRUNC|O_WRONLY, 0600) == -1)
			Done (EXEC_EXIT_NOPEN, errno);
	} else {
		/*
		 * If EX_INTERF, this is still needed to cause the
		 * standard error channel to be #2.
		 */
		if (Open("/dev/null", O_WRONLY) == -1)
			Done (EXEC_EXIT_NOPEN, errno);
	}

	if (errfile) {
		if (Open(errfile, O_CREAT|O_TRUNC|O_WRONLY, 0600) == -1)
			Done (EXEC_EXIT_NOPEN, errno);
	} else {
		if (Open("/dev/null", O_WRONLY) == -1)
			Done (EXEC_EXIT_NOPEN, errno);
	}

	switch (type) {

	case EX_INTERF:
		/*
		 * Opening a ``port'' can be dangerous to our health:
		 *
		 *	- Hangups can occur if the line is dropped.
		 *	- The printer may send an interrupt.
		 *	- A FIFO may be closed, generating SIGPIPE.
		 *
		 * We catch these so we can complain nicely.
		 */
		trap_fault_signals ();

		(void)Close (1);

		if (printer->printer->dial_info) {
			ret = open_dialup(printer->printer);
			if (ret == 0)
				do_undial = 1;
		} else {
			ret = open_direct(printer->printer);
			do_undial = 0;
		}
		if (ret != 0)
			Done (ret, errno);
			
		procuid = request->secure->uid;
		procgid = request->secure->gid;

		if (!(request->request->outcome & RS_FILTERED))
			file_list = request->request->file_list;

		else {
			register int		count	= 0;
			register char		*num	= "999999",
						*prefix;

			prefix = makestr(
				Lp_Temp,
				"/F",
				getreqno(request->secure->req_id),
				"-",
				(char *)0
			);

			file_list = (char **)malloc(
				(lenlist(request->request->file_list) + 1)
			      * sizeof(char *)
			);
			if (!file_list)
				Done (EXEC_EXIT_NOMEM, ENOMEM);

			for (
				listp = request->request->file_list;
				*listp;
				listp++
			) {
				sprintf (num, "%d", count + 1);
				file_list[count] = makestr(
					prefix,
					num,
					(char *)0
				);
				if (!file_list[count])
					Done (EXEC_EXIT_NOMEM, ENOMEM);
				count++;
			}
			file_list[count] = 0;
		}

		if (printer->printer->printer_type)
			addenv("TERM", printer->printer->printer_type);

		if (!(printer->printer->daisy)) {
			register char	*chset = 0;
			register char	*csp;

			if (
				request->form
			     && request->form->form->chset
			     && request->form->form->mandatory
			     && !STREQU(NAME_ANY, request->form->form->chset)
			)
				chset = request->form->form->chset;

			else if (
				request->request->charset
			     && !STREQU(NAME_ANY, request->request->charset)
			)
				chset = request->request->charset;

			if (chset) {
				csp = search_cslist(
					chset,
					printer->printer->char_sets
				);

				/*
				 * The "strtok()" below wrecks the string
				 * for future use, but this is a child
				 * process where it won't be needed again.
				 */
				addenv (
					"CHARSET",
					(csp? strtok(csp, "=") : chset)
				);
			}
		}

		if (request->fast)
			addenv("FILTER", request->fast);

		sprintf (
			argbuf,
			"%s/%s %s %s%s%s \"%s\" %d \"",
			Lp_A_Interfaces,
			printer->printer->name,
			request->secure->req_id,
			NB(request->secure->system),
			(request->secure->system? "!" : ""),
			request->request->user,
			NB(request->request->title),
			request->copies
		);

		if (!request->form) {
			if (printer->cpi) {
				strcat (argbuf, " cpi=");
				strcat (argbuf, printer->cpi);
			}
			if (printer->lpi) {
				strcat (argbuf, " lpi=");
				strcat (argbuf, printer->lpi);
			}
			if (printer->plen) {
				strcat (argbuf, " length=");
				strcat (argbuf, printer->plen);
			}
			if (printer->pwid) {
				strcat (argbuf, " width=");
				strcat (argbuf, printer->pwid);
			}
		}

		strcat (argbuf, " ");

		strcat (argbuf, NB(request->request->options));

		if (request->request->actions & ACT_RAW)
			strcat (argbuf, " stty=raw");
		else if (printer->printer->stty) {
			strcat (argbuf, " stty='");
			strcat (argbuf, printer->printer->stty);
			strcat (argbuf, "'");
		}

		if (request->form) {
			if (request->form->cpi) {
				strcat (argbuf, " cpi=");
				strcat (argbuf, request->form->cpi);
			}
			if (request->form->lpi) {
				strcat (argbuf, " lpi=");
				strcat (argbuf, request->form->lpi);
			}
			if (request->form->plen) {
				strcat (argbuf, " length=");
				strcat (argbuf, request->form->plen);
			}
			if (request->form->pwid) {
				strcat (argbuf, " width=");
				strcat (argbuf, request->form->pwid);
			}
		}

		strcat (argbuf, "\"");

		for (listp = file_list; *listp; listp++) {
			strcat (argbuf, " ");
			strcat (argbuf, *listp);
		}

		(void)chfiles (file_list, procuid, procgid);

		break;


	case EX_SLOWF:
		if (request->slow)
			addenv("FILTER", request->slow);

		procuid = request->secure->uid;
		procgid = request->secure->gid;

		cp = _alloc_files(
			lenlist(request->request->file_list),
			getreqno(request->secure->req_id),
			procuid,
			procgid
		);

		sprintf (
			argbuf,
			"%s %s/%s %s",
			Lp_Slow_Filter,
			Lp_Temp,
			cp,
			sprintlist(request->request->file_list)
		);

		(void)chfiles (request->request->file_list, procuid, procgid);
		break;

	case EX_ALERT:
		procuid = Lp_Uid;
		procgid = Lp_Gid;
		(void)Chown (printer->alert->msgfile, procuid, procgid);

		sprintf (
			argbuf,
			"%s/%s/%s %s",
			Lp_A_Printers,
			printer->printer->name,
			ALERTSHFILE,
			printer->alert->msgfile
		);
		break;

	case EX_PALERT:
		procuid = Lp_Uid;
		procgid = Lp_Gid;
		(void)Chown (pwheel->alert->msgfile, procuid, procgid);

		sprintf (
			argbuf,
			"%s/%s/%s %s",
			Lp_A_PrintWheels,
			pwheel->pwheel->name,
			ALERTSHFILE,
			pwheel->alert->msgfile
		);
		break;

	case EX_FALERT:
		procuid = Lp_Uid;
		procgid = Lp_Gid;
		(void)Chown (form->alert->msgfile, procuid, procgid);

		sprintf (
			argbuf,
			"%s/%s/%s %s",
			Lp_A_Forms,
			form->form->name,
			ALERTSHFILE,
			form->alert->msgfile
		);
		break;

	case EX_NOTIFY:
		if (request->request->alert) {

			procuid = request->secure->uid;
			procgid = request->secure->gid;

			strcpy (argbuf, request->request->alert);

		} else {

			procuid = Lp_Uid;
			procgid = Lp_Gid;

			if (
				(request->request->actions & ACT_WRITE)
			     && !request->secure->system
			)
				sprintf (
					argbuf,
					"%s %s || %s %s",
					BINWRITE,
					request->request->user,
					BINMAIL,
					request->request->user
				);
			else
				sprintf (
					argbuf,
					"%s %s%s%s",
					BINMAIL,
					NB(request->secure->system),
					(request->secure->system? "!" : ""),
					request->request->user
				);

		}
		break;

	}

#if	defined(DEBUG)
	if (debug) {
		FILE			*fp	= open_logfile("exec");

		time_t			now = time((time_t *)0);

		char			buffer[BUFSIZ];

		extern char		*ctime();


		if (fp) {
			setbuf (fp, buffer);
			fprintf (
				fp,
			"EXEC: %24.24s uid %d gid %d pid %d ppid %d\n",
				ctime(&now),
				procuid,
				procgid,
				getpid(),
				getppid()
			);
			fprintf (fp, "      SHELL=%s\n", SHELL);
			fprintf (fp, "      %s\n", argbuf);
			close_logfile (fp);
		}
	}
#endif

	Fork (ep);
	/* only the child returns */
	
	setgid (procgid);
	setuid (procuid);

	/*
	 * The shell doesn't allow the "trap" builtin to set a trap
	 * for a signal ignored when the shell is started. Thus, don't
	 * turn off signals in the last child!
	 */

	execl (SHELL, SHELL, "-c", argbuf, 0);
	Done (EXEC_EXIT_NEXEC, errno);
	/*NOTREACHED*/
}

/**
 ** addenv() - ADD A VARIABLE TO THE ENVIRONMENT
 **/

static void		addenv (name, value)
	char			*name,
				*value;
{
	register char		*cp;

	if ((cp = makestr(name, "=", value, (char *)0)))
		putenv (cp);
	return;
}

/**
 ** Fork() - FORK SECOND CHILD AND WAIT FOR IT
 **/

static void		Fork (ep)
	register EXEC		*ep;
{
	switch ((Child = fork())) {

	case -1:
		Done (EXEC_EXIT_NFORK, errno);
		/*NOTREACHED*/
					
	case 0:
		return;
					
	default:
		/*
		 * Delay calling "ignore_fault_signals()" as long
		 * as possible, to give the child a chance to exec
		 * the interface program and turn on traps.
		 */

#if	defined(MALLOC_3X)
		/*
		 * Reset the allocated space at this point, so the
		 * the process size is reduced. This can be dangerous!
		 */
		if (!do_undial)
			/* rstalloc() */ ;
#endif

#if	defined(LPSCHED_JR)
		/*
		 * Wait for the child with a tiny process.
		 */
		if (!do_undial) {
			char			*s_Child = "99999",
						*type,
						*name;

			switch (ep->type) {

			case EX_INTERF:
				type = "PRINTER";
				name = ep->ex.printer->printer->name;
				break;

			case EX_SLOWF:
				type = "FILTER";
				name = ep->ex.request->secure->req_id;
				break;

			case EX_NOTIFY:
				type = "NOTIFY";
				name = ep->ex.request->secure->req_id;
				break;

			case EX_ALERT:
				type = "FAULT-ALERT";
				name = ep->ex.printer->printer->name;
				break;

			case EX_PALERT:
				type = "PRINTWHEEL-ALERT";
				name = ep->ex.pwheel->pwheel->name;
				break;

			case EX_FALERT:
				type = "FORM-ALERT";
				name = ep->ex.form->form->name;
				break;

			}

			sprintf (argbuf, "(LPSCHED) %s %s PID", type, name);
			sprintf (s_Child, "%d", Child);

			/*
			 * At this point we are vulnerable to a SIGTERM
			 * from the parent (the Spooler), because the
			 * handling for SIGTERM is changed to SIG_DFL
			 * for the execl. We have no choice, though:
			 * Changing it to SIG_IGN may cause us to never
			 * die! With SIGTERM changed to SIG_DFL, we will
			 * get killed and eventually the Spooler will
			 * notice we've disappeared. This is why with
			 * LPSCHED_JR defined we must also define
			 * CHECK_CHILDREN.
			 *
			 * A worse problem is turning off the printer
			 * fault signals too early, before the child
			 * process can turn on traps. If a fault occurs
			 * when no process is ready, we don't detect it.
			 */

			ignore_fault_signals ();
			execl (Lp_Lpsched_Jr, argbuf, s_Child, (char *)0);
		}
#endif
		cool_heels ();
		/*NOTREACHED*/

	}
}

#else	/* !defined(I_AM_LPSCHED_JR) */

extern char		*getenv();

extern int		atoi();

extern long		atol();

#if	defined(DEBUG)
int			debug	= 0;
#endif

int			main (argc, argv)
	int			argc;
	char			*argv[];
{
	Child = atoi(argv[1]);

	signal (SIGTERM, sigtrap);

	cool_heels ();
	/*NOTREACHED*/
}

#endif	/* !defined(I_AM_LPSCHED_JR) */

/**
 ** cool_heels() - WAIT FOR CHILD TO "DIE"
 **/

static void		cool_heels ()
{
	int			status;


#if	!defined(I_AM_LPSCHED_JR)
	/*
	 * At this point our only job is to wait for the child process.
	 * If we hang out for a bit longer, that's okay.
	 * By delaying before turning off the fault signals,
	 * we increase the chance that the child process has completed
	 * its exec and has turned on the fault traps. Nonetheless,
	 * we can't guarantee a zero chance of missing a fault.
	 * (We don't want to keep trapping the signals because the
	 * interface program is likely to have a better way to handle
	 * them; this process provides only rudimentary handling.)
	 *
	 * Note that on a very busy system, or with a very fast interface
	 * program, the tables could be turned: Our sleep below (coupled
	 * with a delay in the kernel scheduling us) may cause us to
	 * detect the fault instead of the interface program.
	 *
	 * What we need is a way to synchronize with the child process.
	 */
	sleep (1);
	ignore_fault_signals ();
#endif

	Waited_Child = 0;
	while ((Waited_Child = wait(&status)) != Child)
		;
			
	if (
		EXITED(status) > EXEC_EXIT_USER
	     && EXITED(status) != EXEC_EXIT_FAULT
	)
		Done (EXEC_EXIT_EXIT, EXITED(status));

	done (status, 0);	/* Don't use Done() */
	/*NOTREACHED*/
}


/**
 ** trap_fault_signals() - TRAP SIGNALS THAT CAN OCCUR ON PRINTER FAULT
 ** ignore_fault_signals() - IGNORE SAME
 **/

#if	!defined(I_AM_LPSCHED_JR)

static void		trap_fault_signals ()
{
	signal (SIGHUP, sigtrap);
	signal (SIGINT, sigtrap);
	signal (SIGQUIT, sigtrap);
	signal (SIGPIPE, sigtrap);
	return;
}

static void		ignore_fault_signals ()
{
	signal (SIGHUP, SIG_IGN);
	signal (SIGINT, SIG_IGN);
	signal (SIGQUIT, SIG_IGN);
	signal (SIGPIPE, SIG_IGN);
	return;
}

#endif

/**
 ** sigtrap() - TRAP VARIOUS SIGNALS
 **/

static void		sigtrap (sig)
	int			sig;
{
	signal (sig, SIG_IGN);
	switch (sig) {

#if	!defined(I_AM_LPSCHED_JR)
	case SIGHUP:
		Done (EXEC_EXIT_HUP, 0);
		/*NOTREACHED*/

	case SIGQUIT:
	case SIGINT:
		Done (EXEC_EXIT_INTR, 0);
		/*NOTREACHED*/

	case SIGPIPE:
		Done (EXEC_EXIT_PIPE, 0);
		/*NOTREACHED*/
#endif

	case SIGTERM:
		/*
		 * If we were killed with SIGTERM, it should have been
		 * via the Spooler who should have killed the entire
		 * process group. We have to wait for the children,
		 * since we're their parent, but WE MAY HAVE WAITED
		 * FOR THEM ALREADY (in cool_heels()).
		 */
		if (Child != Waited_Child) {
			register int		cpid;

			while (
				(cpid = wait((int *)0)) != Child
			     && (cpid != -1 || errno != ECHILD)
			)
				;
		}

		/*
		 * We can't rely on getting SIGTERM back in the wait()
		 * above, because, for instance, some shells trap SIGTERM
		 * and exit instead. Thus we force it.
		 */
		done (SIGTERM, 0);	/* Don't use Done() */
		/*NOTREACHED*/
	}
}

/**
 ** done() - TELL SPOOLER THIS CHILD IS DONE
 **/

static void		done (status, err)
	int			status,
				err;
{
#if	defined(I_AM_LPSCHED_JR)
	key = atol(getenv("SPOOLER_KEY"));
	slot = atoi(getenv("SPOOLER_SLOT"));
# if	defined(DEBUG)
	debug = atoi(getenv("LPDEBUG"));
# endif
#endif

#if	defined(DEBUG)
	if (debug) {
		FILE			*fp	= open_logfile("exec");

		time_t			now	= time((time_t *)0);

		char			buffer[BUFSIZ];

		extern char		*ctime();


		if (fp) {
			setbuf (fp, buffer);
			fprintf (
				fp,
				"DONE: %24.24s pid %d exited ",
				ctime(&now),
				getpid()
			);
			fprintf (
				fp,
				(EXITED(status) == -1? "%d" : "%04o"),
				EXITED(status)
			);
			fprintf (
				fp,
				" killed %d errno %d\n",
				KILLED(status),
				err
			);
			close_logfile (fp);
		}
	}
#endif

#if	!defined(I_AM_LPSCHED_JR)
	if (do_undial)
		undial (1);
#endif

	sendme (S_CHILD_DONE, key, slot, status, err);

	exit (0);
	/*NOTREACHED*/
}
