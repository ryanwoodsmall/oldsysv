/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)uadmin:uadmin.c	1.2"
#include <sys/uadmin.h>

main(argc, argv)
char *argv[];
{
	register cmd, fcn;

	cmd = atoi(argv[1]);
	fcn = atoi(argv[2]);
	uadmin(cmd, fcn, 0);
	perror("uadmin");
}
