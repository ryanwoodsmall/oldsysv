/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libpt:pt_chmod.c	1.1"
/*
 *	         Copyright (c) 1987 AT&T
 *		   All Rights Reserved
 *
 *     THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
 *   The copyright notice above does not evidence any actual
 *   or intended publication of such source code.
 *
 ================================================================
 =   WARNING!!!! This source is not supported in future source  =
 =   releases.                                                  =
 ================================================================
 */


/*
 * change the owner and mode of the pseudo terminal slave device.
 */
main(argc, argv)
int	argc;
char	**argv;
{
	int	fd;

	fd = atoi (argv[1]);
	if(chown(ptsname(fd), getuid(), getgid()))
		exit(-1);
	if(chmod(ptsname(fd), 00600))
		exit(-1);
	exit(0);
}

