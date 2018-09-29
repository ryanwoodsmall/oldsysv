/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpsched/msgs_fifo.c	1.4"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "varargs.h"
#include "limits.h"

#include "lpsched.h"
#include "sys/stat.h"

#define TURN_OFF(X,F)	(void)Fcntl(X, F_SETFL, (Fcntl(X, F_GETFL) & ~(F)))

static void		log_message_fact();

#if	!defined(I_AM_LPSCHED_JR)

#define	USR_FIFO	(S_IFIFO|S_IRUSR)
#define	SYS_FIFO	(S_IFIFO|S_IWUSR|S_IWGRP|S_IWOTH)

extern void		dispatch();

void			shutdown_msgs();

static char		*Message,
			*Response;

static void		send_rejection (),
			send_acceptance(),
			goodbye_client();

static int		MaxClients		= 0,
			do_msg();

static NODE		*Client_List;

extern int		Reserve_Fds,
			Garbage_Bytes,
			Garbage_Messages;

static int		Sched_fifo	= -1;

/*
** take_message() - WAIT FOR INTERRUPT OR ONE MESSAGE FROM USER PROCESS
*/

void
take_message ()
{
    int		bytes;

    /*
    ** This routine will block on opening the FIFO and/or reading
    ** it. Why? Because when the last client process closes the FIFO,
    ** we will cease to block on the read. We have to block, so when
    ** the last close is detected we close the FIFO and then reopen
    ** it, blocking until another client comes along (or a signal
    ** interrupts us).
    */

    for (EVER)		/* not really forever...returns are in the loop */
    {
	Garbage_Bytes = Garbage_Messages = 0;

	while (Sched_fifo == -1)
	    if ((Sched_fifo = open(Lp_FIFO, O_RDONLY)) == -1)
		if (errno == EINTR)
		    return;
		else
		    fail (MSG_FIFO_ALLOC_FAILED, PERROR);

	bytes = read_fifo(Sched_fifo, Message, MSGMAX);

	if (Garbage_Bytes > 0 || Garbage_Messages > 0)
	    note(GARBAGE_IN_FIFO, Garbage_Bytes, Garbage_Messages, Lp_FIFO);

	switch (bytes)
	{
	    case 0:

	        /*
		** The fifo was empty and we have O_NDELAY set,
		** or the last client closed the FIFO. We don't
		** set O_NDELAY, so the first should never happen.
		** But be warned that we can't tell the difference
		** in some versions of the UNIX op. sys.!!
		*/

	        (void)close (Sched_fifo);
		Sched_fifo = -1;
		break;

	    case -1:

		if (errno == EINTR)
		    return;
		else
		    fail (MSG_FIFO_ALLOC_FAILED, PERROR);
		break;

	    default:

		if (do_msg())
		    return;
		break;
	}
    }
}

/*
** do_msg() - HANDLE AN INCOMING MESSAGE
*/

static int
do_msg ()
{
    char		*msg_fifo;
    int			cleaned;
    ushort		stage;
    char		*pub_fifo;
    char		*system;
    extern void		dispatch();
    int			type;
    register NODE	*p;
    short		madenode;
    short		client;
    struct stat		fistat;


    TRACE_ON ("do_msg");

    /*
    ** We received a message. See who sent it and if they
    ** have the right ``password'' (i.e. the random number
    ** we sent them in the last message). New clients have
    ** a S_NEW_QUEUE type and won't be checked.
    **
    ** NOTE: Although a "switch" statement is used here, don't
    ** get the idea that other message cases can be included;
    ** all ongoing messages MUST BE CHECKED for a proper "authcode"
    ** (the "default" case). The "switch" is used only to take
    ** advantage of the "break" statement.
    */
#if	defined(DEBUG)
    if (debug)
    {
	FILE	*fp = open_logfile("messages");

	if (fp)
	{
		time_t	now = time((time_t *)0);
		char	*ctime();
		char	buffer[BUFSIZ];
		int	size	= stoh(Message + HEAD_SIZE);
		int	type	= stoh(Message + HEAD_TYPE);

		setbuf(fp, buffer);
		fprintf(
		    fp,
		    "RECV: %24.24s type %d size %d\n",
		    ctime(&now),
		    type,
		    size - CONTROL_LEN
		);
		fputs("      ", fp);
		fwrite(Message + HEAD_LEN, 1, size - CONTROL_LEN, fp);
		fputs("\n", fp);
		close_logfile(fp);
	}
    }
# endif

    switch ((type = getmessage(Message, I_GET_TYPE)))
    {
	case S_NEW_QUEUE:


	    /*
	    ** A new client wishes to talk. The protocol:
	    **
	    **	client			Spooler (us)
	    **
	    **	S_NEW_QUEUE  ------->
	    **				Create 2nd return fifo
	    **		     <-------   R_NEW_QUEUE on 1st fifo
	    **	close 1st fifo
	    **	prepare 2nd fifo
	    **	S_NEW_QUEUE  ------->
	    **				R_NEW_QUEUE with authcode
	    **				on 2nd fifo
	    **
	    ** Thus this may be the first OR second S_NEW_QUEUE
	    ** message for this client. We tell the difference
	    ** by the "stage" field.
	    */

	    (void)getmessage (Message, type, &stage, &msg_fifo,&system);
	    if (stage == 0)
	    {
		cleaned = 0;
CleanedUp:
		for (client = 0; client < MaxClients; client++)
		    if (Client_List[client].state == CLIENT_NEW)
			break;

		if (client >= MaxClients)
		{
		    if (!cleaned++)
			if (chk_clients())
			    goto CleanedUp;

		    if (pub_fifo = makepath(Lp_Public_FIFOs, msg_fifo, (char *)0))
		    {
			send_rejection (pub_fifo);

#if	defined(DEBUG)
			log_message_fact (
    "      NEW_QUEUE: client (%s) rejected at stage 0 (no more slots)",
			    msg_fifo
			);
#endif
			break;
		    }
		}
		p = Client_List + client;
		
		
		/*
		** Make a new fifo, in the private dir rather
		** than the public dir. Use the original
		** for the ACK or NAK of this message.
		** Make the client the owner (user and group)
		** of the fifo, and make it readable only by the
		** client.
		*/

		(void) memcpy(p->authcode, (char *)&client, sizeof(short));
		p->fifo = makepath(Lp_Private_FIFOs, msg_fifo, (char *)0);
		pub_fifo = makepath(Lp_Public_FIFOs, msg_fifo, (char *)0);
		if
		(
		       strlen(p->fifo)
		    && Stat(pub_fifo, &fistat) == 0
		    && (madenode = (Mknod(p->fifo, USR_FIFO) == 0))
		    && Chown(p->fifo, fistat.st_uid, fistat.st_gid) == 0
		)
		{

		    /*
		    ** For "mknod()" to have succeeded, the
		    ** desired fifo couldn't have existed
		    ** already. Our creation gave modes
		    ** and owner that should prevent
		    ** anyone except who the client claims
		    ** to be from using the fifo. Thus
		    ** this fifo is ``safe''--anything we put
		    ** in can only be read by the client.
		    */

		    p->state = CLIENT_PROTOCOL;
		    p->uid = fistat.st_uid;
		    p->gid = fistat.st_gid;
		    p->admin = isadmin(p->uid);
		    send_acceptance (p, pub_fifo);
		    free(pub_fifo);
		    
#if	defined(DEBUG)
		    log_message_fact (
		"      NEW_QUEUE: client %d (%s) accepted at stage 0",
			client,
			msg_fifo
	      	    );
#endif
		}
		else
		{

		    /*
		    ** Another wise guy.
		    **
		    ** Before removing the second fifo, make sure
		    ** we actually created it!  If we did not get far
		    ** enough to set <madenode>, we never created the
		    ** fifo, so don't remove it!  The wise guy may be
		    ** trying to con us!
		    */

		    if (strlen(p->fifo) && madenode)
			(void)unlink (p->fifo);
		    free (p->fifo);
		    free (pub_fifo);
#if	defined(DEBUG)
		    log_message_fact (
	"      NEW_QUEUE: client %d (%s) rejected at stage 0 (%s)",
			client,
			msg_fifo,
			PERROR
		    );
#endif
		}
	    }
	    else
	    {
		(void) memcpy
		(
		    (char *)&client,
		    Message + HEAD_AUTHCODE,
		    sizeof(short)
		);

		/*
		** If a ``client'' can't obey the
		** protocol, they don't deserve a
		** response. (This minimizes the impact
		** of a wise guy sending us trash).
		*/

		if (client < 0 || MaxClients <= client)
		{
#if	defined(DEBUG)
		    log_message_fact (
    "      NEW_QUEUE: client %d (%s) rejected at stage 1 (out of range)",
			client,
			msg_fifo
		    );
#endif
		    break;
		}

		p = Client_List + client;
		if (p->state != CLIENT_PROTOCOL)
		    break;

		/*
		** The client has just sent the second S_NEW_QUEUE
		** so we will now send back another ACK on the
		** safe fifo.
		*/
		(void) Close(p->fd);
		
		if ((p->fd = Open(p->fifo, O_WRONLY | O_NDELAY)) >= 0)
		{
		    p->state = CLIENT_TALKING;
		    p->system = strdup(system);
		    send (p, R_NEW_QUEUE, MOK);
#if	defined(DEBUG)
		    log_message_fact (
		"      NEW_QUEUE: client %d (%s) accepted at stage 1",
			client,
			msg_fifo
		    );
#endif
		}
		else
		{
		    p->state = CLIENT_NEW;
		    (void)unlink (p->fifo);
		    free (p->fifo);
#if	defined(DEBUG)
		    log_message_fact (
	"      NEW_QUEUE: client %d (%s) rejected at stage 1 (%s)",
			client,
			msg_fifo,
			PERROR
		    );
#endif
		}
	    }
	    break;

	case S_CHILD_DONE:
	{
	    long			key;
	    short			slot,
					status,
					err;

	    (void) getmessage(Message, type, &key, &slot, &status, &err);
	    if (0 <= slot && slot < ET_Size && Exec_Table[slot].key == key)
	    {
#if	defined(DEBUG)
		log_message_fact (
	    "      CHILD_DONE: slot %d, pid %d, status %d, err %d",
		    slot,
		    Exec_Table[slot].pid,
		    status,
		    err
		);
#endif
		Exec_Table[slot].pid = -99;
		Exec_Table[slot].status = status;
		Exec_Table[slot].errno = err;
		DoneChildren++;

		/*
		 * We have to get back to the main processing loop
		 * to handle the impact of this message.
		 */
		TRACE_OFF ("do_msg");
		return(1);

	    }
#if	defined(DEBUG)
	    else
		log_message_fact (
    "      CHILD_DONE (FAKE): slot %d, pid unknown, status %d, err %d",
		    slot,
		    status,
		    err
		);
#endif
	    break;
	}
	    
	default:


	    /*
	    ** We can be so bold as to take the "client" and
	    ** "authcode" values out of the message in a non-portable
	    ** fashion, because only we, i.e. THIS PROCESS, put it
	    ** there for the client to echo back untouched.
	    ** Of course, someone could have given us trash, so we
	    ** have to check it.
	    */

	    (void) memcpy
	    (
	        (char *)&client,
	        Message + HEAD_AUTHCODE,
	        sizeof(short)
	    );


	    /*
	    ** If someone sent us a bogus message, we don't
	    ** know who it was.  It may have come from a bad
	    ** guy pretending to be the real client.  We can't
	    ** send anything back because it may surprise the
	    ** real client.
	    */

	    if (client < 0 || MaxClients <= client)
	    {
#if	defined(DEBUG)
		log_message_fact (
		    "      MESSAGE IGNORED: client %d out of range",
		    client
		);
#endif
		break;
	    }
	    if (memcmp(
		(p = &Client_List[client])->authcode,
		Message + HEAD_AUTHCODE,
		HEAD_AUTHCODE_LEN
	    )) {
#if	defined(DEBUG)
		log_message_fact (
		    "      MESSAGE IGNORED: client %d gave wrong authcode",
		    client
		);
#endif
		break;
	    }

#if	defined(DEBUG)
	    log_message_fact (
		"      MESSAGE ACCEPTED: client %d",
		client
	    );
#endif

	    if (type == S_GOODBYE)
		goodbye_client (p);
	    else
	    {
		dispatch (type, Message, p);

		/*
		 * The message may have caused the need to
		 * schedule something, so go back and check.
		 */
		TRACE_OFF ("do_msg");
		return(1);
	    }
	    break;

	}

	TRACE_OFF ("do_msg");
	return(0);
}

static void
send_rejection (fifo)
char	*fifo;
{
    NODE	tmp;

    tmp.fifo = fifo;
    tmp.flt = 0;
    tmp.mque = 0;
    tmp.state = CLIENT_TALKING;
    if ((tmp.fd = Open(fifo, O_WRONLY | O_NDELAY)) != -1)
    {
	send (&tmp, R_NEW_QUEUE, MNOSPACE);
	goodbye_client(&tmp);
    }
    return;
}

static void
send_acceptance (n, fifo)
NODE	*n;
char	*fifo;
{
    if ((n->fd = Open(fifo, O_WRONLY | O_NDELAY)) != -1)
	send (n, R_NEW_QUEUE, MOK);
    return;
}

/*
** goodbye_client() - REMOVE CLIENT FROM LIST
*/

static void
goodbye_client (n)
NODE	*n;
{
    register MQUE	*m,
			*mnext;

    (void)Close (n->fd);

    (void)Unlink (n->fifo);

    if (n->fifo)
	free (n->fifo);

    if (n->system)
	free (n->system);

    n->fd = -1;
    n->fifo = 0;
    n->system = 0;
    n->state = CLIENT_NEW;

    /*
     * Clean up anything pending for this client.
     */

    for (m = n->mque; m; m = mnext)
    {
	mnext = m->next;
	if (m->msgbuf)
	    free (m->msgbuf);
	free ((char *)m);
    }
    n->mque = 0;

    do_flt_acts(n);

    return;
}

/*
** send() - SEND A MESSAGE TO A USER PROCESS
*/

/*VARARGS1*/

void
send (n, va_alist)
NODE		*n;
va_dcl
{
    va_list	arg;
    int		size;
    short	type;


    va_start (arg);
    type = (short)va_arg(arg, int);

    if (n->state == CLIENT_NEW)
	return;

    if (n->state == CLIENT_PROTOCOL && type != R_NEW_QUEUE)
	return;

    size = _putmessage(Response, type, arg);

    if (size > 0)
    {
	long	key;


	/*
	** The client has to give us the following ``password''
	** in their next message. Before they can ever give us
	** a message that asks us to do something potentially
	** dangerous (like print a file), we have to send them
	** a message to a safe place. We do, namely the ACK
	** of the S_NEW_QUEUE message.
	*/

	key = getkey();

	(void) memcpy
	(
	    n->authcode + sizeof(short),
	    (char *)&key,
	    sizeof(long)
	);
	(void) memcpy
	(
	    Response + HEAD_AUTHCODE,
	    n->authcode,
	    HEAD_AUTHCODE_LEN
	);

	(void) _send (n, Response, (unsigned)size, 0, 0);
    }

    va_end (arg);
    return;
}

#endif		/* !defined(I_AM_LPSCHED_JR) */

/*VARARGS1*/
void
sendme (va_alist)
va_dcl
{
    NODE	tmp;
    int		type;
    int		size;
    va_list	arg;

#if	defined(MALLOC_3X) || defined(LPSCHED_JR)
    /*
     * WARNING: This routine, called only from "exec.c", should
     * not make any use of space allocated earlier. This space may
     * have been cleared (or we may have exec'd a new program).
     */
    char	msgbuf[MSGMAX];
# if	defined(I_AM_LPSCHED_JR)
    char	*Response;
# endif

    Lp_FIFO = makepath(getspooldir(), FIFOSDIR, FIFO, (char *)0);
    Response = msgbuf;
#endif

    tmp.fifo = Lp_FIFO;
    tmp.fd = -1;
    tmp.flt = 0;
    tmp.mque = 0;
    tmp.state = CLIENT_TALKING;

    va_start (arg);
    type = (short)va_arg(arg, int);
    size = _putmessage(Response, type, arg);
    va_end (arg);


    /*
     * This routine is used by the child process to tell the parent
     * it has finished. It is crucial that the message get through
     * if the parent is still running. A single open with O_NDELAY
     * might fail if the parent doesn't have the FIFO open right now,
     * so we have to wait. But we can't turn off O_NDELAY because
     * then we wouldn't know if the parent died. Thus, we loop,
     * checking to see if the parent is alive and sleeping if it is.
     */

    do
    {
	if (tmp.fd != -1)
	    (void) Close(tmp.fd);
	while (
		(tmp.fd = Open(tmp.fifo, O_WRONLY | O_NDELAY)) == -1
	     && errno == ENXIO
	     && getppid() != 1 /* 1 is init which means we're an orphan */
	     && kill(getppid(), 0) == 0
	)
	    sleep (1);

	if (tmp.fd != -1)
	    TURN_OFF(tmp.fd, O_NDELAY);
	else
	    break;

    } 
    while (_send (&tmp, Response, (unsigned)size, 0, 1) == 0);
}

static int
_send (n, msgbuf, size, from_queue, from_child)
register NODE	*n;
char		*msgbuf;
unsigned int	size;
int		from_queue;
int		from_child;
{
    register MQUE		*m,
				*p;


#if	defined(DEBUG)
    if (debug)
    {
	FILE	*fp = open_logfile("messages");

	if (fp)
	{
		time_t	now = time((time_t *)0);
		int	type	= stoh(msgbuf + HEAD_TYPE);
		char	*ctime();
		char	*cp	= strrchr(n->fifo, '/');
		char	buffer[BUFSIZ];

		setbuf(fp, buffer);
		fprintf(
		    fp,
		"SEND: %24.24s type %d size %d uid %d queue %s fd %d\n",
		    ctime(&now),
		    type,
		    size - CONTROL_LEN,
		    n->uid,
		    (cp? cp + 1 : n->fifo),
		    n->fd
		);
		fputs("      ", fp);
		fwrite(msgbuf + HEAD_LEN, 1, size - CONTROL_LEN, fp);
		fputs("\n", fp);
		close_logfile(fp);
	}
    }
#endif


    /*
     * Unless we're trying to re-send a message to a client known
     * to be constipated, don't send the message. The client might
     * not NOW be constipated, but there are other messages that
     * must go out before this one. In this case, we'll simply add
     * this message to the queue of messages to try later.
     */
#if	!defined(I_AM_LPSCHED_JR)
    if (!from_queue && n->mque)
	goto QueueIt;
#endif

Restart_Write:
    switch (write_fifo(n->fd, msgbuf, size))
    {

	case 0:
	    /*
	    ** The client's fifo is full and O_NDELAY is set.
	    ** We set an alarm to wake us up in awhile so
	    ** we can retry the message. Because we may have
	    ** more than one message to send back to the
	    ** constipated client, we have to build our own
	    ** queue.
	    **
	    ** If, however, we are a child and _send has been
	    ** called by sendme, we do not requeue, but just
	    ** return 0.  sendme will take care of retrying.
	    */

	    if (from_child)
		return(0);

#if	!defined(I_AM_LPSCHED_JR)
	    if (!from_queue)
	    {
QueueIt:	if
		(
		       (m = (MQUE *)malloc(sizeof(MQUE))) == NULL
		    || !(m->msgbuf = malloc(size))
		)
		    fail(MEMORY_ALLOC_FAILED);

		(void) memcpy (m->msgbuf, msgbuf, (int)size);
		m->size = size;
		m->next = 0;
		if ((p = n->mque))
		{
		    while (p->next)
			p = p->next;
		    p->next = m;
		}
		else
		    n->mque = m;
	    }
	    schedule(EV_LATER, WHEN_MESSAGES, EV_MESSAGE);
# if	defined(DEBUG)
	    if (debug)
		log_message_fact(
		    "SEND: client (%s) constipated%s, %d bytes held",
		    strrchr(n->fifo, '/')+1,
		    from_queue ? " (on retry)" : "",
		    size
		);
# endif
#endif		/* !defined(I_AM_LPSCHED_JR) */
	    return (0);

	case -1:
	    /*
	    ** An interrupted "write()" gets restarted.
	    ** If errno is EPIPE, the client closed the fifo,
	    ** we assume that means the client died. Any other error
	    ** is bizarre, but we'll take it to mean the
	    ** client can no longer be reached. (E.g. maybe
	    ** RFS is down.)
	    */
	    if (errno == EINTR)
		goto Restart_Write;
#if	defined(DEBUG)
	    log_message_fact(
		    "SEND: client (%s) deceased, goodbye client",
		    strrchr(n->fifo, '/')+1
	    );
#endif
#if	!defined(I_AM_LPSCHED_JR)
	    if (!from_child)
		goodbye_client (n);
#endif
	    return (0);

	default:
	    return (1);
    }
    /*NOTREACHED*/
}

#if	!defined(I_AM_LPSCHED_JR)

/**
 ** remove_message() - CLEAN UP MESSAGE QUEUES
 **/

void		remove_messages ()
{
	return;
}

/*
** calculate_nopen() - DETERMINE # FILE DESCRIPTORS AVAILABLE FOR QUEUES
*/

static void
calculate_nopen ()
{
	register int	fd,
			client,
			nopen;


	/*
	 * How many file descriptorss are currently being used?
	 */
	for (fd = nopen = 0; fd < OpenMax; fd++)
		if (fcntl(fd, F_GETFL, 0) != -1)
			nopen++;

	/*
	 * How many file descriptors are available for use
	 * as open FIFOs? Leave one spare as a way to tell
	 * clients we don't have any to spare (hmmm....) and
	 * one for the incoming fifo.
	 */

	MaxClients = OpenMax;
	MaxClients -= nopen;	/* current overhead */
	MaxClients -= Reserve_Fds;
	MaxClients -= 2;	/* incoming FIFO and spare outgoing */
	MaxClients--;		/* the requests log */
	MaxClients--;		/* HPI routines and lpsched log */

#if	defined(DEBUG)
	if (debug)
	    MaxClients--;
#endif

	if (MaxClients < 1)
	    fail (MSG_FIFO_ALLOC_FAILED, "Not enough file descriptors");

	if (!(Client_List = (NODE *)malloc(MaxClients * sizeof(NODE))))
	    fail (MEMORY_ALLOC_FAILED);

	for (client = 0; client < MaxClients; client++)
	{
	    Client_List[client].state = CLIENT_NEW;
	    Client_List[client].mque = 0;
	    Client_List[client].flt = 0;
	    Client_List[client].fd = -1;
	}

	return;
}

/*
** init_messages() - INITIALIZE MAIN MESSAGE QUEUE
*/

void			init_messages ()
{
	char	*cmd;

	(void) signal(SIGPIPE, SIG_IGN);

	calculate_nopen ();

	if (!(Message = malloc(MSGMAX)) || !(Response = malloc(MSGMAX)))
	    fail(MEMORY_ALLOC_FAILED);
    

	(void) Unlink(Lp_FIFO);

	if (Mknod(Lp_FIFO, SYS_FIFO) == -1)
	    fail (MSG_FIFO_ALLOC_FAILED, PERROR);

	(void) Chmod(Lp_FIFO, 0222);
	if (cmd = makestr("/bin/rm -f ", Lp_Public_FIFOs, "/*", (char *)0))
	{
	    System(cmd);
	    free(cmd);
	}
	if (cmd = makestr("/bin/rm -f ", Lp_Private_FIFOs, "/*", (char *)0))
	{
	    System(cmd);
	    free(cmd);
	}
	(void) Chmod(Lp_Public_FIFOs, 0773);
	(void) Chmod(Lp_Private_FIFOs, 0771);

	return;
}

/*
** ev_message() - retry previously failed messages
*/

void
ev_message()
{
    register NODE	*p;
    register int	i;
    register MQUE	*m;

#if	defined(DEBUG)
    log_message_fact("RETRY: check constipated clients");
#endif
    for (i = 0; i < MaxClients; i++)
    {
	p = Client_List + i;
	if (p->state == CLIENT_TALKING && p->mque)
	{
#if	defined(DEBUG)
		log_message_fact (
		    "RETRY: client %d (%s):",
		    i,
		    strrchr(p->fifo, '/')+1
		);
#endif
	    while ((m = p->mque) && _send(p, m->msgbuf, m->size, 1, 0))
	    {
#if	defined(DEBUG)
		log_message_fact (
		    "       took held message, %d bytes",
		    m->size
		);
#endif
		p->mque = m->next;
	    }
	}
    }
}

#if	defined(NEW_LPSHUT)
void
shutdown_msgs()
{
    Chmod(Lp_Public_FIFOs, 0770);
}
#endif

#endif		/* !defined(I_AM_LPSCHED_JR) */

#if	defined(DEBUG)
/*VARARGS0*/
void			log_message_fact (va_alist)
	va_dcl
{
	va_list			ap;

	va_start (ap);
	if (debug) {
		FILE			*fp = open_logfile("messages");

		char			*format,
					buffer[BUFSIZ];


		format = va_arg(ap, char *);
		if (fp) {
			setbuf (fp, buffer);
			vfprintf (fp, format, ap);
			fputs ("\n", fp);
			close_logfile (fp);
		}
	}
	va_end (ap);
	return;
}
#endif

#if	!defined(I_AM_LPSCHED_JR)

chk_clients()
{
    int		found = 0;
    int		i;
    short	size;

    for (i = 0; i < MaxClients; i++)
	if (Client_List[i].state == CLIENT_TALKING)
	{
	    if ((size = _putmessage(Response, I_QUEUE_CHK)) > 0)
		if (! _send (Client_List + i, Response, (unsigned)size, 1, 0))
		{
#if	defined(DEBUG)
		    if (debug)
			log_message_fact(
		    "UHOH: need client slot, found client %d deceased",
			    i
			);
#endif
		    found++;
		}
	}
    return(found);
}

/*
**	The following function is an adaptation of the libc function
**	system(3C).  The main difference being that in the child we
**	set the signals to their defaults, close all file descriptors,
**	and attach stdin, stdout, and stderr to /dev/null.  All of this
**	is to accomodate brain-damaged shells that are foolish enough to
**	make assumptions about the state of the world.  Specifically,
**	the pre-Release 3.1 shells expect that SIGCLD will be SIG_DFL.
**	If it is not, the shell will hang.
*/

static char shell[] = "sh";
static char shflg[]= "-c";

int
System(s)
char    *s;
{
        int     status, pid, w;
        void (*istat)(), (*qstat)();

        if((pid = fork()) == 0) {
		for (w = 0; w < OpenMax; w++)
		    (void) close(w);
		for (w = 0; w < NSIG; w++)
		    (void) signal(w, SIG_DFL);
		setpgrp();
		w = open("/dev/null", O_RDWR, 0);
		dup(w);
		dup(w);
                (void) execl(SHELL, shell, shflg, s, (char *)0);
                _exit(127);
        }
        istat = signal(SIGINT, SIG_IGN);
        qstat = signal(SIGQUIT, SIG_IGN);
        while((w = wait(&status)) != pid && w != -1)
                ;
        (void) signal(SIGINT, istat);
        (void) signal(SIGQUIT, qstat);
        return((w == -1)? w: status);
}

#endif		/* !defined(I_AM_LPSCHED_JR) */
