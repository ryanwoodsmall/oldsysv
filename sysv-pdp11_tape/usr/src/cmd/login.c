/*	@(#)login.c	1.2	*/
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
 */

#include <sys/types.h>
#include <utmp.h>
#include <signal.h>
#include <pwd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/dir.h>

#define	TRUE		1
#define	FALSE		0
#define	FAILURE		-1

#define SCPYN(a, b)	strncpy(a, b, sizeof(a))
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

struct	passwd nouser = {"", "nope"};
struct	utmp utmp, ubuf;
char	minusnam[16] = "-";
char	shell[64] = { "SHELL=" } ;
char	home[64] = { "HOME=" } ;
char	logname[30] = { "LOGNAME=" } ;

#ifndef	NO_MAIL

char	mail[30] = { "MAIL=/usr/mail/" } ;

#endif

char	*envinit[5+MAXARGS] =
	{
		home,PATH,logname,0,0
	} ;
int	basicenv ;
char	envblk[MAXENV] ;
struct	passwd *pwd;

struct	passwd *getpwnam();
int	setpwent();
char	*crypt();
char	*getpass();
char	*strrchr(),*strchr(),*strcat() ;
extern	char **environ;

#define	WEEK	(24L * 7 * 60 * 60) /* 1 week in seconds */
time_t	when;
time_t	maxweeks;
time_t	minweeks;
time_t	now;
long 	a64l();

main(argc, argv ,renvp)
char **argv,**renvp;
{
	register char *namep;
	int c,j,k,l_index,length ;
	char *ttyn,*ttyntail;
	int nopassword ;
	register int i ;
	register struct utmp *u ;
	struct utmp *getutent() ;
	FILE *fp ;
	struct stat statbuf ;
	char **envp,*ptr,*endptr ;
	int sublogin ;
	extern char **getargs() ;
	extern char *terminal() ;

/*	Set flag to disable the pid check if you find that you are	*/
/*	a subsystem login.						*/

	sublogin = FALSE;
	if (*renvp && strcmp(*renvp,SUBLOGIN) == 0)
		sublogin = TRUE;

	umask(0);
	alarm(60);
	signal(SIGQUIT, 1);
	signal(SIGINT, 1);
	nice(0);
	ttyn = terminal(0);
	if (ttyn==0)
		ttyn = "/dev/tty??";
	if (ttyntail = strrchr(ttyn,'/')) ttyntail++ ;
	else ttyntail = ttyn ;

	if (--argc) {
		SCPYN(utmp.ut_user, *++argv);
		for(namep = &utmp.ut_user[0]; *namep; namep++);
		envp = ++argv ;
		goto first ;
	}

loop:
	SCPYN(utmp.ut_user, "");
first:
	while (utmp.ut_user[0] == '\0') {
		namep = &utmp.ut_user[0];
		printf("login: ");
		if ((envp = getargs()) != (char**)NULL) {
			SCPYN(utmp.ut_user,*envp++) ;
			for (namep = &utmp.ut_user[0] ; *namep &&
				namep < &utmp.ut_user[sizeof(utmp.ut_user)];)
				namep++ ;
		}
	}
	setpwent();
	if ((pwd = getpwnam(&utmp.ut_user[0])) == NULL)
		pwd = &nouser;
	endpwent();
	if (strncmp("pri=", pwd->pw_gecos, 4) == 0) {
		int	i, mflg, pri;

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
	while (namep < &utmp.ut_user[sizeof(utmp.ut_user)])
		*namep++ = '\0';
	nopassword = 1;
	if (*pwd->pw_passwd != '\0') {
		nopassword = 0;
		if(gpass("Password:", pwd->pw_passwd))
			goto loop;
	}

	/*
	 * get dialup password, if necessary
	 */
	if(dialpass(ttyn))
		goto loop;

	if(chdir(pwd->pw_dir) < 0) {
		printf("Unable to change directory to \"%s\"\n",pwd->pw_dir);
		if(chdir("/") < 0) {
			printf("Unable to change directory to \"/\"\n");
			goto loop;
		}
		printf("You are in \"/\" instead.\n");
	}
	time(&utmp.ut_time);
	i = utmp.ut_pid = getpid();

/*	Find the entry for this pid (or line if we are a sublogin) in	*/
/*	the utmp file.							*/

	while ((u = getutent()) != NULL) {
		if ((u->ut_type == INIT_PROCESS || u->ut_type ==
		    LOGIN_PROCESS || u->ut_type == USER_PROCESS)
		    && ( (sublogin && strncmp(u->ut_line,ttyntail,
		    sizeof(u->ut_line)) == 0) || u->ut_pid == i) ) {

/*	Copy in the name of the tty minus the "/dev/", the id, and set	*/
/*	the type of entry to USER_PROCESS.				*/

			SCPYN(utmp.ut_line,(ttyn+sizeof("/dev/")-1)) ;
			utmp.ut_id[0] = u->ut_id[0] ;
			utmp.ut_id[1] = u->ut_id[1] ;
			utmp.ut_id[2] = u->ut_id[2] ;
			utmp.ut_id[3] = u->ut_id[3] ;
			utmp.ut_type = USER_PROCESS ;

/*	Return the new updated utmp file entry.				*/

			pututline(&utmp) ;
			break ;
		}
	}
	endutent() ;		/* Close utmp file */

	if (u == (struct utmp *)NULL){
		printf("No utmp entry.  You must exec \"login\" from\
 the lowest level \"sh\".\n") ;
		exit(1) ;
	}

/*	Now attempt to write out this entry to the wtmp file if we	*/
/*	were successful in getting it from the utmp file and the	*/
/*	wtmp file exists.						*/

	if (u != NULL && (fp = fopen(WTMP_FILE,"r+")) != NULL) {
		fseek(fp,0L,2) ;		/* Seek to end of file. */
		fwrite(&utmp,sizeof(utmp),1,fp) ;
		fclose(fp) ;
	}
	chown(ttyn, pwd->pw_uid, pwd->pw_gid);

/*	If the shell field starts with a '*', do a chroot to the home	*/
/*	directory and perform a new login.				*/

	if(*pwd->pw_shell == '*')
	{
		if(chroot(pwd->pw_dir) < 0)
		{
			printf("No Root Directory\n");
			goto loop;
		}

/*	Set the environment flag <!sublogin> so that the next login	*/
/*	knows that it is a sublogin.					*/

		envinit[0] = SUBLOGIN ;
		envinit[1] = (char*)NULL ;
		printf("Subsystem root: %s\n",pwd->pw_dir);
		execle("/bin/login", "login", 0,&envinit[0]);
		execle("/etc/login", "login", 0,&envinit[0]);
		printf("No /bin/login or /etc/login on root\n");
		goto loop;
	}

	if (setgid(pwd->pw_gid) == FAILURE)
	  {
	    printf("Bad group id.\n") ;
	    exit(1) ;
	  }
	if (setuid(pwd->pw_uid) == FAILURE)
	  {
	    printf("Bad user id.\n") ;
	    exit(1) ;
	  }

/*	Set up the basic environment for the exec.  This includes	*/
/*	HOME, PATH, LOGNAME, SHELL, and MAIL.				*/

	strcat(home, pwd->pw_dir);
	strcat(logname, pwd->pw_name);
	if(pwd->pw_uid == ROOTUID)
	{
#ifdef CONSOLE
		if(strcmp(ttyn, CONSOLE))
		{
			printf("Not on system console\n");
			exit(10);
		}
#endif
		envinit[1] = ROOTPATH ;
	}
	if (*pwd->pw_shell == '\0') {
		pwd->pw_shell = SHELL;
		strcat(shell, pwd->pw_shell);
	}
#ifndef NOSHELL
	else {
		envinit[3] = shell;
		strcat(shell, pwd->pw_shell);
	}
#endif

/*	Find the name of the shell.					*/

	if ((namep = strrchr(pwd->pw_shell, '/')) == NULL)
		namep = pwd->pw_shell;
	else
		namep++;

/*	Generate the name of the shell with a '-' sign in front of it.	*/
/*	This causes .profile processing when a shell is exec'ed.	*/

	strcat(minusnam, namep);

#ifndef	NO_MAIL
	if (envinit[3] == (char*)NULL) envinit[3] = mail ;
	else envinit[4] = mail ;
	strcat(mail,pwd->pw_name) ;
#endif

/*	Find the end of the basic environment.				*/

	for (basicenv=3; envinit[basicenv] ;basicenv++) ;

/*	Add in all the environment variables picked up from the		*/
/*	argument list to "login" or from the user response to the	*/
/*	"login" request.						*/

	for (j=0,k=0,l_index=0,ptr= &envblk[0]; *envp && j < MAXARGS-1
		;j++,envp++) {

/*	Scan each string provided.  If it doesn't have the format	*/
/*	xxx=yyy, then add the string "Ln=" to the beginning.		*/

		if ((endptr = strchr(*envp,'=')) == (char*)NULL) {
			envinit[basicenv+k] = ptr ;
			sprintf(ptr,"L%d=%s",l_index,*envp) ;

/*	Advance "ptr" to the beginning of the next argument.		*/

			while (*ptr++) ;
			k++ ;
			l_index++ ;
		} else {

/*	Check to see whether this string replaces any previously	*/
/*	defined string.							*/

			for (i=0,length= endptr+1-*envp; i < basicenv+k ;i++) {
				if (strncmp(*envp,envinit[i],length) == 0) {

/*	If the variable to be changed is "SHELL=" or "PATH=", don't	*/
/*	allow the substitution.						*/

					if (strncmp(*envp,"SHELL=",6) != 0
					    && strncmp(*envp,"PATH=",5) != 0)
						envinit[i] = *envp ;
					break ;
				}
			}

/*	If it doesn't, place it at the end of environment array.	*/

			if (i == basicenv+k) {
				envinit[basicenv+k] = *envp ;
				k++ ;
			}
		}
	}

/*	Switch to the new environment.					*/

	environ = envinit;
	alarm(0);
#ifdef PASSREQ
	if (nopassword) {
		printf("You don't have a password.  Choose one.\n");
		for(namep = utmp.ut_user; *namep != ' '; namep++)
			if(namep == &utmp.ut_user[sizeof(utmp.ut_user)])
				break;
		*namep = '\0';
		utmp.ut_user[0] &= ~0200;
		printf("passwd %s\n", utmp.ut_user);
		execl("/bin/passwd", "passwd", utmp.ut_user, 0);
		printf("Can not execute /bin/passwd\n");
		exit(1);
	}
#endif

/* is the age of the password to be checked? */

	if (*pwd->pw_age != NULL) {
		/* retrieve (a) week of previous change; 
			(b) maximum number of valid weeks	*/
		when = (long) a64l (pwd->pw_age);
		/* max, min and weeks since last change are packed radix 64 */
		maxweeks = when & 077;
		minweeks = (when >> 6) & 077;
		when >>= 12;
		now  = time(0)/WEEK;
		if ((now > when + maxweeks) && (maxweeks >= minweeks)) {
			printf ("Your password has expired. Choose\
 a new one\n");
			execl("/bin/passwd", "passwd", utmp.ut_user, 0);
			exit(9);
		}
	}

	signal(SIGQUIT, 0);
	signal(SIGINT, 0);
	execlp(pwd->pw_shell, minusnam, 0);
	printf("No shell\n");
	exit(1);
}

dialpass(ttyn)
char *ttyn;
{
	register FILE *fp;
	char defpass[30];
	char line[80];
	register char *p1, *p2;

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
	while((p1 = fgets(line, sizeof(line)-1, fp)) != NULL) {
		while(*p1 && *p1 != ':')
			p1++;
		*p1++ = '\0';
		p2 = p1;
		while(*p1 && *p1 != ':')
			p1++;
		*p1 = '\0';
		if(strcmp(pwd->pw_shell, line) == 0) {
			fclose(fp);
			if (*p2 == '\0')
				return(0);
			return(gpass("Dialup Password:", p2));
		}
		if(strcmp(SHELL, line) == 0)
			SCPYN(defpass, p2);
	}
	fclose(fp);
	return(gpass("Dialup Password:", defpass));
}

gpass(prmt, pswd)
char *prmt, *pswd;
{
	register char *p1;

	p1 = crypt(getpass(prmt), pswd);
	if(strcmp(p1, pswd)) {
		printf("Login incorrect\n");
		return(1);
	}
	return(0);
}

#define	WHITESPACE	0
#define	ARGUMENT	1

char **getargs()
{
	static char envblk[MAXLINE] ;
	static char *args[MAXARGS] ;
	register char *ptr,**answer ;
	register int c ;
	int state ;
	extern int quotec() ;

	for (ptr= &envblk[0]; ptr < &envblk[sizeof(envblk)] ;) *ptr++ = '\0' ;
	for (answer= &args[0]; answer < &args[MAXARGS] ;)
		*answer++ = (char *)NULL ;
	for (ptr= &envblk[0],answer= &args[0],state = WHITESPACE
		; (c = getc(stdin)) != EOF;) {

		switch (c) {

		    case '\n' :
			if (ptr == &envblk[0]) return((char **)NULL) ;
			else return(&args[0]) ;

		    case ' ' :
		    case '\t' :
			if (state == ARGUMENT) {
				*ptr++ = '\0' ;
				state = WHITESPACE ;
			}
			break ;

		    case '\\' :
			c = quotec() ;
		    default :
			if (state == WHITESPACE) {
				*answer++ = ptr ;
				state = ARGUMENT ;
			}
			*ptr++ = c ;
		}

/*	If the buffer is full, force the next character to be read to	*/
/*	be a <newline>.							*/

		if (ptr == &envblk[sizeof(envblk)-1]) {
			ungetc('\n',stdin) ;
			putc('\n',stdout) ;
		}
	}

/*	If we left loop because an EOF was received, exit immediately.	*/

	exit(0) ;
}

int quotec()
{
	register int c,i,num ;

	switch(c = getc(stdin)) {
	    case 'n' :
		c = '\n' ;
		break ;
	    case 'r' :
		c = '\r' ;
		break ;
	    case 'v' :
		c = 013 ;
		break ;
	    case 'b' :
		c = '\b' ;
		break ;
	    case 't' :
		c = '\t' ;
		break ;
	    case 'f' :
		c = '\f' ;
		break ;
	    case '0' :
	    case '1' :
	    case '2' :
	    case '3' :
	    case '4' :
	    case '5' :
	    case '6' :
	    case '7' :
		for (num=0,i=0; i < 3 ;i++) {
			num = num * 8 + (c - '0') ;
			if ((c = getc(stdin)) < '0' || c > '7')
				break ;
		}
		ungetc(c,stdin) ;
		c = num & 0377 ;
		break ;
	    default :
		break ;
	}
	return (c) ;
}
/*
 * termainal(f): return "/dev/ttyXX" which the the name of the
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

char *
terminal(f)
int	f;
{
	struct stat fsb, tsb;
	struct direct db;
	register int df, pass1;
	extern char *strcpy(), *strcat();
	extern long lseek();
	static char rbuf[32], dev[]="/dev/";

	if(isatty(f) == 0)
		return(NULL);
	if(fstat(f, &fsb) < 0)
		return(NULL);
	if((fsb.st_mode & S_IFMT) != S_IFCHR)
		return(NULL);
	if((df = open(dev, 0)) < 0)
		return(NULL);
	pass1 = 1;
	do {
		while(read(df, (char *)&db, sizeof(db)) == sizeof(db)) {
			if(db.d_ino == 0)
				continue;
			if(pass1 && db.d_ino != fsb.st_ino)
				continue;
			if (strncmp(&db.d_name[0],"syscon") == 0 ||
				strncmp(&db.d_name[0],"systty") == 0)
				continue ;
			(void) strcpy(rbuf, dev);
			(void) strcat(rbuf, db.d_name);
			if(stat(rbuf, &tsb) < 0)
				continue;
			if(tsb.st_rdev == fsb.st_rdev &&
				(tsb.st_mode&S_IFMT) == S_IFCHR &&
				(!pass1 || tsb.st_ino == fsb.st_ino)) {
				(void) close(df);
				return(rbuf);
			}
		}
		(void) lseek(df, 0L, 0);
	} while(pass1--);
	(void) close(df);
	return(NULL);
}
