/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)fumount:sndmes.c	1.8"

#include <stdio.h>
#include <sys/types.h>
#include <sys/message.h>
#include <sys/rfsys.h>
#include <sys/nserve.h>

sndmes(sysid, time, resrc)
sysid_t sysid;
char *time;
char *resrc;
{
	static char msg[200];

		/* 
		   send a warning message to a remote system.

		   syntax of message:	fuwarn domain.resource [time] 
		*/
	strcpy(msg, "fuwarn ");
	strncat(msg, resrc, sizeof(msg) - strlen(msg));
	strncat(msg, " ", sizeof(msg) - strlen(msg));
	strncat(msg, time, sizeof(msg) - strlen(msg));
	return(rfsys(RF_SENDUMSG, sysid, msg, strlen(msg)));
}
