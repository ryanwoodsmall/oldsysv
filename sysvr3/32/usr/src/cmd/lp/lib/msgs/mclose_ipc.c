/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/msgs/mclose_ipc.c	1.1"

#include "errno.h"
#include "sys/types.h"
#include "sys/ipc.h"
#include "sys/msg.h"

extern int		Sched_id,
			My_id;

/**
 ** mclose() - CLOSE A MESSAGE PATH THAT USES IPC
 **/

int			mclose ()
{
	char			*msgbuf;

	int			size;


	if (Sched_id == -1) {
		errno = ENXIO;
		return (-1);
	}

	if (msgctl(My_id, IPC_RMID, (struct msqid_ds *)0) == -1) { 
		if (errno == EINVAL)
			errno = EIDRM;
		Sched_id = -1;
		return (-1);
	}

	(void)nudge_sched ();
	Sched_id = -1;

	return (0);
}
