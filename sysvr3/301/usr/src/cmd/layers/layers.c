/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)xt:layers.c	2.21"

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
	Bring up the Blit layers universe
	This version of layers provides a .profile-like startup with
	layers coordinates and commands brought up from a file specification. 
	to execute:
	layers -f file
	where file contains
		origin.x origin.y corner.x corner.y shell command 1
		origin.x origin.y corner.x corner.y shell command 2
	The first layer will be created at the rectangle coordinates
		given in the first line and 'shell command 1' will be executed.
	The second layer will be created at the rectangle coordinates
		given in the second line and 'shell command 1' will be executed.
	and so on.
	The last line of the file will be the current layer.
	An {8, 8, 792, 1016} border will be enforced complying with mouse
		created layers. Layers which have an origin outside the
		border will be moved inside. (corners are truncated)
*/

#include	"sys/types.h"
#include	"sys/stat.h"
#include	"sys/errno.h"
#include	"sys/tty.h"
#include	"sys/jioctl.h"
#include	"sys/xtproto.h"
#include	"sys/xt.h"
#include	"sys/termio.h"
#include	"fcntl.h"
#include	"signal.h"
#include	"stdio.h"
#include	"sxtstat.h"

#define 	DEBUG	0
#define		MAXSTR	256

#define TERM_1_0 "\033[?8;7;1"
#define TERMB_1_0 "\033[?8;7;2"
#define TERM_1_1 "\033[?8;7;3c"
#define ENC_CHK "\033[F"
#define	ENC_LEN	4
#define TERMIDSIZE 9

FILE		*fpf, *fopen();
char	cntlf[]		 = "/dev/xt000";
char	dmdprog[MAXSTR]	 = "";
char	dmdenv[MAXSTR];
char	shell[MAXSTR]	 = "sh";
char	iopt[]		 = "-i";
char	icopt[]		 = "-ic";

char	*	execarg[] = {
	shell,
	iopt,
	0,
	0
};


char	relogin[]	 = "/usr/lib/layersys/relogin";
char	usage[]		 = "usage: %s [-dpst] [-f start-prgm] [layersys-prgm]\n";

#define		TERMEQZ		5
#define		LINKTIMO	10

#ifdef	vax
#define	JMSGSIZE	sizeof (struct jerqmesg)
#else
#define	JMSGSIZE	2
#endif

struct jerqmesg Termesg		 = 
{
	JTERM
};


struct termio ttysave, ttyraw;
struct stat myttystat;


char	sld = 0;
char	dbg = 0;
char	errbuf[BUFSIZ];
char	*		myttyfn;
char	*		name;
int	shells[MAXPCHAN];
char	firstshell;

void		control();
void		dorun();
void		doexec();
void		error();
void		raw();
void		resetlogin();
void		sigcatch();
void		undoshell();

extern void	xtraces();
extern void	xtstats();

extern unsigned	alarm();
extern void	exit();
extern char *	strcpy();
extern int	strncmp();
extern int	strlen();
extern char *	ttyname();
extern char *	getenv();

extern int	errno,
sys_nerr;
extern char	*	sys_errlist[];

#define		SYSERROR	(-1)

#if DEBUG == 1
#define	trace(A)	Trace(A)
#define	trace2(A,B)	Trace(A,B)
#define	trace3(A,B,C)	Trace(A,B,C)
void			Trace();
extern char	*		ctime();
extern long	time();
#else
#define	trace(A)
#define	trace2(A,B)
#define	trace3(A,B,C)
#endif

#define BINARY_LOAD	0
#define HEX_LOAD	1
#define	ENC_ENABLE	2		/* Value that enables terminal encoding */

int Loadtype = BINARY_LOAD;
char romversion;


int
main(argc, argv, envp)
int	argc;
register char *argv[];
register char *envp[];
{
	struct sxtblock sxtstat;
	register int	retval;
	char		ignbrk = 0;
	char		statson = 0;
	char		traceon = 0;
	char		ffile = 0;
	char	       *myshell;
	register int	cntlfd;
	extern int	optind;
	extern char    *optarg;


	name = *argv;		/* save our name */
	myttyfn = ttyname(1);

	if (ioctl(1, JMPX, 0) != -1) {
		fprintf(stderr,"layers: Cannot invoke layers from within a layer\n");
		exit(1);
	}
	errno = 0;
	if (ioctl(1,SXTIOCSTAT,&sxtstat) != -1) {
		fprintf(stderr,"layers: Cannot invoke layers from within shl\n");
		exit(1);
	}
	errno = 0;

	while ((retval = getopt (argc, argv, "dpstf:")) != EOF)
		switch ( retval ) {
		case 'b':
			ignbrk++;
			break;
		case 's':
			statson++;
			break;
		case 't':
			traceon++;
			break;
		case 'p':
			sld++;
			break;
		case 'd':
			dbg++;
			break;
		case 'f':
			if (openf(optarg))
				exit(1);
			ffile++;
			break;
		default:
			fprintf(stderr,usage, name);
			exit(1);
		}
	if (optind < argc-1) {
		error("extra arguments at end of command line", "");
		fprintf(stderr,usage, name);
		exit(1);
	}
	if (optind == (argc - 1)) {
		strcpy(dmdprog,argv[optind]);
	}

	if ((myshell = getenv("SHELL")) != NULL && myshell[0] != '\0')
	{
		strcpy (shell, myshell);
	}

	if ( (cntlfd = opencntlf()) == SYSERROR )
		exit(2);

	sigcatch(SIGALRM);
	sigcatch(SIGTERM);
	sigcatch(SIGHUP);
	if (!ignbrk)
		sigcatch(SIGINT);
	setbuf(stderr, errbuf);
	raw(1,&ttysave);
	(void)fstat(1, &myttystat);
	(void)chmod(myttyfn, 0600);

	if ( boot(dmdprog) == SYSERROR ) {
		(void)ioctl(1, TCSETAW, &ttysave);
		(void)fflush(stderr);
		exit(3);
	}

	trace("boot returns");
	(void)alarm(3);
	(void)pause();

	if ( multiplex(cntlfd, 1) ) {
		ttyraw.c_cc[VMIN] = JMSGSIZE;
		(void) ioctl(cntlfd, TCSETAW, &ttyraw);
		if ( traceon )
			xtraces(cntlfd, stderr);
		if (!ignbrk) {
			struct termio ttybrk;

			(void)ioctl(1,TCGETA,&ttybrk);
			ttybrk.c_iflag = BRKINT;
			ttybrk.c_cc[VMIN] = 4;
			(void)ioctl(1,TCSETAW,&ttybrk);
			trace("done setting up brk");
		}

		signal(SIGCLD, SIG_IGN);

		if ( ffile ) {
			trace("readf enters");
			if (readf(cntlfd) == 1 )
				error("executing start-up script - command aborted","");
			trace("readf returns");
		}
		control(cntlfd);
		trace("control returns");
		termshells();
		trace("termshells returns");
		(void)write(cntlfd, (char *) & Termesg, JMSGSIZE);
		(void)alarm(2);
		(void)pause();
		cntlf[strlen(cntlf)-1] = '\0';	/* Name of channel group */
#ifndef u3b5
		resetlogin(cntlfd, cntlf);
#endif
		retval = 0;
	} else
getout:		
		retval = 4;

#ifdef u3b5
	(void)close(cntlfd);
#endif
	(void)ioctl(1, TCSETAW, &ttysave);
	(void)fflush(stderr);

	if ( traceon )
		xtraces(cntlfd, stderr);

	if ( statson )
		xtstats(cntlfd, stderr);

#ifdef	u3b5
	if(retval == 0)
		resetlogin(cntlfd, cntlf);
#else
	(void)close(cntlfd);
#endif
	(void)chmod(myttyfn, myttystat.st_mode & 0777);
	trace("layers returns");

	exit(retval);
}


int
openf(argv)
char	*argv;
{
	if ( (fpf = fopen(argv, "r")) == (FILE * )NULL ) {
		error("opening file: '%s'", argv);
		return 1;
	}
	trace3("opened file %d (fd=%d)", fpf, fileno(fpf));
	return 0;
}


int
opencntlf()
{
	register int	fd;
	register int	i;
	int	geteuid();
	register int	mode;
	struct stat	buf;
	int	fstat();

	if ( geteuid() != 0 ) {
		error("layers must have U+S privileges", "");
		return SYSERROR;
	}

	i = sizeof cntlf - 3;
	trace2("cntlf index = %d",i);

	while (1)
		if ( (fd = open(cntlf, O_RDWR | O_EXCL)) == SYSERROR ) {
			trace2("couldn't open %s", cntlf);

			switch ( errno ) {
			case ENXIO:
			case EBUSY:
				if ( cntlf[i] == '9' ) {
					cntlf[i-1]++;
					cntlf[i] = '0';
				} else
					cntlf[i]++;
				continue;
			}
			if ( cntlf[i-1] != '0' || cntlf[i] != '0' ) {
				errno = 0;
				error("no spare channels", "");
			} else
				error("can't open multiplexor", cntlf);
			break;
		}
		else
		 {
			int	uid = getuid();
			int	gid = getgid();

			i++;

			if ( fstat(1, &buf) != 0) /* keep the same mode as on original tty */
				mode = 0622; /* if can't stat, 622 is a safe bet */
			else
				mode = buf.st_mode & 0777; /* else do it right */

			while ( chown(cntlf, uid, gid) != SYSERROR ) {
				(void)chmod(cntlf, mode);
				if ( cntlf[i]++ == '7' )
					break;
			}
			if (cntlf[i] != '8' && errno != ENOENT) {
				error("can't chown '%s'", cntlf);
				(void)close(fd);
				fd = SYSERROR;
			} else {
				cntlf[i] = '0';

				if (setgid(gid) == SYSERROR || setuid(uid) == SYSERROR) {
					error("can't setgid/setuid");
					(void)close(fd);
					fd = SYSERROR;
				}
				break;
			}
		}

	trace3("opened %s, fd=%d", cntlf, fd);

	return fd;
}



int
boot(prog)
char	*prog;
{
	char   *dmdenv;
	char	command[MAXSTR];
	char	loader[MAXSTR];
	int	ttyp, tver;
	short	supplied, v1, DMDset;
	short	exists, empty;
	struct stat  statbuf;

	if(check_term(&ttyp,&tver) == SYSERROR)
		return SYSERROR;

	v1 = (ttyp == 7 && tver <= 4);
	supplied = (prog[0] != '\0');
	dmdenv = getenv("DMD");
	DMDset = (dmdenv != NULL && dmdenv[0] != '\0');
	if (!supplied)
		sprintf(prog, "%s/lib/layersys/lsys.8;%d;%d",
			DMDset ? dmdenv : "/usr", ttyp,tver);
	exists = (stat(prog,&statbuf) != -1);
	empty = (statbuf.st_size == 0);

	if (supplied && !exists) {
		error("can't load terminal program",prog);
		return SYSERROR;
	}
	if (v1 && (!exists || empty)) {
		error("5620 version 1 firmware requires non-zero download: %s",prog);
		return SYSERROR;
	}
	if (!DMDset) {
		/*
		 *  Use Generic Windowing package
		 */
		if (!exists || empty)  /* && not v1 */ {
			/* default init */
			write(1, Loadtype ? "\033[2;2v" : "\033[2;0v", 6);
			return 0;
		}
		else
			(void)strcpy(loader,"/usr/lib/layersys/wtinit");
	}
	else {
		/*
		 *  Use Terminal Feature Package
		 */
		if (!exists)  /* && not supplied */ {
			prog[strlen(prog)-1] = '?';
			if (access(prog,4) == -1) {
				error("terminal id unknown to $DMD software",prog);
				return SYSERROR;
			}
		}
		(void)sprintf(loader, "%s/bin/wtinit", dmdenv);
		if (access(loader,1) == -1) {
			(void)sprintf(loader, "%s/bin/32ld", dmdenv);
			errno = 0;
		}
	}

	if(access(loader,1) == -1) {
		error("executing file: '%s'", loader);
		return SYSERROR;
	}

	if (v1) {		/* 1.1 or 1.2 5620 firmware */
		(void)sprintf(command,"%s /usr/lib/layersys/set_enc.j",loader);
		trace(command);
		if ( system(command) ) {
			error("can't download /usr/lib/layersys/set_enc.j","");
			return SYSERROR;
		}
		printf("%d",Loadtype);
		fflush(stdout);
		sleep(2);
		printf("\n\n\n\n\t\tPlease stand by.  Downloading layers...\r\n");
		printf("\n\t\t(May not be visible for several seconds) ");
		fflush(stdout);
		sleep(3);
	}

	(void)sprintf(command,"%s -l %s %s '%s'",
	              loader, sld?"-p":"", dbg?"-d":"", prog);
	trace(command);
	errno = 0;
	if ( system(command) ) {
		error("can't load '%s'", prog);
		return SYSERROR;
	}
	return 0;
}


int
check_term(type,vers)
int *type, *vers;
{
	char   *dwnldflag;
	int	i;
	int	enc_flg;

	if ( (dwnldflag = getenv("DMDLOAD")) != NULL && strcmp(dwnldflag,"hex") == 0 )
		Loadtype = HEX_LOAD;

	i = enq_term("\033[c","\033[?8;%d;%dc",type,vers);
	if (i != 2) {
		error("Not an XT windowing terminal", "");
		return SYSERROR;
	}
	if (*type == 7 && *vers <= 2) {
		error("obsolete 5620 firmware version","");
		return SYSERROR;
	}
	if (*type != 7 || *vers >= 5) {
		i = enq_term("\033[F","\033[%dF",&enc_flg,0);
		if (i != 1) {
			error("invalid encoding response from terminal","");
			return SYSERROR;
		}
		if (enc_flg || Loadtype)
			Loadtype = ENC_ENABLE;
	}
	return 0;
}


enq_term(enq,resp,val1,val2)
char *enq, *resp;
int *val1, *val2;
{
	register i;

	/* assume raw(1) executed before this routine called */
	write(1,enq,strlen(enq));
	/* assume sigcatch(SIGALRM) executed before this routine */
	alarm(7);
	i = fscanf(stdin,resp,val1,val2);
	alarm(0);
	if (ferror(stdin))  i = EOF;
	return i;
}


int
multiplex(xtfd, ttyfd)
int	xtfd;
int	ttyfd;
{
	struct xtioclm lm;
	register char	*	e1;
	register char	*	e2;
	register int	i;
	short	int cmd;

	trace3("multiplex entered, xtfd=%d ttyfd=%d", xtfd, ttyfd);

	if (Loadtype != BINARY_LOAD)
		cmd = HXTIOCLINK;		/* "hex-mode" */
	else
		cmd = XTIOCLINK;

	lm.fd = ttyfd;
	lm.nchans = MAXPCHAN;
	(void)alarm(LINKTIMO);

	if ( ioctl(xtfd, cmd, lm) == SYSERROR ) {
		(void)alarm(0);
		e2 = cntlf;

		switch ( errno ) {
		case EINTR:	
			e1 = "timeout - no response after %d seconds";
			e2 = (char *)LINKTIMO;	
			errno = 0;	
			break;
		case EINVAL:	
			e1 = "bad arg for link ioctl, call a DMD administrator";	
			break;
		case ENOTTY:	
			e1 = "not connected to a tty device";	
			e2 = myttyfn;	
			break;
		case ENXIO:	
			e1 = "'linesw' not configured for 'xt' driver, call a DMD administrator";	
			break;
		case ENOMEM:	
			e1 = "no memory for kernel configuration, try again";	
			break;
		case EBUSY:	
			e1 = "multiplex pre-empted, call a DMD administrator";	
			break;
		case EIO:	
			for ( i = 0 ; i < 2 ; i++) {
				(void)alarm(2);
				(void)pause();
				if ( ioctl(xtfd, JTIMO, 0) != SYSERROR )
					break;
			}
			if ( i == 2 ) {
				e1 = "system out of clists, try again";	
				break;
			}
			return 1;
		default:	
			e1 = "unknown link ioctl error, call a DMD administrator";	
			break;
		}
		error(e1, e2);
		return 0;
	}

	(void)alarm(0);
	return 1;
}


int
readf(fd)
int	fd;
{
	int	x1, y1, x2, y2;
	int	chan, err;
	int	good_chan = -1;
	int	rv = 0;
	char	string[MAXSTR], command[MAXSTR];

	while (fgets(string, MAXSTR, fpf) != NULL ) {
		strcpy(command, shell);
		err = sscanf(string, "%d %d %d %d %[^\n]", &x1, &y1, &x2, &y2, command);
		if ( err == 5 ) {
			if( (chan = Newlayer(fd, x1, y1, x2, y2)) == EOF )
			{
				trace("error from Newlayer");
				error("executing start-up script - error from Newlayer","");
				rv = 1;
				break;
			}
			(void)doexec(chan, command);
			good_chan = chan;
		} else
			error("file input line illegal", "ignored");
	}

	if (good_chan != -1)
		Current(fd, good_chan);

	return (rv);
}


void
control(fd)
int	fd;
{
	struct jerqmesg rbuf;

	trace("control entered");

	errno = 0;
	while ( read(fd, (char *) & rbuf, JMSGSIZE) == JMSGSIZE )
		switch ( rbuf.cmd ) {
		case C_NEW:	
			doexec(rbuf.chan, shell);		
			continue;
		case C_DELETE:	
			undoshell(rbuf.chan);	
			continue;
		case C_RUN:
			dorun(fd,rbuf.chan);
			continue;
		case C_EXIT:	
			return;
		default:	
			error("bad control command '%d'", (char *)(unsigned char)rbuf.cmd);
			return;
		}

	error("control channel read", cntlf);
}



void
raw(fd,ttyp)
int	fd;
struct termio *ttyp;
{
	(void)ioctl(fd, TCGETA, ttyp);
	ttyraw.c_iflag = BRKINT;
	ttyraw.c_cflag = (ttyp->c_cflag & (CBAUD | CLOCAL)) | CS8 | CREAD;
	ttyraw.c_cc[VMIN] = 4;
	(void)ioctl(fd, TCSETAW, &ttyraw);
}



void
error(s1, s2)
char	*	s1;
char	*	s2;
{
	trace((char *)0);
	(void)fprintf(stderr, "%s - error - ", name);
	(void)fprintf(stderr, s1, s2);
	if ( errno ) {
		register char	*	ep;

		if ( errno < sys_nerr )
			ep = sys_errlist[errno];
		else
			ep = "Unknown error";
		(void)fprintf(stderr, " %s: %s\r\n", s2, ep);
	} else
		(void)fprintf(stderr, "\r\n");
}



void
dorun(fd,chan)
int fd,chan;
{
	char command[TTYHOG+1],c,*tc;

	tc = command;
	c='\01';
	while ( c != '\0'){
		if (read(fd,&c,1) != 1)
			{
			error("control channel read",cntlf);
			return;
			}
		*tc++ = c;
	}
	doexec(chan,command);
}


void
doexec(chan, command)
int	chan;
char	*command;
{
	register int	i, j;

	trace2("doexec chan %d", chan);
	trace2("doexec command=%s", command);
	if ( shells[chan] )
		undoshell(chan);

	j = strlen(cntlf) - 1;
	cntlf[j] = '0' + chan;

	switch ( i = fork() ) {
	case 0:
		(void)setpgrp();
		while ( close(i++) != SYSERROR );
		signal(SIGCLD, SIG_DFL);

		if ( open(cntlf, O_RDWR) != 0 ) {
			exit(1);
			break;
		}
		(void)dup(0);	
		(void)dup(0);
		/* ttysave.c_iflag &= ~(IXON | IXANY); Don't require this anymore (bgs) */
		(void)ioctl(0, TCSETA, &ttysave);
		if ( firstshell == 0 )
			resetlogin(3, myttyfn);
		if ( strcmp(shell, command) == 0 ) {
			execarg[1] = iopt;
			execarg[2] = 0;
		} else
		 {
			execarg[1] = icopt;
			execarg[2] = command;
		}
		(void)execvp(*execarg, execarg);
		error("execvp", *execarg);
		exit(1);
		break;

	case SYSERROR:
		if ( (i = open(cntlf, O_WRONLY)) != SYSERROR ) {
			(void)write(i, "Cannot fork, try again.", 23);
			(void)close(i);
		}
		break;

	default:
		shells[chan] = i;
		firstshell = 1;
		break;
	}

	cntlf[j] = '0';
}



void
undoshell(chan)
register int	chan;
{
	register int	spid;

	trace2("undoshell chan %d", chan);

	if (shells[chan] == 0)
		return;
	if (killcheck(chan,SIGHUP,6) == 0)	/* if hangup doesn't work, */
		killcheck(chan,SIGKILL,2);	/* kill it for sure */
}


killcheck(chan,sig,timeout)
{
	int pid, i, rv = 0;

	signal(SIGCLD, SIG_DFL);

	/*
	 *  Kill entire process group on channel
	 */
	if (kill(-shells[chan],sig) == -1) { /* process already gone */
		shells[chan] = 0;
		rv = 1;
	} else {
		(void)alarm(timeout);
		while ( (pid = wait((int *)0)) != SYSERROR ) {
			if (pid == shells[chan]) {
				(void)alarm(0);
				shells[chan] = 0;
				trace2("shell terminated chan %d", chan);
				rv = 1;
				break;
			}
			for (i = 0; i < MAXPCHAN; i++ )
				if (pid == shells[i] ) {
					shells[i] = 0;
					trace2("shell terminated chan %d", i);
				}
		}
		(void)alarm(0);
	}

	signal(SIGCLD, SIG_IGN);

	return(rv);
}


void
sigcatch(sig)
int	sig;
{
	(void)signal(sig, sigcatch);
}



termshells()
{
	register int	i;

	for ( i = 0 ; i < MAXPCHAN ; i++)
		if ( shells[i] )
			undoshell(i);
} 


void
resetlogin(fd, fn)
int	fd;
char	*	fn;
{
	switch ( fork() ) {
	case 0:	
		(void)close(fd);
		(void)execl(relogin, relogin, "-", fn, 0);
		error("execl", relogin);
		exit(1);
		break;

	case SYSERROR:	
		error("fork", relogin);
		break;
	}
}




#if DEBUG == 1

/*VARARGS1*/
void
Trace(a, b, c)
char	*	a;
char	*	b;
char	*	c;
{
	long	t;

	(void)time(&t);
	(void)fprintf(stderr, "%.8s ", ctime(&t) + 11);
	if ( a == (char *)0 )
		return;
	(void)fprintf(stderr, a, b, c);
	(void)fprintf(stderr, "\n");
}


#endif



