/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
/* @(#)cu.c	1.12 */
#define ddt
/***************************************************************
 *	cu [-s baud] [-l line] [-h] [-t] [-n] [-o | -e] telno | `dir` | remote
 *
 *	legal baud rates: 110,134,150,300,600,1200,2400,4800,9600.
 *
 *	-l is for specifying a line unit from the file whose
 *		name is defined under LDEVS below.
 *	-h is for half-duplex (local echoing).
 *	-t is for adding CR to LF on output to remote (for terminals).
 *	-d can be used (with ddt) to get some tracing & diagnostics.
 *	-o or -e is for odd or even parity on transmission to remote.
 *	-n will request the phone number from the user.
 *	Telno is a telephone number with `=' for secondary dial-tone.
 *	If "-l dev" is used, speed is taken from LDEVS.
 *
 *	Escape with `~' at beginning of line.
 *	Silent output diversions are ~>:filename and ~>>:filename.
 *	Terminate output diversion with ~> alone.
 *	~. is quit, and ~![cmd] gives local shell [or command].
 *	Also ~$ for canned local procedure pumping remote.
 *	Both ~%put from [to]  and  ~%take from [to] invoke built-ins.
 *	Also, ~%break or just ~%b will transmit a BREAK to remote.
 *	~%nostop toggles on/off the DC3/DC1 input control from remote,
 *		(certain remote systems cannot cope with DC3 or DC1).
 *
 *	As a device-lockout semaphore mechanism, create an entry
 *	in the directory #defined as LOCK whose name is LCK..dev
 *	where dev is the device name taken from the "line" column
 *	in the file #defined as LDEVS.  Be sure to trap every possible
 *	way out of cu execution in order to "release" the device.
 *	This entry is `touched' from the dial() library routine
 *	every hour in order to keep uucp from removing it on
 *	its 90 minute rounds.  Also, have the system start-up
 *	procedure clean all such entries from the LOCK directory.
 ***************************************************************/
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/termio.h>
#include <sys/errno.h>
#include "dial.h"
#include <sys/types.h>
#include <sys/stat.h>

#define LOW	300		/* default speed for modems */
#define HIGH	1200		/* high speed for modems */
#define HIGHER	4800
#define MID	BUFSIZ/2	/* mnemonic */
#define	RUB	'\177'		/* mnemonic */
#define	XON	'\21'		/* mnemonic */
#define	XOFF	'\23'		/* mnemonic */
#define	TTYIN	0		/* mnemonic */
#define	TTYOUT	1		/* mnemonic */
#define	TTYERR	2		/* mnemonic */
#define	YES	1		/* mnemonic */
#define	NO	0		/* mnemonic */
#define	EQUALS	!strcmp		/* mnemonic */
#define EXIT	0		/* exit code */
#define CMDERR	1		/* exit code */
#define NODIAL	2		/* exit code */
#define HUNGUP	3		/* exit code */
#define IOERR	4		/* exit code */
#define SIGQIT	5		/* exit code */
#define NOFORK	6		/* exit code */
#define	GETID	7		/* exit code */
#define	MAXN	100


#define F_NAME 	0
#define	F_TIME	1
#define	F_LINE	2
#define	F_SPEED 3
#define	F_PHONE	4
#define	F_LOGIN	5

#define	CF_PHONE	-1
#define	SAME		0
#define	MAXPH		60
#define MAXC 		300
#define SYSNSIZE	6
#define CF_SYSTEM	-1

int trynum=0;
int sflag=0;
int spelfg=0;


extern int
	_debug,			/* flag for more diagnostics */
	errno,			/* supplied by system interface */
	optind,			/* variable in getopt() */
	strcmp();		/* c-lib routine */

extern unsigned
	sleep();		/* c-lib routine */

extern char
	ttyname,
	*optarg;		/* variable in getopt() */
static FILE *Lsysfp = NULL;

static struct termio tv, tv0;	/* for saving, changing TTY atributes */
static struct termio lv;	/* attributes for the line to remote */
static CALL call;		/* from dial.h */
static char device[15+sizeof(DEVDIR)];

static char
	cxc,			/* place into which we do character io*/
	tintr,			/* current input INTR */
	tquit,			/* current input QUIT */
	terase,			/* current input ERASE */
	tkill,			/* current input KILL */
	teol,			/* current sencondary input EOL */
	myeof;			/* current input EOF */

static int
	terminal=0,		/* flag; remote is a terminal */
	echoe,			/* save users ECHOE bit */
	echok,			/* save users ECHOK bit */
	rlfd,			/* fd for remote comm line */
	child,			/* pid for recieve proccess */
	intrupt=NO,		/* interrupt indicator */
	duplex=YES,		/* half(NO), or full(YES) duplex */
	sstop=YES,		/* NO means remote can't XON/XOFF */
	rtn_code=0,		/* default return code */
	takeflag=NO,		/* indicates a ~%take is in progress */
	w_char(),		/* local io routine */
	r_char(),		/* local io routine */
	savuid,			/* save the uid */
	savgid,			/* save gid */
	savmode;		/* save the mode */

static
	onintrpt(),		/* interrupt routine */
	rcvdead(),		/* interrupt routine */
	hangup(),		/* interrupt routine */
	quit(),			/* interrupt routine */
	bye();			/* interrupt routine */

static void
	flush(),
	shell(),
	dopercen(),
	receive(),
	mode(),
	say(),
#ifdef ddt
	tdmp(),
#endif
	w_str();

char *	sysphone();

char	*msg[]= {
/*  0*/	"usage: %s [-s baud] [-l line] [-h] [-n] [-t] [-d] [-m] [-o|-e] telno | 'dir' | remote\n",
/*  1*/	"interrupt",
/*  2*/	"dialer hung",
/*  3*/	"no answer",
/*  4*/	"illegal baud-rate",
/*  5*/	"acu problem",
/*  6*/	"line problem",
/*  7*/	"can't open L-devices file",
/*  8*/	"Requested device not available\r\n",
/*  9*/	"Requested device/system name not known\r\n",
/* 10*/	"No device available at %d baud\r\n",
/* 11*/	"No device known at %d baud\r\n",
/* 12*/	"Connect failed: %s\r\n",
/* 13*/	"Cannot open: %s\r\n",
/* 14*/	"Line gone\r\n",
/* 15*/	"Can't execute shell\r\n",
/* 16*/	"Can't divert %s\r\n",
/* 17*/	"Use `~~' to start line with `~'\r\n",
/* 18*/	"character missed\r\n",
/* 19*/	"after %ld bytes\r\n",
/* 20*/	"%d lines/%ld characters\r\n",
/* 21*/	"File transmission interrupted\r\n",
/* 22*/	"Cannot fork -- try later\r\n",
/* 23*/	"\r\nCan't transmit special character `%#o'\r\n",
/* 24*/	"\nLine too long\r\n",
/* 25*/	"r\nIO error\r\n",
/* 26*/ "Use `~$'cmd \r\n",
/* 27*/ "Cannot obtain status of the terminal\r\n",
/* 28*/ "Sorry you cannot cu from 3B console\r\n",
/* 29*/ "requested speed does not match\r\n",
/* 30*/ "requested system name not known\r\n",
};
/***************************************************************
 *	main: get command line args, establish connection, and fork.
 *	Child invokes "receive" to read from remote & write to TTY.
 *	Main line invokes "transmit" to read TTY & write to remote.
 ***************************************************************/

main(argc, argv)
char *argv[];
{
	struct stat buff;
	struct stat bufsave;
	char phonebuf[40];
	char s[40];
	char *cp;
	char *string;
	char *phdir;
	int getp;
	int getu;
	int i, j =0;
	int cflag=0;
	int errflag=0;
	int nflag=0;

	lv.c_iflag = (IGNPAR | IGNBRK | ISTRIP | IXON | IXOFF);
	lv.c_cc[VMIN] = '\1';

	call.attr = &lv;
	call.baud = -1;
	call.speed = LOW;
	call.line = NULL;
	call.telno = NULL;
	call.modem = 0;
	call.device = device;
	call.dev_len = sizeof(device);
		
	while((i = getopt(argc, argv, "dhtceomns:l:")) != EOF)
		switch(i) {
			case 'd':
#ifdef	ddt
				_debug = YES;
#else
				++errflag;
				say("Cu not compiled with debugging flag\r\n");
#endif
				break;
			case 'h':
				duplex ^= YES;
				lv.c_iflag &= ~(IXON | IXOFF);
				sstop = NO;
				break;
			case 't':
				terminal = YES;
				lv.c_oflag |= (OPOST | ONLCR);
				break;
			case 'e':
				if(lv.c_cflag & PARENB)
					++errflag;
				else
					goto PAROUT;
				break;
			case 'o':
				if(lv.c_cflag & PARENB)
					++errflag;
				else
					lv.c_cflag = PARODD;
			PAROUT:
					lv.c_cflag |= (CS7 | PARENB);
				break;
			case 's':
				sflag++;
				call.baud = atoi(optarg);
				if(call.baud > LOW)
					call.speed = call.baud;
			  	break;
			case 'l':
				call.line = optarg;
			  	break;
			case 'm':
				call.modem = 1;  /* override modem control */
				break;
			case 'n':
				nflag++;
				printf("Please enter the number: ");
				gets(s);
				break;
			case '?':
				++errflag;
		}

#ifdef  u3b
	if(fstat(1, &buff) < 0) {
		say(msg[27]);
		exit(1);
	} else if(buff.st_rdev == 0) {
		say(msg[28]);
		exit(1);
		}
#endif

	if(call.baud == -1 && call.line == NULL)
		call.baud = LOW;

	if((optind < argc && optind > 0) || (nflag && optind > 0)) {  
		if(nflag) 
			string=s;
		else
			string = argv[optind];
		if(strlen(string) == strspn(string, "0123456789=-*:#;f")) {
			call.telno = string;
		} else {
			if(EQUALS(string, "dir")) {
				if(call.line == NULL)
					++errflag;
			} else { 
			 	phdir = sysphone(string);
			 	trynum++;
				if(strspn(phdir, "0123456789=-*:#;f")) {
					call.telno=phdir;
					if(call.telno == NULL)
						++errflag;
				} else {
					call.line=phdir;
					if(call.line==NULL)
						++errflag;
				}
			}
	 	}
	} else
		if(call.line == NULL)
			++errflag;
	
	if(errflag) {
		say(msg[0], argv[0]);
		exit(1);
	}

	(void)ioctl(TTYIN, TCGETA, &tv0); /* save initial tty state */
	tintr = tv0.c_cc[VINTR]? tv0.c_cc[VINTR]: '\377';
	tquit = tv0.c_cc[VQUIT]? tv0.c_cc[VQUIT]: '\377';
	terase = tv0.c_cc[VERASE]? tv0.c_cc[VERASE]: '\377';
	tkill = tv0.c_cc[VKILL]? tv0.c_cc[VKILL]: '\377';
	teol = tv0.c_cc[VEOL]? tv0.c_cc[VEOL]: '\377';
	myeof = tv0.c_cc[VEOF]? tv0.c_cc[VEOF]: '\04';
	echoe = tv0.c_lflag & ECHOE;
	echok = tv0.c_lflag & ECHOK;

	(void)signal(SIGHUP, hangup);
	(void)signal(SIGQUIT, hangup);
	(void)signal(SIGINT, onintrpt);

	if((rlfd = dial(call)) < 0) {
		if(trynum){ /*Try the next possible number from L.sys*/
			phdir=sysphone(string);
				call.telno = NULL;
				call.line = NULL;
				if(strspn(phdir, "0123456789-=*:#;f")) {
					call.telno=phdir;
					if(call.telno == NULL)
						++errflag;
				} else {
					call.line=phdir;
					if(call.line==NULL)
						++errflag;
				}
				if(errflag) {
					say(msg[0], argv[0]);
					exit(1);
				}
				rlfd=dial(call);
		}
	}
	if(rlfd < 0) {
			if(rlfd == NO_BD_A || rlfd == NO_BD_K)  
				say(msg[-rlfd], call.baud == -1 ? LOW : call.baud);
			else if(rlfd == DV_NT_E)
				say(msg[29]);
			else
				say(msg[12], msg[-rlfd]);
				exit(NODIAL);
	}


/*	This section of the code will provide  a security to cu. 
	The device used by cu will have the owner and group id of
	the user. When cu is ready to be disconnected it will set the 
	owner and group id of the device as it was found, done in the
	hangup().
*/

	getu = getuid();
	getp = getgid();
	if(stat(call.device, &bufsave) <0)  {
		say("Cannot get the status of the device");
		undial(rlfd);
		exit(1);
	}
	savmode = bufsave.st_mode;
	savuid = bufsave.st_uid;
	savgid = bufsave.st_gid;
	if(chown(call.device, getu, getp) <0) {
		say("Cannot chown on %s\n", call.device);
		undial(rlfd);
		exit(1);
	}
	if(chmod(call.device, 0600) <0) {
		say("Cannot chmod on %s\n", call.device);
		undial(rlfd);
		exit(1);
	}
	if(setuid(getu) < 0) {
		say("Cannot set the uid %d\n", getu);
		undial(rlfd);
		exit(1);
	}

	/* When we get this far we have an open communication line */
	mode(1);			/* put terminal in `raw' mode */

	if (Lsysfp != NULL)
		(void) fclose(Lsysfp);
	say("Connected\007\r\n");

	recfork();		/* checks for child == 0 */
	if(child > 0) {
		(void)signal(SIGUSR1, bye);
		(void)signal(SIGHUP, SIG_IGN);
		(void)signal(SIGQUIT, onintrpt);
		rtn_code = transmit();
		quit(rtn_code);
	} else {
		hangup(rlfd);
	}
}

/*
 *	Kill the present child, if it exists, then fork a new one.
 */

recfork()
{
	if (child)
		kill(child, SIGKILL);
	child = dofork();
	if(child == 0) {
		(void)signal(SIGHUP, rcvdead);
		(void)signal(SIGQUIT, SIG_IGN);
		(void)signal(SIGINT, SIG_IGN);
		receive();	/* This should run until killed */
		/*NOTREACHED*/
	}
}

/***************************************************************
 *	transmit: copy stdin to rlfd, except:
 *	~.	terminate
 *	~!	local login-style shell
 *	~!cmd	execute cmd locally
 *	~$proc	execute proc locally, send output to line
 *	~%cmd	execute builtin cmd (put, take, or break)
 ****************************************************************/

int
transmit()
{
	char b[BUFSIZ];
	char prompt[10];
	register char *p;
	register int escape;

#ifdef	ddt
	if(_debug == YES) say("transmit started\n\r");
#endif
	sysname(prompt);
	while(1) {
		p = b;
		while(r_char(TTYIN) == YES) {
			if(p == b)  	/* Escape on leading  ~    */
				escape = (cxc == '~');
			if(p == b+1)   	/* But not on leading ~~   */
				escape &= (cxc != '~');
			if(escape) {
				if(cxc == '\n' || cxc == '\r' || cxc == teol) {
					*p = '\0';
					if(tilda(b+1) == YES)
						return(EXIT);
					break;
				}
				if(cxc == tintr || cxc == tkill || cxc == tquit ||
					    (intrupt && cxc == '\0')) {
					if(!(cxc == tkill) || echok)
						say("\r\n");
					break;
				}
				if(p == b+1 && cxc != terase)
					say("[%s]", prompt);
				if(cxc == terase) {
					p = (--p < b)? b:p;
					if(p > b)
						if(echoe)
							say("\b \b");
						else
						 (void)w_char(TTYOUT);
				} else {
					(void)w_char(TTYOUT);
					if(p-b < BUFSIZ) 
						*p++ = cxc;
					else {
						say(msg[24]);
						break;
					}
				}
			} else {
				if(intrupt && cxc == '\0') {
#ifdef	ddt
					if(_debug == YES)
						say("got break in transmit\n\r");
#endif
					intrupt = NO;
					(void)ioctl(rlfd, TCSBRK, 0);
					flush();
					break;
				}
				if(w_char(rlfd) == NO) {
					say(msg[14]);
					return(IOERR);
				}
				if(duplex == NO)
					if(w_char(TTYERR) == NO)
						return(IOERR);
				if ( (cxc == tintr) || (cxc == tquit) || ( (p==b) && (cxc == myeof) ) ) {
#ifdef	ddt
					if(_debug == YES) say("got a tintr\n\r");
#endif
					flush();
					break;
				}
				if(cxc == '\n' || cxc == '\r' ||
						cxc == teol || cxc == tkill) {
					takeflag = NO;
					break;
				}
				p = (char*)0;
			}
		}
	}
}

/***************************************************************
 *	routine to halt input from remote and flush buffers
 ***************************************************************/
static void
flush()
{
	(void)ioctl(TTYOUT, TCXONC, 0);	/* stop tty output */
	(void)ioctl(rlfd, TCFLSH, 0);		/* flush remote input */
	(void)ioctl(TTYOUT, TCFLSH, 1);	/* flush tty output */
	(void)ioctl(TTYOUT, TCXONC, 1);	/* restart tty output */
	if(takeflag == NO) {
		return;		/* didn't interupt file transmission */
	}
	say(msg[21]);
	(void)sleep(3);
	w_str("echo '\n~>\n';mesg y;stty echo\n");
	takeflag = NO;
}

/**************************************************************
 *	command interpreter for escape lines
 **************************************************************/
int
tilda(cmd)
char	*cmd;
{
	say("\r\n");
#ifdef	ddt
	if(_debug == YES) say("call tilda(%s)\r\n", cmd);
#endif
	switch(cmd[0]) {
		case '.':
			if(call.telno == NULL)
				if(cmd[1] != '.')
					w_str("\04\04\04\04\04");
			return(YES);
		case '!':
			shell(cmd);	/* local shell */
			say("\r%c\r\n", *cmd);
			break;
		case '$':
			if(cmd[1] == '\0')
				say(msg[26]);
			else {
				shell(cmd);	/* Local shell */
				say("\r%c\r\n", *cmd);
			}
			break;
		case '%':
			dopercen(++cmd);
			break;
#ifdef ddt
		case 't':
			tdmp(TTYIN);
			break;
		case 'l':
			tdmp(rlfd);
			break;
#endif
		default:
			say(msg[17]);
	}
	return(NO);
}

/***************************************************************
 *	The routine "shell" takes an argument starting with
 *	either "!" or "$", and terminated with '\0'.
 *	If $arg, arg is the name of a local shell file which
 *	is executed and its output is passed to the remote.
 *	If !arg, we escape to a local shell to execute arg
 *	with output to TTY, and if arg is null, escape to
 *	a local shell and blind the remote line.  In either
 *	case, RUBout or '^D' will kill the escape status.
 **************************************************************/

static void
shell(str)
char	*str;
{
	int	fk, (*xx)(), (*yy)();

#ifdef	ddt
	if(_debug == YES) say("call shell(%s)\r\n", str);
#endif
	fk = dofork();
	if(fk < 0)
		return;
	mode(0);	/* restore normal tty attributes */
	xx = signal(SIGINT, SIG_IGN);
	yy = signal(SIGQUIT, SIG_IGN);
	if(fk == 0) {
		/***********************************************
		 * Hook-up our "standard output"
		 * to either the tty or the line
		 * as appropriate for '!' or '$'
		 ***********************************************/
		(void)close(TTYOUT);
		(void)fcntl((*str == '$')? rlfd:TTYERR,F_DUPFD,TTYOUT);
		(void)close(rlfd);
		(void)signal(SIGINT, SIG_DFL);
		(void)signal(SIGHUP, SIG_DFL);
		(void)signal(SIGQUIT, SIG_DFL);
		(void)signal(SIGUSR1, SIG_DFL);
		if(*++str == '\0')
			(void)execl("/bin/sh","",(char*)0,(char*)0,0);
		else
			(void)execl("/bin/sh","sh","-c",str,0);
		say(msg[15]);
		exit(0);
	}
	while(wait((int*)0) != fk);
	(void)signal(SIGINT, xx);
	(void)signal(SIGQUIT, yy);
	mode(1);
}


/***************************************************************
 *	This function implements the 'put', 'take', 'break', and
 *	'nostop' commands which are internal to cu.
 ***************************************************************/

static void
dopercen(cmd)
register char *cmd;
{
	char	*arg[5];
	char	*getpath, *getenv();
	char	mypath[80];
	int	narg;

	blckcnt((long)(-1));

#ifdef	ddt
	if(_debug == YES) say("call dopercen(\"%s\")\r\n", cmd);
#endif
	arg[narg=0] = strtok(cmd, " \t\n");
	/* following loop breaks out the command and args */
	while((arg[++narg] = strtok((char*) NULL, " \t\n")) != NULL) {
		if(narg < 5)
			continue;
		else
			break;
	}

	if(EQUALS(arg[0], "take")) {
		if(narg < 2 || narg > 3) {
			say("usage: ~%%take from [to]\r\n");
			return;
		}
		if(narg == 2)
			arg[2] = arg[1];
		w_str("stty -echo;mesg n;echo '~>':");
		w_str(arg[2]);
		w_str(";cat ");
		w_str(arg[1]);
		w_str(";echo '~>';mesg y;stty echo\n");
		takeflag = YES;
		return;
	}
	else if(EQUALS(arg[0], "put")) {
		FILE	*file;
		char	ch, buf[BUFSIZ], spec[NCC+1], *b, *p, *q;
		int	i, j, len, tc=0, lines=0;
		long	chars=0L;

		if(narg < 2 || narg > 3) {
			say("usage: ~%%put from [to]\r\n");
			goto R;
		}
		if(narg == 2)
			arg[2] = arg[1];

		if((file = fopen(arg[1], "r")) == NULL) {
			say(msg[13], arg[1]);
R:
			w_str("\n");
			return;
		}
		w_str("stty -echo; cat - > ");
		w_str(arg[2]);
		w_str("; stty echo\n");
		intrupt = NO;
		for(i=0,j=0; i < NCC; ++i)
			if((ch=tv0.c_cc[i]) != '\0')
				spec[j++] = ch;
		spec[j] = '\0';
		mode(2);
		(void)sleep(5);
		while(intrupt == NO &&
				fgets(b= &buf[MID],MID,file) != NULL) {
			len = strlen(b);
			chars += len;		/* character count */
			p = b;
			while(q = strpbrk(p, spec)) {
				if(*q == tintr || *q == tquit ||
							*q == teol) {
					say(msg[23], *q);
					(void)strcpy(q, q+1);
					intrupt = YES;
				}
				b = strncpy(b-1, b, q-b);
				*(q-1) = '\\';
				p = q+1;
			}
			if((tc += len) >= MID) {
				(void)sleep(1);
				tc = len;
			}
			if(write(rlfd, b, (unsigned)strlen(b)) < 0) {
				say(msg[25]);
				intrupt = YES;
				break;
			}
			++lines;		/* line count */
			blckcnt((long)chars);
		}
		mode(1);
		blckcnt((long)(-2));		/* close */
		(void)fclose(file);
		if(intrupt == YES) {
			intrupt = NO;
			say(msg[21]);
			w_str("\n");
			say(msg[19], ++chars);
		} else
			say(msg[20], lines, chars);
		w_str("\04");
		(void)sleep(3);
		return;
	}
	else if(EQUALS(arg[0], "b") || EQUALS(arg[0], "break")) {
		(void)ioctl(rlfd, TCSBRK, 0);
		return;
	}
	else if(EQUALS(arg[0], "nostop")) {
		(void)ioctl(rlfd, TCGETA, &tv);
		if(sstop == NO)
			tv.c_iflag |= IXOFF;
		else
			tv.c_iflag &= ~IXOFF;
		(void)ioctl(rlfd, TCSETAW, &tv);
		sstop = !sstop;
		mode(1);
		return;
	}else if(EQUALS(arg[0], "cd")) {
		/* Change local current directory */
		if (narg < 2) {
			getpath = getenv("HOME");
			strcpy(mypath, getpath);
			if(chdir(mypath) < 0)
				say("Cannot chang to %s\r\n", mypath);
			w_str("\r");
			return;
		}
		if (chdir(arg[1]) < 0) {
			say("Cannot change to %s\r\n", arg[1]);
			return;
		}
		recfork();	/* fork a new child so it know about change */
		w_str("\r");
		return;
	}
	say("~%%%s unknown to cu\r\n", arg[0]);
}

/***************************************************************
 *	receive: read from remote line, write to fd=1 (TTYOUT)
 *	catch:
 *	~>[>]:file
 *	.
 *	. stuff for file
 *	.
 *	~>	(ends diversion)
 ***************************************************************/

static void
receive()
{
	register silent=NO, file;
	register char *p;
	int	tic;
	char	b[BUFSIZ];
	long	lseek(), count;

#ifdef	ddt
	if(_debug == YES) say("receive started\r\n");
#endif
	file = -1;
	p = b;
	while(r_char(rlfd) == YES) {
		if(silent == NO)
			if(w_char(TTYOUT) == NO) 
				rcvdead(IOERR);	/* this will exit */
		/* remove CR's and fill inserted by remote */
		if(cxc == '\0' || cxc == '\177' || cxc == '\r')
			continue;
		*p++ = cxc;
		if(cxc != '\n' && (p-b) < BUFSIZ)
			continue;
		/***********************************************
		 * The rest of this code is to deal with what
		 * happens at the beginning, middle or end of
		 * a diversion to a file.
		 ************************************************/
		if(b[0] == '~' && b[1] == '>') {
			/****************************************
			 * The line is the beginning or
			 * end of a diversion to a file.
			 ****************************************/
			if((file < 0) && (b[2] == ':' || b[2] == '>')) {
				/**********************************
				 * Beginning of a diversion
				 *********************************/
				int	append;

				*(p-1) = NULL; /* terminate file name */
				append = (b[2] == '>')? 1:0;
				p = b + 3 + append;
				if(append && (file=open(p,O_WRONLY))>0)
					(void)lseek(file, 0L, 2);
				else
					file = creat(p, 0666);
				if(file < 0) {
					say(msg[16], p);
					perror("");
					(void)sleep(10);
				} else {
					silent = YES; 
					count = tic = 0;
				}
			} else {
				/*******************************
				 * End of a diversion (or queer data)
				 *******************************/
				if(b[2] != '\n')
					goto D;		/* queer data */
				if(silent = close(file)) {
					say(msg[16], b);
					silent = NO;
				}
				blckcnt((long)(-2));
				say("~>\r\n");
				say(msg[20], tic, count);
				file = -1;
			}
		} else {
			/***************************************
			 * This line is not an escape line.
			 * Either no diversion; or else yes, and
			 * we've got to divert the line to the file.
			 ***************************************/
D:
			if(file > 0) {
				(void)write(file, b, (unsigned)(p-b));
				count += p-b;	/* tally char count */
				++tic;		/* tally lines */
				blckcnt((long)count);
			}
		}
		p = b;
	}
	say("\r\nLost Carrier\r\n");
	rcvdead(IOERR);
}

/***************************************************************
 *	change the TTY attributes of the users terminal:
 *	0 means restore attributes to pre-cu status.
 *	1 means set `raw' mode for use during cu session.
 *	2 means like 1 but accept interrupts from the keyboard.
 ***************************************************************/
static void
mode(arg)
{
#ifdef	ddt
	if(_debug == YES) say("call mode(%d)\r\n", arg);
#endif
	if(arg == 0) {
		(void)ioctl(TTYIN, TCSETAW, &tv0);
	} else {
		(void)ioctl(TTYIN, TCGETA, &tv);
		if(arg == 1) {
			tv.c_iflag &= ~(INLCR | ICRNL | IGNCR |
						IXOFF | IUCLC);
			tv.c_iflag |= ISTRIP;
			tv.c_oflag |= OPOST;
			tv.c_oflag &= ~(OLCUC | ONLCR | OCRNL | ONOCR
						| ONLRET);
			tv.c_lflag &= ~(ICANON | ISIG | ECHO);
			if(sstop == NO)
				tv.c_iflag &= ~IXON;
			else
				tv.c_iflag |= IXON;
			if(terminal) {
				tv.c_oflag |= ONLCR;
				tv.c_iflag |= ICRNL;
			}
			tv.c_cc[VEOF] = '\01';
			tv.c_cc[VEOL] = '\0';
		}
		if(arg == 2) {
			tv.c_iflag |= IXON;
			tv.c_lflag |= ISIG;
		}
		(void)ioctl(TTYIN, TCSETAW, &tv);
	}
}

static int
dofork()
{
	register int x, i=0;

	while(++i < 6)
		if((x = fork()) >= 0) {
			/* Do not give setuid shells */
			if (x == 0) {
				if (setuid(getuid())<0 && getuid()!=0) {
					say("Cannot set uid\r\n");
					hangup(GETID);
				}
			if (getgid() != getegid())
				if (setgid(getgid())<0 && getgid()!=0) {
					say("Cannot set gid\r\n");
					hangup(GETID);
				}
			}
			return(x);
		}
#ifdef	ddt
	if(_debug == YES) perror("dofork");
#endif
	say(msg[22]);
	return(x);
}

static int
r_char(fd)
{
	int rtn;

	while((rtn = read(fd, &cxc, 1)) < 0)
		if(errno == EINTR)
			if(intrupt == YES) {
				cxc = '\0';	/* got a BREAK */
				return(YES);
			} else
				continue;	/* alarm went off */
		else {
#ifdef	ddt
			if(_debug == YES)
				say("got read error, not EINTR\n\r");
#endif
			break;			/* something wrong */
		}
	return(rtn == 1? YES: NO);
}

static int
w_char(fd)
{
	int rtn;

	while((rtn = write(fd, &cxc, 1)) < 0)
		if(errno == EINTR)
			if(intrupt == YES) {
				say("\ncu: Output blocked\r\n");
				quit(IOERR);
			} else
				continue;	/* alarm went off */
		else
			break;			/* bad news */
	return(rtn == 1? YES: NO);
}

/*VARARGS1*/
static void
say(fmt, arg1, arg2, arg3, arg4, arg5)
char	*fmt;
{
	(void)fprintf(stderr, fmt, arg1, arg2, arg3, arg4, arg5);
}

static void
w_str(string)
register char *string;
{
	int len;

	len = strlen(string);
	if(write(rlfd, string, (unsigned)len) != len)
		say(msg[14]);
}

static
onintrpt()
{
	(void)signal(SIGINT, onintrpt);
	(void)signal(SIGQUIT, onintrpt);
	intrupt = YES;
}

static
rcvdead(arg)	/* this is executed only in the receive proccess */
int arg;
{
#ifdef ddt
	if(_debug == YES) say("call rcvdead(%d)\r\n", arg);
#endif
	(void)kill(getppid(), SIGUSR1);
	exit((arg == SIGHUP)? SIGHUP: arg);
	/*NOTREACHED*/
}

static
quit(arg)	/* this is executed only in the parent proccess */
int arg;
{
#ifdef ddt
	if(_debug == YES) say("call quit(%d)\r\n", arg);
#endif
	(void)kill(child, SIGKILL);
	bye(arg);
	/*NOTREACHED*/
}

static
bye(arg)	/* this is executed only in the parent proccess */
int arg;
{
	int status;
#ifdef ddt
	if(_debug == YES) say("call bye(%d)\r\n", arg);
#endif
	turnoff();
	(void)wait(&status);
	say("\r\nDisconnected\007\r\n");
	hangup((arg == SIGUSR1)? (status >>= 8): arg);
	/*NOTREACHED*/
}

turnoff()
{
	(void)signal(SIGINT, SIG_IGN);
	(void)signal(SIGQUIT, SIG_IGN);
}


static
hangup(arg)	/* this is executed only in the parent process */
int arg;
{
#ifdef ddt
	if(_debug == YES) say("call hangup(%d)\r\n", arg);
#endif
	if(chmod(call.device, savmode) || chown(call.device, savuid, savgid) <0)
		say(stderr, "Cannot chown/chmod on %s\n", call.device);
	undial(rlfd);
	mode(0);
	exit(arg);	/* restore users prior tty status */
	/*NOTREACHED*/
}


#ifdef ddt
static void
tdmp(arg)
{

	struct termio xv;
	int i;

	say("\rdevice status for fd=%d\n", arg);
	say("\F_GETFL=%o,", fcntl(arg, F_GETFL,1));
	if(ioctl(arg, TCGETA, &xv) < 0) {
		char	buf[100];
		i = errno;
		(void)sprintf(buf, "\rtdmp for fd=%d", arg);
		errno = i;
		perror(buf);
		return;
	}
	say("iflag=`%o',", xv.c_iflag);
	say("oflag=`%o',", xv.c_oflag);
	say("cflag=`%o',", xv.c_cflag);
	say("lflag=`%o',", xv.c_lflag);
	say("line=`%o'\r\n", xv.c_line);
	say("cc[0]=`%o',",  xv.c_cc[0]);
	for(i=1; i<8; ++i)
		say("[%d]=`%o', ", i, xv.c_cc[i]);
	say("\r\n");
}
#endif


#include	<sys/utsname.h>

sysname(name)
char * name;
{

	register char *s, *t;
	struct utsname utsn;

	if(uname(&utsn) < 0)
		s = "Local";
	else
		s = utsn.nodename;

	t = name;
	while((*t = *s++ ) &&  (t < name+7))
		*t++;

	*(name+7) = '\0';
	return;
}


#define	NPL	50
blckcnt(count)
long count;
{
	static long lcnt = 0;
	register long c1, c2;
	register int i;
	char c;

	if(count == (long) (-1)) {       /* initialization call */
		lcnt = 0;
		return;
	}
	c1 = lcnt/BUFSIZ;
	if(count != (long)(-2)) {	/* regular call */
		c2 = count/BUFSIZ;
		for(i = c1; i++ < c2;) {
			c = '0' + i%10;
			write(2, &c, 1);
			if(i%NPL == 0)
				write(2, "\n\r", 2);
		}
		lcnt = count;
	}
	else {
		c2 = (lcnt + BUFSIZ -1)/BUFSIZ;
		if(c1 != c2)
			write(2, "+\n\r", 3);
		else if(c2%NPL != 0)
			write(2, "\n\r", 2);
		lcnt = 0;
	}
}


char *
sysphone(sysnam)
char *sysnam;
{

	FILE *fsys;
	char sys[100];
	int nf;
	char *flds[50];
	static char phone[MAXN];

	if (Lsysfp == NULL) {	/* only open once */
		if((Lsysfp=fopen(SYSFILE, "r")) == NULL) {
			printf("cannot open L.sys file\n");
			exit(1);
		}
	}
	nf=finds(Lsysfp, sysnam, flds, phone);
	return((char *) (nf !=0?(int)phone:0));
}

/*
 * set system attribute vector
 * return:
 *	0	-> number of arguments in vector succeeded
 *	CF_PHONE	-> phone number not found
 */
finds(fsys, sysnam, flds, phone)
 FILE *fsys;
char *sysnam;
char *phone;
char *flds[];
{

	register int na;
	register int flg;
	register int s;
	static char info[MAXC];
	char sysn[32];

	/*
	 * format of fields
	 *	0	-> name;
	 *	1	-> time
	 *	2	-> acu/hardwired
	 *	3	-> speed
	 *	4	-> phone number
	 */
	flg = 0;
	if(strlen(sysnam) > 4)
		flg++;
	while (fgets(info, MAXC, fsys) != NULL) {
		if((info[0] == '#') || (info[0] == ' ') || (info[0] == '\t') || 
			(info[0] == '\n'))
			continue;
		if(flg)
			if(sysnam[4] != info[4])
				continue;
		if(info[0] != sysnam[0])
			continue;
		na=getargs(info, flds);
		sprintf(sysn, "%.6s", flds[F_NAME]);
		if(strncmp(sysnam, sysn, SYSNSIZE) != SAME)
			continue;

		/* find the phone number or the direct line 
		 ** If a baud was specified, lookfor that baud ONLY!
		*/

		s = atoi(flds[F_SPEED]);
		if(sflag) {
			if(call.baud != -1 && s != call.baud) 
				continue;
		}

		call.baud = s;
		call.speed = s;
		if(strcmp(flds[F_LINE], flds[F_PHONE]) == SAME) {
			strcpy(phone, flds[F_LINE]);
			return(na);
		} else {
			strcpy(phone, flds[F_PHONE]);
			exphone(flds[F_PHONE], phone);

			/* check for "VOID"ed phone numbers */
			if (isupper(phone[0]))
				continue;
			return(na);
			}
	}
	return(CF_PHONE);
}



/*
 * generate a vector of pointers (arps) to the
 * substrings in string "s".
 * Each substring is separated by blanks and/or tabs.
 *	s	-> string to analyze
 *	arps	-> array of pointers
 * returns:
 *	i	-> # of subfields
 * Bug:
 * Should pass # of elements in arps in case s
 * is garbled from file.
 */
getargs(s, arps)
register char *s, *arps[];
{
	register int i;

	i = 0;
	while (1) {
		arps[i] = NULL;
		while (*s == ' ' || *s == '\t')
			*s++ = '\0';
		if (*s == '\n')
			*s = '\0';
		if (*s == '\0')
			break;
		arps[i++] = s++;
		while (*s != '\0' && *s != ' '
			&& *s != '\t' && *s != '\n')
				s++;
	}
	return(i);
}
/*
 * expand phone number for given prefix and number
 * return:
 *	none
 */
exphone(in, out)
register char *in;
char *out;
{
	register FILE *fn;
	register char *s1;
	char pre[MAXPH], npart[MAXPH]; 
	char buf[BUFSIZ], tpre[MAXPH], p[MAXPH];
	char *strcpy(), *strcat();

	if (!isalpha(*in)) {
		strcpy(out, in);
		return;
	}

	s1=pre;
	while (isalpha(*in))
		*s1++ = *in++;
	*s1 = '\0';
	s1 = npart;
	while (*in != '\0')
		*s1++ = *in++;
	*s1 = '\0';

	tpre[0] = '\0';
	fn = fopen(DIALFILE, "r");
	if (fn != NULL) {
		while (fgets(buf, BUFSIZ, fn)) {
		if((buf[0] == '#') || (buf[0] == ' ') || (buf[0] == '\t') || 
			(buf[0] == '\n'))
				continue;
			sscanf(buf, "%s%s", p, tpre);
			if (strcmp(p, pre) == SAME)
				break;
			tpre[0] = '\0';
		}
		fclose(fn);
	}

	strcpy(out, tpre);
	strcat(out, npart);
	return;
}

