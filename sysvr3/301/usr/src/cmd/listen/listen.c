/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)listen:listen.c	1.14"

/*
 * Network Listener Process
 *
 *		data base version:
 *
 *		accepts the following message to start servers:
 *
 *		NLPS:000:001:svc_code	where svc_code is a null terminated 
 *					char string ( <= SVC_CODE_SZ bytes)
 *
 *		command line:
 *
 *		listen [ -n netspec ] [ -r external_addr ] [ -l external_addr ]
 *		all options take appropriate defaults for STARLAN NETWORK.
 *		-r: address to listen for remote login requests.
 *		-l: address to listen for listener service requests.
 *
 */

/* system include files	*/

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <varargs.h>
#include <string.h>
#include <errno.h>
#include <sys/utsname.h>
#include <sys/tiuser.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <values.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>
#include <sys/poll.h>
#include <sys/dir.h>
#include <sys/stropts.h>

/* S4 status manager interface					*/
#ifdef	S4
#include <status.h>
#endif

/* listener include files */

#include "lsparam.h"		/* listener parameters		*/
#include "lsfiles.h"		/* listener files info		*/
#include "lserror.h"		/* listener error codes		*/
#include "lsnlsmsg.h"		/* NLPS listener protocol	*/
#include "lssmbmsg.h"		/* MS_NET identifier		*/
#include "lsdbf.h"		/* data base file stuff		*/
#include "nlsenv.h"		/* environ variable names	*/
#include "listen.h"

/* defines	*/

#define min(a,b) 		(a>b ? b : a)

#define NAMESIZE	(NAMEBUFSZ-1)

#define SPLhi()		Splflag = 1
#define SPLlo()		Splflag = 0


/* global variables	*/

int	Child;		/* set if process is a listener child (worker bee) */

int	Pid;		/* listener's process ID (original listener's child) */

char	*Progname;		/* listener's basename (from argv[0])	*/

static char Provbuf[PATHSIZE];
char	*Provider = Provbuf;	/* name of transport provider		*/
char	*Netspec = NETSPEC;

int	Debuglvl = 0;

int	Nfd1, Nfd2, Nfd3;		/* Net fd's			*/

char	_rlbuf[NAMEBUFSZ];		/* remote login name		*/
char	_lsbuf[NAMEBUFSZ];		/* listener service name	*/

char	*Nodename = _rlbuf;		/* my normalized name (login svc)*/
char	*Pnodename = _lsbuf;		/* my permuted name (listener svc) */

char	*Rxname = NULL;			/* remote login external name	*/
char	*Lxname = NULL;			/* listener svc external name	*/

int	Rlen = NAMEBUFSZ;
int	Llen = NAMEBUFSZ;

char	*Basedir;			/* base dir (from /etc/passwd)	*/
char	Basebuf[256];
char	*Home;				/* listener's home directory	*/
char	Homebuf[350];			/* Basebuf + "/" + DIRSZ	*/
FILE	*Debugfp = stderr;
FILE	*Logfp;
int	Pidfd;

int	Background;			/* are we in background?	*/

char	Lastmsg[512];		/* contains last msg logged (by stampbuf) */

int	Logmax = LOGMAX;

int	Validate;		/* from -v flag: validate data base only */

int	Lckfd;

int	Sigusr1;		/* data base file changed		*/

int	Splflag;		/* logfile critical region flag		*/

char *Nenviron[NENVIRON + 1];	/* server's environment array for execve */




static char *badpwdmsg = "Missing/bad password file entry for %s?\n";
static char *badnspmsg = "Bad netpec on command line ( Pathname too long )\n";

main(argc, argv)
	int argc;
	char **argv;
{
	register char **av;
	int ret;
	char	scratch[50];
	register struct passwd *pwdp;
	register struct group *grpp;
	extern struct passwd *getpwnam();
	extern struct group *getgrnam();
	extern void exit(), endpwent(), endgrent();

	/*
	 * pre-initialization:
	 */

	/* change real and effective uid/gid to non-root */

	if (!(grpp = getgrnam(LSGRPNAME)))  {	/* get group entry	*/
		fprintf(stderr, "No group entry for %s?\n", LSGRPNAME);
		exit(1);
	}

	if (setgid(grpp->gr_gid))  {
		sprintf(scratch, "Cannot set group id to %s", LSGRPNAME);
		perror(scratch);
		exit(1);
	}

	endgrent();

	/* get password entry for name LSUIDNAME.  Basedir = LSUIDNAME's home */

	if (!(pwdp = getpwnam(LSUIDNAME)) || !(strlen(pwdp->pw_dir)))  {
		fprintf(stderr,badpwdmsg,LSUIDNAME);
		exit(1);
	}

	if (setuid(pwdp->pw_uid))  {
		sprintf(scratch, "Cannot set user id to %s", LSUIDNAME);
		perror(scratch);
		exit(1);
	}

	strcpy(Basebuf, pwdp->pw_dir);
	Basedir = Basebuf;

	endpwent();

	/*
	 * quickly, find -n [ and -v ] argument on command line	
	 */

	av = argv; ++av;		/* skip argv[0]	*/

	while(*av)  {
		if (!strncmp(*av,"-n", 2)) {
			if (*(*av + 2) == NULL)
				Netspec = *++av;
			else
				Netspec = (*av + 2);
		} else {
			if (!strcmp(*av, "-v"))
				++Validate;
		}
		++av;
	}

	/* calc homedir/provider from passwd file (dir entry) + "/netspec" */

	if (strlen(Netspec) > PATHSIZE)  {
		fprintf(stderr, badnspmsg);
		exit(1);
	}

	strcpy(Homebuf,Basedir);
	strcat(Homebuf,"/");
	strcat(Homebuf,Netspec);
	Home = Homebuf;

	strcpy(Provbuf, "/dev/");
	strcat(Provbuf, Netspec);

	if (chdir(Home))  {
		fprintf(stderr,"Cannot change directory to %s\n",Home);
		exit(1);
	}

	/*
	 * -v flag tells listener to validate the data base only
	 */

	if (Validate)  {
		Logfp = stderr;
#ifdef	DEBUGMODE
		umask(022);
		if (!(Debugfp = fopen(VDBGNAME, "w")))  {
			fprintf(stderr,"Can't create debug file: %s in %s\n",
				VDBGNAME, Home);
			exit(1);
		}
#endif
		ret = init_dbf();
		logmessage(ret ? "Bad data base file": "Good data base file");
		exit(ret);
	}

	umask(0);

	if (((Lckfd = open(LCKNAME, O_WRONLY | O_CREAT, 0666 )) == -1) || 
	     (locking(Lckfd, 2, 0L) == -1)) {
		fprintf(stderr, "Errno %d opening lock file %s/%s.\n", 
			errno, Home, LCKNAME);
		fprintf(stderr, "Listener may already be running!\n");
		exit(1);
	}

	write(Lckfd, "lock", 4);

	if (locking(Lckfd, 1, 0L) == -1)  {
		fprintf(stderr, "Errno %d while locking lock file\n", errno);
		exit(1);
	}

	umask(022);

	Logfp = fopen(LOGNAME, "w");

#ifdef	DEBUGMODE
	Debugfp = fopen(DBGNAME, "w");
#endif


#ifdef	DEBUGMODE
	if ((!Logfp) || (!Debugfp))  {
#else
	if (!Logfp) {
#endif
		fprintf(stderr,"listener cannot create file(s) in %s\n",Home);
		exit(1);
	}

	logmessage("@(#)listen:listen.c	1.8");

#ifdef	DEBUGMODE
	logmessage("Listener process with DEBUG capability");
#endif

	initialize(argv, argc);
	listen();
}



initialize(argv, argc)
	int argc;
	char **argv;
{

	/* try to get my "basename"		*/

	DEBUG((9,"in initialize"));
	Progname = strrchr(argv[0], '/');
	if (Progname && Progname[1])
		++Progname;
	else
		Progname = argv[0];

	cmd_line_args(argv,argc);
	init_dbf();
	pid_open();			/* create pid file		*/
	net_open();			/* init, open, bind names 	*/
	catch_signals();
	divorce_parent();
	logmessage("Initialization Complete");
}

/*
 * cmd_line_args uses getopt(3) to read command line arguments.
 */

#ifdef	DEBUGMODE

static char *Optflags = "n:r:l:D:L:";

#else

static char *Optflags = "n:r:l:L:";

#endif	/* DEBUGMODE */


cmd_line_args(argv,argc)
	int argc;
	char **argv;
{
	register i;
	extern char *optarg;
	extern int optind, opterr;
	extern int isdigits();
	int errflag = 0;

	DEBUG((9,"in cmd_line_args"));

#ifdef	DEBUGMODE
	DEBUG((9,"cmd_line_args: argc = %d",argc));
	for (i=0;  i < argc; i++)
		DEBUG((9,"argv[%d] = %s",i,argv[i]));
#else
	opterr = 1;		/* disable getopt's stderr output */
#endif	/* DEBUGMODE */


	while ( (i = getopt(argc, argv, Optflags)) != EOF )
		switch (i)  {
		case 'n':		/* netspec			*/
			break;		/* (read previously)		*/

		case 'r':		/* remote login external addr	*/
			Rxname = optarg;
			break;

		case 'l':		/* listener svc external addr	*/
			Lxname = optarg;
			break;

#ifdef	DEBUGMODE

		case 'D':		/* Debug level			*/
			if (isdigits(optarg))
				Debuglvl = atoi( optarg );
			else
				++errflag;
			break;

#endif	/* DEBUGMODE */

		case 'L':		/* max log entries		*/
			if (isdigits(optarg))  {
				Logmax = atoi( optarg );
				if (Logmax < LOGMIN)
					Logmax = LOGMIN;
				DEBUG((5, "Logmax = %d", Logmax));
			} else
				++errflag;
			break;

		case '?':
			++errflag;
			break;
		}

#ifndef S4

		if (!Rxname || !Lxname)
			++errflag;

#endif

	if (errflag)
		error(E_CMDLINE, EXIT | NOCORE);

}

/*
 * pid_open:
 *
 * open pidfile with specified oflags and modes
 *
 */

static char *pidopenmsg ="Can't create process ID file in home directory";

pid_open()
{
	int pid;
	int ret;
	unsigned i;
	char pidstring[20];

	DEBUG((9,"in pid_open()"));

	if ((Pidfd = open(PIDNAME, PIDOFLAG, PIDMODE)) == -1)  {
		logmessage(pidopenmsg);
		error(E_CREAT, EXIT | NOCORE | NO_MSG);
	}
	pid = getpid();
	i = sprintf(pidstring, "%d", pid) + 1;
	if ((ret = write(Pidfd, pidstring, i)) != i) {
		if (ret < 0)
			sys_error(E_PIDWRITE, EXIT);
		else
			error(E_PIDWRITE, EXIT);
	}
	(void) lseek(Pidfd, 0L, 0);
}





/*
 * net_open: open and bind communications channels
 *		The name generation code in net_open, open_bind and bind is, 
 * 		for the	most part, specific to STARLAN NETWORK.  
 *		This name generation code is included in the listener
 *		as a developer debugging aid.
 */

net_open()
{
	extern char *memcpy();
	extern int nlsc2addr();
#ifdef	S4
	register char *p;
	extern char *getnodename();
	extern char *nlsname();		/* STARLAN specific	*/
#endif
#ifdef	CHARADDR
	char pbuf[NAMEBUFSZ + 1];
#endif

	DEBUG((9,"in net_open"));

	DEBUG((3, "Rxname = %s", Rxname));
	DEBUG((3, "Lxname = %s", Lxname));

	if (Rxname)  {			/* -r cmd line argument		*/
		DEBUG((3, "Decoding remote login address"));
		if ((Rlen = nlsc2addr(Nodename, NAMEBUFSZ, Rxname)) < 0)  {
			logmessage("Error decoding remote login name");
			error(E_CMDLINE, EXIT | NOCORE | NO_MSG);
		}
	}

#ifdef	S4
	else  {
		p = getnodename();
		DEBUG((3, "machine nodename = %s", p));
		Rlen = strlen(p);
		(void)memcpy(Nodename, p, Rlen);
	}
#endif	/* S4 */


	if (Lxname)  {			/* -l cmd line argument		*/
		DEBUG((3, "Decoding listener service address"));
		if ((Llen = nlsc2addr(Pnodename, NAMEBUFSZ, Lxname)) < 0)  {
			logmessage("Error decoding listener service name");
			error(E_CMDLINE, EXIT | NOCORE | NO_MSG);
		}
	}

#ifdef	S4
	else  {
		p = getnodename();
		DEBUG((3, "machine nodename = %s", p));
		p = nlsname(p);
		Llen = strlen(p);
		(void)memcpy(Pnodename,p,Llen);
		DEBUG((3,"nlsname = %s, Pnodename = %s, Llen = %d",
			p, Pnodename, Llen));
	}
#endif /* S4 */

#ifdef	CHARADDR
	/* this debug code assumes addresses are printable characters */
	/* not necessarily null terminated.	*/

	(void)memcpy(pbuf, Nodename, Rlen);
	*(pbuf+Rlen) = (char)0;
	DEBUG((3, "Nodename = %s, length = %d", pbuf, Rlen));
	(void)memcpy(pbuf, Pnodename, Llen);
	*(pbuf+Llen) = (char)0;
	DEBUG((3, "Pnodename = %s, length = %d", pbuf, Llen));
#endif	/* CHARADDR	*/

	/*
	 * QLEN must be 1 for S4.  However,
	 * for 3b2, QLEN may be greater, but a lot more code must be
	 *	added to maintain the list of connect/disconnect
	 *	events pulled off the stream.
	 */

	Nfd1 = open_bind(Nodename,QLEN, Rlen);
	Nfd2 = open_bind(Pnodename,QLEN, Llen);

	if ( (Nfd1 == -1) || (Nfd2 == -1) )
		error(E_OPENBIND, EXIT);

	logmessage("Net opened, names bound");
}

/*
 * open_bind:
 *
 * open the network and bind the endpoint to 'name'
 * this routine is also used by listen(), so it can't exit
 * under all error conditions: specifically, if there are
 * no minor devices avaliable in the network driver, open_bind
 * returns -1.  (error message will be logged).  All other errors
 * cause an exit.
 *
 * If clen is zero, transport provider picks the name and these
 * routines (open_bind and bind) ignore name and qlen -- 
 * this option is used when binding a name for accepting a connection 
 * (not for listening.)  You MUST supply a name, qlen and clen when
 * opening/binding a name for listening.
 *
 * Assumptions: driver returns ENXIO when all devices are allocated.
 */

int
open_bind(name,qlen, clen)
	char *name;
	int qlen;
	int clen;
{
	register fd;
	struct t_info t_info;
	extern int t_errno, errno;

	fd = t_open(Provider, NETOFLAG, &t_info);
	if (fd < 0)  {
		if ( (t_errno == TSYSERR) && ((errno == ENXIO) ||
		    (errno == ENOSR) || (errno == EAGAIN) || (errno == ENOSPC)) )  {
			tli_error(E_FD1OPEN, CONTINUE);
			logmessage("No network minor devices (ENXIO/ENOSR)");
			return(-1);
		}
		tli_error(E_FD1OPEN, EXIT);
	}

	DEBUG((7,"fd %d opened",fd));

	return(bind(fd, name, qlen, clen));
}


bind(fd, name, qlen, clen)
	register fd;
	char *name;
	int qlen;
	register int clen;
{
	register struct t_bind *req = (struct t_bind *)0;
	register struct t_bind *ret = (struct t_bind *)0;
	extern char *t_alloc();
	extern int memcmp();
	extern int errno;
	extern char *memcpy();

#ifdef S4
	extern struct netbuf *lname2addr();
#endif /* S4 */

#ifdef	CHARADDR
	char pbuf[NAMEBUFSZ + 1];
#endif
	char scratch[256];

	DEBUG((9,"in bind, clen = %d", clen));
	
	if (clen)  {
		errno = t_errno = 0;
		while (!(req = (struct t_bind *)t_alloc(fd,T_BIND,T_ALL)) ) {
			if ((t_errno != TSYSERR) || (errno != EAGAIN))
				tli_error( E_T_ALLOC, EXIT);
			else
				tli_error( E_T_ALLOC, CONTINUE);
		}

		errno = t_errno = 0;
		while (!(ret = (struct t_bind *)t_alloc(fd,T_BIND,T_ALL)) ) {
			if ((t_errno != TSYSERR) || (errno != EAGAIN))
				tli_error( E_T_ALLOC, EXIT);
			else
				tli_error( E_T_ALLOC, CONTINUE);
		}

		if (clen > req->addr.maxlen)  {
			sprintf(scratch,"Truncating name size from %d to %d", 
				clen, req->addr.maxlen);
			logmessage(scratch);
			clen = req->addr.maxlen;
		}

		(void)memcpy(req->addr.buf, name, clen);
		req->addr.len = clen;
		req->qlen = qlen;

#if defined(CHARADDR) && defined(DEBUGMODE)
		(void)memcpy(pbuf, req->addr.buf, req->addr.len);
		pbuf[req->addr.len] = (char)0;
		DEBUG((3,"bind: fd=%d, logical name=%c%s%c, len=%d",
			fd, '\"',pbuf, '\"', req->addr.len));
#endif	/* CHARADDR  && DEBUGMODE */


#ifdef S4
		if (!lname2addr(fd, &(req->addr)))  {
			sprintf(scratch, "lname2addr failed, errno %d", errno);
			logmessage(scratch);
			error(E_SYS_ERROR, EXIT | NO_MSG);
		}
#endif /* S4 */

#if defined(CHARADDR) && defined(DEBUGMODE)
		(void)memcpy(pbuf, req->addr.buf, req->addr.len);
		pbuf[req->addr.len] = (char)0;
		DEBUG((3,"bind: fd=%d, address=%c%s%c, len=%d",
			fd, '\"',pbuf, '\"', req->addr.len));
#endif	/* CHARADDR  && DEBUGMODE */


	}

	if (t_bind(fd, req, ret))  {
		DEBUG((1,"t_bind failed; t_errno %d errno %d", t_errno, errno));
		if (qlen)	/* starup only */
			tli_error(E_T_BIND, EXIT | NOCORE);
		/* here during normal service */
		if ((t_errno == TNOADDR) || ((t_errno == TSYSERR) && (errno == EAGAIN))) {
			/* our name space is all used up */
			tli_error(E_T_BIND, CONTINUE);
			t_close(fd);
			return(-1);
		}
		/* otherwise, irrecoverable error */
		tli_error(E_T_BIND, EXIT | NOCORE);
	}

#if defined(S4) && defined(DEBUGMODE)
	t_dump(Debugfp);  /* show TLI internal name structures on Debugfp */
#endif


	if (clen)  {

		if ( (ret->addr.len != req->addr.len) ||
		     (memcmp( req->addr.buf, ret->addr.buf, req->addr.len)) )  {
			error(E_BIND_REQ, EXIT | NOCORE);
		}

		if ( t_free(req, T_BIND) )
			tli_error(E_T_FREE, EXIT);

		if ( t_free(ret, T_BIND) )
			tli_error(E_T_FREE, EXIT);
	}
	return(fd);
}



/*
 * divorce_parent: fork a child, make child independent of parent.
 *		   parent writes child's pid to process id file
 *		   and exits.  If it can't create or write pidfile,
 *		   child must be killed.
 *
 *	Assumptions: directory previously changed to '/'.
 *
 *	Notes: define FOREGROUND to inhibit the listener from running
 *		in the background.  Useful with DEBUGMODE.
 */

divorce_parent()
{
	char pidstring[50];	/* holds childs pid in ascii decimal */
	char scratch[128];
	register ret, i;
	extern void exit();

	DEBUG((9,"in divorce_parent"));

	setpgrp();		/* listener + kids in own p-group	*/

	if ( (Pid = fork()) < 0 )
		sys_error(E_LSFORK, EXIT);

	if (Pid)  {

		/* parent: open pid output file and
		 *	   write childs process ID to it.
		 *	   If it works, exit 0.
		 *	   If it doesn't, kill the child and exit non-zero.
		 */

		i = sprintf(pidstring,"%d",Pid) + 1; /* add null */
		if ( (ret = write(Pidfd,pidstring,(unsigned)i)) != i ) {

			if (ret < 0)
				sys_error(E_PIDWRITE, CONTINUE);

			signal(SIGCLD, SIG_IGN);
			kill(Pid, SIGTERM);
			error(E_PIDWRITE, EXIT); /* exit with proper code */
		}

	/*
	 * Lock file will be unlocked when parent exits.
	 */

		exit( 0 );	/* parent exits -- child does the work */
	}

	/*
	 * child: close everything but the network fd's
	 * but first, lock the lock file -- blocking lock will wait
	 * for parent to exit.
	 */

	if (locking(Lckfd, 1, 0L) == -1)  {
		sprintf(scratch, "Error (%d) re-locking the lock file", errno);
		logmessage(scratch);
		error(E_SYS_ERROR, EXIT | NOCORE | NO_MSG);
	}

	Background = 1;
	close_files();

}

/*
 * close_files:
 *		Close all files except what we opened explicitly
 *		Including stdout, stderr and anything else
 *		that may be open by whatever exec'ed me.
 *		We also set the close on exec flag on all fd's
 *		Note, that we keep fd's 0, 1, and 2 reserved for
 *		servers by opening them to "/dev/null".
 */

close_files()
{
	register i;

	fclose(stdin);
	fclose(stdout);
	fclose(stderr);

	if (((i = open("/dev/null", O_RDWR)) != 0) || (dup(i) != 1) ||
	    (dup(i) != 2))  {
		logmessage("Trouble opening/duping /dev/null");
		sys_error(E_SYS_ERROR, EXIT | NOCORE);
	}

	for (i = 3; i < NOFILE; i++)  {	/* leave stdout, stderr open	*/
		fcntl(i, F_SETFD, 1);	/* set close on exec flag*/
	}
}





/*
 * catch_signals:
 *		Ignore some, catch the rest. Use SIGTERM to kill me.
 */

catch_signals()
{
	extern void sigterm();
	register int i;

	for (i = 1; i < SIGKILL; i++)
		signal(i, SIG_IGN);
	for (i = SIGKILL + 1; i < SIGTERM; i++)
		signal(i, SIG_IGN);
	signal(SIGTERM,	sigterm);
	for (i = SIGTERM + 1; i < NSIG; i++)
		signal(i, SIG_IGN);
	errno = 0;	/* SIGWIND and SIGPHONE only on UNIX PC */
}


/*
 * rst_signals:
 *		After forking but before exec'ing a server,
 *		reset all signals to defaults.
 *		exec automatically resets 'caught' sigs to SIG_DFL.
 *		(This is quick and dirty for now.)
 */

rst_signals()
{
	register int i;

	for (i = 1; i < SIGKILL; i++)
		signal(i, SIG_DFL);
	for (i = SIGKILL + 1; i < SIGTERM; i++)
		signal(i, SIG_DFL);
	for (i = SIGTERM + 1; i < NSIG; i++)
		signal(i, SIG_DFL);
	errno = 0;	/* SIGWIND and SIGPHONE only on UNIX PC */
}

/*
 * sigterm:	Clean up and exit.
 */

void
sigterm()
{
	extern void exit();

	signal(SIGTERM, SIG_IGN);
	error(E_SIGTERM, EXIT | NORMAL | NOCORE);	/* calls cleanup */
	exit(0);

}



/*
 * listen:	listen for and process connection requests.
 */

static struct pollfd pollfds[2];

listen()
{
	register fd, pid, i, tl;
	register struct t_call *call;
	register struct pollfd *sp;
	char scratch[256];
	unsigned sleep();
	extern void exit();

	DEBUG((9,"in listen"));

	pollfds[0].fd = Nfd1;	/* tells poll() what to do		*/
	pollfds[1].fd = Nfd2;
	pollfds[0].events = pollfds[1].events = POLLIN;

	errno = t_errno = 0;
	while (!(call = (struct t_call *)t_alloc(Nfd1,T_CALL,T_ALL)) ) {
		if ((t_errno != TSYSERR) || (errno != EAGAIN))
			tli_error(E_T_ALLOC, EXIT);
		else
			tli_error(E_T_ALLOC, CONTINUE);
	}

	for (;;)  {

	    DEBUG((9,"listen(): TOP of loop"));

	    pollfds[0].revents = pollfds[1].revents = 0;

	    DEBUG((7,"waiting for a request (via poll)"));

	    if (poll(pollfds, 2, -1) == -1)
		sys_error(E_POLL, EXIT);

	    for (i = 0, sp = pollfds; i < 2; i++, sp++)
	      if (sp->revents)  {

		DEBUG((5,"fd %d poll revents: %x", sp->fd,
			sp->revents));
		fd = sp->fd;

		if (sp->revents != POLLIN)
			sys_error(E_POLL, EXIT);

		if (t_listen(fd, call)) {
			DEBUG((9,"t_listen got t_errno = %d", t_errno));
			if ((t_errno == TLOOK) && ((tl = t_look(fd)) >= 0)) {
				sprintf(scratch, 
					"t_listen error: t_look(fd) = %d", tl);
				logmessage(scratch);
			}
			tli_error(E_T_LISTEN, CONTINUE);
			continue;
		}

		sprintf(scratch,"Connect pending on fd %d",fd);
		logmessage(scratch);

		close(0);

		SPLhi();

		if ((Nfd3 = open_bind(Nodename,0,0)) != 0)  {
			error(E_OPENBIND, CONTINUE);
			if (t_snddis(fd, call))  {
				DEBUG((1,"snddis failed, errno %d, t_errno %d",
					errno, t_errno));
				logmessage("t_snddis failed, exiting");
				error(E_OPENBIND, EXIT | NO_MSG | NOCORE);
			}
			goto lclean;
		}

		SPLlo();

		if (t_accept(fd, Nfd3, call)) {
			DEBUG((9,"t_accept got t_errno = %d", t_errno));
			if ((t_errno == TLOOK) && ((tl = t_look(fd)) >= 0)) {
				if (tl == T_DISCONNECT)
					(void) t_rcvdis(fd, NULL);
				sprintf(scratch, 
					"t_accept error: t_look(fd) = %d", tl);
				logmessage(scratch);
			}
			tli_error(E_T_ACCEPT, CONTINUE);
		}

		else  {
			sprintf(scratch, 
				"Accepted connection request on fd %d",Nfd3);
			logmessage(scratch);


			/*
			 * check if the data base is current, before forking
			 */

			(void) check_dbf();

			/*
			 * fork now, to minimize the window in which connection
			 * requests are ignored.If we can't fork, we end up just
			 * doing an abortive close.
			 */

			if ( (pid = fork()) == -1)  {
				log( E_FORK_SERVICE );
			}

			else if (!pid)  {	/* child -- do the work	*/

				setpgrp();	/* make child independent */
				Child = 1;	/* flag for exit rtns	*/
				if (fd == Nfd1)	/* request on nodename? */
					login(Nfd3,call);   /* nodename	*/
				else
					process(Nfd3,call);  /* netnodename*/

				/*
				 * if login() or process() returns, an error
				 * was encountered.  Errors should be logged
				 * by the subroutines -- exit done here.
				 * (Subroutines shouldn't care if they are
				 * a parent/child/whatever.)
				 *
				 * NOTE: timeout() is an exception; 
				 * it does an exit.
				 * There can be no TLI calls after this point.
				 * (non-tli modules may have been pushed)
				 *
				 */


#ifdef	COREDUMP
				abort();
#endif
				exit(1);
			}
		}

lclean:

		(void) memset(call->addr.buf, (char)0, call->addr.maxlen);
		(void) memset(call->opt.buf, (char)0, call->opt.maxlen);
		(void) memset(call->udata.buf, (char)0, call->udata.maxlen);
		call->addr.len = 0;
		call->opt.len = 0;
		call->udata.len = 0;

		if (Nfd3 >= 0) {

			DEBUG((7,"listen(): closing fd %d",Nfd3));

			t_close(Nfd3);
		}

		SPLhi();

		if (dup(1) != 0)
			logmessage("Trouble duping fd 0");

		SPLlo();

	    }
	}
}




/*
 * process:	process an accepted connection request
 *		for ms-net or NLP service.
 */

process(fd, call)
	register fd;
	struct t_call *call;
{
	register size;
	char buf[RCVBUFSZ];
	char **argv;
	register char *bp = buf;
	register dbf_t *dbp;
	extern dbf_t *getdbfentry();
	extern char **mkdbfargv();

	DEBUG((9,"in process (NLPS/SMB message)"));

	if ((size = getrequest(fd, bp)) <= 0)  {
		logmessage("process(): No/bad service request received");
		return(-1);
	}

	if (size < MINMSGSZ)  {
		DEBUG((7,"process(): msg size (%d) too small",size));
		logmessage("process(): Message size too small");
		return(-1);
	}

	/*
	 * if message is NLPS protocol...
	 */

	if ((!strncmp(bp,NLPSIDSTR,NLPSIDSZ))  && 	/* NLPS request	*/
	    (*(bp + NLPSIDSZ) == NLPSSEPCHAR)) {
		nls_service(fd, bp, size, call);
		(void)sleep(10);	/* if returned to here, then 
				 * must sleep for a short period of time to
				 * insure that the client received any possible
				 * exit response message from the listener.
					 */

	/*
	 * else if message is for the MS-NET file server...
	 */

	} else if ( (*bp == (char)0xff) && (!strncmp(bp+1,SMBIDSTR,SMBIDSZ)) )  {
		if (dbp = getdbfentry(DBF_SMB_CODE))
		    if (dbp->dbf_flags & DBF_OFF)
			logmessage("SMB message, server disabled in data base");
		    else    {
			argv = mkdbfargv(dbp);
			smbservice(fd, bp, size, call, argv);
		    }
		else
			logmessage("SMB message, no data base entry");

	/*
	 * else, message type is unknown...
	 */

	} else  {
		logmessage("Process(): Unknown service request (ignored)");
		DEBUG((7,"msg size: %d; 1st four chars (hex) %x %x %x %x",
			*bp, *(bp+1), *(bp+2), *(bp+3)));
	}

	/*
	 * the routines that start servers return only if there was an error
	 * and will have logged their own errors.
	 */

	return(-1);
}



/*
 * getrequest:	read in a full message.  Timeout, in case the client died.
 *		returns: -1 = timeout or other error.
 *			 positive number = message size.
 */

int
getrequest(fd, bp)
	register fd;
	register char *bp;
{
	register size;
	int flags;
	extern void timeout();
	extern unsigned alarm();

	DEBUG((9,"in getrequest"));

	signal(SIGALRM, timeout);
	(void)alarm(ALARMTIME);

	size = l_rcv(fd, bp, RCVBUFSZ, &flags);

	(void)alarm(0);
	signal(SIGALRM, SIG_IGN);

	DEBUG((7,"t_rcv returned %d, flags: %x, errno = %d, t_errno = %d\n",size,flags,errno,t_errno));

	if (size < 0)  {
		tli_error(E_RCV_MSG, CONTINUE);
		return(size);
	}

	return(size);
}


/*
 * timeout:	SIGALRM signal handler.  Invoked if t_rcv timed out.
 *		See comments about 'exit' in process().
 */


void
timeout()
{
	error(E_RCV_TMO, EXIT | NOCORE);
}



/*
 * nls_service:	Validate and start a server requested via the NLPS protocol
 *
 *		version 0:1 -- expect "NLPS:000:001:service_code".
 *
 *	returns only if there was an error (either msg format, or couldn't exec)
 */

static char *badversion = 
	"Unknown version of an NLPS service request: %d:%d";
static char *disabledmsg = 
	"Request for service code <%s> denied, service is disabled";
static char *nlsunknown =
	"Request for service code <%s> denied, unknown service code";


/*
 * Nlsversion can be used as a NLPS flag (< 0 == not nls service)
 * and when >= 0, indicates the version of the NLPS protocol used
 */

static int Nlsversion = -1;	/* protocol version	*/

int
nls_service(fd, bp, size, call)
	int fd, size;
	char *bp;
	struct t_call *call;
{
	int low, high;
	char svc_buf[64];
	register char *svc_code_p = svc_buf;
	char scratch[256];
	register dbf_t *dbp;
	extern dbf_t *getdbfentry();
	extern char **mkdbfargv();
	

	if (nls_chkmsg(bp, size, &low, &high, svc_code_p))  {
		if ((low == 0) || (low == 2))
			Nlsversion = low;
		else  {
			sprintf(scratch, badversion, low, high);
			logmessage(scratch);
			error(E_BAD_VERSION, CONTINUE);
			return(-1);
		}

		DEBUG((9,"nls_service: protocol version %d", Nlsversion));

		/*
		 * common code for protocol version 0 or 2
		 * version 0 allows no answerback message
		 * version 2 allows exactly 1 answerback message
		 */

		if (dbp = getdbfentry(svc_code_p))
		    if (dbp->dbf_flags & DBF_OFF)  {
			sprintf(scratch, disabledmsg, svc_code_p);
			logmessage(scratch);
			nls_reply(NLSDISABLED, scratch);
		    }  else
			start_server(fd, dbp, (char **)0, call);
			/* return is an error	*/
		else  {
			sprintf(scratch, nlsunknown, svc_code_p);
			logmessage(scratch);
			nls_reply(NLSUNKNOWN, scratch);
		}

	}  else
		error(E_BAD_FORMAT, CONTINUE);

	/* if we're still here, server didn't get exec'ed	*/

	return(-1);
}



/*
 * nls_chkmsg:	validate message and return fields to caller.
 *		returns: TRUE == good format
 *			 FALSE== bad format
 */

nls_chkmsg(bp, size, lowp, highp, svc_code_p)
	char *bp, *svc_code_p;
	int size, *lowp, *highp;
{

	/* first, make sure bp is null terminated */

	if ((*(bp + size - 1)) != (char)0)
		return(0);

	/* scanf returns number of "matched and assigned items"	*/

	return(sscanf(bp, "%*4c:%3d:%3d:%s", lowp, highp, svc_code_p) == 3);

}

/*
 * nls_reply:	send the "service request response message"
 *		when appropriate.  (Valid if running version 2 or greater).
 *		Must use write(2) since unknown modules may be pushed.
 *
 *		Message format:
 *		protocol_verion_# : message_code_# : message_text
 */

static char *srrpprot = "%d:%d:%s";

nls_reply(code, text)
	register code;
	register char *text;
{
	char scratch[256];

	/* Nlsversion = -1 for login service */

	if (Nlsversion >= 2)  {
		DEBUG((7, "nls_reply: sending response message"));
		sprintf(scratch, srrpprot, Nlsversion, code, text);
		t_snd(0, scratch, strlen(scratch)+1, 0);
	}
}


/*
 * common code to  start a server process (for any service)
 * if optional argv is given, info comes from o_argv, else pointer
 * to dbf struct is used.  In either case, first argument in argv is
 * full pathname of server. Before exec-ing the server, the caller's
 * logical address, opt and udata are addded to the environment. 
 */

start_server(netfd, dbp, o_argv, call)
	int  netfd;
	register dbf_t *dbp;
	register char **o_argv;
	struct t_call *call;
{
	char *path;
	char **argvp;
	char msgbuf[256];
	extern svr_environ();


	/*
	 * o_argv is set during SMB service setup only, in
	 * which case dbp is NULL.
	 */

	if (o_argv)
		argvp = o_argv;
	else
		argvp = mkdbfargv(dbp);
	path = *argvp;

	if (senviron(call))  {
		logmessage("Can't expand server's environment");
	}

	/* set up stdout and stderr before pushing optional modules */

	(void) close(1);
	(void) close(2);

	if (dup(0) != 1 || dup(0) != 2) {
		logmessage("Dup of fd 0 failed");
		exit(2);
	}

	sprintf(msgbuf,"Starting server (%s)",path);
	nls_reply(NLSSTART, msgbuf);
	logmessage(msgbuf);

	/* after pushmod, tli calls are questionable?		*/

	if (dbp && pushmod(netfd, dbp->dbf_modules)) {
		logmessage("Can't push server's modules: exit");
		exit(2);
	}

	rst_signals();
	execve(path, argvp, Nenviron);

	/* exec returns only on failure!		*/

	logmessage("listener could not exec server");
	sys_error(E_SYS_ERROR, CONTINUE);
	return(-1);
}

/*
 * senviron:	Update environment before exec-ing the server:
 *		The callers logical address is placed in the
 *		environment in hex/ascii character representation.
 *
 * Note:	no need to free the malloc'ed buffers since this process
 *		will either exec or exit.
 *
 *		If you add additional parameters, be sure to increase
 *		NENVIRON.
 */

static char provenv[2*PATHSIZE];

int
senviron(call)
	register struct t_call *call;
{
	register char *p;
	register char **np = Nenviron;
	extern void nlsaddr2c();
	extern char *mkenv();

	if (p = mkenv(HOME))
		*np++ = p;

	if ((p = (char *)malloc(((call->addr.len)<<1) + 18)) == NULL)
		return(-1);
	strcpy(p, NLSADDR);
	strcat(p, "=");
	nlsaddr2c(p + strlen(p), call->addr.buf, call->addr.len);
	DEBUG((7, "Adding %s to server's environment", p));
	*np++ = p;

	if ((p = (char *)malloc(((call->opt.len)<<1) + 16)) == NULL)
		return(-1);
	strcpy(p, NLSOPT);
	strcat(p, "=");
	nlsaddr2c(p + strlen(p), call->opt.buf, call->opt.len);
	DEBUG((7, "Adding %s to server's environment", p));
	*np++ = p;

	p = provenv;
	strcpy(p, NLSPROVIDER);
	strcat(p, "=");
	strcat(p, Netspec);
	DEBUG((7, "Adding %s to environment", p));
	*np++ = p;

	if ((p = (char *)malloc(((call->udata.len)<<1) + 20)) == NULL)
		return(-1);
	strcpy(p, NLSUDATA);
	strcat(p, "=");
	if ((int)call->udata.len >= 0)
		nlsaddr2c(p + strlen(p), call->udata.buf, call->udata.len);
	DEBUG((7, "Adding %s to server's environment", p));
	*np++ = p;

	if (p = mkenv(PATH))
		*np++ = p;

	*np = (char *)0;
	return (0);
}


/*
 * mkenv:	malloc space for a new copy of an existing environ
 *		varaible.  Special code for HOME to use current directory.
 */

char *
mkenv(s)
	register char *s;
{
	register char *p, *m;
	extern char *getenv();

	if (!strcmp(s, HOME))
		p = Home;
	else if (!(p = getenv(s)))  {
		DEBUG((7, "mkenv: getenv(%s) failed", s));
		goto fail;
	}

	if (m = (char *)malloc((unsigned)(strlen(s)+strlen(p)+2)))  {
		strcpy(m, s);
		strcat(m,"=");
		strcat(m, p);
		DEBUG((7,"mkenv: %s", m));
		return(m);
	}
#ifdef	DEBUGMODE
	else  {
		DEBUG((7,"mkenv: malloc(%d) failed", strlen(s)+strlen(p)+2));
	}
#endif

fail:
	DEBUG((7, "mkenv(%s) failed", s));
	return ((char *)0);
}

/*
 * login:	Start the intermediary process that handles
 *		pseudo-tty getty/login service.
 */

login(fd, call)
	register fd;
	struct t_call *call;
{
	register dbf_t *dbp;

	DEBUG((9,"in login (request for intermediary)"));

	if (!(dbp = getdbfentry(DBF_INT_CODE)) || (dbp->dbf_flags & DBF_OFF))  {
		log( E_NOINTERMEDIARY );
		return(-1);
	}

	start_server(fd, dbp, (char **)0, call);

	/* returns only on failure!			*/

	logmessage("listener could not exec the intermediary process");
	sys_error(E_SYS_ERROR, CONTINUE);

	return(-1);
}


/*
 * isdigits:	string version of isdigit.  (See ctype(3))
 */

int
isdigits(p)
	register char *p;
{
	register int flag = 1;

	if (!strlen(p))
		return(0);

	while (*p)
		if (!isdigit(*p++))
			flag = 0;
	return(flag);
}


/*
 * pushmod:	push modules if defined in the data base entry.
 *
 *		NOTE: there are no modules to push in WTLI.
 *		so this code is a noop on the S4.  However,
 *		the data base file is compatible.
 *
 *		WARNING: This routine writes into the in-memory copy
 *		of the database file.  Therefore, after it is called,
 *		the incore copy of the database file will no longer be valid.
 */

int
pushmod(fd, mp)
int fd;
register char *mp;
{
#ifndef	S4
	register char *cp = mp;
	register int pflag = 0;
	char name[32];

	DEBUG((9,"in pushmod:"));
	if (!mp) {
		DEBUG((9,"NULL list: exiting pushmod"));
		return(0);
	}
	while (*cp) {
	    if (*cp == ',') {
	    	*cp = (char)0;
		if (*mp && strcmp(mp, "NULL")) {

	/*
	 * if first time thru, pop off TIMOD if it is on top of stream
	 */

		    if  (!pflag) {
			pflag++;
			if (ioctl(fd, I_LOOK, name) >= 0) {
			    if (strcmp(name, "timod") == 0) {
				if (ioctl(fd, I_POP) < 0)
				    DEBUG((9,"pushmod: I_POP failed"));
			    }
			}
		    }

		    DEBUG((9,"pushmod: about to push %s",mp));
		    if (ioctl(fd, I_PUSH, mp) < 0) {
			DEBUG((9,"pushmod: ioctl failed, errno = %d",errno));
			return(1);
		    }

		} /* if */
		mp = ++cp;
		continue;
	    } /* if */
	    cp++;
	} /* while */
	DEBUG((9,"exiting pushmod:"));
#endif
	return(0);
}


int
l_rcv(fd, bufp, bytes, flagp)
int fd;
char *bufp;
int bytes;
int *flagp;
{
	register int n;
	register int count = bytes;
	register char *bp = bufp;


	do {
		*flagp = 0;
		n = t_rcv(fd, bp, count, flagp);
		if (n < 0)
			return(n);
		count -= n;
		bp += n;
	} while (((*flagp) & T_MORE) && (count > 0));

	return(bp - bufp);
}

