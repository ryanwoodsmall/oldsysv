/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)xt:xts.c	2.4"

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
**	Print out Xt driver statistics
*/

char		Usage[]	=	"Usage: %s [-f]\n";

#include	"stdio.h"
#include	"sys/jioctl.h"

char *		name;
int		display;

extern int	xtstats();

extern char	_sobuf[];

#define		Fprintf		(void)fprintf
#define		SYSERROR	(-1)

int
main(argc, argv)
	int		argc;
	char *		argv[];
{
	name = *argv;

	if (argc != 1)
		if (!(argc == 2 && (strcmp("-f", argv[1]) == 0))) {
			Fprintf(stderr, Usage, name);
			exit(1);
		} else
			 ++display;

	if (ioctl(0, JMPX, 0) == -1){
		Fprintf(stderr, "%s: Must be invoked with stdin attached to an xt channel.\n", name);
		exit(1);
	}

	setbuf(stdout, _sobuf);
	if ( xtstats(name, 0, stdout) )
		exit(1);
	if ( display )
		Fprintf(stdout, "\f");

	exit(0);
}
