/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ct:ct.c	2.8"

/*
 *
 *	ct [-h] [-v] [-w n] [-x n] [-s speed] telno ...
 *
 *	dials the given telephone number, waits for the
 *	modem to answer, and initiates a login process.
 *
 *	ct uses several routines from uucp:
 *	- getto(flds) takes a vector of fields needed to make
 *	  a connection and returns a file descriptor or -1
 *	- rddev( ... ) takes several arguments and returns lines
 *	  from the /usr/lib/uucp/Devices that match the type
 *	  (in ct the type will be ACU)
 *	- fdig(string) takes a string that is zero or more
 *	  alphabetic characters follow by a number (baud rate)
 *	  and returns a pointer to the first digit in the string.
 *	- mlock(dev) takes the last part of a device (cul01 for
 *	  device /dev/cul01) and tries to create a lock file
 *	  LCK..cul01 in this example.  It returns FAIL if it fails.
 *	- rmlock(pointer) removes the lock file.  In ct pointer is
 *	  always CNULL (a null pointer) causing rmlock to remove
 *	  all LCK files associated with this execution of ct.
 */

#include "uucp.h"
#include "sysfiles.h"
#include <pwd.h>
#include <utmp.h>

#define ROOT	0
#define SYS	3
#define TTYMOD	0644
#define DEV	"/dev/"
#define TELNOSIZE	32		/* maximum phone # size is 31 */
#define LEGAL	"0123456789-*#="
#define USAGE	"[-h] [-v] [-w n] [-x n] [-s speed] telno ..."
#define LOG	"/usr/adm/ctlog"
#define	GETTY	"/etc/getty"
#define TRUE	1
#define FALSE	0

static 
int	_Status,		/* exit status of child */
	_Pid = 0;		/* process id of child */

static
char	*_Num,			/* pointer to a phone number */
	_Tty[sizeof DEV+DIRSIZ] = "",  /* /dev/ttyxx for connection device */
	*_Dev[D_MAX + 1],	/* Filled in by rddev and used globally */
	_Devbuf[BUFSIZ],	/* buffer for rddev */
	*_Flds[7];		/* Filled in as if finds() in uucp did it */

static 
time_t	_Log_on,
	_Log_elpsd;

static 
FILE	*_Fdl;

extern int  optind;
extern char *optarg;
extern  void cleanup();
extern struct passwd  *getpwuid ();
extern struct utmp *getutid (), *getutent(), *pututline();
extern void setutent(), endutent();

static int gdev(), uugetty(), exists();
static void startat(), stopat(), disconnect(), zero();

/*
 * These two dummy routines are needed because the uucp routines
 * used by ct reference them, but they will never be
 * called when executing from ct
 */
void assert () { }		/* for ASSERT in gnamef.c */
void logent () { }		/* so we can load ulockf() */

char   *Myline = NULL;		/* referenced in conn.c; must be NULL for ct */
jmp_buf Sjbuf;			/* used by uucp routines */

main (argc, argv)
char   *argv[];
{
    register int    c;
    register char  *dp,
                   *aptr;
    int		found = 0,
		errors = 0,
		first = TRUE,
		i;
    char    tbuf[TELNOSIZE];
    int     dl,
            count,
	    uugettyflag,	/* is there a uugetty on the line */
            hangup = 1,		/* hangup by default */
            minutes = 0;	/* number of minutes to wait for dialer */
    int     fdl;
    struct termio   termio;
    FILE * dfp;
    typedef int (*save_sig)();
    save_sig	save_hup,
		save_quit,
		save_int;

    save_hup = signal (SIGHUP, cleanup);
    save_quit = signal (SIGQUIT, cleanup);
    save_int = signal (SIGINT, cleanup);
    (void) signal (SIGTERM, cleanup);
    (void) strcpy (Progname, "ct");

    if ((dfp = fopen (DEVICES, "r")) == NULL) {
	(void) fprintf(stderr, "ct: can't open %s\n", DEVICES);
	cleanup(101);
    }

    /* Set up the _Flds vector as if finds() [from uucico] built it */
    _Flds[F_NAME] = "dummy";		/* never used */
    _Flds[F_TIME] = "Any";		/* never used */
    _Flds[F_TYPE] = "ACU";
    _Flds[F_CLASS] = "1200";		/* default at 1200 */
    _Flds[F_PHONE] = "";			/* filled in by arguments */
    _Flds[F_LOGIN] = "";			/* never used */
    _Flds[6] = NULL;

    while ((c = getopt (argc, argv, "hvw:s:x:")) != EOF) {
	switch (c) {
	    case 'h': 
		hangup = 0;
		break;

	    case 'v': 
		Verbose = 1;
		break;

	    case 'w': 
		minutes = atoi (optarg);
		if (minutes < 1) {
		    (void) fprintf(stderr,
			"\tusage: %s %s\n", Progname, USAGE);
		    (void) fprintf(stderr, "(-w %s) Wait time must be > 0\n",
			optarg);
		    cleanup(101);
		}
		break;

	    case 's': 
		_Flds[F_CLASS] = optarg;
		break;

	    case 'x':
		Debug = atoi(optarg);
		if (Debug < 0 || Debug > 9) {
		    (void) fprintf(stderr,
			"\tusage: %s %s\n", Progname, USAGE);
		    (void) fprintf(stderr, "(-x %s) value must be 0-9\n",
			optarg);
		    cleanup(101);
		}
		break;

	    case '?': 
		(void) fprintf(stderr, "\tusage: %s %s\n", Progname, USAGE);
		cleanup(101);
		/* NOTREACHED */
	}
    }

    if (optind == argc) {
	(void) fprintf(stderr, "\tusage: %s %s\n", Progname, USAGE);
	(void) fprintf(stderr, "No phone numbers specified!\n");
	cleanup(101);
    }

    /* check for valid phone number(s) */
    for (count = argc - 1; count >= optind; --count) {
	_Num = argv[count];
	if (strlen (_Num) >= sizeof tbuf - 1) {
	    (void) fprintf(stderr, "ct: phone number too long -- %s\n", _Num);
	    ++errors;
	}
	if (strspn (_Num, LEGAL) < strlen (_Num)) {
	    (void) fprintf(stderr, "ct: bad phone number -- %s\n", _Num);
	    ++errors;
	}
    }
    if (errors)
	cleanup(101);

    /************************************************************/
    /*		Begin Loop:  Find an available Dialer		*/
    /************************************************************/
    for (count = 0;; count++) { /* count will be wait time after first
				 * time through the loop.
				 * break will be used exit loop.
				 */
	if ( (found = gdev (dfp, _Flds)) > 0) {  /* found a dialer */
	    (void) fprintf(stdout, "Allocated dialer at %s baud\n",
		_Flds[F_CLASS]);
	    break;
	}
	else if (found == 0) {	/* no dialers of that on system */
	    (void) fprintf(stdout, "No %s dialers on this system\n",
		fdig(_Flds[F_CLASS]) );
    	    cleanup(101);
	}
	    
	if (!first) { /* not the first time in loop */
	    VERBOSE("%s busy", (found == -1) ? "Dialer is" : "Dialers are");
	    VERBOSE(" (%d minute(s))\n", count);
	    if (count < minutes) {
	        sleep(60);
	        continue;
	    }
	    /* This is the end of the loop - no time left */
	    break;
	}

	/**************************************************************/
	/* First time through loop - get wait minutes if no -w option */
	/**************************************************************/
	first = FALSE;
	(void) fprintf(stdout, "The (%d) %s dialer%s busy\n", -found,
	    _Flds[F_CLASS], (found == -1 ? " is" : "s are"));
	if (minutes) {	/* -w already set wait minutes */
	    (void) fprintf(stdout, "Waiting for %d minute%s\n", minutes,
		(minutes > 1 ? "s" : "") );
	    sleep(60);
	    continue;
	}

	if (!isatty(fileno(stdin)) )  {  /* not a terminal - get out */
	    cleanup(101);
	}

	/* Ask user if she/he wants to wait */
	(void) fputs("Do you want to wait for dialer? (y for yes): ", stdout);
	if ((c = getchar ()) == EOF || tolower (c) != 'y')
	    cleanup(101);
	while ( (c = getchar()) != EOF && c != '\n')
	    ;

	(void) fputs ("Time, in minutes? ", stdout);
	(void) scanf ("%d", &minutes);
	while ( (c = getchar()) != EOF && c != '\n')
	    ;

	if (minutes <= 0)
	    cleanup(101);

	(void) fputs ("Waiting for dialer\n", stdout);
	sleep(60);
	continue;

    }
    /************************************************************/
    /*		End Loop:  Find an available Dialer		*/
    /************************************************************/

    /* check why loop terminated */
    if (found < 0) {	/* no dialer found - get out */
        (void) fputs("*** TIMEOUT ***\n", stdout);
        cleanup(101);
    }

    (void) signal(SIGHUP, SIG_IGN);
    /* found a dialer. now try to call */
    if (!isatty (fileno (stdin)))
        hangup = 0;

    if (hangup) {  /* -h option not specified */
        (void) fputs ("Confirm hang-up? (y to hang-up): ", stdout);
        if ((c = getchar ()) == EOF || tolower (c) != 'y')
    	    cleanup(101);
        while ( (c = getchar()) != EOF && c != '\n')
    	    ;

	/* close 1; if stderr is not redirected, close 2 also */
	(void) close(1);
        if (isatty (fileno (stderr))) {
            Verbose = 0;
	    Debug = 0;
            (void) close (2);
	}

	(void) ioctl (fileno(_Fdl), TCGETA, &termio);
        termio.c_cflag = 0;	/* speed to zero for hangup */
        (void) ioctl (0, TCSETAW, &termio);  /* hang up terminal */
        (void) sleep (5);
    }

    (void) fclose (dfp);  /* close Devices file */

    /* Try each phone number until a connection is made, or non work */
    for (count = optind; count < argc; count++) {
	/* call getto routine to make connection */
	_Flds[F_PHONE] = argv[count];
	rmlock(CNULL);	/* remove temporary lock set by gdev */
	fdl = getto(_Flds);
	if (fdl > 0) {
	    /*
	     * If there is a uugetty on the line, get rid
	     * of the lock file quickly so that when the uugetty
	     * reads the first character, the lock file will be gone
	     * indicating that the uugetty should act as a getty
	     * rather than going into the sleep-check_lock loop
	     */
	    if ( (uugettyflag = uugetty(Dc)) ) /* really an assignment! */
		rmlock(CNULL);

	    _Fdl = fdopen(fdl, "r+");
	    (void) sprintf(_Tty, "%s%s", DEV, Dc);
	    /* NOTE:  Dc is set in the caller routines */
	    break;
	}
    }

    /* check why the loop ended (connected or no more numbers to try) */
    if (count == argc)
	cleanup(101);

    /****** Successfully made connection ******/
    VERBOSE("Connected\n", "");

    /* ignore some signals if they were ignored upon invocation of ct */
    /* or else, have them go to graceful disconnect */
    if (save_hup == SIG_IGN)
	(void) signal (SIGHUP, SIG_IGN);
    else
	(void) signal (SIGHUP, disconnect);

    if (save_quit == SIG_IGN)
	(void) signal (SIGQUIT, SIG_IGN);
    else
	(void) signal (SIGQUIT, disconnect);

    if (save_int == SIG_IGN)
	(void) signal (SIGINT, SIG_IGN);
    else
	(void) signal (SIGINT, disconnect);

    (void) signal (SIGTERM, disconnect);
    (void) signal (SIGALRM, disconnect);

    (void) sleep (2);		/* time for phone line/modem to settle */

    if (!hangup) {	/* close files - for neatness */
	(void) close(0);
	(void) close(1);
	if (isatty (2))
            (void) close (2);
    }

    _Log_on = time ((long *) 0);

    /*
     * if there is a uugetty on this line,
     * tell the user to hit a carriage return to make
     * the waiting uugetty get past the inital read,
     * Then exit.
     */
    if (uugettyflag) {	/* there is a uugetty on the line */
	(void) fputs("Hit carriage return ", _Fdl);
	(void) fclose(_Fdl);
	CDEBUG(4, "there is a uugetty; exit\n", 0);
	exit(0);
    }

    CDEBUG(4, "start getty -h -t60 %s ", Dc );
    CDEBUG(4, "%s\n", fdig(_Flds[F_CLASS]));
    for (;;) {
	switch(_Pid = fork()) {
	case -1:	/* fork failed */
	    if ((!hangup || Verbose))
		(void) fputs ("ct: can't fork for getty\n", stderr);
	    cleanup(101);
	    /*NOTREACHED*/

	case 0:		/* child process */
	    startat ();
	    (void) setpgrp ();	      /* getting /dev/tty right for children */
	    (void) signal(SIGHUP, SIG_DFL);  /* so child will exit on hangup */
	    (void) execl (GETTY, "getty", "-h", "-t60",
		Dc, fdig(_Flds[F_CLASS]), NULL);
	    /* exec failed */
	    cleanup(101);
	    /*NOTREACHED*/

	default:	/* parent process */
	    break;
	}

	/* Parent process */

	while (wait (&_Status) != _Pid);
	if ((_Status & 0xff00) < 0) {
	    if (!hangup)
		VERBOSE("ct: can't exec getty\n", "");
	    cleanup(101);
	}

        rewind (_Fdl);	/* flush line */
        (void) fputs ("\nReconnect? ", _Fdl);

        rewind (_Fdl);
        (void) alarm (20);
        c = getc (_Fdl);

        if (c == EOF || tolower (c) == 'n')
	    disconnect (0);	/* normal disconnect */
        while ( (c = getc(_Fdl)) != EOF && c != '\n')
    	    ;
        (void) alarm (0);
    }
}

static
void
disconnect (code) {
register int    pid,
                hrs,
                mins,
                secs;
register char  *aptr;
extern char *ctime (),
            *getenv ();
struct termio   termio;

    (void) alarm(0);
    (void) signal (SIGALRM, SIG_IGN);
    (void) signal (SIGINT, SIG_IGN);
    (void) signal (SIGTERM, SIG_IGN);

    _Log_elpsd = time ((long *) 0) - _Log_on;

    (void) ioctl (fileno(_Fdl), TCGETA, &termio);
    termio.c_cflag = 0;				/* speed to zero for hangup */
    (void) ioctl (fileno(_Fdl), TCSETAW, &termio);  /* hang up terminal */
    (void) fclose (_Fdl);

    DEBUG(5, "Disconnect(%d)\n", code);
    VERBOSE("Disconnected\n", "");

    stopat (_Flds[F_PHONE]);

    cleanup(code);
}

/*
 * clean and exit with "code" status
 */
void
cleanup (code)
register int    code;
{
    CDEBUG(5, "cleanup(%d)\n", code);
    rmlock (CNULL);
    if (*_Tty != '\0') {
	CDEBUG(5, "chmod/chown %s\n", _Tty);
	if (chown (_Tty , UUCPUID, UUCPGID) < 0 || chmod (_Tty , TTYMOD) < 0) {
	    CDEBUG(5, "Can't chown/chmod on %s ", _Tty);
	    CDEBUG(5, "to %d\n", UUCPUID);
	}
    }
    if (_Pid) { /* kill the child process */
	(void) signal(SIGHUP, SIG_IGN);
	(void) signal(SIGQUIT, SIG_IGN);
	(void) kill (_Pid, SIGKILL);
    }
    exit (code);
}

/*	gdev()
 * Find an available line with a dialer on it.
 * Set a temporary lock file for the line.
 * Return:
 *	>0 - got a dialer
 *	<0 - failed - return the number of possible dialers
 *	0 - not dialers of requested class on the system.
 */

static
int
gdev (dfp, flds)
FILE * dfp;
char   *flds[];
{
    int	count = 0,
	status;

    (void) fseek (dfp, (long) 0, 0);
    while ((status = rddev (dfp, "ACU", _Dev, _Devbuf, D_MAX)) != FAIL) {
	/* check caller type */
	if (!EQUALS (flds[F_TYPE] /* "ACU" */, _Dev[D_TYPE]))
	    continue;
	/* check class, check (and possibly set) speed */
	if (!EQUALS (flds[F_CLASS] /* speed */, _Dev[D_CLASS]))
	    continue;
	count++;

	if (mlock (_Dev[D_LINE]) == FAIL)
	    continue;

	/* found available dialer and set temporary lock */
	return (count);

    }
    return (- count);
}

/*
 * Check if there is a uugetty active on this line.
 * Return:
 *	0 - there is no getty on this line
 *	1 - found a uugetty on this line
 */

static
int
uugetty(line)
char *line;
{
    register struct utmp   *u;

    while ((u = getutent()) != NULL) {
	if (u->ut_type == LOGIN_PROCESS
	    && EQUALS(u->ut_line, line)
	    && EQUALS(u->ut_user, "LOGIN") ) {
		CDEBUG(7, "ut_line %s, ", u->ut_line);
		CDEBUG(7, "ut_user %s, ", u->ut_user);
		CDEBUG(7, "ut_id %.4s, ", u->ut_id);
		CDEBUG(7, "ut_pid %d\n", u->ut_pid);

		/* see if the process is still active */
		if (kill(u->ut_pid, 0) == 0 || errno == EPERM) {
		    CDEBUG(4, "process still active\n", 0);
		    return(1);
		}
	}
    }
    return(0);
}

/*
 * Create an entry in utmp file if one does not already exist.
 */
static
void
startat () {
	
    struct utmp utmpbuf;
    register struct utmp   *u,
                           *oldu;
    FILE * fp;



/*	Set up the prototype for the utmp structure we want to write.	*/

    u = &utmpbuf;
    zero (&u -> ut_user[0], sizeof (u -> ut_user));
    zero (&u -> ut_line[0], sizeof (u -> ut_line));

/*	Fill in the various fields of the utmp structure.		*/

    u -> ut_id[0] = _Tty[strlen(_Tty)-2];
    u -> ut_id[1] = _Tty[strlen(_Tty)-1];
    u -> ut_id[2] = '\0';
    u -> ut_id[3] = '\0';
    u -> ut_pid = getpid ();

    u -> ut_exit.e_termination = 0;
    u -> ut_exit.e_exit = 0;
    u -> ut_type = INIT_PROCESS;
    time (&u -> ut_time);
    setutent ();		/* Start at beginning of utmp file. */

/*	For INIT_PROCESSes put in the name of the program in the	*/
/*	"ut_user" field.						*/

    strncpy (&u -> ut_user[0], "getty", sizeof (u -> ut_user));
    strncpy (&u -> ut_line[0], Dc, sizeof (u -> ut_line));

/*	Write out the updated entry to utmp file.			*/
    pututline (u);

/*	Now attempt to add to the end of the wtmp file.  Do not create	*/
/*	if it doesn't already exist.  **  Note  ** This is the reason	*/
/*	"r+" is used instead of "a+".  "r+" won't create a file, while	*/
/*	"a+" will.							*/

    if ((fp = fopen (WTMP_FILE, "r+")) != NULL) {
	fseek (fp, 0L, 2);	/* Seek to end of file */
	fwrite (u, sizeof (*u), 1, fp);
	fclose (fp);
    }
    endutent ();
}

/*
 * Change utmp file entry to "dead".
 * Make entry in ct log.
 */

static
void
stopat (num)
char   *num;
{
    register long   stopt;
    struct utmp utmpbuf;
    register struct utmp   *u,
                           *oldu;
    struct utmp *getutid ();
    FILE * fp;

    stopt = time ((long *) 0);

/*	Set up the prototype for the utmp structure we want to write.	*/

    setutent();
    u = &utmpbuf;
    zero (&u -> ut_user[0], sizeof (u -> ut_user));
    zero (&u -> ut_line[0], sizeof (u -> ut_line));

/*	Fill in the various fields of the utmp structure.		*/

    u -> ut_id[0] = _Tty[strlen(_Tty)-2];
    u -> ut_id[1] = _Tty[strlen(_Tty)-1];
    u -> ut_id[2] = '\0';
    u -> ut_id[3] = '\0';
    u -> ut_pid = _Pid;
    strncpy (&u -> ut_line[0], Dc, sizeof (u -> ut_line));
    u -> ut_type = USER_PROCESS;

/*	Find the old entry in the utmp file with the user name and	*/
/*	copy it back.							*/

    if (u = getutid (u)) {
	utmpbuf = *u;
	u = &utmpbuf;
    }

    u -> ut_exit.e_termination = _Status & 0xff;
    u -> ut_exit.e_exit = (_Status >> 8) & 0xff;
    u -> ut_type = DEAD_PROCESS;
    time (&u -> ut_time);

/*	Write out the updated entry to utmp file.			*/

    pututline (u);

/*	Now attempt to add to the end of the wtmp file.  Do not create	*/
/*	if it doesn't already exist.  **  Note  ** This is the reason	*/
/*	"r+" is used instead of "a+".  "r+" won't create a file, while	*/
/*	"a+" will.							*/

    if ((fp = fopen (WTMP_FILE, "r+")) != NULL) {
	fseek (fp, 0L, 2);	/* Seek to end of file */
	fwrite (u, sizeof (*u), 1, fp);
	fclose (fp);
    }
    endutent ();

/*	Do the log accounting 					*/

    if (exists (LOG) && (fp = fopen (LOG, "a")) != NULL) {
	char   *aptr;
	int     hrs,
	        mins,
	        secs;
	extern char *ctime (),
	           *getenv ();

 	/* ignore user set TZ for logfile purposes */
	if ( (aptr = getenv ("TZ")) != NULL )
		*aptr = '\0';

	(aptr = ctime (&_Log_on))[16] = '\0';
	hrs = _Log_elpsd / 3600;
	mins = (_Log_elpsd %= 3600) / 60;
	secs = _Log_elpsd % 60;
	(void) fprintf(fp, "%-8s ", getpwuid (getuid ()) -> pw_name);
	(void) fprintf(fp, "(%4s)  %s ", fdig(_Flds[F_CLASS]), aptr);
	if (hrs)
	    (void) fprintf(fp, "%2d:%.2d", hrs, mins);
	else
	    (void) fprintf(fp, "   %2d", mins);
	(void) fprintf(fp, ":%.2d  %s\n", secs, num);
	(void) fclose (fp);
    }
}
static
int
exists (file)
char   *file;
{
    struct stat statb;
    extern  errno;

    if (stat (file, &statb) == -1 && errno == ENOENT)
	return (0);
    return (1);
}

static
void
zero (adr, size)
register char  *adr;
register int    size;
{
    while (size--)
	*adr++ = '\0';
}
