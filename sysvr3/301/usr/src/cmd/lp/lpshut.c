/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident "@(#)lp:lpshut.c	3.3"
/* lpshut -- shut the line printer scheduler
	All busy printers will stop printing,
	but no requests will be cancelled because of this.
*/

#include	"lp.h"


main(argc, argv)
int argc;
char *argv[];
{
	extern char *f_name;

	f_name = argv[0];

	if(! ISADMIN)
		fatal(ADMINMSG, 1);

	if(chdir(SPOOL) == -1)
		fatal("spool directory non-existent", 1);
	if(enqueue(F_QUIT, "")) {
		/* Scheduler is running */
                printf("Line printer scheduler stopped\n");
		exit(0);
	}
	else {
		/* Scheduler is not running -- remove the FIFO and SCHEDLOCK */
		unlink(FIFO);
		unlink(SCHEDLOCK);
		fatal("scheduler not running", 1);
	}
}
