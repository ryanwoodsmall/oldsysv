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
/*	@(#)tty.c	1.2	*/

/*
** Type tty name
*/

#include	"stdio.h"
#include	"sys/stermio.h"

char	*ttyname();

extern int	optind;
int		lflg;
int		sflg;

main(argc, argv)
char **argv;
{
	register char *p;
	register int	i;

	while((i = getopt(argc, argv, "ls")) != EOF)
		switch(i) {
		case 'l':
			lflg = 1;
			break;
		case 's':
			sflg = 1;
			break;
		case '?':
			printf("Usage: tty [-l] [-s]\n");
			exit(2);
		}
	p = ttyname(0);
	if(!sflg)
		printf("%s\n", (p? p: "not a tty"));
	if(lflg) {
		if((i = ioctl(0, STWLINE, 0)) == -1)
			printf("not on an active synchronous line\n");
		else
			printf("synchronous line %d\n", i);
	}
	exit(p? 0: 1);
}
