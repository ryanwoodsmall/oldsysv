/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.bk:src/bk/background.c	1.2"

#include	<stdio.h>
#include	<errno.h>
#include	<signal.h>
#include	"wish.h"
#include	"vtdefs.h"
#include	"terror.h"
#include	"retcodes.h"

extern char	*Home;
extern char	*Filecabinet;
extern char	*Wastebasket;
extern char	*Filesys;
extern char 	*Oasys;
extern char	*Progname;

static int
sig_catch(n)
int	n;
{
	signal(n, sig_catch);
}

void
wish_init(argc, argv)
int	argc;
char	*argv[];
{
	static char	filecabinet[] = "/FILECABINET";
	static char	wastebasket[] = "/WASTEBASKET";
	char	*p;
	int	exit();
	char	*filename();
	char	*getenv();
	char	*strnsave();
	long	strtol();

	if (p = getenv("DEBUG"))
		_Debug = (int) strtol(p, 0, 0);
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, exit);
	if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
		signal(SIGHUP, exit);
	if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
		signal(SIGTERM, exit);
	(void) signal(SIGALRM, sig_catch);
	if ((Home = getenv("HOME")) == NULL)
		fatal(MUNGED, "everything!");
	if ((Filesys = getenv("VMSYS")) == NULL)
		fatal(MUNGED, "VMSYS");
	if ((Oasys = getenv("OASYS")) == NULL)
		fatal(MUNGED, "OASYS");
	Progname = filename(argv[0]);
	Filecabinet = strnsave(Home, strlen(Home) + sizeof(filecabinet) - 1);
	strcat(Filecabinet, filecabinet);
	Wastebasket = strnsave(Home, strlen(Home) + sizeof(wastebasket) - 1);
	strcat(Wastebasket, wastebasket);
}

void
mess_temp(s)
char	*s;
{
	fprintf(stderr, "%s\n", s);
}

void
mess_perm(s)
char	*s;
{
	mess_temp(s);
}

void
mess_flash(s)
char	*s;
{
	mess_temp(s);
}

void
working()
{
}
