/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ckauto:ckauto.c	1.3"
/*
 *		Copyright 1984 AT&T
 */
#include <stdio.h>
#include <errno.h>
#include <sys/sys3b.h>


main(argc,argv)
int	argc;
char	*argv[];
{

	int	runflg;
	extern int errno;

	runflg = sys3b( S3BAUTO );
	if((runflg < 0) && (errno == EPERM)) {
		printf("ckauto: User must be super user\n");
		return(-1);
	}

	exit( runflg );
}
