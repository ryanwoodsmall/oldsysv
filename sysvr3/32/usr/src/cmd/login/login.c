/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)login:login.c	1.41"

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
#include <unistd.h>	/* For logfile locking */
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/utsname.h>
#include <termio.h>
#include <sys/stropts.h>
#include <shadow.h>	/* shadow password header file */
#include <time.h>

#ifndef	MAXTRYS
#	define	MAXTRYS	5	/* default */
#endif
#ifndef	MAXTIME
#	define	MAXTIME	60	/* default */
#endif
#ifdef	SECURITY
#ifndef	NFAILURES
#	define	NFAILURES 5	/* default */
#endif
#define	LOGINLOG	"/usr/adm/loginlog"	/* login log file */
#define LNAME_SIZE	20	/* size of logged logname */
#define TTYN_SIZE	15	/* size of logged tty name */
#define TIME_SIZE	30	/* size of logged time string */
#define ENT_SIZE	68	/* last three numbers + 3 */
#define L_WAITTIME	5	/* waittime for log file to unlock */
#endif	/* SECURITY */

#ifndef	SLEEPTIME
#	define	SLEEPTIME 4	/* sleeptime before login incorrect msg */
#endif
#ifndef	DISABLETIME
#	define	DISABLETIME	20	/* seconds login disabled after
					   NFAILURES or MAXTRYS unsuccessful 
					   attempts */
#endif

#define SCPYN(a, b)	strncpy(a, b, sizeof(a))
#define EQN(a, b)	(!strncmp(a, b, sizeof(a)-1))
#define DIAL_FILE	"/etc/dialups"
#define DPASS_FILE	"/etc/d_passwd"
#define SHELL		"/bin/sh"
#define	PATH		"PATH=:/bin:/usr/bin"
#define	ROOTPATH	"PATH=/bin:/etc:/usr/bin"
#define SUBLOGIN	"<!sublogin>"

#define	ROOTUID	 0
#define PBUFSIZE 8	/* max significant characters in a password */

#define	MAXARGS 63
#define	MAXENV 1024
#define MAXLINE 256

/*	Illegal passwd entries.
*/
static struct passwd nouser = { "", "no:password", ~ROOTUID };
static struct spwd noupass = { "", "no:password" };

static struct utsname un;
static struct utmp utmp;
static char u_name[64];
static char minusnam[16] = "-";
static char shell[256] = { "SHELL=" };
static char home[256] = { "HOME=" };
static char logname[30] = { "LOGNAME=" };
static char timez[100] = { "TZ=" };

static char loginmsg[] = "login: ";
static char passwdmsg[] = "Password:";
static char incorrectmsg[] = "Login incorrect\n";

#ifndef	NO_MAIL
static char mail[30] = { "MAIL=/usr/mail/" };
#endif

static char *envinit[6+MAXARGS] = { home,PATH,logname,0,0 };
static int basicenv;
static int intrupt;
static char envblk[MAXENV];
static struct passwd *pwd;
static struct spwd *shpwd ;	/* Shadow password structure */

struct passwd *getpwnam();
struct spwd *getspnam() ;
int setpwent();
char *crypt();
char *getpass(), *fgetpass();
char *strrchr(),*strchr(),*strcat();
extern char **environ;

#define	WEEK	(24L * 7 * 60 * 60) /* 1 week in seconds */
static time_t when;
static time_t maxweeks;
static time_t minweeks;
static time_t now;
extern long a64l(), time();

extern void setbuf();
extern FILE *fopen();
extern int fclose(), fprintf(), findiop();
extern int kill(), ioctl(), getpid();

#ifdef	SECURITY
static char *log_entry[NFAILURES] ;
static int writelog=0 ;
#endif

main(argc, argv ,renvp)
char **argv,**renvp;
{
	register char *namep;
	int j,k,l_index,length;
	char *ttyn, *ttyntail, *envtz, *getenv();
#ifdef PASSREQ
	int nopassword = 1;
#endif
	register int i;
	register struct utmp *u;
	struct utmp *getutent(), *pututline();
	FILE *fp;
	char **envp,*ptr,*endptr,*passwdp;
	int sublogin, shad_flag=1, pwdexp_flg=0;
	extern char **getargs();
	extern char *terminal();
	char inputline[MAXLINE];
	int n;

#ifdef	SECURITY
	int timenow ;
	struct stat dbuf ;
#endif

#if	MAXTRYS > 0
	int trys = 0;
#endif

	/*Set flag to disable the pid check if you find that you are	*/
	/*a subsystem login.						*/

	sublogin = 0;
	if( *renvp && strcmp(*renvp,SUBLOGIN) == 0 )
		sublogin = 1;

	umask(0);
	alarm(MAXTIME);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	nice(0);
	ttyn = terminal(0);
	if( ttyn==0 )
		ttyn = "/dev/tty??";
	if( ttyntail = strrchr(ttyn,'/') )
		ttyntail++;
	else
		ttyntail = ttyn;

#ifdef	SECURITY
	/* if the logfile exist, turn on attempt logging and
	   initialize the string storage area */
	if ( stat (LOGINLOG, &dbuf) == 0 )
		{
		writelog = 1 ;
		for ( i = 0 ; i < NFAILURES ; i++ )
			{
			if ( !(log_entry[i] = (char *) malloc ((unsigned) ENT_SIZE) ) )
				{
				writelog = 0 ;
				break ;
				}
			* log_entry[i] = '\0' ;
			}
		}
#endif

	if( argc > 1 ) {
		SCPYN(utmp.ut_user, argv[1]);
		SCPYN(u_name, argv[1]);
		strcpy(inputline, u_name);
		strcat(inputline, "   \n");
		envp = &argv[2];
		goto first;
	}

loop:
#ifdef	SECURITY
	/* If logging is turned on and there is an unsuccessful
	   login attempt, put it in the string storage area */
	if ( (trys > 0) && (writelog == 1) )
		{
		time (&timenow) ;
		(void) strncat ( log_entry[trys-1], u_name, LNAME_SIZE ) ;
		(void) strncat ( log_entry[trys-1], ":", (size_t) 1 ) ;
		(void) strncat ( log_entry[trys-1], ttyn, TTYN_SIZE ) ;
		(void) strncat ( log_entry[trys-1], ":", (size_t) 1 ) ;
		(void) strncat ( log_entry[trys-1], ctime(&timenow), TIME_SIZE ) ;
		}
	if ( trys >= NFAILURES )
		{
		/* If logging is turned on, output the string storage
		   area to the log file, and sleep for DISABLETIME 
		   seconds before exiting. */
		if ( writelog )
			badlogin () ;
		sleep (DISABLETIME) ;
		exit (1) ;
		}
#endif	/* SECURITY */
	u_name[0] = utmp.ut_user[0] = '\0';
first:
	fflush(stdout);
#if	MAXTRYS > 0
	if( ++trys > MAXTRYS )
	{
		sleep (DISABLETIME);
		exit(1);
	}
#endif
	while( utmp.ut_user[0] == '\0' )
	{
		fputs(loginmsg, stdout);
		fflush(stdout);
		if( (envp = getargs(inputline)) != (char**)NULL )
		{
			SCPYN(utmp.ut_user,*envp);
			SCPYN(u_name, *envp++);
		}
	}

	/* If any of the common login messages was the input, we must be
	 * looking at a tty that is running login.  We exit because
	 * they will chat at each other until one times out otherwise.
	 * In time, init(1M) sees this and decides something is amiss.
	 */
	if( EQN(loginmsg, inputline)  ||  EQN(passwdmsg, inputline)  ||
	    EQN(incorrectmsg, inputline) )
	{
		printf("Looking at a login line.\n");
		exit(8);
	}
	  
	/* Check the existence of SHADOW.  If it is there, then we are
	 * running a two-password-file system.
	 */
	if ( access(SHADOW,0) )
		shad_flag = 0 ;		/* SHADOW is not there */

	setpwent();
	if ( shad_flag )
		(void) setspent () ;	/* Setting the shadow password file */

	if ( (pwd = getpwnam (u_name) ) == NULL ||
	     ( shad_flag && (shpwd = getspnam (u_name) ) == NULL) )
	{
		pwd = &nouser ;
		if ( shad_flag )
			shpwd = &noupass ;
	}

	if ( shad_flag )
	{
		passwdp = shpwd->sp_pwdp ;
		(void) endspent () ;	/* Closing the shadow password file */
	} 
	else {
		passwdp = pwd->pw_passwd ;
	}
	endpwent();

#ifdef CONSOLE
	if( pwd->pw_uid == ROOTUID  &&  strcmp(ttyn, CONSOLE) )
	{
		printf("Not on system console\n");
		exit(10);
	}
#endif
	if( *passwdp != '\0' ) 
	{
#ifdef PASSREQ
		nopassword = 0;
#endif
		if( gpass(passwdmsg, passwdp) ) 
			goto loop;
	}

	/*
	 * get dialup password, if necessary
	 */
	if( dialpass(ttyn) )
		goto loop;

	/*
	 * optionally adjust nice(2)
	 */
	if( strncmp("pri=", pwd->pw_gecos, 4) == 0 )
	{
		int mflg, pri;

		pri = 0;
		mflg = 0;
		i = 4;
		if( pwd->pw_gecos[i] == '-' )
		{
			mflg++;
			i++;
		}
		while( pwd->pw_gecos[i] >= '0' && pwd->pw_gecos[i] <= '9' )
			pri = (pri * 10) + pwd->pw_gecos[i++] - '0';
		if( mflg )
			pri = -pri;
		nice(pri);
	}

	if( chdir(pwd->pw_dir) < 0 )
	{
		printf("Unable to change directory to \"%s\"\n",pwd->pw_dir);
		goto loop;
	}
	time(&utmp.ut_time);
	utmp.ut_pid = getpid();

	/*Find the entry for this pid (or line if we are a sublogin) in	*/
	/*the utmp file.						*/


	while( (u = getutent()) != NULL )
	{
		if( (u->ut_type == INIT_PROCESS ||
			u->ut_type == LOGIN_PROCESS ||
			u->ut_type == USER_PROCESS) &&
			( (sublogin && strncmp(u->ut_line,ttyntail,
			sizeof(u->ut_line)) == 0) || u->ut_pid == utmp.ut_pid) )
		{

	/* Copy in the name of the tty minus the "/dev/", the id, and set */
	/* the type of entry to USER_PROCESS.				  */

			SCPYN(utmp.ut_line,(ttyn+sizeof("/dev/")-1));
			utmp.ut_id[0] = u->ut_id[0];
			utmp.ut_id[1] = u->ut_id[1];
			utmp.ut_id[2] = u->ut_id[2];
			utmp.ut_id[3] = u->ut_id[3];
			utmp.ut_type = USER_PROCESS;

	/* Return the new updated utmp file entry. */

			pututline(&utmp);

			break;
		}
	}
	endutent();		/* Close utmp file */

	if( u == (struct utmp *)NULL )
	{
		printf("No utmp entry.  You must exec \"login\" from\
 the lowest level \"sh\".\n");
		exit(1);
	}

	/* Now attempt to write out this entry to the wtmp file if we	*/
	/* were successful in getting it from the utmp file and the	*/
	/* wtmp file exists. Lock wtmp file so simultaneous logins	*/
	/* will not conflict.					 	*/

	if (u != NULL && (fp = fopen(WTMP_FILE,"r+")) != NULL) {
		for ( i=0 ; i<10 ; i++ ) {
			if ( lockf(fileno(fp), F_TLOCK, 0) == -1 ) {
				if ( i < 9 ) sleep (1) ;
				else perror("login: unable to lock accounting file.\n") ;
			}
			else {
				fseek(fp,0L,2) ; /* Seek to end of file */
				fwrite(&utmp,sizeof(utmp),1,fp);
				rewind(fp) ;
				(void) lockf(fileno(fp), F_ULOCK, 0) ;
				i = 10 ;
			}
		}
		fclose(fp);
	}

	chown(ttyn, pwd->pw_uid, pwd->pw_gid);

	/* If the shell field starts with a '*', do a chroot to the home */
	/* directory and perform a new login.			 */

	if( *pwd->pw_shell == '*' )
	{
		if( chroot(pwd->pw_dir) < 0 )
		{
			printf("No Root Directory\n");
			goto loop;
		}

	/* Set the environment flag <!sublogin> so that the next login	*/
	/* knows that it is a sublogin.					*/

		envinit[0] = SUBLOGIN;
		envinit[1] = (char*)NULL;
		printf("Subsystem root: %s\n",pwd->pw_dir);
		execle("/bin/login", "login", (char*)0, &envinit[0]);
		execle("/etc/login", "login", (char*)0, &envinit[0]);
		printf("No /bin/login or /etc/login on root\n");
		goto loop;
	}

	if( setgid(pwd->pw_gid) == -1 )
	{
		printf("Bad group id.\n");
		exit(1);
	}
	if( setuid(pwd->pw_uid) == -1 )
	{
		printf("Bad user id.\n");
		exit(1);
	}


	alarm(0);	/*give user time to come up with new password if needed*/

#ifdef PASSREQ
	if( nopassword )
	{
		printf("You don't have a password.  Choose one.\n");
		printf("passwd %s\n", u_name);
		fflush(stdout);
		n = system("/bin/passwd");
		if( n > 0 )
			goto loop;
		if( n < 0 )
		{
			printf("Cannot execute /bin/passwd\n");
			exit(1);
		}
	}
#endif

	/* Is the age of the password to be checked? */

	if ( shad_flag )	/* with shadow */
	{
		now  = DAY_NOW ;
		if( ( shpwd->sp_lstchg == 0 )				||
		    ( shpwd->sp_lstchg > now )				||
		    ( (shpwd->sp_max >= 0 )		&&
		      ( now > (shpwd->sp_lstchg + shpwd->sp_max) ) &&
		      ( shpwd->sp_max >= shpwd->sp_min )	 )       )
		      pwdexp_flg = 1 ;
	}
	else {			/* without shadow */
		if( *pwd->pw_age != NULL )
		{
			/*
			 * retrieve (a) week of previous change 
			 *          (b) maximum number of valid weeks
			 */
			when = a64l(pwd->pw_age);
			/* max, min and weeks since last change are packed radix 64 */
			maxweeks = when & 077;
			minweeks = (when >> 6) & 077;
			when >>= 12;
			now  = time(0)/WEEK;
			if( when > now || (now > when + maxweeks) && (maxweeks >= minweeks) )
				pwdexp_flg = 1 ;

		}
	}

	/* If user's password has expired */
	if ( pwdexp_flg )
	{
		printf("Your password has expired. Choose a new one\n");
		n = system("/bin/passwd");
		if( n > 0 )
			exit(9);
		if( n < 0 )
		{
			printf("Cannot execute /bin/passwd\n");
			exit(9);
		}
	}

	/* Set up the basic environment for the exec.  This includes	*/
	/* HOME, PATH, LOGNAME, SHELL, and MAIL.			*/

	strcat(home, pwd->pw_dir);
	strcat(logname, pwd->pw_name);
	if( pwd->pw_uid == ROOTUID )
		envinit[1] = ROOTPATH;

	if( *pwd->pw_shell == '\0' )
		pwd->pw_shell = SHELL;

#ifndef NOSHELL
	else
		envinit[3] = shell;
#endif
	strcat(shell, pwd->pw_shell);

	/* Find the name of the shell.*/

	if( (namep = strrchr(pwd->pw_shell, '/')) == NULL )
		namep = pwd->pw_shell;
	else
		namep++;

	/* Generate the name of the shell with a '-' sign in front of it. */
	/* This causes .profile processing when a shell is exec'ed. */

	strcat(minusnam, namep);

#ifndef	NO_MAIL
	if( envinit[3] == (char*)NULL )
		envinit[3] = mail;
	else
		envinit[4] = mail;
	strcat(mail,pwd->pw_name);
#endif

	/* Find the end of the basic environment */

	for( basicenv=3; envinit[basicenv]; basicenv++ );

	/* Add in the time zone (TZ) variable to the end of the */
	/* basic environment */

	if ( (envtz=getenv("TZ")) != NULL ) {
		strcat(timez, envtz) ;
		envinit[basicenv++] = timez ;
	}

	/* Add in all the environment variables picked up from the */
	/* argument list to "login" or from the user response to the */
	/* "login" request. */

	for( j=0,k=0,l_index=0,ptr= &envblk[0]; *envp && j<(MAXARGS-1);
		j++,envp++ )
	{

	/* Scan each string provided.  If it doesn't have the format */
	/* xxx=yyy, then add the string "Ln=" to the beginning. */

		if( (endptr = strchr(*envp,'=')) == (char*)NULL )
		{
			envinit[basicenv+k] = ptr;
			sprintf(ptr,"L%d=%s",l_index,*envp);

	/* Advance "ptr" to the beginning of the next argument.	*/

			while( *ptr++ );
			k++;
			l_index++;
		}

	/* Is this an environmental variable we permit?	*/

		else if( !legalenvvar(*envp) )
			continue;


	/* Check to see whether this string replaces any previously- */
	/* defined string. */

		else
		{
			for( i=0,length=endptr+1-*envp; i<basicenv+k; i++ )
			{
				if( strncmp(*envp,envinit[i],length) == 0 )
				{
					envinit[i] = *envp;
					break;
				}
			}

	/* If it doesn't, place it at the end of environment array. */

			if( i == basicenv+k )
			{
				envinit[basicenv+k] = *envp;
				k++;
			}
		}
	}

	/* Switch to the new environment. */

	environ = envinit;
	alarm(0);

	signal(SIGQUIT, SIG_DFL);
	signal(SIGINT, SIG_DFL);
	uname(&un);
#if u3b || u3b2 || u3b15
	printf("UNIX System V Release %s AT&T %s\n%s\n\
Copyright (c) 1984, 1986, 1987, 1988 AT&T\nAll Rights Reserved\n",
		un.release, un.machine, un.nodename);
#else
	printf("UNIX System V Release %s %s\n%s\n\
Copyright (c) 1984 AT&T\nAll Rights Reserved\n",
		un.release, un.machine, un.nodename);
#endif /*u3b,u3b2,u3b15*/

#ifdef	SECURITY
	/*	
	 *	Advise the user the time and date that this login-id
	 *	was last used. 
	 *	Check to see if home directory is writable.
	 *	If not, do not perform lastlogin.
	 */

	if ( !access(".",02))
	{
		lastlogin();
	}

#endif	/* SECURITY */

	execl(pwd->pw_shell, minusnam, (char*)0);
	
	/* pwd->pw_shell was not an executable object file, maybe it
	 * is a shell proceedure or a command line with arguments.
	 * If so, turn off the SHELL= environment variable.
	 */
	if( !strncmp( envinit[3], shell, 6 ) )
		envinit[3][6] = '\0';
	if( access( pwd->pw_shell, 05 ) == 0 )
		execl(SHELL, "sh", pwd->pw_shell, minusnam, (char*)0);

	printf("No shell\n");
	exit(1);
	/* NOTREACHED */
}

static
dialpass(ttyn)
char *ttyn;
{
	register FILE *fp;
	char defpass[30];
	char line[80];
	register char *p1, *p2;

	if( (fp=fopen(DIAL_FILE, "r")) == NULL )
		return(0);
	while( (p1 = fgets(line, sizeof(line), fp)) != NULL )
	{
		while( *p1 != '\n' && *p1 != ' ' && *p1 != '\t' )
			p1++;
		*p1 = '\0';
		if( strcmp(line, ttyn) == 0 )
			break;
	}
	fclose(fp);
	if( p1 == NULL || (fp = fopen(DPASS_FILE, "r")) == NULL )
		return(0);
	defpass[0] = '\0';
	p2 = 0;
	while( (p1 = fgets(line, sizeof(line)-1, fp)) != NULL )
	{
		while( *p1 && *p1 != ':' )
			p1++;
		*p1++ = '\0';
		p2 = p1;
		while( *p1 && *p1 != ':' )
			p1++;
		*p1 = '\0';
		if( strcmp(pwd->pw_shell, line) == 0 )
			break;

		if( strcmp(SHELL, line) == 0 )
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

static
gpass(prmt, pswd)
char *prmt, *pswd;
{
	register char *p1;
	time_t time() ;

	/* getpass() fails if it cannot open /dev/tty.
	 * If this happens, and the real UID is root,
	 * then use the current stdin and stderr.
	 * This allows login to work with network connections
	 * and other non-ttys.
	 */
	if( ((p1 = getpass(prmt)) == (char *)0) && (getuid() == ROOTUID) ) {
		p1 = fgetpass(stdin, stderr, prmt);
	}
	if( !p1 || strcmp(crypt(p1, pswd), pswd) )
	{
	/* A sleep was done with the following code:
	#if  MAXTRYS > 0  &&  MAXTIME > 0  &&  (MAXTIME - 2*MAXTRYS) > 0
		sleep( (MAXTIME - 2*MAXTRYS)/MAXTRYS );
	#endif
	It was changed to sleep(SLEEPTIME) because the above algorithm
	will cause login to sleep for a long time if MAXTRYS were made
	small in relation to MAXTIME.
	*/
		sleep (SLEEPTIME) ;
		printf( incorrectmsg );
		return(1);
	}
	return(0);
}

#define	WHITESPACE	0
#define	ARGUMENT	1

static char **
getargs(inline)
char *inline;
{
	static char envbuf[MAXLINE];
	static char *args[MAXARGS];
	register char *ptr,**answer;
	register int c;
	int state;
	extern int quotec();

	for( ptr= envbuf; ptr < &envbuf[sizeof(envbuf)]; )
		*ptr++ = '\0';

	for( answer= args; answer < &args[MAXARGS]; )
		*answer++ = (char *)NULL;

	for( ptr= envbuf,answer= &args[0],state = WHITESPACE;
	     (c = getc(stdin)) != EOF; )
	{

		*(inline++) = c;
		switch( c ) {
	
		case '\n':
			if( ptr == &envbuf[0] ) return((char **)NULL);
			else return(&args[0]);
		case ' ':
		case '\t':
			if( state == ARGUMENT )
			{
				*ptr++ = '\0';
				state = WHITESPACE;
			}
			break;
		case '\\':
			c = quotec();
		default:
			if( state == WHITESPACE )
			{
				*answer++ = ptr;
				state = ARGUMENT;
			}
			*ptr++ = c;
		}

	/* If the buffer is full, force the next character to be read to */
	/* be a <newline>.						 */

		if( ptr == &envbuf[sizeof(envbuf)-1] )
		{
			ungetc('\n',stdin);
			putc('\n',stdout);
		}
	}

	/* If we left loop because an EOF was received, exit immediately. */

	exit(0);
	/* NOTREACHED */
}

static int
quotec()
{
	register int c, i, num;

	switch( c = getc(stdin) )
	{
	case 'n':
		c = '\n';
		break;
	case 'r':
		c = '\r';
		break;
	case 'v':
		c = '\013';
		break;
	case 'b':
		c = '\b';
		break;
	case 't':
		c = '\t';
		break;
	case 'f':
		c = '\f';
		break;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
		for( num=0,i=0; i<3; i++ )
		{
			num = num * 8 + (c - '0');
			if( (c = getc(stdin)) < '0' || c > '7' )
				break;
		}
		ungetc(c,stdin);
		c = num & 0377;
		break;
	default:
		break;
	}
	return(c);
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

static char *
terminal(f)
int f;
{
	struct stat fsb, tsb;
	struct dirent *dp;
	register DIR *df;
	register int pass1;
	static char rbuf[64];
	static char dev[]="/dev/";

	if( isatty(f) == 0 )
		return(NULL);

	if( fstat(f, &fsb) < 0 )
		return(NULL);

	if( (fsb.st_mode & S_IFMT) != S_IFCHR )
		return(NULL);

	if( (df=opendir(dev)) == NULL )
		return(NULL);
	pass1 = 1;
	do
	{
		while( (dp = readdir(df)) != NULL )
		{
			if( pass1  &&  dp->d_ino != fsb.st_ino )
				continue;

			if( strncmp(dp->d_name, "syscon", 6) == 0 ||
				strncmp(dp->d_name, "systty", 6) == 0 )
				continue;

			(void)strcpy(rbuf, dev);
			(void)strncat(rbuf, dp->d_name, sizeof rbuf - sizeof dev);
			if( stat(rbuf, &tsb) < 0 )
				continue;

			if( tsb.st_rdev == fsb.st_rdev &&
				(tsb.st_mode&S_IFMT) == S_IFCHR &&
				(!pass1 || tsb.st_ino == fsb.st_ino) )
			{
				closedir(df);
				return(rbuf);
			}
		}
		rewinddir(df);
	} while( pass1-- );
	closedir(df);
	return(NULL);
}


static char *illegal[] = {
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

/* Is it legal to insert this environmental variable? */
static int
legalenvvar(s)
char *s;
{
	register char **p;

	for( p=illegal; *p; p++ )
		if( !strncmp(s, *p, strlen(*p)) )
			return(0);
	return(1);
}

#ifdef	SECURITY
/*
 *  lastlogin() -- used to inform the user of the time and
 *        date that this login was last used.  Uses a
 *        file in the login directory called .lastlogin to
 *        keep track of the date/time.  Inode change time is
 *        used to prevent unauthorized changing of file times. If file
 *	  is not owned by current uid, do not notify user of last
 *	  login.
 */
lastlogin()
{
	register int fd;
	static char fname[] = ".lastlogin";
	struct stat s;
	struct utimbuf
	{
		time_t actime;
		time_t modtime;
	} utbuf;

	if( stat(fname, &s) == 0 )
	{
		/* if .lastlogin not owned by current uid, return */
		if ( s.st_uid != getuid() )
			return;

		if( s.st_mtime - s.st_atime != 2 )
		{
			printf("Warning:  %s was altered since last login\n",
				fname);
		}
		printf("Login last used: %s", ctime(&s.st_ctime));
	}
	else
	{
		printf("Warning: %s did not exist, creating it\n", fname);
		if( (fd = open(fname, O_RDONLY|O_CREAT|O_TRUNC, 0400)) == -1 )
		{
			perror("cannot write file in login directory");
			return;
		}
		close(fd);
	}

	utbuf.modtime = time((long *)0);
	utbuf.actime = utbuf.modtime - 2;

	if ( utime(fname, &utbuf) < 0 )
		printf("Warning: .lastlogin cannot be updated\n");

}

/*
 * badlogin() - log to the log file after
 *     NFAILURES unsuccessful attempts
 */
badlogin ()
{
	int retval, count1, fildes, donothing() ;

	/* Tries to open the log file. If succeed, lock it and write
	   in the failed attempts */
	if ( (fildes = open (LOGINLOG, O_APPEND|O_WRONLY)) == -1 )
		return (0) ;
	else	{
		(void) sigset ( SIGALRM, donothing ) ;
		(void) alarm ( L_WAITTIME ) ;
		retval = lockf ( fildes, F_LOCK, 0L ) ;
		(void) alarm ( 0 ) ;
		(void) sigset ( SIGALRM, SIG_DFL ) ;
		if ( retval == 0 )
			{
			for ( count1 = 0 ; count1 < NFAILURES ; count1++ )
			   write (fildes, log_entry[count1],
				  (unsigned) strlen (log_entry[count1])) ;
			(void) lockf (fildes, F_ULOCK, 0L) ;
			(void) close (fildes) ;
			}
		return (0) ;
		}
}

donothing()
{}

#endif	/* SECURITY */


char *
getpass(prompt)
char *prompt;
{
	char *p;
	FILE *fi;

	if( (fi = fopen("/dev/tty", "r")) == NULL ) {
			return((char*)NULL);
	}
	setbuf(fi, (char*)NULL);
	p = fgetpass(fi, stderr, prompt);
	if( fi != stdin )
		(void)fclose(fi);
	return(p);
}


char *
fgetpass(fi, fo, prompt)
FILE *fi, *fo;
char *prompt;
{
	struct termio ttyb;
	unsigned short flags;
	register char *p;
	register int c;
	static char pbuf[PBUFSIZE + 1];
	void (*sig)(), catch();

	sig = signal(SIGINT, catch);
	intrupt = 0;
	(void)ioctl(fileno(fi), TCGETA, &ttyb);
	flags = ttyb.c_lflag;
	ttyb.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
	(void)ioctl(fileno(fi), TCSETAF, &ttyb);
	(void)fputs(prompt, fo);
	for( p=pbuf; !intrupt && (c = getc(fi)) != '\n' && c != EOF; )
	{
		if( p < &pbuf[PBUFSIZE] )
			*p++ = c;
	}
	*p = '\0';
	(void)putc('\n', fo);
	ttyb.c_lflag = flags;
	(void)ioctl(fileno(fi), TCSETAW, &ttyb);
	(void)signal(SIGINT, sig);
	if( intrupt )
		(void)kill(getpid(), SIGINT);

	
	return(pbuf);
}


static void
catch()
{
	++intrupt;
}



