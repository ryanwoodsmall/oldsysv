/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/msgs/mopen_ipc.c	1.3"

/*LINTLIBRARY*/

#include "stdio.h"
#include "string.h"
#include "fcntl.h"
#include "pwd.h"
#include "errno.h"
#include "sys/types.h"
#include "sys/ipc.h"
#include "sys/msg.h"
#include "sys/sem.h"

#include "lp.h"
#include "msgs.h"

extern char		*malloc();

extern struct passwd	*getpwuid();

extern ushort		getuid();

extern int		getpid();

extern void		free();

int			Sched_id	= -1,
			My_id		= -1;

static int		Sched_Semid	= -1;

/**
 ** mopen() - OPEN A MESSAGE PATH THAT USES IPC
 **/

int			mopen ()
{
	char			*msgbuf = 0,
				buf[16];

	int			fd,
				size,
				save_My_id;

	short			status;

	struct flock		lock;

	struct passwd		*p;


	if (Sched_id != -1) {
		errno = EEXIST;
		return (-1);
	}

	if (!(p = getpwuid((int)getuid()))) {
		errno = EINVAL;
		return (-1);
	}

	if (!Lp_Schedlock)
		getpaths ();
	if ((fd = Open(Lp_Schedlock, O_RDONLY, 0666)) == -1) {
		errno = ENOENT;
		return (-1);
	}

	/*
	 * The Spooler maintains a write lock on the SCHEDLOCK file
	 * as long as it runs. Thus, if we can get a read lock, the
	 * Spooler must not be running.
	 */
	lock.l_type = F_RDLCK;
	lock.l_whence = 0;
	lock.l_start = 0;
	lock.l_len = 0;
	if (Fcntl(fd, F_SETLK, &lock) != -1 || errno != EACCES) {
		(void)Close (fd);
		errno = ENOENT;
		return (-1);
	}

	/*
	 * Note: The Spooler maintains an ADVISORY ONLY write lock,
	 * so we can still read the file.
	 */
	if (Read(fd, buf, 16) != 16) {
		(void)Close (fd);
		errno = ENOENT;
		return (-1);
	}

	(void)Close (fd);

	Sched_id = stol(buf);
	Sched_Semid = stol(buf + 8);

	My_id = msgget(IPC_PRIVATE, IPC_CREAT|IPC_EXCL|U_MSG_MODE);
	if (My_id == -1) {
		errno = ENOMEM;
		Sched_id = -1;
		return (-1);
	}

	size = putmessage((char *)0, S_NEW_QUEUE, (long)getpid(), (long)My_id, ubuf.nodename);
	if (!(msgbuf = malloc((unsigned)size))) {
		errno = ENOMEM;
		Sched_id = -1;
		return (-1);
	}
	(void)putmessage (msgbuf, S_NEW_QUEUE, (long)getpid(), (long)My_id, ubuf.nodename);
	save_My_id = My_id;
	My_id = Sched_id;
	if (msend(msgbuf) == -1) {
		(void)msgctl (My_id, IPC_RMID, (struct msqid_ds *)0);
		Sched_id = -1;
		free (msgbuf);
		return (-1);
	}
	My_id = save_My_id;

	if (
		mrecv(msgbuf, size) == -1
	     || getmessage(msgbuf, R_NEW_QUEUE, &status) != R_NEW_QUEUE
	) {
		(void)msgctl (My_id, IPC_RMID, (struct msqid_ds *)0);
		Sched_id = -1;
		free (msgbuf);
		return (-1);
	}

	free (msgbuf);

	if (status != MOK) {
		(void)msgctl (My_id, IPC_RMID, (struct msqid_ds *)0);
		Sched_id = -1;
		errno = ENOENT;
		return (-1);
	}

	return (0);
}

/**
 ** nudge_sched
 **/

int			nudge_sched ()
{
	struct sembuf		sem_buf[1],
				*semptr = sem_buf;

	sem_buf[0].sem_num = 0;
	sem_buf[0].sem_op  = 1;
	sem_buf[0].sem_flg = 0;

	return (semop(Sched_Semid, semptr, 1));
}
