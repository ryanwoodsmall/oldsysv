/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)login:login.c	1.23"
/*
 * login [ name ] [ environment args ]
 *
 *	Conditional assemblies:
 *
 *	PASSREQ	causes password to be required
 *	NO_MAIL	causes the MAIL environment variable not to be set
 *	NOSHELL does not put non-standard shell names into environment
 *	CONSOLE, if defined, only allows root logins on the device
 *		specified by CONSOLE.  CONSOLE MUST NOT be defined as
 *		either "/dev/syscon" or "/dev/systty"!!
 *	MAXTRYS is the number of attempts permitted.  0 is "no limit".
 *	MAXTIME is the number of seconds before giving up.  0 is "no limit".
 */

#include <sys/types.h>
#include <utmp.h>
#include <signal.h>
#include <pwd.h>
#include <stdio.h>
#ifdef	SECURITY
#include <fcntl.h>
#endif	/* SECURITY */
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/utsname.h>

#ifndef	MAXTRYS
#	define	MAXTRYS	10	/* default */
#endif
#ifndef	MAXTIME
#	define	MAXTIME	60	/* default */
#endif
#ifdef	SECURITY
#ifndef	NFAILURES
#	define	NFAILURES 2	/* default */
#endif
#endif	/* SECURITY */

#define SCPYN(a, b)	strncpy(a, b, sizeof(a))
#define EQN(a, b)	(!strncmp(a, b, sizeof(a)-1))
#define DIAL_FILE	"/etc/dialups"
#define DPASS_FILE	"/etc/d_passwd"
#define SHELL		"/bin/sh"
#define	PATH		"PATH=:/bin:/usr/bin"
#define	ROOTPATH	"PATH=:/bin:/usr/bin:/etc"
#define SUBLOGIN	"<!sublogin>"

#define	ROOTUID		0

#define	MAXARGS		63
#define	MAXENV		1024
#define MAXLINE		256

/*	Illegal passwd entry.
*/
static struct	passwd nouser = { "", "no:password", ~ROOTUID };

static struct utsname	un;
static struct	utmp utmp;
static char	u_name[64];
static char	minusnam[16] = "-";
static char	shell[64] = { "SHELL=" };
static char	home[64] = { "HOME=" };
static char	logname[30] = { "LOGNAME=" };

static char	loginmsg[] = "login: ";
static char	passwdmsg[] = "Password:";
static char	incorrectmsg[] = "Login incorrect\n";

#ifndef	NO_MAIL
static char	mail[30] = { "MAIL=/usr/mail/" };
#endif

static char	*envinit[5+MAXARGS] = {
		home,PATH,logname,0,0
	};
static int	basicenv;
static char	envblk[MAXENV];
static struct	passwd *pwd;

struct	passwd *getpwnam();
int	setpwent();
char	*crypt();
char	*getpass();
char	*strrchr(),*strchr(),*strcat();
extern	char **environ;

#define	WEEK	(24L * 7 * 60 * 60) /* 1 week in seconds */
static time_t	when;
static time_t	maxweeks;
static time_t	minweeks;
static time_t	now;
extern long 	a64l(), time();

main(argc, argv ,renvp)
char **argv,**renvp;
{
	register char	*namep;
	int	j,k,l_index,length;
	char	*ttyn,*ttyntail;
#ifdef PASSREQ
	int 	nopassword = 1;
#endif
	register int	i;
	register struct utmp	*u;
	struct utmp	*getutent(), *pututline();
	FILE	*fp;
	char	**envp,*ptr,*endptr;
	int	sublogin;
	extern char	**getargs();
	extern char	*terminal();
	char	inputline[ MAXLINE ];
	int	n;
#if	MAXTRYS > 0
	int	trys = 0;
#endif

/*	Set flag to disable the pid check if you find that you are	*/
/*	a subsystem login.						*/

	sublogin = 0;
	if (*renvp && strcmp(*renvp,SUBLOGIN) == 0)
		sublogin = 1;

	umask(0);
	alarm( MAXTIME );
	signal(SIGQUIT, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	nice(0);
	ttyn = terminal(0);
	if (ttyn==0)
		ttyn = "/dev/tty??";
	if (ttyntail = strrchr(ttyn,'/'))
		ttyntail++;
	else
		ttyntail = ttyn;

	if (argc > 1) {
		SCPYN(utmp.ut_user, argv[1]);
		SCPYN(u_name, argv[1]);
		strcpy( inputline, u_name );
		strcat( inputline, "   \n" );
		envp = &argv[2];
		goto first;
	}

loop:
#ifdef	SECURITY
	if (trys >= NFAILURES)
		badlogin(u_name, ttyn);
#endif	/* SECURITY */
	u_name[0] = utmp.ut_user[0] = '\0';
first:
#if	MAXTRYS > 0
	if( ++trys > MAXTRYS ) {
		sleep( MAXTIME );
		exit( 1 );
	}
#endif
	while (utmp.ut_user[0] == '\0') {
		printf( loginmsg );
		if ((envp = getargs( inputline )) != (char**)NULL) {
			SCPYN(utmp.ut_user,*envp);
			SCPYN(u_name, *envp++);
		}
	}

	/*	If any of the common login messages was the input, we must be
		looking at a tty that is running login.  We exit because
		they will chat at each other until one times out otherwise.
		In time, init(1M) sees this and decides something is amiss.
	*/
	if( EQN( loginmsg, inputline )  ||  EQN( passwdmsg, inputline )  ||
	    EQN( incorrectmsg, inputline ) ) {
		printf( "Looking at a login line.\n" );
		exit( 8 );
	}
	  
	setpwent();
	if ((pwd = getpwnam(u_name)) == NULL)
		pwd = &nouser;
	endpwent();
#ifdef CONSOLE
	if( pwd->pw_uid == ROOTUID  &&  strcmp(ttyn, CONSOLE) ) {
		printf("Not on system console\n");
		exit(10);
	}
#endif
	if (*pwd->pw_passwd != '\0') {
#ifdef PASSREQ
		nopassword = 0;
#endif
		if(gpass(passwdmsg, pwd->pw_passwd))
			goto loop;
	}

	/*
	 * get dialup password, if necessary
	 */
	if(dialpass(ttyn))
		goto loop;

	/*
	 * optionally adjust nice(2)
	 */
	if (strncmp("pri=", pwd->pw_gecos, 4) == 0) {
		int	mflg, pri;

		pri = 0;
		mflg = 0;
		i = 4;
		if (pwd->pw_gecos[i] == '-') {
			mflg++;
			i++;
		}
		while(pwd->pw_gecos[i] >= '0' && pwd->pw_gecos[i] <= '9')
			pri = (pri * 10) + pwd->pw_gecos[i++] - '0';
		if (mflg)
			pri = -pri;
		nice(pri);
	}

	if(chdir(pwd->pw_dir) < 0) {
		printf("Unable to change directory to \"%s\"\n",pwd->pw_dir);
		goto loop;
	}
	time(&utmp.ut_time);
	utmp.ut_pid = getpid();

/*	Find the entry for this pid (or line if we are a sublogin) in	*/
/*	the utmp file.							*/

	while ((u = getutent()) != NULL) {
		if ((u->ut_type == INIT_PROCESS || u->ut_type ==
		    LOGIN_PROCESS || u->ut_type == USER_PROCESS)
		    && ( (sublogin && strncmp(u->ut_line,ttyntail,
		    sizeof(u->ut_line)) == 0) || u->ut_pid == utmp.ut_pid) ) {

/*	Copy in the name of the tty minus the "/dev/", the id, and set	*/
/*	the type of entry to USER_PROCESS.				*/

			SCPYN(utmp.ut_line,(ttyn+sizeof("/dev/")-1));
			utmp.ut_id[0] = u->ut_id[0];
			utmp.ut_id[1] = u->ut_id[1];
			utmp.ut_id[2] = u->ut_id[2];
			utmp.ut_id[3] = u->ut_id[3];
			utmp.ut_type = USER_PROCESS;

/*	Return the new updated utmp file entry.				*/

			pututline(&utmp);
			break;
		}
	}
	endutent();		/* Close utmp file */

	if (u == (struct utmp *)NULL){
		printf("No utmp entry.  You must exec \"login\" from\
 the lowest level \"sh\".\n");
		exit(1);
	}

/*	Now attempt to write out this entry to the wtmp file if we	*/
/*	were successful in getting it from the utmp file and the	*/
/*	wtmp file exists.						*/

	if (u != NULL && (fp = fopen(WTMP_FILE,"r+")) != NULL) {
		fseek(fp,0L,2);		/* Seek to end of file. */
		fwrite(&utmp,sizeof(utmp),1,fp);
		fclose(fp);
	}
	chown(ttyn, pwd->pw_uid, pwd->pw_gid);

/*	If the shell field starts with a '*', do a chroot to the home	*/
/*	directory and perform a new login.				*/

	if(*pwd->pw_shell == '*') {
		if(chroot(pwd->pw_dir) < 0) {
			printf("No Root Directory\n");
			goto loop;
		}

/*	Set the environment flag <!sublogin> so that the next login	*/
/*	knows that it is a sublogin.					*/

		envinit[0] = SUBLOGIN;
		envinit[1] = (char*)NULL;
		printf("Subsystem root: %s\n",pwd->pw_dir);
		execle("/bin/login", "login", (char*)0, &envinit[0]);
		execle("/etc/login", "login", (char*)0, &envinit[0]);
		printf("No /bin/login or /etc/login on root\n");
		goto loop;
	}

	if (setgid(pwd->pw_gid) == -1) {
		printf("Bad group id.\n");
		exit(1);
	}
	if (setuid(pwd->pw_uid) == -1) {
		printf("Bad user id.\n");
		exit(1);
	}


	alarm(0);	/*give user time to come up with new password if needed*/

#ifdef PASSREQ
	if (nopassword) {
		printf("You don't have a password.  Choose one.\n");
		printf("passwd %s\n", u_name);
		n = system("/bin/passwd");
		if (n > 0)
			goto loop;
		if (n < 0) {
			printf("Cannot execute /bin/passwd\n");
			exit(1);
		}
	}
#endif

/* is the age of the password to be checked? */

	if (*pwd->pw_age != NULL) {
		/* retrieve (a) week of previous change; 
			(b) maximum number of valid weeks	*/
		when = a64l (pwd->pw_age);
		/* max, min and weeks since last change are packed radix 64 */
		maxweeks = when & 077;
		minweeks = (when >> 6) & 077;
		when >>= 12;
		now  = time(0)/WEEK;
		if (when > now || (now > when + maxweeks) && (maxweeks >= minweeks)) {
			printf ("Your password has expired. Choose\
 a new one\n");
			n = system("/bin/passwd");
			if (n > 0)
				goto loop;
			if (n < 0) {
				printf("Cannot execute /bin/passwd\n");
				exit(9);
			}
		}
	}

/*	Set up the basic environment for the exec.  This includes	*/
/*	HOME, PATH, LOGNAME, SHELL, and MAIL.				*/

	strcat(home, pwd->pw_dir);
	strcat(logname, pwd->pw_name);
	if(pwd->pw_uid == ROOTUID) {
		envinit[1] = ROOTPATH;
	}
	if (*pwd->pw_shell == '\0') {
		pwd->pw_shell = SHELL;
	}
#ifndef NOSHELL
	else {
		envinit[3] = shell;
	}
#endif
	strcat(shell, pwd->pw_shell);

/*	Find the name of the shell.					*/

	if ((namep = strrchr(pwd->pw_shell, '/')) == NULL)
		namep = pwd->pw_shell;
	else
		namep++;

/*	Generate the name of the shell with a '-' sign in front of it.	*/
/*	This causes .profile processing when a shell is exec'ed.	*/

	strcat(minusnam, namep);

#ifndef	NO_MAIL
	if (envinit[3] == (char*)NULL)
		envinit[3] = mail;
	else
		envinit[4] = mail;
	strcat(mail,pwd->pw_name);
#endif

/*	Find the end of the basic environment.				*/

	for (basicenv=3; envinit[basicenv];basicenv++);

/*	Add in all the environment variables picked up from the		*/
/*	argument list to "login" or from the user response to the	*/
/*	"login" request.						*/

	for (j=0,k=0,l_index=0,ptr= &envblk[0]; *envp && j < MAXARGS-1
		;j++,envp++) {

/*	Scan each string provided.  If it doesn't have the format	*/
/*	xxx=yyy, then add the string "Ln=" to the beginning.		*/

		if ((endptr = strchr(*envp,'=')) == (char*)NULL) {
			envinit[basicenv+k] = ptr;
			sprintf(ptr,"L%d=%s",l_index,*envp);

/*	Advance "ptr" to the beginning of the next argument.		*/

			while (*ptr++);
			k++;
			l_index++;
		}

/*	Is this an environmental variable we permit?			*/

		else if ( !legalenvvar( *envp ) ) {
			continue;
		}


/*	Check to see whether this string replaces any previously	*/
/*	defined string.							*/

		else {
			for (i=0,length= endptr+1-*envp; i < basicenv+k;i++) {
				if (strncmp(*envp,envinit[i],length) == 0) {
					envinit[i] = *envp;
					break;
				}
			}

/*	If it doesn't, place it at the end of environment array.	*/

			if (i == basicenv+k) {
				envinit[basicenv+k] = *envp;
				k++;
			}
		}
	}

/*	Switch to the new environment.					*/

	environ = envinit;
	alarm(0);

	signal(SIGQUIT, SIG_DFL);
	signal(SIGINT, SIG_DFL);
	uname( &un );
#if u3b || u3b2 || u3b5
	printf("UNIX System V Release %s AT&T %s\n%s\n\
Copyright (c) 1984 AT&T\nAll Rights Reserved\n",
		un.release, un.machine, un.nodename);
#else
	printf("UNIX System V Release %s %s\n%s\n\
Copyright (c) 1984 AT&T\nAll Rights Reserved\n",
		un.release, un.machine, un.nodename);
#endif /*u3b,u3b2,u3b5*/

#ifdef	SECURITY
/*	
 *	Advise the user the time and date that this login-id
 *	was last used
 */
	lastlogin();
#endif	/* SECURITY */

	execl(pwd->pw_shell, minusnam, (char*)0);
	
	/*	pwd->pw_shell was not an executable object file, maybe it
		is a shell proceedure or a command line with arguments.
		If so, turn off the SHELL= environment variable.
	*/
	if( !strncmp( envinit[3], shell, 6 ) )
		envinit[3][6] = '\0';
	if( access( pwd->pw_shell, 05 ) == 0 )
		execl(SHELL, "sh", pwd->pw_shell, minusnam, (char*)0);

	printf("No shell\n");
	exit(1);
	/* NOTREACHED */
}

static dialpass(ttyn)
char *ttyn;
{
	register FILE	*fp;
	char		defpass[30];
	char		line[80];
	register char	*p1, *p2;

	if((fp=fopen(DIAL_FILE, "r")) == NULL)
		return(0);
	while((p1 = fgets(line, sizeof(line), fp)) != NULL) {
		while(*p1 != '\n' && *p1 != ' ' && *p1 != '\t')
			p1++;
		*p1 = '\0';
		if(strcmp(line, ttyn) == 0)
			break;
	}
	fclose(fp);
	if(p1 == NULL || (fp = fopen(DPASS_FILE, "r")) == NULL)
		return(0);
	defpass[0] = '\0';
	p2 = 0;
	while((p1 = fgets(line, sizeof(line)-1, fp)) != NULL) {
		while(*p1 && *p1 != ':')
			p1++;
		*p1++ = '\0';
		p2 = p1;
		while(*p1 && *p1 != ':')
			p1++;
		*p1 = '\0';
		if(strcmp(pwd->pw_shell, line) == 0) {
			break;
		}
		if(strcmp(SHELL, line) == 0)
			SCPYN(defpass, p2);
		p2 = 0;
	}
	fclose(fp);
	if( !p2 )
		p2 = defpass;
	if( *p2 != '\0' )
		return(gpass("Dialup Password:", p2));
	return(0);
}

static gpass(prmt, pswd)
char *prmt, *pswd;
{
	register char *p1;

	p1 = crypt(getpass(prmt), pswd);
	if(strcmp(p1, pswd)) {
#if  MAXTRYS > 0  &&  MAXTIME > 0  &&  (MAXTIME - 2*MAXTRYS) > 0
		sleep( (MAXTIME - 2*MAXTRYS)/MAXTRYS );
#endif
		printf( incorrectmsg );
		return(1);
	}
	return(0);
}

#define	WHITESPACE	0
#define	ARGUMENT	1

static
char **getargs( inline )
char	*inline;
{
	static char envbuf[MAXLINE];
	static char *args[MAXARGS];
	register char *ptr,**answer;
	register int c;
	int state;
	extern int quotec();

	for (ptr= &envbuf[0]; ptr < &envbuf[sizeof(envbuf)];) *ptr++ = '\0';
	for (answer= &args[0]; answer < &args[MAXARGS];)
		*answer++ = (char *)NULL;
	for (ptr= &envbuf[0],answer= &args[0],state = WHITESPACE;
	     (c = getc(stdin)) != EOF;) {

		*(inline++) = c;
		switch (c) {
		case '\n' :
			if (ptr == &envbuf[0]) return((char **)NULL);
			else return(&args[0]);
		case ' ' :
		case '\t' :
			if (state == ARGUMENT) {
				*ptr++ = '\0';
				state = WHITESPACE;
			}
			break;
		case '\\' :
			c = quotec();
		default :
			if (state == WHITESPACE) {
				*answer++ = ptr;
				state = ARGUMENT;
			}
			*ptr++ = c;
		}

/*	If the buffer is full, force the next character to be read to	*/
/*	be a <newline>.							*/

		if (ptr == &envbuf[sizeof(envbuf)-1]) {
			ungetc('\n',stdin);
			putc('\n',stdout);
		}
	}

/*	If we left loop because an EOF was received, exit immediately.	*/

	exit(0);
	/* NOTREACHED */
}

static
int quotec()
{
	register int c,i,num;

	switch(c = getc(stdin)) {
	case 'n' :
		c = '\n';
		break;
	case 'r' :
		c = '\r';
		break;
	case 'v' :
		c = 013;
		break;
	case 'b' :
		c = '\b';
		break;
	case 't' :
		c = '\t';
		break;
	case 'f' :
		c = '\f';
		break;
	case '0' :
	case '1' :
	case '2' :
	case '3' :
	case '4' :
	case '5' :
	case '6' :
	case '7' :
		for (num=0,i=0; i < 3;i++) {
			num = num * 8 + (c - '0');
			if ((c = getc(stdin)) < '0' || c > '7')
				break;
		}
		ungetc(c,stdin);
		c = num & 0377;
		break;
	default :
		break;
	}
	return (c);
}
/*
 * terminal(f): return "/dev/ttyXX" which the the name of the
 * tty belonging to file f.  This routine is the same as ttyname()
 * except that it rejects /dev/syscon and /dev/systty, which are
 * links to other device names.
 *
 * This program works in two passes: the first pass tries to
 * find the device by matching device and inode numbers; if
 * that doesn't work, it tries a second time, this time doing a
 * stat on every file in /dev and trying to match device numbers
 * only. If that fails too, NULL is returned.
 */

static
char *
terminal(f)
int	f;
{
	struct stat	fsb, tsb;
	struct dirent	*dp;
	register DIR	*df;
	register int	pass1;
	static char	rbuf[64];
	static char	dev[]="/dev/";

	if(isatty(f) == 0)
		return(NULL);
	if(fstat( f, &fsb ) < 0)
		return(NULL);
	if((fsb.st_mode & S_IFMT) != S_IFCHR)
		return(NULL);
	if( (df = opendir(dev)) == NULL )
		return(NULL);
	pass1 = 1;
	do {
		while( (dp = readdir(df)) != NULL ) {
			if(pass1  &&  dp->d_ino != fsb.st_ino)
				continue;
			if (strncmp(dp->d_name, "syscon", 6) == 0 ||
				strncmp(dp->d_name, "systty", 6) == 0)
				continue;
			(void) strcpy(rbuf, dev);
			(void) strncat(rbuf, dp->d_name,
				sizeof rbuf - sizeof dev);
			if(stat(rbuf, &tsb) < 0)
				continue;
			if(tsb.st_rdev == fsb.st_rdev &&
				(tsb.st_mode&S_IFMT) == S_IFCHR &&
				(!pass1 || tsb.st_ino == fsb.st_ino)) {
				closedir(df);
				return(rbuf);
			}
		}
		rewinddir(df);
	} while(pass1--);
	closedir(df);
	return(NULL);
}


static
char	*illegal[] = {
		"SHELL=",
		"HOME=",
		"LOGNAME=",
#ifndef	NO_MAIL
		"MAIL=",
#endif
		"CDPATH=",
		"IFS=",
		"PATH=",
		0
	};

/*	Is it legal to insert this environmental variable.
*/
static
int
legalenvvar( s )
char	*s;
{
	register char	**p;

	for( p = illegal;  *p;  p++ )
		if( !strncmp( s, *p, strlen( *p ) ) )
			return  0;
	return  1;
}

#ifdef	SECURITY
/*
 *  lastlogin() -- used to inform the user of the time and
 *        date that this login was last used.  Uses a
 *        file in the login directory called .lastlogin to
 *        keep track of the date/time.  Inode change time is
 *        used to prevent unauthorized changing of file times.
 */
lastlogin()
{
	register int		fd;
	static char		fname[] = ".lastlogin";
	struct stat		s;
	struct utimbuf {
		time_t	actime;
		time_t	modtime;
	} utbuf;

	if (stat(fname, &s) == 0) {
		if (s.st_atime != s.st_mtime
		    || s.st_ctime - s.st_atime != 2) {
			printf("Warning:  %s was altered since last login\n",
				fname);
		}
		printf("Login last used: %s", ctime(&s.st_ctime));
	} else {
		printf("Warning: %s did not exist, creating it\n", fname);
		if ((fd = open(fname, O_RDONLY|O_CREAT|O_TRUNC, 0400)) == -1) {
			perror("cannot write file in login directory");
			return;
		}
		close(fd);
	}
	do {
		utbuf.actime = utbuf.modtime = time((long *)0) - 2;
		utime(fname, &utbuf);
		stat(fname, &s);
	} while( s.st_ctime - s.st_atime != 2 );
}

/*
 * badlogin() - log to the system console after
 *     NFAILURES unsuccessful attempts
 */
badlogin(name, ttyn)
char *name;
char *ttyn;
{
	FILE	*console;
	int	clock;

	if ((console = fopen(CONSOLE, "w")) == NULL) {
		perror("cannot write to system console");
		return;
	}
	time(&clock);
	fprintf(console, "\nUNSUCCESSFUL LOGIN: %s on %s: %s", name, ttyn,
		ctime(&clock));
	fclose(console);
}
#endif	/* SECURITY */
