/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/msgs/mrecv_ipc.c	1.2"

/*LINTLIBRARY*/

#include "stdio.h"
#include "errno.h"
#include "string.h"
#include "memory.h"
#include "sys/types.h"
#include "sys/ipc.h"
#include "sys/msg.h"

#include "msgs.h"

extern int		Sched_id,
			My_id;

extern long		Code;

extern void		free();

extern char		*malloc();

/**
 ** mrecv() - RECEIVE A MESSAGE VIA IPC
 **/

int			mrecv (msgbuf, size)
	char			*msgbuf;
	int			size;
{
	short			   type;

	struct msgbuf		*buf;

	if (Sched_id == -1) {
		errno = ENXIO;
		return (-1);
	}

	if (!msgbuf || size <= 0) {
		errno = EINVAL;
		return (-1);
	}

	if (!(buf = (struct msgbuf *)malloc(BASE_MSG_SIZE(size)))) {
		errno = ENOMEM;
		return (-1);
	}

	if ((size = msgrcv(My_id, buf, (int) BASE_MSG_SIZE(size), 3, 0)) == -1) {
		free (buf);
		if (errno == EIDRM) {
			(void)mclose ();
			errno = EIDRM;
		}
		return (-1);
	}

	if (
		size < CONTROL_LEN
	     || (type = stoh(buf->mtext + HEAD_TYPE)) < 0
	     || type > LAST_UMESSAGE
	) {
		errno = ENOMSG;
		return (-1);
	}

	(void)memcpy (msgbuf, buf->mtext, size);
	free (buf);

	return (type);
}
