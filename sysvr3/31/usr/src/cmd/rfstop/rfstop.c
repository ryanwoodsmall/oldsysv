/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)rfstop:rfstop.c	1.11.1.1"
#include <stdio.h>
#include <nserve.h>
#include <time.h>
#include <sys/rfsys.h>
#include <errno.h>
#include <sys/signal.h>

#define ERROR(str)	fprintf(stderr, "%s: %s\n", argv[0], str)

extern int errno;

main( argc, argv )
int   argc;
char *argv[];
{
	char cmd[512];

	if (argc != 1) {
		ERROR("extra arguments given");
		ERROR("usage: rfstop");
		exit(1);
	}

	if (geteuid() != 0) {
		ERROR("must be super-user");
		exit(1);
	}

	sigset(SIGHUP,  SIG_IGN);
	sigset(SIGINT,  SIG_IGN);
	sigset(SIGQUIT, SIG_IGN);

	/*
	 *	Issue the rfstop(2) system call to stop all 
	 *	kernel functions of Distributed UNIX.
	 */

	if (rfstop() < 0) {
		if (errno == EBUSY)
			ERROR("remote resources currently mounted");
		else if (errno == ESRMNT)
			ERROR("remote clients are using local resources");
		else if (errno == EADV)
			ERROR("resources are still advertised");
		else if (errno == ENONET) {
			ERROR("RFS is not running");
			exit(1);
		} else
			perror(argv[0]);
		ERROR("cannot stop RFS");
		exit(1);
	}

	/*
	 *	Execute the shell script to stop the name server
	 *	process and have the name server relinquish primary
	 *	responsibilities, if necessary.
	 */

	if (system("rfadmin -p >/dev/null 2>&1") == 0x200)
		ERROR("warning: no secondary name servers active");

	sprintf(cmd, "kill `cat %s 2>/dev/null` 2>/dev/null", NSPID);

	if (system(cmd) != 0) {
		ERROR("error in killing name server");
		rfsys(RF_LASTUMSG);
		exit(1);
	}

	/*
	 *	Kill the user-level daemon that accepts masseges from
	 *	other systems by sending a "last message' signal.
	 */

	rfsys(RF_LASTUMSG);

	exit(0);
}
