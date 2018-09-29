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
/*	@(#)mkdir.c	1.1	*/
/*
** make directory
*/

#include	<signal.h>
#include	<stdio.h>

int	Errors = 0;
char	*strcat();
char	*strcpy();

main(argc, argv)
char *argv[];
{

	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	if(argc < 2) {
		fprintf(stderr, "mkdir: arg count\n");
		exit(2);
	}
	while(--argc)
		mkdir(*++argv);
	exit(Errors? 2: 0);
}

mkdir(d)
char *d;
{
	char pname[128], dname[128];
	register i, slash = 0;

	pname[0] = '\0';
	for(i = 0; d[i]; ++i)
		if(d[i] == '/')
			slash = i + 1;
	if(slash)
		strncpy(pname, d, slash);
	strcpy(pname+slash, ".");
	if (access(pname, 02)) {
		fprintf(stderr,"mkdir: cannot access %s\n", pname);
		++Errors;
		return;
	}
	if ((mknod(d, 040777, 0)) < 0) {
		fprintf(stderr,"mkdir: cannot make directory %s\n", d);
		++Errors;
		return;
	}
	chown(d, getuid(), getgid());
	strcpy(dname, d);
	strcat(dname, "/.");
	if((link(d, dname)) < 0) {
		fprintf(stderr, "mkdir: cannot link %s\n", dname);
		unlink(d);
		++Errors;
		return;
	}
	strcat(dname, ".");
	if((link(pname, dname)) < 0) {
		fprintf(stderr, "mkdir: cannot link %s\n",dname);
		dname[strlen(dname)] = '\0';
		unlink(dname);
		unlink(d);
		++Errors;
	}
}
