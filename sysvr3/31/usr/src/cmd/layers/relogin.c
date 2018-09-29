/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)xt:relogin.c	2.10"

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
**	relogin [-s] line
**
**	Find 'line' in /etc/utmp and change it to 'ttyname' of stdin.
*/

char		Usage[]		= "usage: relogin [-s] [line]\n";

#include	<sys/types.h>
#include	<utmp.h>
#include	<stdio.h>
#include	<pwd.h>

struct utmp	utmp;
char		toolong[]	= "name '%s' too long for utmp field";
char		nulls[sizeof utmp.ut_line];
char *	name;
char		utmpfn[]	= UTMP_FILE;
char		silent;
char		devdir[]	= "/dev/";

typedef struct passwd *	Pwp;

void		error();

extern void	endpwent();
extern Pwp	getpwuid();
extern int	getuid();
extern int	strlen();
extern int	strncmp();
extern char *	strncpy();
extern char *	ttyname();


main(argc, argv)
int   argc;
char *argv[];
{
	register char *new_line;
	register int   new_llen;
	register char *utmp_line;
	register int   utmp_llen;
	register int   retval;
	register FILE *ufd;
	register Pwp   pwp;

	name = argv[0];

	while ((argc > 1) && (argv[1][0] == '-'))
		if ( strcmp("-",argv[1])==0 || strcmp("-s",argv[1])==0) {
			silent++;
			argv++;
			argc--;
		} else {
			fprintf(stderr, Usage);
			exit(1);
		}

	if ( argc > 2 ) {
		(void)fprintf(stderr, Usage);
		exit(1);
	}

	if ( (new_line = ttyname(0)) == (char *)0 )
		error("stdin not a tty");

	new_line += sizeof devdir - 1;
	if ((new_llen = strlen(new_line)) > sizeof(utmp.ut_line))
		error(toolong, new_llen);

	if (argc == 2) {
		/*
		** 	The user has specified which utmp line entry
		**	to change.
		*/
		utmp_line = argv[1];

		if ( strncmp(utmp_line, devdir, sizeof devdir - 1) == 0 )
			utmp_line += sizeof devdir - 1;

		if ((utmp_llen = strlen(utmp_line)) > sizeof(utmp.ut_line))
			error(toolong, utmp_line);
	} else {
		/*
		** 	The user has not specified which utmp line entry
		**	to change. We subtract 1 from new_llen so xt--?
		**	identifies the user's entry in /etc/utmp where
		**	"--" is the user's set of xt channels.
		*/
		utmp_line = new_line;
		utmp_llen = new_llen - 1;
	}

	if ( (pwp = getpwuid(getuid())) == (Pwp)0 )
		error("can't identify your login id");

	endpwent();

	if ( (ufd = fopen(utmpfn, "r+")) == NULL )
		error("cannot open '%s'", utmpfn);

	while ( fread((char *)&utmp, sizeof utmp, 1, ufd) == 1 )
		if ((strncmp(utmp.ut_line, utmp_line, utmp_llen) == 0) &&
		    (strncmp(utmp.ut_user, pwp->pw_name, sizeof utmp.ut_user) == 0 )) {
			(void)strncpy(utmp.ut_line, nulls, sizeof utmp.ut_line);
			(void)strncpy(utmp.ut_line, new_line, new_llen);
			(void)fseek(ufd, -(sizeof utmp), 1);
			if ( fwrite((char *)&utmp, sizeof utmp, 1, ufd) != 1 )
				error("cannot write '%s'", utmpfn);
			exit(0);
		}

	error("cannot find '%s' logged in on '%s' in '%s'",
	      utmp.ut_user, utmp_line, utmpfn);
}



/*VARARGS1*/
void
error(s1, s2, s3, s4, s5)
	char *	s1;
	char *	s2;
	char *	s3;
	char *	s4;
	char *	s5;
{
	if ( ! silent ) {
		(void)fprintf(stderr, "%s: error - ", name);
		(void)fprintf(stderr, s1, s2, s3, s4, s5);
		(void)fprintf(stderr, ".\n");
	}

	exit(1);
}
