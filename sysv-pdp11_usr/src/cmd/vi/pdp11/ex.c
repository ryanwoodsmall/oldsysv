/* Copyright (c) 1979 Regents of the University of California */
#include "ex.h"
#include "ex_argv.h"
#include "ex_temp.h"
#include "ex_tty.h"

#ifdef TRACE
char	tttrace[]	= { '/','d','e','v','/','t','t','y','x','x',0 };
#endif

/*
 * The code for ex is divided as follows:
 *
 * ex.c			Entry point and routines handling interrupt, hangup
 *			signals; initialization code.
 *
 * ex_addr.c		Address parsing routines for command mode decoding.
 *			Routines to set and check address ranges on commands.
 *
 * ex_cmds.c		Command mode command decoding.
 *
 * ex_cmds2.c		Subroutines for command decoding and processing of
 *			file names in the argument list.  Routines to print
 *			messages and reset state when errors occur.
 *
 * ex_cmdsub.c		Subroutines which implement command mode functions
 *			such as append, delete, join.
 *
 * ex_data.c		Initialization of options.
 *
 * ex_get.c		Command mode input routines.
 *
 * ex_io.c		General input/output processing: file i/o, unix
 *			escapes, filtering, source commands, preserving
 *			and recovering.
 *
 * ex_put.c		Terminal driving and optimizing routines for low-level
 *			output (cursor-positioning); output line formatting
 *			routines.
 *
 * ex_re.c		Global commands, substitute, regular expression
 *			compilation and execution.
 *
 * ex_set.c		The set command.
 *
 * ex_subr.c		Loads of miscellaneous subroutines.
 *
 * ex_temp.c		Editor buffer routines for main buffer and also
 *			for named buffers (Q registers if you will.)
 *
 * ex_tty.c		Terminal dependent initializations from termcap
 *			data base, grabbing of tty modes (at beginning
 *			and after escapes).
 *
 * ex_v*.c		Visual/open mode routines... see ex_v.c for a
 *			guide to the overall organization.
 */

/*
 * Main procedure.  Process arguments and then
 * transfer control to the main command processing loop
 * in the routine commands.  We are entered as either "ex", "edit" or "vi"
 * and the distinction is made here.  Actually, we are "vi" if
 * there is a 'v' in our name, and "edit" if there is a 'd' in our
 * name.  For edit we just diddle options; for vi we actually
 * force an early visual command, setting the external initev so
 * the q command in visual doesn't give command mode.
 */
main(ac, av)
	register int ac;
	register char *av[];
{
	char *erpath = EXSTRINGS;
	register char *cp;
	register int c;
	bool recov = 0;
	bool ivis;
	bool itag = 0;
	bool fast = 0;
#ifdef TRACE
	register char *tracef;
#endif

	/*
	 * Immediately grab the tty modes so that we wont
	 * get messed up if an interrupt comes in quickly.
	 */
	gTTY(1);
#ifndef USG3TTY
	normf = tty.sg_flags;
#else
	normf = tty;
#endif
	ppid = getpid();

	/*
	 * If a 'd' in our name, then set options for edit.
	 */
	ivis = any('v', av[0]);	/* vi */
#ifdef RDONLY
	if (any('w', av[0]))	/* view */
		value(READONLY) = 1;
#endif
#ifdef EX
	if (any('d', av[0])) {	/* edit */
		value(OPEN) = 0;
		value(REPORT) = 1;
		value(MAGIC) = 0;
	}
#endif

	/*
	 * Open the error message file.
	 */
	draino();
	erfile = open(erpath, 0);
	pstop();

	/*
	 * Initialize interrupt handling.
	 */
	/* oldhup = signal(SIGHUP, SIG_IGN); 
	if (oldhup == SIG_DFL)
		signal(SIGHUP, onhup); */
	signal(SIGHUP, onhup);
	oldquit = signal(SIGQUIT, SIG_IGN);
	ruptible = signal(SIGINT, SIG_IGN) == SIG_DFL;
	if (signal(SIGTERM, SIG_IGN) == SIG_DFL)
		signal(SIGTERM, onhup);

	/*
	 * Initialize end of core pointers.
	 * Normally we avoid breaking back to fendcore after each
	 * file since this can be expensive (much core-core copying).
	 * If your system can scatter load processes you could do
	 * this as ed does, saving a little core, but it will probably
	 * not often make much difference.
	 */
	fendcore = (line *) sbrk(0);
	endcore = fendcore - 2;

	/*
	 * Process flag arguments.
	 */
	ac--, av++;
	while (ac && av[0][0] == '-') {
		c = av[0][1];
		switch (c) {
#ifdef EX
		case 0:
			hush = 1;
			value(AUTOPRINT) = 0;
			fast++;
#endif

#ifdef TRACE
		case 'T':
			if (av[0][2] == 0)
				tracef = "trace";
			else {
				tracef = tttrace;
				tracef[8] = av[0][2];
				if (tracef[8])
					tracef[9] = av[0][3];
				else
					tracef[9] = 0;
			}
			trace = fopen(tracef, "w");
			if (trace == NULL)
				printf("Trace create error\n");
			setbuf(trace, tracbuf);
			break;

#endif

#ifdef RDONLY
		case 'R':
			value(READONLY) = 1;
			break;
#endif

#ifdef LISPCODE
		case 'l':
			value(LISP) = 1;
			value(SHOWMATCH) = 1;
			break;
#endif

		case 'r':
			recov++;
			break;

#ifdef TAGSCODE
		case 't':
			if (ac > 1 && av[1][0] != '-') {
				ac--, av++;
				itag = 1;
				/* BUG: should check for too long tag. */
				CP(lasttag, av[0]);
			}
			break;
#endif

		case 'v':
			ivis = 1;
			break;

		default:
			smerror("Unknown option %s\n", av[0]);
			break;
		}
		ac--, av++;
	}
	if (ac && av[0][0] == '+') {
		firstpat = &av[0][1];
		ac--, av++;
	}

	/*
	 * If we are doing a recover and no filename
	 * was given, then execute an exrecover command with
	 * the -r option to type out the list of saved file names.
	 * Otherwise set the remembered file name to the first argument
	 * file name so the "recover" initial command will find it.
	 */
	if (recov) {
		if (ac == 0) {
			ppid = 0;
			setrupt();
			execl(EXRECOVER, "exrecover", "-r", 0);
			filioerr(EXRECOVER);
			exit(1);
		}
		CP(savedfile, *av++), ac--;
	}

	/*
	 * Initialize the argument list.
	 */
	argv0 = av;
	argc0 = ac;
	args0 = av[0];
	erewind();

	/*
	 * Initialize a temporary file (buffer) and
	 * set up terminal environment.  Read user startup commands.
	 */
	init();
	if (setexit() == 0) {
		setrupt();
		intty = isatty(0);
		value(PROMPT) = intty;
		if (cp = getenv("SHELL"))
			CP(shell, cp);
		if (fast || !intty)
			setterm("dumb");
		else {
			gettmode();
			if ((cp = getenv("TERM")) != 0 && *cp)
				setterm(cp);
		}
	}
	if (setexit() == 0 && !fast && intty)
		if ((globp = getenv("EXINIT")) && *globp)
			commands(1, 1);
#ifdef SOURCECMD
		else if ((cp = getenv("HOME")) != 0 && *cp)
			source(strcat(strcpy(genbuf, cp), "/.exrc"), 1);
#endif

	/*
	 * Initial processing.  Handle tag, recover, and file argument
	 * implied next commands.  If going in as 'vi', then don't do
	 * anything, just set initev so we will do it later (from within
	 * visual).
	 */
	if (setexit() == 0) {
		if (recov)
			globp = "recover";
#ifdef TAGSCODE
		else if (itag)
			globp = ivis ? "tag" : "tag|p";
#endif
		else if (argc)
			globp = "next";
		if (ivis)
			initev = globp;
#ifdef EX
		else if (globp) {
			inglobal = 1;
			commands(1, 1);
			inglobal = 0;
		}
#endif
	}

	/*
	 * Vi command... go into visual.
	 * Strange... everything in vi usually happens
	 * before we ever "start".
	 */
	if (ivis) {
		/*
		 * Don't have to be upward compatible with stupidity
		 * of starting editing at line $.
		 */
		if (dol > zero)
			dot = one;
		globp = "visual";
		if (setexit() == 0)
			commands(1, 1);
	}

	/*
	 * Clear out trash in state accumulated by startup,
	 * and then do the main command loop for a normal edit.
	 * If you quit out of a 'vi' command by doing Q or ^\,
	 * you also fall through to here.
	 */
	ungetchar(0);
	globp = 0;
	initev = 0;
	setlastchar('\n');
	setexit();
	commands(0, 0);
	cleanup(1);
	exit(0);
}

/*
 * Initialization, before editing a new file.
 * Main thing here is to get a new buffer (in fileinit),
 * rest is peripheral state resetting.
 */
init()
{
	register int i;

	fileinit();
	dot = zero = truedol = unddol = dol = fendcore;
	one = zero+1;
	undkind = UNDNONE;
	chng = 0;
	edited = 0;
#ifdef USG
	signal (SIGHUP, SIG_IGN);
#endif
#ifdef USG3TTY
# ifndef USG
	/* signal (SIGHUP, SIG_IGN); */
	signal(SIGHUP, onhup);
# endif
#endif
	for (i = 0; i <= 'z'-'a'+1; i++)
		names[i] = 1;
	anymarks = 0;
}

/*
 * When a hangup occurs our actions are similar to a preserve
 * command.  If the buffer has not been [Modified], then we do
 * nothing but remove the temporary files and exit.
 * Otherwise, we sync the temp file and then attempt a preserve.
 * If the preserve succeeds, we unlink our temp files.
 * If the preserve fails, we leave the temp files as they are
 * as they are a backup even without preservation if they
 * are not removed.
 */
onhup()
{

#ifdef USG
	/*
	 * USG tty driver can send multiple HUP's!!!
	 */
	signal (SIGINT, SIG_IGN);
	signal (SIGHUP, SIG_IGN);
#endif
	if (chng == 0) {
		cleanup(1);
		exit(0);
	}
	if (setexit() == 0) {
		if (preserve()) {
			cleanup(1);
			exit(0);
		}
	}
	exit(1);
}

/*
 * An interrupt occurred.  Drain any output which
 * is still in the output buffering pipeline.
 * Catch interrupts again.  Unless we are in visual
 * reset the output state (out of -nl mode, e.g).
 * Then like a normal error (with the \n before Interrupt
 * suppressed in visual mode).
 */
onintr()
{

#ifndef CBREAK
	signal(SIGINT, onintr);
#else
	signal(SIGINT, inopen ? vintr : onintr);
#endif
	draino();
	if (!inopen) {
		pstop();
		setlastchar('\n');
#ifdef CBREAK
	}
#else
	} else
		vraw();
#endif
	error("\nInterrupt" + inopen);
}

/*
 * If we are interruptible, enable interrupts again.
 * In some critical sections we turn interrupts off,
 * but not very often.
 */
setrupt()
{

	if (ruptible)
#ifndef CBREAK
		signal(SIGINT, onintr);
#else
		signal(SIGINT, inopen ? vintr : onintr);
#endif
}

preserve()
{

	synctmp();
	pid = fork();
	if (pid < 0)
		return (0);
	if (pid == 0) {
		close(0);
		dup(tfile);
		execl(EXPRESERVE, "expreserve", (char *) 0);
		exit(1);
	}
	waitfor();
	if (rpid == pid && status == 0)
		return (1);
	return (0);
}

#ifndef V6
exit(i)
	int i;
{
#ifdef TRACE
	if (trace)
		fclose(trace);
#endif

	_exit(i);
}
#endif

#ifdef ROOMTOBURN
/*
 * Return last component of unix path name p.
 */
char *
tailpath(p)
register char *p;
{
	register char *r;

	for (r=p; *p; p++)
		if (*p == '/')
			r = p+1;
	return(r);
}
#endif
