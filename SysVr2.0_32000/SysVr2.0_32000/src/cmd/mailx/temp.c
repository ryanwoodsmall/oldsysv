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
#

#include "rcv.h"

/*
 * mailx -- a modified version of a University of California at Berkeley
 *	mail program
 *
 * Give names to all the temporary files that we will need.
 */

/* static char *SccsId = "@(#)temp.c	1.2"; */

char	tempMail[14];
char	tempQuit[14];
char	tempEdit[14];
char	tempSet[14];
char	tempResid[14];
char	tempMesg[14];
char	tempZedit[14];

tinit()
{
	register char *cp, *cp2;
	char uname[PATHSIZE];
	register int err = 0;
	register int pid;
	char tbuf[PATHSIZE];

	pid = getpid();
	sprintf(tempMail, "/tmp/Rs%-d", pid);
	sprintf(tempResid, "/tmp/Rq%-d", pid);
	sprintf(tempQuit, "/tmp/Rm%-d", pid);
	sprintf(tempEdit, "/tmp/Re%-d", pid);
	sprintf(tempSet, "/tmp/Rx%-d", pid);
	sprintf(tempMesg, "/tmp/Rx%-d", pid);
	sprintf(tempZedit, "/tmp/Rz%-d", pid);

	if (strlen(myname) != 0) {
		uid = getuserid(myname);
		if (uid == -1) {
			printf("\"%s\" is not a user of this system\n",
			    myname);
			exit(1);
		}
	}
	else {
		uid = getuid() & UIDMASK;
		if (username(uid, uname) < 0) {
			copy("ubluit", myname);
			err++;
			if (rcvmode) {
				printf("Who are you!?\n");
				exit(1);
			}
		}
		else
			copy(uname, myname);
	}
	strcpy(homedir, Getf("HOME"));
	findmail();
	assign("MBOX", Getf("MBOX"));
	assign("MAILRC", Getf("MAILRC"));
	assign("DEAD", Getf("DEAD"));
	assign("save", "");
	assign("asksub", "");
	assign("header", "");
}
