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
/*	@(#)su.c	1.8	*/
/*
 *	su [-] [name [arg ...]] change userid, `-' changes environment.
 *	If SULOG is defined, all attempts to su to another user are
 *	logged there.
 *	If CONSOLE is defined, all successful attempts to su to uid 0
 *	are also logged there.
 *
 *	If su cannot create, open, or write entries into SULOG,
 *	(or on the CONSOLE, if defined), the entry will not
 *	be logged -- thus losing a record of the su's attempted
 *	during this period.
 */
#include <stdio.h>
#include <pwd.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#define SULOG	"/usr/adm/sulog"	/*log file*/
#define PATH	"PATH=:/bin:/usr/bin"	/*path for users other than root*/
#define SUPATH	"PATH=/bin:/etc:/usr/bin"	/*path for root*/
#define SUPRMT	"PS1=# "		/*primary prompt for root*/
#define ELIM 128
#define ROOT 0
#define SYS 3
long time();
void pause();
void to();
struct	passwd *pwd, *getpwnam();
struct	tm *localtime();
char	*malloc(), *strcpy();
char	*getpass(), *ttyname(), *strrchr();
char	*shell = "/bin/sh";	/*default shell*/
char	su[16] = "su";		/*arg0 for exec of shprog*/
char	homedir[64] = "HOME=";
char	logname[20] = "LOGNAME=";
char	*path = PATH;
char	*supath = SUPATH;
char	*suprmt = SUPRMT;
char	*envinit[ELIM];
extern	char **environ;
char *ttyn;

main(argc, argv)
int	argc;
char	**argv;
{
	char *nptr, *password;
	char	*pshell = shell;
	int badsw = 0;
	int eflag = 0;
	int uid, gid;
	char *dir, *shprog, *name;

	if (argc > 1 && *argv[1] == '-') {
		eflag++;	/*set eflag if `-' is specified*/
		argv++;
		argc--;
	}

	/*determine specified userid, get their password file entry,
	  and set variables to values in password file entry fields
	*/
	nptr = (argc > 1)? argv[1]: "root";
	if((pwd = getpwnam(nptr)) == NULL) {
		fprintf(stderr,"su: Unknown id: %s\n",nptr);
		exit(1);
	}
	uid = pwd->pw_uid;
	gid = pwd->pw_gid;
	dir = strcpy(malloc(strlen(pwd->pw_dir)+1),pwd->pw_dir);
	shprog = strcpy(malloc(strlen(pwd->pw_shell)+1),pwd->pw_shell);
	name = strcpy(malloc(strlen(pwd->pw_name)+1),pwd->pw_name);
	if((ttyn=ttyname(0))==NULL)
		if((ttyn=ttyname(1))==NULL)
			if((ttyn=ttyname(2))==NULL)
				ttyn="/dev/tty??";

#ifdef SULOG
	/*if SULOG defined, create SULOG, if it does not exist, with
	  mode 600. Change owner and group to root
	*/
	close( open(SULOG, O_WRONLY | O_APPEND | O_CREAT, 0600) );
	chown(SULOG, ROOT, ROOT);
#endif

	/*Prompt for password if invoking user is not root or
	  if specified(new) user requires a password
	*/
	if(pwd->pw_passwd[0] == '\0' || getuid() == 0 )
		goto ok;
	password = getpass("Password:");
	if(badsw || (strcmp(pwd->pw_passwd, crypt(password, pwd->pw_passwd)) != 0)) {
#ifdef SULOG
		log(SULOG, nptr, 0);	/*log entry*/
#endif
		fprintf(stderr,"su: Sorry\n");
		exit(2);
	}
ok:
	endpwent();	/*close password file*/
#ifdef SULOG
		log(SULOG, nptr, 1);	/*log entry*/
#endif

	/*set user and group ids to specified user*/
	if((setgid(gid) != 0) || (setuid(uid) != 0)) {
		printf("su: Invalid ID\n");
		exit(2);
	}

	/*set environment variables for new user;
	  arg0 for exec of shprog must now contain `-'
	  so that environment of new user is given
	*/
	if (eflag) {
		strcat(homedir, dir);
		strcat(logname, name);
		envinit[2] = logname;
		chdir(dir);
		envinit[0] = homedir;
		if (uid == 0)
			envinit[1] = supath;
		else	envinit[1] = path;
		envinit[3] = NULL;
		environ = envinit;
		strcpy(su, "-su");
	}

	/*if new user is root:
		if CONSOLE defined, log entry there;
		if eflag not set, change environment to that of root.
	*/
	if (uid == 0)
	{
#ifdef CONSOLE
		if(strcmp(ttyn, CONSOLE) != 0) {
			signal(SIGALRM, to);
			alarm(30);
			log(CONSOLE, nptr, 1);
			alarm(0);
		}
#endif
		if (!eflag) envalt();
	}

	/*if new user's shell field is not NULL or equal to /bin/sh,
	  set:
		pshell = their shell
		su = [-]last component of shell's pathname
	*/
	if (shprog[0] != '\0' && (strcmp(shell,shprog) != 0) ) {
		pshell = shprog;
		strcpy(su, eflag ? "-" : "" );
		strcat(su, strrchr(pshell,'/') + 1);
	}

	/*if additional arguments, exec shell program with array
	    of pointers to arguments:
		-> if shell = /bin/sh, then su = [-]su
		-> if shell != /bin/sh, then su = [-]last component of
						     shell's pathname

	  if no additional arguments, exec shell with arg0 of su
	    where:
		-> if shell = /bin/sh, then su = [-]su
		-> if shell != /bin/sh, then su = [-]last component of
						     shell's pathname
	*/
	if (argc > 2) {
		argv[1] = su;
		execv(pshell, &argv[1]);
	} else {
		execl(pshell, su, 0);
	}

	fprintf(stderr,"su: No shell\n");
	exit(3);
}

/*Environment altering routine -
	This routine is called when a user is su'ing to root
	without specifying the - flag.
	The user's PATH and PS1 variables are reset
	to the correct value for root.
	All of the user's other environment variables retain
	their current values after the su (if they are
	exported).
*/
envalt()
{
	int putenv();

	/*If user has PATH variable in their environment, change its value
			to /bin:/etc:/usr/bin ;
	  if user does not have PATH variable, add it to the user's
			environment;
	  if either of the above fail, an error message is printed.
	*/
	if ( ( putenv(supath) ) != 0 ) {
		printf("su: unable to obtain memory to expand environment");
		exit(4);
	}

	/*If user has PROMPT variable in their environment, change its value
			to # ;
	  if user does not have PROMPT variable, add it to the user's
			environment;
	  if either of the above fail, an error message is printed.
	*/
	if ( ( putenv(suprmt) ) != 0 ) {
		printf("su: unable to obtain memory to expand environment");
		exit(4);
	}

	return;

}

/*Logging routine -
	where = SULOG or CONSOLE
	towho = specified user ( user being su'ed to )
	how = 0 if su attempt failed; 1 if su attempt succeeded
*/
log(where, towho, how)
char *where, *towho;
int how;
{
	FILE *logf;
	long now;
	struct tm *tmp;

	now = time(0);
	tmp = localtime(&now);

	/*open SULOG or CONSOLE -
		if open fails, return
	*/
	if ((logf=fopen(where,"a")) == NULL) return;

	/*write entry into SULOG or onto CONSOLE -
		 if write fails, return
	*/
	if ((fprintf(logf,"SU %.2d/%.2d %.2d:%.2d %c %s %s-%s\n",
		tmp->tm_mon+1,tmp->tm_mday,tmp->tm_hour,tmp->tm_min,
		how?'+':'-',(strrchr(ttyn,'/')+1),
		cuserid((char *)0),towho)) < 0)
	{
		fclose(logf);
		return;
	}

	fclose(logf);	/*close SULOG or CONSOLE*/

	return;
}

void to(){}
