/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/msgs/mclose_fifo.c	1.3"

#include "errno.h"

#include "lp.h"
#include "msgs.h"

extern int		Sched_fifo,
			My_fifo;

extern char		*My_fifo_path;

extern char		*malloc();

extern void		free();

int			mclose_is_running	= 0;

/*
** mclose() - CLOSE A MESSAGE PATH THAT USES FIFOS
*/

int
mclose ()
{
    char	*msgbuf = 0;
    int		size;


    mclose_is_running = 1;

    if (Sched_fifo != -1)
    {
	size = putmessage((char *)0, S_GOODBYE);
	if ((msgbuf = malloc((unsigned)size)))
	{
	    (void)putmessage (msgbuf, S_GOODBYE);
	    (void)msend (msgbuf);
	    free (msgbuf);
	}

	(void) Close (Sched_fifo);
	Sched_fifo = -1;
    }

    if (My_fifo != -1)
    {
	(void) _Close (My_fifo);
	My_fifo = -1;
    }

    if (My_fifo_path)
    {
	(void) _Unlink (My_fifo_path);
	free (My_fifo_path);
	My_fifo_path = 0;
    }	

    mclose_is_running = 0;
    return (0);
}
