/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)chgrp:chgrp.c	1.4"
/*
 * chgrp gid file ...
 */

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <grp.h>

struct	group	*gr,*getgrnam();
struct	stat	stbuf;
ushort	gid;
int	status;

main(argc, argv)
char *argv[];
{
	register c;
	ushort atoushort() ;

	if(argc < 3) {
		fprintf(stderr,"chgrp: usage: chgrp gid file ...\n");
		exit(4);
	}
	if(isnumber(argv[1])) {
		gid = atoushort(argv[1]); /* gid is unsigned short */
	} else {
		if((gr=getgrnam(argv[1])) == NULL) {
			fprintf(stderr,"chgrp: unknown group: %s\n",argv[1]);
			exit(4);
		}
		gid = gr->gr_gid;
	}
	for(c=2; c<argc; c++) {
		stat(argv[c], &stbuf);
		if(chown(argv[c], stbuf.st_uid, gid) < 0) {
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

ushort
atoushort(s)
register char *s;
{
	register char c;
	register int i = 0 ;
	register ushort maxushort = ((ushort) ~0) ;

	while(c = *s++) {
		i = c - '0' + 10 * i;
		if(i > maxushort) {
			fprintf(stderr,"chgrp: numeric group id too large\n");
			exit(4);
   		}
	}
	return (ushort) i ;
}
