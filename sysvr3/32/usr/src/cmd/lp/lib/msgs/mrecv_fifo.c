/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/msgs/mrecv_fifo.c	1.3"

/*LINTLIBRARY*/

#include <errno.h>
#include <memory.h>
#include <fcntl.h>

#include "lp.h"
#include "msgs.h"

extern int		Sched_fifo,
			My_fifo;

extern char		*My_fifo_path;

extern char		AuthCode[];

/*
** mrecv() - RECEIVE A MESSAGE VIA FIFOS
*/

int
mrecv (msgbuf, size)
char	*msgbuf;
int	size;
{
    short	type;


    if (Sched_fifo == -1)
    {
	errno = ENXIO;
	return (-1);
    }

    if (!msgbuf)
    {
	errno = EINVAL;
	return (-1);
    }

    if (size < CONTROL_LEN)
    {
	errno = E2BIG;
	return (-1);
    }

    (void) Close (Open(My_fifo_path, O_RDONLY));

RdLoop:
    switch (read_fifo(My_fifo, msgbuf, size))
    {
	case 0:
	    /*
	    ** The fifo was empty and we have O_NDELAY set,
	    ** or the Spooler closed our FIFO.
	    ** We don't set O_NDELAY in the user process,
	    ** so that should never happen. But be warned
	    ** that we can't tell the difference in some versions
	    ** of the UNIX op. sys.!!
	    **
	    ** The closing of the FIFO is akin to the removal
	    ** of an IPC message during a "msgrcv", so we return
	    ** the same "errno".
	    **
	    */

	    errno = EIDRM;
	    return (-1);

	case -1:
            /*
	    ** We restart interrupted reads.
	    */
            if (errno == EINTR)
		goto RdLoop;
	    return (-1);
    }


    if ((type = stoh(msgbuf + HEAD_TYPE)) < 0 || LAST_IMESSAGE < type)
    {
	errno = ENOMSG;
	return (-1);
    }
    if (type == I_QUEUE_CHK)
	goto RdLoop;

    (void)memcpy (AuthCode, msgbuf + HEAD_AUTHCODE, HEAD_AUTHCODE_LEN);

    return (type);
}
