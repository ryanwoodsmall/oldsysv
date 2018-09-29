/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/msgs/msend_ipc.c	1.2"

/*LINTLIBRARY*/

#include "stdio.h"
#include "string.h"
#include "errno.h"
#include "sys/types.h"
#include "sys/ipc.h"
#include "sys/msg.h"

#include "msgs.h"


extern int		Sched_id,
			My_id;

extern long		Code;

extern char		*malloc();

extern void		free();

/**
 ** msend() - SEND MESSAGE VIA IPC
 **/

int			msend (msgbuf)
	char			*msgbuf;
{
	int			size;

	short			type;

	struct msgbuf		*buf;


	if (Sched_id == -1) {
		errno = ENXIO;
		return (-1);
	}

	if (
		!msgbuf
	     || (size = stoh(msgbuf + HEAD_SIZE)) < CONTROL_LEN
	     || (type = stoh(msgbuf + HEAD_TYPE)) < 0
	     || LAST_UMESSAGE < type
	) {
		errno = EINVAL;
		return (-1);
	}

	if (!(buf = (struct msgbuf *)malloc(BASE_MSG_SIZE(size)))) {
		errno = ENOMEM;
		return (-1);
	}

	buf->mtype = 1;
	(void)memcpy (buf->mtext, msgbuf, size);

	if (msgsnd(My_id, buf, size, 0) == -1) {
		free (buf);
		if (errno == EIDRM) {
			(void)mclose ();
			errno = EIDRM;
		}
		return (-1);
	}
	free (buf);

	if (nudge_sched() == -1)
		return (-1);

	return (0);
}
