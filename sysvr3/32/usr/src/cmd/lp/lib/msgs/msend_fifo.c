/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/msgs/msend_fifo.c	1.2"

/*LINTLIBRARY*/

#include "errno.h"
#include "memory.h"
#include "signal.h"

#include "msgs.h"


extern int		Sched_fifo,
			My_fifo,
			mclose_is_running;

char			AuthCode[HEAD_AUTHCODE_LEN];

static void		(*callers_sigpipe_trap)()	= SIG_DFL;

/**
 ** msend() - SEND A MESSAGE VIA FIFOS
 **/

int
msend (msgbuf)
char			*msgbuf;
{
    int		size;
    short	type;

    if (Sched_fifo == -1)
    {
	errno = ENXIO;
	return (-1);
    }

    if (
	   !msgbuf
	|| (size = stoh(msgbuf + HEAD_SIZE)) < CONTROL_LEN
	|| (type = stoh(msgbuf + HEAD_TYPE)) < 0
	|| LAST_UMESSAGE < type
	)
    {
	errno = EINVAL;
	return (-1);
    }

    (void)memcpy (msgbuf + HEAD_AUTHCODE, AuthCode, HEAD_AUTHCODE_LEN);

    callers_sigpipe_trap = signal(SIGPIPE, SIG_IGN);

Retry_Write:

    switch (write_fifo(Sched_fifo, msgbuf, size))
    {

	case 0:
	    /*
	    ** The Spooler's FIFO is full and O_NDELAY is set.
	    ** Since we don't set O_NDELAY in the user process,
	    ** this case should never happen. But it's here
	    ** to illustrate the possibility, in case the code
	    ** changes to include O_NDELAY.
	    */
	    /* put something here! */
	     break;

	case -1:
	    /*
	     * An interrupted "write()" gets restarted. We ignore SIGPIPE
	     * (to avoid confusing the caller if IT is trapping SIGPIPE)
	     * so we'll get EPIPE if the Spooler closed its FIFO.
	     * That's equivalent to a message queue being removed during
	     * a "msgsnd", so we return the same "errno".
	     */
	     if (errno == EINTR)
		 goto Retry_Write;
	     if (errno == EPIPE)
	     {
		 /*
		  * "mclose()" will try to say goodbye to the Spooler,
		  * and that, of course, will fail. But we'll call
		  * "mclose()" anyway, for the other cleanup it does.
		  * We have to watch out for loops, though.
		  */
		 if (!mclose_is_running)
		     (void)mclose ();
		 errno = EIDRM;
	     }

	     (void) signal(SIGPIPE, callers_sigpipe_trap);

	     return (-1);
    }

    (void) signal(SIGPIPE, callers_sigpipe_trap);

    return (0);
}
