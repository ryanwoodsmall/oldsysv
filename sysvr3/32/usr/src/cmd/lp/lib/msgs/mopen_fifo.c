/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/msgs/mopen_fifo.c	1.3"

/*LINTLIBRARY*/

#include "fcntl.h"
#include "errno.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "sys/utsname.h"

#include "lp.h"
#include "msgs.h"

#define TURN_OFF(X,F)	(void)Fcntl(X, F_SETFL, (Fcntl(X, F_GETFL) & ~(F)))

extern char		*malloc();
extern ushort		getuid();
extern int		getpid();
extern void		free();

int			Sched_fifo	= -1,
			My_fifo		= -1;

char			*My_fifo_path	= 0;


/*
** mopen_fifo() - OPEN A MESSAGE PATH THAT USES FIFOS
*/

int
mopen ()
{
    char		*msgbuf = 0,
			*fifo_name	= "UUUUUUUUNNNNN";

    int			fd,
			tmp_fd = -1,
			size;

    short		status;
    struct flock	lock;
    struct utsname	ubuf;


    if (Sched_fifo != -1)
    {
	errno = EEXIST;
	return (-1);
    }

    /*
    ** Initialize paths (if necessary) and open the lock file
    */

    if (!Lp_Schedlock)
	getpaths();

    if ((fd = Open(Lp_Schedlock, O_RDONLY, 0666)) == -1)
    {
	errno = ENOENT;
	return (-1);
    }

    /*
    ** Now, we try to read-lock the lock file.  This can only succeed if
    ** the Spooler is down, since an active Spooler will be maintaining
    ** a write lock.  So, if it works, something must be up.
    ** Ergo, we return an error.
    */

    lock.l_type = F_RDLCK;
    lock.l_whence = 0;
    lock.l_start = 0;
    lock.l_len = 0;
    if (Fcntl(fd, F_SETLK, &lock) != -1 || errno != EACCES)
    {
	(void)Close (fd);
	errno = ENOENT;
	return (-1);
    }
    (void)Close (fd);

    /*
    ** We open <Lp_FIFO> with O_NDELAY set so that we don't block
    ** Blocking could get us stuck if the spooler went away between
    ** the call to fcntl and the call to open.
    */

    if ((Sched_fifo = Open(Lp_FIFO, O_WRONLY)) == -1)
    {
	errno = ENOENT;
	return (-1);
    }

    /*
    ** Combine the machine node-name with the process ID to
    ** get a name that will be unique across the network.
    ** The 0xFFFF is just a safety precaution against over-running
    ** the buffer.
    ** 
    ** We initially create the fifo in the public directory.
    ** After the first S_NEW_QUEUE, lpsched will grant us a queue by
    ** the same name in the private directory.
    */

    (void)uname (&ubuf);
    sprintf (fifo_name, "%.8s%u", ubuf.nodename, (getpid() & 0xFFFF));
    if (!(My_fifo_path = makepath(Lp_Public_FIFOs, fifo_name, (char *)0)))
    {
	errno = ENOMEM;
	goto Error;
    }

    (void) Unlink(My_fifo_path);
    
    if (Mknod(My_fifo_path, S_IFIFO | U_MSG_MODE) == -1)
	goto Error;

    if ((My_fifo = Open(My_fifo_path, O_RDONLY|O_NDELAY)) == -1)
	goto Error;
    
    TURN_OFF (My_fifo, O_NDELAY);

    size = putmessage((char *)0, S_NEW_QUEUE, 0, fifo_name, ubuf.nodename);
    if (!(msgbuf = malloc((unsigned)size)))
    {
	errno = ENOMEM;
	goto Error;
    }
    (void) putmessage(msgbuf, S_NEW_QUEUE, 0, fifo_name, ubuf.nodename);

    if (
	   msend(msgbuf) == -1
	|| mrecv(msgbuf, size) == -1
	|| getmessage(msgbuf, R_NEW_QUEUE, &status) != R_NEW_QUEUE
       )
    {
	free (msgbuf);
	goto Error;
    }
    else
	if (status != MOK)
	{
	    free(msgbuf);
	    errno = ENOSPC;
	    goto Error;
	}

    free (msgbuf);

    /*
     * The Spooler has acknowledged the request, so now we prepare
     * to use the one the Spooler created.  This new FIFO can be
     * read ONLY by who we said we are, so if we lied, tough luck for us!
     */

    (void)Unlink (My_fifo_path);	/* must exist to get here */
    tmp_fd = My_fifo;			/* save the old fd */

    free(My_fifo_path);
    if (!(My_fifo_path = makepath(Lp_Private_FIFOs, fifo_name, (char *)0)))
    {
	errno = ENOMEM;
	goto Error;
    }

    if ((My_fifo = Open(My_fifo_path, O_RDONLY|O_NDELAY)) == -1)
	goto Error;

    TURN_OFF (My_fifo, O_NDELAY);

    size = putmessage((char *)0, S_NEW_QUEUE, 1, fifo_name, ubuf.nodename);
    if (!(msgbuf = malloc((unsigned)size)))
    {
        errno = ENOMEM;
        goto Error;
    }
    (void) putmessage(msgbuf, S_NEW_QUEUE, 1, fifo_name, ubuf.nodename);

    if (
	   msend(msgbuf) == -1
	|| mrecv(msgbuf, size) == -1
	|| getmessage(msgbuf, R_NEW_QUEUE, &status) != R_NEW_QUEUE
       )
    {
	free (msgbuf);
	goto Error;
    }
    else
	if (status != MOK)
	{
	    free(msgbuf);
	    errno = ENOSPC;
	    goto Error;
	}

    free (msgbuf);
    (void) Close(tmp_fd);
    return (0);

Error:
    if (Sched_fifo != -1)
    {
	(void) Close (Sched_fifo);
	Sched_fifo = -1;
    }
    if (My_fifo != -1)
    {
	(void) Close (My_fifo);
	My_fifo = -1;
    }
    if (My_fifo_path)
    {
	(void) Unlink (My_fifo_path);
	free (My_fifo_path);
	My_fifo_path = 0;
    }
    if (tmp_fd != -1)
	(void) Close(tmp_fd);
    
    return (-1);
}
