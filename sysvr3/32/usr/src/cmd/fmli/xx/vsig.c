/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.xx:src/xx/vsig.c	1.3"

/* 
 * This short program is called by a co-process if it wishes to 
 * communicate asynchronously with the controlling FMLI process during
 * the course of its execution.  It blocks til FMLI is ready for a
 * signal then sends a SIGUSR2. 
 */

#include <stdio.h>
#include <signal.h>
#include <fcntl.h>

#define PATHSIZ 128

char Semaphore[PATHSIZ] = "/tmp/FMLIsem.";

main(argc, argv)
int argc;
char *argv[];
{
	FILE *fp;
	char name[PATHSIZ];
	char *vpid;
	char *getenv();

	if ((vpid = getenv("VPID")) == NULL)
		exit(1);
	strcat(Semaphore, vpid);
	fflush(stdout);
	fflush(stderr);
	/*
	 * The reason for the close(open) is to
	 * block until FACE says its is OK
	 * to send a signal ... A signal when
	 * FACE is updating the screen
	 * can create garbage .....
	 */
	close(open(Semaphore, O_WRONLY));
	kill(atoi(vpid), SIGUSR2);
	exit(0);
}
