/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpsched/msgs_ipc.c	1.4"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "varargs.h"
#include "limits.h"

#include "lpsched.h"

#include "sys/ipc.h"
#include "sys/sem.h"
#include "sys/msg.h"

#if	defined(NEW_LPSHUT)
extern int		Shutdown;
#endif

extern void		dispatch();
	
#define RMMSG(ID) (void)msgctl (ID, IPC_RMID, (struct msqid_ds *)0)
#define RMSEM(ID) (void)semctl (ID, 0, IPC_RMID, (struct semid_ds *)0)

static NODE		*Msg_List,
			*addnode();

static void		delnode(),
			no_IPC(),
			domessage();

static int		Sem_Id			= -1;
static int		Msg_Id			= -1;
	
static struct msgbuf	*Message,
			*Response;


/**
 ** take_message() - WAIT FOR INTERRUPT OR ONE MESSAGE FROM USER PROCESS
 **/

void			take_message ()
{
	NODE			*qp,
				*nextqp;

	short			size;

	static struct sembuf	sem_op[1] = { 0, -1, 0 };


	if (semop(Sem_Id, sem_op, 1) == -1 && errno == EINTR)
		return;

	qp = Msg_List;
	while (qp) {
		size = msgrcv(qp->ident, Message, MSGMAX, -1, IPC_NOWAIT);
		if (size == -1) {
			nextqp = qp->next;
			switch (errno) {

			case ENOMSG:
				if (kill(qp->pid, 0) != -1)
					break;
				RMMSG (qp->ident);
				/* DROP THROUGH */

			case EINVAL: /* queue was deleted */
			case EIDRM:  /* queue was deleted WHILE IN MSGRCV */
				do_flt_acts(qp);
				delnode (qp);
				break;

			default:
				break;

			}
			qp = nextqp;

		} else
			domessage (Message->mtext, qp);
	}
	return;
}

/**
 ** send() - SEND A MESSAGE TO A USER PROCESS
 **/

static void		_send();

/*VARARGS1*/
void			send (n, va_alist)
	NODE			*n;
	va_dcl
{
	va_list			arg;

	va_start (arg);
	_send (n, 3, arg);
	va_end (arg);
	return;
}

/*VARARGS1*/
void			sendme (va_alist)
	va_dcl
{
	va_list			arg;

	struct sembuf		sem_buf[1],
				*semptr = sem_buf;


#if	defined(MALLOC_3X)
	/*
	 * WARNING: This routine, called only from "exec()", should
	 * not make any use of space allocated earlier. "exec()" clears
	 * ALL the allocated space to reduce the size of the process.
	 */
	NODE			tmp;

	char			msgbuf[BASE_MSG_SIZE(MSGMAX)];


	tmp.pid = getpid();
	tmp.uid = Lp_Uid;
	tmp.gid = Lp_Gid;
	tmp.ident = Msg_Id;
	tmp.master = 1;
	tmp.admin = 1;
	Msg_List = &tmp;
	Response = (struct msgbuf *)msgbuf;
#endif

	va_start (arg);
	_send (Msg_List, 1, arg);
	va_end (arg);

	sem_buf[0].sem_num = 0;
	sem_buf[0].sem_op  = 1;
	sem_buf[0].sem_flg = 0;
	semop(Sem_Id, semptr, 1);

	return;
}

static void		_send (n, mtype, arg)
	NODE			*n;
	int			mtype;
	va_list			arg;
{
	int			size;

	short			type;

	type = (short)va_arg(arg, int);
	size = _putmessage(Response->mtext, type, arg);
# if defined (DEBUG)
	if (debug)
	{
	    FILE	*fp;
	    char	*path;

	    if (path = makepath(Lp_Logs, "messages", (char *)0))
	    {
		if (fp = fopen(path, "a"))
		{
		    time_t	now = time((time_t *)0);
		    char	*ctime();

		    fprintf(
			fp,
			"SEND: %24.24s type %d size %d uid %d queue %d\n",
			ctime(&now),
			type,
			size - CONTROL_LEN,
			n->uid,
			n->ident
		    );
		    fputs("      ", fp);
		    fwrite(Response->mtext + HEAD_LEN, 1, size - CONTROL_LEN, fp);
		    fputs("\n", fp);
		    fclose(fp);
		}
		free(path);
	    }
	}
# endif

	if (size > 0) {
		Response->mtype = mtype;
		(void)msgsnd (n->ident, Response, size, 0);
	}

	return;
}

/**
 ** remove_message() - CLEAN UP MESSAGE QUEUES
 **/

void			remove_messages ()
{
	register NODE		*n;
	
	register int		busy	= 1,
				cycle;

	struct msqid_ds		msqbuf;


	for (cycle = 0; cycle < 5 && busy; cycle++) {
		busy = 0;
		for (n = Msg_List; n; n = n->next)
			if (
				cycle < 4
			     && msgctl(n->ident, IPC_STAT, &msqbuf) == 0
			     && msqbuf.msg_qnum != 0
			)
				busy = 1;
			else
				RMMSG (n->ident);
		if (busy)
			sleep (2);
	}

	RMSEM (Sem_Id);
	return;
}

/**
 ** domessage() - HANDLE AN INCOMING MESSAGE
 **/

static void		domessage (m, n)
	char			*m;
	NODE			*n;
{
	short			type;

	long			pid,
				msg_id;

	struct msqid_ds		mqstat;

	register NODE		*p;

	char			*system;

# if defined (DEBUG)
	if (debug)
	{
	    FILE	*fp;
	    char	*path;

	    if (path = makepath(Lp_Logs, "messages", (char *)0))
	    {
		if (fp = fopen(path, "a"))
		{
		    time_t	now = time((time_t *)0);
		    char	*ctime();
		    int	size	= stoh(Message->mtext + HEAD_SIZE);
		    int	type	= stoh(Message->mtext + HEAD_TYPE);

		    fprintf(
			fp,
			"RECV: %24.24s type %d size %d uid %d queue %d\n",
			ctime(&now),
			type,
			size - CONTROL_LEN,
			n->uid,
			n->ident
		    );
		    fputs("      ", fp);
		    fwrite(Message->mtext + HEAD_LEN, 1, size - CONTROL_LEN, fp);
		    fputs("\n", fp);
		    fclose(fp);
		}
		free(path);
	    }
	}
# endif
	type = getmessage(m, I_GET_TYPE);
	if (n->master && type == S_NEW_QUEUE) {

		(void) getmessage(m, type, &pid, &msg_id, &system);
		if (msgctl(msg_id, IPC_STAT, &mqstat) == 0) {

#if	defined(NEW_LPSHUT)
			if (Shutdown && !isadmin(mqstat.msg_perm.uid))
			    refused = 1;
			
			if (
			       !refused
			    && mqstat.msg_perm.cuid == mqstat.msg_perm.uid
#else
			if (
			       mqstat.msg_perm.cuid == mqstat.msg_perm.uid
#endif
			    && mqstat.msg_perm.cgid == mqstat.msg_perm.gid
			    && (mqstat.msg_perm.mode & 0777) == U_MSG_MODE
			    && kill(pid, 0) == 0
			) {
				p = addnode();
				p->uid = mqstat.msg_perm.uid;
				p->gid = mqstat.msg_perm.gid;
				p->pid = pid;
				p->master = 0;
				p->system = strdup(system);
				p->admin = isadmin(p->uid);
				p->ident = msg_id;
				while (msgrcv(msg_id, Message, MSGMAX, 0, IPC_NOWAIT) != -1)
					;
				send (p, R_NEW_QUEUE, MOK);

			} else {
				NODE			tmp;

				tmp.ident = msg_id;
				send (&tmp, R_NEW_QUEUE, MNOOPEN);
			}

		}

	} else if (n->master && type == S_CHILD_DONE) {

		long			key;
		short			slot,
					status,
					err;

		(void) getmessage(m, type, &key, &slot, &status, &err);
		if (
			0 <= slot && slot < ET_Size
		     && Exec_Table[slot].key == key
		) {
			Exec_Table[slot].pid = -99;
			Exec_Table[slot].status = status;
			Exec_Table[slot].errno = err;
			DoneChildren++;
		}

	} else if (!n->master)
		dispatch (type, m, n);

	return;
}

/**
 ** init_messages() - INITIALIZE MAIN MESSAGE QUEUE
 **/

void			init_messages ()
{
	void			(*oldsys)();

	NODE			*p;

	char			buf[16];

	struct msqid_ds		mqstat;

	struct semid_ds		smstat;


	if ((Message = (struct msgbuf *)malloc(BASE_MSG_SIZE(MSGMAX))) == NULL)
		fail(MEMORY_ALLOC_FAILED);
	if ((Response = (struct msgbuf *)malloc(BASE_MSG_SIZE(MSGMAX))) == NULL)
		fail(MEMORY_ALLOC_FAILED);
    
	oldsys = signal(SIGSYS, no_IPC);

	/*
	 * Try to pick up the old queue so we can deal
	 * with whatever is in it.
	 */
	if (fread(buf, 1, 16, LockFp) == 16) {
		Msg_Id = stol(buf);
		Sem_Id = stol(buf + 8);
		if (msgctl(Msg_Id, IPC_STAT, &mqstat) == -1) {
			RMSEM (Sem_Id);
			Msg_Id = Sem_Id = -1;
		} else if (semctl(Sem_Id, 0, IPC_STAT, &smstat) == -1) {
			RMMSG (Msg_Id);
			RMSEM (Sem_Id);
			Msg_Id = Sem_Id = -1;
		}
	}

	if (Msg_Id == -1 || Sem_Id == -1) {
		Msg_Id = msgget(IPC_PRIVATE, IPC_CREAT|IPC_EXCL|S_MSG_MODE);
		Sem_Id = semget(IPC_PRIVATE, 8, IPC_CREAT|IPC_EXCL|S_SEM_MODE);
	}
	if (Msg_Id == -1 || Sem_Id == -1) {
		if (Msg_Id != -1)
			RMMSG (Msg_Id);
		if (Sem_Id != -1)
			RMSEM (Sem_Id);
		(void)fail (MSG_IPC_ALLOC_FAILED, PERROR);
	}

	(void)signal (SIGSYS, oldsys);

	/*
	 * Add the message id to the node list.
	 */
	p = addnode();
	p->pid = getpid();
	p->uid = Lp_Uid;
	p->gid = Lp_Gid;
	p->ident = Msg_Id;
	p->master = 1;
	p->admin = 1;

	/*
	 * Advertise our ``address''.
	 */
	ltos (buf, Msg_Id);
	ltos (buf + 8, Sem_Id);
	setbuf (LockFp, (char *)0);
	fseek (LockFp, 0L, 0);
	fwrite (buf, 1, 16, LockFp);
	return;
}

/*
**+
**      Function:    void  no_IPC();
**     
**      Called By:   init_IPC() via signal handler
**     
**      Purpose:     An error was encountered and a SIGSYS signal
**                   received due to the IPC package not being
**                   installed.  An error message is displayed,
**                   and lpsched is gracefully terminated.
**     
**      Return:      never.
**+
*/
void
no_IPC()
{
	fail(NEED_IPC_PKG);
}

NODE	*
addnode()
{
    NODE	*n;
    NODE	*p;
    
    if ((n = (NODE *)malloc(sizeof(NODE))) == NULL)
	fail(MEMORY_ALLOC_FAILED);

    (void) memset((char *)n, 0, sizeof(NODE));

    if (Msg_List)
    {
	p = Msg_List;

	while(p->next)
	    p = p->next;

	p->next = n;
	n->prev = p;
    }
    else
	Msg_List = n;

    return(n);
}

void
delnode(n)
NODE	*n;
{
    if (n->master)
	return;
    
    if (n == Msg_List)
	return;
    
    if (n->next)
	n->next->prev = n->prev;
    
    if (n->prev)
	n->prev->next = n->next;
    
    free(n->system);
    free((char *)n);
}

void
ev_message()
{
    return;
}

void
shutdown_msgs()
{
    return;
}
