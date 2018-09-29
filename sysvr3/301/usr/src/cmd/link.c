/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)link:link.c	1.2"
main(argc, argv) char *argv[]; {
	if(argc!=3) {
		write(2, "Usage: /etc/link from to\n", 25);
		exit(1);
	}
	exit((link(argv[1], argv[2])==0)? 0: 2);
}
