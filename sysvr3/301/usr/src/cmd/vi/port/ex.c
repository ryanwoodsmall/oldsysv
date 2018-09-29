/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/* Copyright (c) 1981 Regents of the University of California */
#ident	"@(#)vi:port/ex.c	1.15"
#include "ex.h"
#include "ex_argv.h"
#include "ex_temp.h"
#include "ex_tty.h"

#ifdef TRACE
char	tttrace[]	= { '/','d','e','v','/','t','t','y','x','x',0 };
#endif

#define OPTIONS	'-'
#define COMMANDS '+'
#define VI	0
#define VIEW	1
#define VEDIT	2
#define EDIT	3
#define EX	4
#define EQ(a,b)	!strcmp(a,b)

char	*strrchr();

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
 * ex_unix.c		Routines for the ! command and its variations.
 *
 * ex_v*.c		Visual/open mode routines... see ex_v.c for a
 *			guide to the overall organization.
 */

/*
 * This sets the Version of ex/vi for both the exstrings file and
 * the version command (":ver"). The following comment puts the text
 * into the /usr/lib/exstrings file:
 * 					error("Version 5.3a");
 */

char *Version = "Version 5.3a";	/* variable used by ":ver" command */

/*
 * NOTE: when changing the Version number, it must be changed in the
 *  	 following files:
 *
 *			  port/READ_ME
 *			  port/ex.c  (the *two* occurrences above)
 *			  port/ex.news
 *
 *	 The number scheme is as follows: 5.3 stands for System V Release 3.0;
 *	 the letter following is an aid to the developer to determine if the
 *       /usr/lib/exstrings file is in sync with the current version of ex/vi.
 *	 Whenever the developer has altered an error message, causing a new
 * 	 version of /usr/lib/exstrings, the letter should be bumped up.
 */

/*
 * Main procedure.  Process arguments and then
 * transfer control to the main command processing loop
 * in the routine commands.  We are entered as either "ex", "edit", "vi"
 * or "view" and the distinction is made here. For edit we just diddle options;
 * for vi we actually force an early visual command.
 */

main(ac, av)
	register int ac;
	register char *av[];
{
#ifndef VMUNIX
	char *erpath = EXSTRINGS;
#endif
	extern	char	*argval;
	extern	char	argtyp;
	extern 	int	argind;
	char	*rcvname;
	char	usage[80];
	register char *cp;
	register int c;
	register int editor;
	char	*cmdnam;
	bool recov = 0;
	bool ivis;
	bool itag = 0;
	bool fast = 0;
	extern int onemt();
	extern int oncore();
	extern int verbose;
#ifdef TRACE
	register char *tracef;
#endif

	/*
	 * Immediately grab the tty modes so that we won't
	 * get messed up if an interrupt comes in quickly.
	 */
	gTTY(2);
#ifndef USG
	normf = tty.sg_flags;
#else
	normf = tty;
#endif
	ppid = getpid();
	/* Note - this will core dump if you didn't -DSINGLE in CFLAGS */
	lines = 24;
	columns = 80;	/* until defined right by setupterm */
	/*
	 * Defend against d's, v's, w's, and a's in directories of
	 * path leading to our true name.
	 */
	if ((cmdnam = strrchr(av[0], '/')) != 0)
		cmdnam++;
	else
		cmdnam = av[0];

	if (EQ(cmdnam, "vi")) {
		editor = VI;
		ivis = 1;
	} else if (EQ(cmdnam, "view")) {
			editor = VIEW;
			ivis = 1;
			value(vi_READONLY) = 1;
		} else if (EQ(cmdnam, "vedit")) {
				editor = VEDIT;
				ivis = 1;
				value(vi_NOVICE) = 1;
				value(vi_REPORT) = 1;
				value(vi_MAGIC) = 0;
				value(vi_SHOWMODE) = 1;
			} else if (EQ(cmdnam, "edit")) {
					editor = EDIT;
					value(vi_NOVICE) = 1;
					value(vi_REPORT) = 1;
					value(vi_MAGIC) = 0;
					value(vi_SHOWMODE) = 1;
				} else
					editor = EX;

#ifndef VMUNIX
	/*
	 * For debugging take files out of . if name is a.out.
	 */
	if (av[0][0] == 'a')
		erpath = tailpath(erpath);
#endif
	/*
	 * Open the error message file.
	 */
	draino();
#ifndef VMUNIX
	erfile = open(erpath+4, 0);
	if (erfile < 0) {
		erfile = open(erpath, 0);
	}
#endif
	pstop();

	/*
	 * Initialize interrupt handling.
	 */
	oldhup = signal(SIGHUP, SIG_IGN);
	if (oldhup == SIG_DFL)
		signal(SIGHUP, onhup);
	oldquit = signal(SIGQUIT, SIG_IGN);
	ruptible = signal(SIGINT, SIG_IGN) == SIG_DFL;
	if (signal(SIGTERM, SIG_IGN) == SIG_DFL)
		signal(SIGTERM, onhup);
	if (signal(SIGEMT, SIG_IGN) == SIG_DFL)
		signal(SIGEMT, onemt);
	signal(SIGILL, oncore);
	signal(SIGTRAP, oncore);
	signal(SIGIOT, oncore);
	signal(SIGFPE, oncore);
	signal(SIGBUS, oncore);
	signal(SIGSEGV, oncore);
	signal(SIGPIPE, oncore);

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
	 * Process flag arguments based on editor, argument type, and command.
	 * Show proper usage messages and allow proper options/commands
	 * based on defines.
	 */
	switch (editor) {

		case VI:
		case VIEW:
		case VEDIT:
#ifdef TRACE
		while ((c = tmpgetopt(ac,av,"VS:Lc:T;t:r;lw:xR","+:")) != EOF)
#else
		while ((c = tmpgetopt(ac,av,"VLc:t:r;lw:xR","+:")) != EOF)
#endif
		{
			switch(argtyp) {
			  case OPTIONS:
				  switch(c) {
					case 'R':
						value(vi_READONLY) = 1;
						break;
#ifdef TRACE
					case 'T':
						if (argval == NULL)
						  tracef = "trace";
						else 
					case 'S':
						{
						     tracef = tttrace;
						     strcat(tracef,argval);	
					        }		
						trace = fopen(tracef,"w");
#define	tracebuf	NULL
						if (trace == NULL)
							printf("Trace create error\n");
						else
							setbuf(trace,tracbuf);
						break;
#endif
					case 'c':
						firstpat = argval;
						break;
					
					case 'l':
						value(vi_LISP) = 1;
						value(vi_SHOWMATCH) = 1;
						break;

					case 'r':
						rcvname = argval;
					case 'L':
						recov++;
						break;

					case 'V':
						verbose = 1;
						break;

					case 't':
						itag = 1;
						CP(lasttag, argval);
						break;
					
					case 'w':
						defwind = 0;
						if (argval[0] == NULL)
							defwind = 3;
						else for (cp = argval; isdigit(*cp); cp++)
							defwind = 10*defwind + *cp - '0';
						break;
#ifdef CRYPT
					case 'x':
						/* 
						 * encrypted mode
						 */
						xflag = 1;
						break;
#endif
					default:
#ifdef TRACE
			sprintf(usage,"Usage: %s [-l] [-L] [-r [file]] [-t tag] [-T [-S suffix]] \n", cmdnam);
#else
			sprintf(usage,"Usage: %s [-l] [-L] [-r [file]] [-t tag]\n", cmdnam);
#endif
			write(2,usage,strlen(usage));
			sprintf(usage, "           [-R] [-V] [-wn] [-x] [+cmd | -c cmd] file...\n");
			write(2,usage,strlen(usage));
			exit(1);
			}
			break;   	/* default */

			case COMMANDS:
				switch(c) {
					case '\0':
						firstpat = argval;
						break;
					default:
#ifdef TRACE
			sprintf(usage,"Usage: %s [-l] [-L] [-r [file]] [-t tag] [-T [-S suffix]]\n", cmdnam);
#else
			sprintf(usage,"Usage: %s [-l] [-L] [-r [file]] [-t tag]\n", cmdnam);
#endif
			write(2,usage,strlen(usage));
			sprintf(usage, "           [-R] [-V] [-wn] [-x] [+cmd | -c cmd] file...\n");
			write(2,usage,strlen(usage));
			exit(1);

			}
    		  }
	}
	break;
	case EDIT:
#ifdef TRACE
		while ((c = tmpgetopt(ac,av,"S:LT;r;","")) != EOF)
#else
		while ((c = tmpgetopt(ac,av,"Lr;","")) != EOF)
#endif
		{
		  switch (c) {
			case 'r':
				rcvname = argval;
			case 'L':
				recov++;
				break;

#ifdef TRACE
			case 'T':
				if (argval == NULL)
				  tracef = "trace";
				else 
			case 'S':
				{
				     tracef = tttrace;
				     strcat(tracef,argval);	
			        }		
				trace = fopen(tracef,"w");
				if (trace == NULL)
					printf("Trace create error\n");
				else
					setbuf(trace,tracbuf);
				break;
#endif
			default:
#ifdef TRACE
				sprintf(usage,"Usage: %s [-L] [-r [file]] [-T [-S suffix]] file....\n", cmdnam);
#else
				sprintf(usage,"Usage: %s [-L] [-r [file]] file....\n", cmdnam);
#endif
				write(2, usage, strlen(usage));
				exit(1);
				}
			}
			break;
		case EX:
#ifdef TRACE
		while ((c = tmpgetopt(ac,av,"-VLsS:c:T;vt:r;Rlx","+:")) != EOF)
#else
		while ((c = tmpgetopt(ac,av,"-VLsc:vt:r;Rlx","+:")) != EOF)
#endif
		{
		switch (argtyp) {
			case OPTIONS:
			  switch (c) {
				case '\0':
				case 's':
					hush = 1;
					value(vi_AUTOPRINT) = 0;
					fast++;
					break;
				case 'R':
					value (vi_READONLY) = 1;
					break;
#ifdef TRACE
				case 'T':
					if (argval == NULL)
					  tracef = "trace";
					else 
				case 'S':
					{
					     tracef = tttrace;
					     strcat(tracef,argval);	
				        }		
					trace = fopen(tracef,"w");
#define	tracebuf	NULL
					if (trace == NULL)
						printf("Trace create error\n");
					else
						setbuf(trace,tracbuf);
					break;
#endif
				case 'c':
					firstpat = argval;
					break;
				case 'l':
					value(vi_LISP) = 1;
					value(vi_SHOWMATCH) = 1;
					break;

				case 'r':
					rcvname = argval;
				case 'L':
					recov++;
					break;

				case 'V':
					verbose = 1;
					break;

				case 't':
					itag = 1;
					CP(lasttag, argval);
					break;
					
				case 'v':
					ivis = 1;
					break;
#ifdef CRYPT
				case 'x':
					/* 
					 * encrypted mode
					 */
					xflag = 1;
					break;
#endif
				default:
#ifdef TRACE
			sprintf(usage,"Usage: %s [- | -s] [-l] [-L] [-R] [-r [file]] [-t tag] [-T [-S suffix]]\n", cmdnam);
#else
			sprintf(usage,"Usage: %s [- | -s] [-l] [-L] [-R] [-r [file]] [-t tag]\n", cmdnam);
#endif
			write(2,usage,strlen(usage));
			sprintf(usage, "       [-v] [-V] [-x] [+cmd | -c cmd] file...\n");
			write(2,usage,strlen(usage));
			exit(1);
			}
			break;   	/* default */

			case COMMANDS:
				switch(c) {
					case '\0':
						firstpat = argval;
						break;
					default:
#ifdef TRACE
						sprintf(usage,"Usage: %s [- | -s] [-l] [-L] [-R] [-r [file]] [-t tag] [-T [-S suffix]]\n", cmdnam);
#else
						sprintf(usage,"Usage: %s [- | -s] [-l] [-L] [-R] [-r [file]] [-t tag]\n", cmdnam);
#endif
						write(2, usage, strlen(usage));
						sprintf(usage, "       [-v] [-V] [-x] [+cmd | -c cmd] file...\n");
						write(2, usage, strlen(usage));
						exit(1);
						}
					}
				}
		}
	ac -= argind;
	av  = &av[argind];
					
#ifdef SIGTSTP
	if (!hush && signal(SIGTSTP, SIG_IGN) == SIG_DFL)
		signal(SIGTSTP, onsusp), dosusp++;
#endif

#ifdef CRYPT
	if(xflag) {
		if (recov)
			write(2, "Encryption key for this editing session\n", 40);
		if ((kflag = run_setkey(perm, getpass("Enter key:"))) == -1) {
			kflag = 0;
			xflag = 0;
			smerror("Encryption facility not available\n");
		}
	}
#endif

	/*
	 * If we are doing a recover and no filename
	 * was given, then execute an exrecover command with
	 * the -r option to type out the list of saved file names.
	 * Otherwise set the remembered file name to the first argument
	 * file name so the "recover" initial command will find it.
	 */
	if (recov) {
		if (ac == 0 && (rcvname == NULL || *rcvname == NULL)) {
			ppid = 0;
			setrupt();
			execlp(EXRECOVER, "exrecover", "-r", 0);
			filioerr(EXRECOVER);
			exit(++errcnt);
		}
		if (rcvname && *rcvname)
			CP(savedfile, rcvname);
		else
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
	if (setexit() == 0) {
		setrupt();
		intty = isatty(0);
		value(vi_PROMPT) = intty;
		if (cp = getenv("SHELL"))
			CP(shell, cp);
		if (fast)
			setterm("dumb");
		else {
			gettmode();
			cp = getenv("TERM");
			if (cp == NULL || *cp == '\0')
				cp = "unknown";
			setterm(cp);
		}
	}
	if (setexit() == 0 && !fast) {
		if ((globp = getenv("EXINIT")) && *globp)
			commands(1,1);
		else {
			globp = 0;
			if ((cp = getenv("HOME")) != 0 && *cp)
				source(strcat(strcpy(genbuf, cp), "/.exrc"), 1);
		}
		/*
		 * Allow local .exrc too.  This loses if . is $HOME,
		 * but nobody should notice unless they do stupid things
		 * like putting a version command in .exrc.  Besides,
		 * they should be using EXINIT, not .exrc, right?
		 */
		source(".exrc", 1);
	}
	init();	/* moved after prev 2 chunks to fix directory option */

	/*
	 * Initial processing.  Handle tag, recover, and file argument
	 * implied next commands.  If going in as 'vi', then don't do
	 * anything, just set initev so we will do it later (from within
	 * visual).
	 */
	if (setexit() == 0) {
		if (recov)
			globp = "recover";
		else if (itag)
			globp = ivis ? "tag" : "tag|p";
		else if (argc)
			globp = "next";
		if (ivis)
			initev = globp;
		else if (globp) {
			inglobal = 1;
			commands(1, 1);
			inglobal = 0;
		}
	}

	/*
	 * Vi command... go into visual.
	 */
	if (ivis) {
		/*
		 * Don't have to be upward compatible 
		 * by starting editing at line $.
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
	seenprompt = 1;
	ungetchar(0);
	globp = 0;
	initev = 0;
	setlastchar('\n');
	setexit();
	commands(0, 0);
	cleanup(1);
	exit(errcnt);
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
	for (i = 0; i <= 'z'-'a'+1; i++)
		names[i] = 1;
	anymarks = 0;
#ifdef CRYPT
        if(xflag) {
                xtflag = 1;
                if (makekey(tperm) != 0) {
			xtflag = 0;
			smerror("Warning--Cannot encrypt temporary buffer\n");
        	}
	}
#endif
}

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
