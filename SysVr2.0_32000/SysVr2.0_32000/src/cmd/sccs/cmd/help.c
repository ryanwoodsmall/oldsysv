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
#include	<stdio.h>
#include	<sys/types.h>
#include	<macros.h>

SCCSID(@(#)help.c	1.1);

char	Ohelpcmd[]   =   "/usr/lib/help/lib/help2";
extern	int	errno;

main(argc,argv)
int argc;
char *argv[];
{
	execv(Ohelpcmd,argv);
	fprintf(stderr,"help: Could not exec: %s.  Errno=%d\n",Ohelpcmd,errno);
	exit(1);
}
