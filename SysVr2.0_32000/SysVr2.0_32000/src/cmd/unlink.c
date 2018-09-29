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
/*	@(#)unlink.c	1.1	*/
main(argc, argv) char *argv[]; {
	if(argc!=2) {
		write(2, "Usage: /etc/unlink name\n", 24);
		exit(1);
	}
	unlink(argv[1]);
	exit(0);
}
