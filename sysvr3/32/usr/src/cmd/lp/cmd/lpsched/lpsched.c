/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpsched/lpsched.c	1.15"

#include "limits.h"
#include "sys/utsname.h"

#include "lpsched.h"

#include "varargs.h"
#include "sys/stat.h"

#if	defined(MALLOC_3X)
# include "malloc.h"
#endif

FILE			*LockFp		= 0;

int			Starting	= 0;
int			Shutdown	= 0;
int			DoneChildren	= 0;
int			Sig_Alrm	= 0;
int			OpenMax		= OPEN_MAX;
int			Reserve_Fds	= 0;

char			*Local_System	= 0;
char			*SHELL		= 0;

ushort			Lp_Gid;
ushort			Lp_Uid;

#if	defined(DEBUG)
int			debug = 0;
int			signals = 0;
#endif

extern int		errno;
extern void		shutdown_msgs();

int			am_in_background	= 0;
int			leave_stdout_open	= 0;

/*
**+
**      Function:    main();
**     
**      Called By:   no one.
**     
**      Purpose:     Spool and Schedule lp requests.
**     
**-
*/
main(argc, argv)
int	argc;
char	*argv[];
{
    int		c;
    extern int	optind;
    extern char	*optarg;
    extern int	optopt;
    extern int	opterr;
    char	*getenv();

    TRACE_ON ("main");

    if (!(SHELL = getenv("SHELL")))
	SHELL = DEFAULT_SHELL;

#if	defined(MDL)
# ident "lpsched has MDL"
    /*
    **	Set the output of MDL to be MDL-LOG
    */
    mdl(SET_FILE_STREAM, 0, 0, 0, "MDL-LOG");
    /*
    **	Set the toggle Flag to cause the file to be opened
    **	and closed as needed rather than opened once and kept.
    **	(ie, it saves a file descriptor at the cost or performance).
    */
    mdl(TOGGLE_OPEN, 0, 0, 0, 0);
#endif

#if	defined(MALLOC_3X)

# if	defined(DEF_MXFAST)
    mallopt (M_MXFAST, DEF_MXFAST);
# endif
# if	defined(DEF_NLBLKS)
    mallopt (M_NLBLKS, DEF_NLBLKS);
# endif
# if	defined(DEF_GRAIN)
    mallopt (M_GRAIN, DEF_GRAIN);
# endif
    mallopt (M_KEEP);

#endif

    opterr = 0;
    while((c = getopt(argc, argv, "dsf:n:r:OM:")) != EOF)
        switch(c)
        {
# if defined (DEBUG)
	    case 'd':
		debug++;
		break;

	    case 's':
		signals++;
		break;
# endif /* DEBUG */

	    case 'f':
		if ((ET_SlowSize = atoi(optarg)) < 1)
		    ET_SlowSize = 1;
		break;

	    case 'n':
		if ((ET_NotifySize = atoi(optarg)) < 1)
		    ET_NotifySize = 1;
		break;

	    case 'r':
		if ((Reserve_Fds = atoi(optarg)) < 1)
		    Reserve_Fds = 0;
		break;

	    case 'O':
		leave_stdout_open = 1;
		break;

#if	defined(MALLOC_3X)
	    case 'M':
		{
			int			value;

			value = atoi(optarg);
			printf ("M_MXFAST set to %d\n", value);
			mallopt (M_MXFAST, value);

			value = atoi(argv[optind++]);
			printf ("M_NLBLKS set to %d\n", value);
			mallopt (M_NLBLKS, value);

			value = atoi(argv[optind++]);
			printf ("M_GRAIN set to %d\n", value);
			mallopt (M_GRAIN, value);
		}
		break;
#endif

	    case '?':
		if (optopt == '?') {
		    usage ();
		    exit (0);
		} else
		    fail(ILLEGAL_OPTION, argv[0], optopt);
	}
    
    TRACE_OFF ("main");
    TRACE_ON ("startup");
    startup();
    TRACE_OFF ("startup");

    process();

# if defined(NEW_LPSHUT)
    remove_messages();
    (void) close_lpfile(LockFp);
    (void) Unlink(Lp_Schedlock);
    note(PRINT_SERVICES_STOPPED);
    exit (0);
# endif

    /* NOTREACHED */
}

void
startup()
{
    extern ushort		getuid();
    extern struct passwd	*getpwnam();
    extern void			disable_signals();
    extern long			ulimit();
    struct passwd		*p;
    struct utsname		utsbuf;

    
    Starting = 1;
    getpaths();

    /*
     * There must be a user named "lp".
     */
    if ((p = getpwnam(LPUSER)) == NULL)
	fail(NO_LP_ADMIN);
    endpwent();
    
    Lp_Uid = p->pw_uid;
    Lp_Gid = p->pw_gid;

    /*
     * Only "root" and "lp" are allowed to run us.
     */
    if (getuid() && getuid() != Lp_Uid)
	fail(NOT_LP_ADMIN);

    setuid (0);

    uname(&utsbuf);
    Local_System = strdup(utsbuf.nodename);

    /*
     * Try setting the lock file to see if another Spooler is running.
     * We'll release it immediately; this allows us to fork the child
     * that will run in the background. The child will relock the file.
     */
    if (!(LockFp = open_lpfile(Lp_Schedlock, "a", 0664)))
	if (errno == EAGAIN)
	    fail(LPSCHED_ALREADY_ACTIVE);
	else
	    fail(CANNOT_OPEN_FILE, NB(Lp_Schedlock), PERROR);
    close_lpfile(LockFp);

    background();
    /*
     * We are the child process now.
#if	defined(DEBUG)
     * (That is, unless the debug flag is set.)
#endif
     */

    if (!(LockFp = open_lpfile(Lp_Schedlock, "w", 0664)))
	fail(FILE_LOCK_FAILED, NB(Lp_Schedlock), PERROR);

    Close (0);
    Close (2);
    if (am_in_background && !leave_stdout_open)
    {
	Close (1);
    }

    if ((OpenMax = ulimit(4, 0L)) == -1)
	OpenMax = OPEN_MAX;

    disable_signals();

    init_messages();

    init_memory();

    note(PRINT_SERVICES_STARTED);
    Starting = 0;
}

# if defined(NEW_LPSHUT)
void
lpshut()
{
    int	i;

    Shutdown = 1;

    (void)alarm(0);

    shutdown_msgs();
    
    for (i = 0; i < ET_Size; i++)
	terminate(&Exec_Table[i]);
}
# else
void
lpshut()
{
    int	i;

    remove_messages();
    (void) close_lpfile(LockFp);
    (void) Unlink(Lp_Schedlock);

    for (i = 0; i < ET_Size; i++)
	terminate(&Exec_Table[i]);

    note(PRINT_SERVICES_STOPPED);
    exit (0);
    /* NOTREACHED */
}
# endif

void
process()
{
    register FSTATUS	*pfs;
    register PWSTATUS	*ppws;


    for (pfs = walk_ftable(1); pfs; pfs = walk_ftable(0))
	check_form_alert (pfs, (_FORM *)0);
    for (ppws = walk_pwtable(1); ppws; ppws = walk_pwtable(0))
	check_pwheel_alert (ppws, (PWHEEL *)0);
    
    /*
     * Clear the alarm, then schedule an EV_ALARM. This will clear
     * all events that had been scheduled for later without waiting
     * for the next tick.
     */
    alarm (0);
    schedule (EV_ALARM);

    /*
     * Start the ball rolling.
     */
    schedule (EV_INTERF, (PSTATUS *)0);
    schedule (EV_NOTIFY, (RSTATUS *)0);
    schedule (EV_SLOWF, (RSTATUS *)0);

#if	defined(CHECK_CHILDREN)
    schedule (EV_CHECKCHILD);
#endif	/* CHECK_CHILDREN */

    for (EVER)
    {
	TRACE_ON ("take_message");
	take_message ();
	TRACE_OFF ("take_message");

	if (Sig_Alrm) {
		TRACE_ON ("schedule ALARM");
		schedule (EV_ALARM);
		TRACE_OFF ("schedule ALARM");
	}

	if (DoneChildren) {
		TRACE_ON ("dowait");
		dowait ();
		TRACE_OFF ("dowait");
	}

# if defined(NEW_LPSHUT)
	if (Shutdown)
	    check_children();
	if (Shutdown == 2)
	    break;
# endif
    }
}

void
ticktock()
{
	Sig_Alrm = 1;
	(void)signal (SIGALRM, ticktock);
	return;
}
			    
void
background()
{
    if (debug)
	return;
    
    switch(fork())
    {
	case -1:
	    fail(CHILD_FORK_FAILED, PERROR);
	    /*NOTREACHED*/

	case 0:
	    (void) setpgrp();
	    am_in_background = 1;
	    return;
	    
	default:
	    note(PRINT_SERVICES_STARTED);
	    exit(0);
	    /* NOTREACHED */
    }
}

usage()
{
    note(LP_USAGE_MESSAGE);
}

void
Exit(n)
int	n;
{
    fail(RECEIVED_SIGNAL, n);
}

void
disable_signals()
{
    int		i;

# if defined(DEBUG)
    if (!signals)
# endif
    for (i = 0; i < NSIG; i++)
	(void) signal(i, Exit);
    
    (void) signal(SIGINT, SIG_IGN);
    (void) signal(SIGHUP, SIG_IGN);
    (void) signal(SIGQUIT, SIG_IGN);
    (void) signal(SIGTERM, lpshut);
    (void) signal(SIGCLD, SIG_IGN);
    (void) signal(SIGALRM, ticktock);
}
