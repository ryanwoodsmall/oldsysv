/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)chown:chown.c	1.3"
/*
 * chown uid file ...
 */

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>

struct	passwd	*pwd,*getpwnam();
struct	stat	stbuf;
ushort	uid;
int	status;

main(argc, argv)
char *argv[];
{
	register c;

	if(argc < 3) {
		fprintf(stderr, "usage: chown uid file ...\n");
		exit(4);
	}
	if(isnumber(argv[1])) {
		uid = atoushort(argv[1]); /* uid is unsigned short */
		goto cho;
	}
	if((pwd=getpwnam(argv[1])) == NULL) {
		fprintf(stderr, "chown: unknown user id %s\n",argv[1]);
		exit(4);
	}
	uid = pwd->pw_uid;

cho:
	for(c=2; c<argc; c++) {
		stat(argv[c], &stbuf);
		if(chown(argv[c], uid, stbuf.st_gid) < 0) {
			perror(argv[c]);
			status = 1;
		}
	}
	exit(status);
}

isnumber(s)
char *s;
{
	register c;

	while(c = *s++)
		if(!isdigit(c))
			return(0);
	return(1);
}

atoushort(s)
register char *s;
{
	register char c;
	ushort register i, previ;
	i = previ = 0;
	while(c = *s++) {
		i = c - '0' + 10 * i;
		if(i < previ) {
			fprintf(stderr,"chown: numeric user id too large\n");
			exit(4);
    		}
		previ = i;
	}
	return(i);
}
