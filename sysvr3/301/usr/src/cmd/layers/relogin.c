/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)xt:relogin.c	2.9"

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
**	relogin [-] line
**
**	Find 'line' in /etc/utmp and change it to 'ttyname'.
*/

char		Usage[]		= "usage: relogin [-s] [line]\n";

#include	<sys/types.h>
#include	<utmp.h>
#include	<stdio.h>
#include	<pwd.h>

struct utmp	utmp;
char		toolong[]	= "name '%s' too long for utmp field";
char		nulls[sizeof utmp.ut_line];
char *		name;
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
	int		argc;
	char *		argv[];
{
	register char *	line;
	register int	linelen;
	register char *	mytty;
	register int	myttylen;
	register FILE *	ufd;
	register Pwp	pwp;

	name = argv[0];

	if (argc > 1 && argv[1][0] == '-')
		if ( strcmp("-",argv[1])==0 || strcmp("-s",argv[1])==0) {
			silent++;
			argv++;
			argc--;
		} else {
			fprintf(stderr, Usage);
			exit(1);
		}

	if ( argc > 2 )
	{
		(void)fprintf(stderr, Usage);
		exit(1);
	}

	if (argc == 2) {
		line = argv[1];
		if ( strncmp(line, devdir, sizeof devdir - 1) == 0 )
			line += sizeof devdir - 1;
		linelen = strlen(line);

		if ( linelen > sizeof utmp.ut_line )
		{
			error(toolong, line);
			exit(1);
		}
	}

	if ( (mytty = ttyname(0)) == (char *)0 )
	{
		error("stdin not a tty");
		exit(1);
	}
	mytty += sizeof devdir - 1;
	myttylen = strlen(mytty);

	if ( myttylen > sizeof utmp.ut_line )
	{
		error(toolong, mytty);
		exit(1);
	}
	/* undocumented feature "relogin line", used only by the layers program */
	if (argc == 2) {
		if ( strncmp(mytty,"xt",2) != 0 && strncmp(line,"xt",2) != 0 ) {
			error("must be invoked under layers");
			exit(1);
		}
		if ( linelen == myttylen && strncmp(line, mytty, linelen) == 0 )
			exit(0);
	}

	if (argc == 1) {
		if ( strncmp(mytty,"xt",2) != 0 ) {
			error("must be invoked under layers");
			exit(1);
		}
		line = mytty;
		linelen = myttylen - 1;
	}

	if ( (pwp = getpwuid(getuid())) == (Pwp)0 )
	{
		error("who are you?");
		exit(1);
	}

	endpwent();

	if ( (ufd = fopen(utmpfn, "r+")) != NULL )
	{
		while ( fread((char *)&utmp, sizeof utmp, 1, ufd) == 1 )
		if ( strncmp(utmp.ut_line, line, linelen) == 0 )
		{
			if ( argc == 1 && strncmp(utmp.ut_line, mytty, myttylen) == 0 )
				exit(0);
			if ( pwp->pw_uid && strncmp(utmp.ut_user, pwp->pw_name, sizeof utmp.ut_user) != 0 )
			{
				error("'%s' has '%.*s', not '%s', logged in"
					,line, sizeof utmp.ut_user, utmp.ut_user, pwp->pw_name);
				exit(1);
			}
			(void)strncpy(utmp.ut_line, nulls, sizeof utmp.ut_line);
			(void)strncpy(utmp.ut_line, mytty, myttylen);
			(void)fseek(ufd, -(sizeof utmp), 1);
			if ( fwrite((char *)&utmp, sizeof utmp, 1, ufd) != 1 )
			{
				error("cannot write '%s'", utmpfn);
				exit(1);
			}
			exit(0);
		}
	}
	else
	{
		error("cannot open '%s' for writing", utmpfn);
		exit(1);
	}

	error("cannot find '%s' in %s", line, utmpfn);
	exit(1);
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
	if ( silent )
		return;
	(void)fprintf(stderr, "%s: error - ", name);
	(void)fprintf(stderr, s1, s2, s3, s4, s5);
	(void)fprintf(stderr, "\n");
}
