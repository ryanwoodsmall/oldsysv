/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

# ident	"@(#)sadmin:admin/shade.c	1.2"

# define	SYSADM	"sysadm"

# include	<string.h>
# include	<sys/types.h>
# include	<stdio.h>
# include	<pwd.h>

struct	passwd	*getpwnam();
struct	passwd	*userp;

char	Lname[BUFSIZ];
char	*lnamep = &Lname[0];
char	Command[BUFSIZ];
char	*commandp = &Command[0];
char	Home[BUFSIZ];
char	*homep = &Home[0];
char	Aptr[BUFSIZ];
char	*ap = &Aptr[0];

/*
	The following three assignments are needed for the sysadm
	commands.  They are used in envalt().
*/

char	pt[BUFSIZ] = "PATH=/bin:/usr/bin:/usr/lbin";
char	mt[BUFSIZ] = "MENUTOP=/usr/admin";
char	sh[BUFSIZ] = "SHELL=/bin/sh";

char	*sname();
char	*getenv();

main(argc,argv)
int	argc;
char	*argv[];
{
	char	*nlnamep;

	/*
	  Determine who is invoking the shell for the administrative
	  environment (shade) by checking the simple name of argv[0].
	*/

	lnamep = sname(argv[0]);
	if (*lnamep == '-') {
		++lnamep;
		nlnamep = lnamep;
	}
	else nlnamep = lnamep;

	/*
	  Based on the simple name of argv[0], get information
	  about the user from the /etc/passwd file.
	*/

	if ((userp = getpwnam(nlnamep)) == NULL) {
		fprintf(stderr,"Cannot find `%s' in password file\n",
			nlnamep);
		exit(1);
	}

	/*
	  If the name from the password file is "sysadm" check
	  to see if an argument was supplied.  If so, pass it
	  since this is valid for the "sysadm" function.
	*/

	if ((strcmp(userp->pw_name,SYSADM) == 0)) {
		if (getppid() == 1)		/* called from login level */
			ap = getenv("L0");
		else if (argv[1])		/* called from shell level */
			strcat(ap,argv[1]);
	}

	/*
	  If the user's effective uid and the uid from /etc/passwd
	  don't match, exec "su -" with name from password file.  This
	  will handle the case when called by user at shell level.
	*/

	if (geteuid() != userp->pw_uid) {
		if (*ap)
			execl("/bin/su", "/bin/su", "-", userp->pw_name, ap, 0);
		else
			execl("/bin/su", "/bin/su", "-", userp->pw_name, 0);
	}

	/*
	 * If user has PATH, SHELL, MENUTOP, or HOME variable in their
	 * environment, change its value. If not, add it to the user's
	 * environment.  If either of the above fail, an error message
	 * printed.
	 */

	envalt();

	/*
	   Build the command string that "exec" will execute 
	   when called.
	*/

	sprintf(commandp,"%s/%s",userp->pw_dir,userp->pw_name);

	/*
	   change current working directory to the simple admin
	   home directory in order for certain commands to work
	*/

	if (chdir(userp->pw_dir) != 0) {
		fprintf(stderr,"Unable to change directory to \"%s\"\n",
			userp->pw_dir);
		exit(1);
	}

	if (*ap)
		execl("/bin/sh", "/bin/sh", commandp, ap, 0);
	else
		execl("/bin/sh", "/bin/sh", commandp, 0);
}


envalt()
{
	int putenv();

	/*
	   setup "home" variable for use later.
	*/
	sprintf(homep,"HOME=%s",userp->pw_dir);

	if ((putenv(pt)) != 0)
		env_err();

	if ((putenv(sh)) != 0)
		env_err();

	if ((putenv(mt)) != 0)
		env_err();

	if ((putenv(homep)) != 0)
		env_err();
	return;
}


char	*sname(strp)
char *strp;
{
	register char *p;
	register int n;
	register int j;

	n = strlen(strp);
	--n;
	if (strp[n] == '/') {
		for (j=n; j >= 0; --j)
			if (strp[j] != '/') {
				strp[++j] = '\0';
				break;
			}
	}

	for(p=strp; *p; p++)
		if(*p == '/') strp = p + 1;
	return(strp);
}


env_err()
{
	fprintf(stderr,"unable to obtain memory to expand environment\n");
		exit(1);
}
