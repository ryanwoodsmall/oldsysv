/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpsched/logmsg.c	1.9"
/**
 ** logmsg[] - ARRAY OF LOG MESSAGES
 **/

char			*logmsg[] = {
	0,

/* MEMORY_ALLOC_FAILED */
	"Memeory allocation failed!\n",

/* NEED_IPC_PKG */
	"Print services need Inter-Process Communication Utilities!\n",

/* MSG_IPC_ALLOC_FAILED */
	"Failed to allocate message queue, semaphore (%s)!\n",

/* MSG_FIFO_ALLOC_FAILED */
	"Failed to allocate message fifo (%s)!\n",

/* CANNOT_DETERMINE_USERNAME */
	"Can't determine who you are!\n",

/* NOT_LP_ADMIN */
	"You must be \"lp\" or \"root\" to run this program.\n",

/* CANNOT_OPEN_FILE */
	"Can't open file \"%s\" (%s).\n",

/* READ_FILE_FAILED */
	"Had trouble reading file \"%s\".",

/* LPSCHED_ALREADY_ACTIVE */
	"Print services already active.\n",

/* FILE_LOCK_FAILED */
	"Failed to lock the file \"%s\" (%s).\n",

/* PRINT_SERVICES_STARTED */
	"Print services started.\n",

/* PRINT_SERVICES_STOPPED */
	"Print services stopped.\n",

/* LP_USAGE_MESSAGE */
#if	defined(DEBUG)
	"usage: lpsched [ options ]\n\
	[ -d ]			(debug mode, log execs, log messages)\n\
	[ -s ]			(don't trap most signals)\n\
	[ -f #filter-slots ]	(increase no. concurrent slow filters)\n\
	[ -n #notify-slots ]	(increase no. concurrent notifications)\n\
	[ -r #reserved-fds ]	(increase margin of file descriptors)\n\
\n\
WARNING: all these options are currently unsupported\n",
#else
	"usage: lpsched [ options ]\n\
	[ -f #filter-slots ]	(increase no. concurrent slow filters)\n\
	[ -n #notify-slots ]	(increase no. concurrent notifications)\n\
	[ -r #reserved-fds ]	(increase no. spare file descriptors)\n\
\n\
WARNING: all these options are currently unsupported\n",
#endif

/* NO_LP_ADMIN */
	"Can't find the user \"lp\" on this system!\n",

/* EXEC_FAILED */
	"Failed to exec child process (%s).\n",

/* EXEC_NOPEN */
	"Failed to open a print service file (%s).\n",

/* CHILD_FORK_FAILED */
	"Failed to fork child process (%s).\n",

/* BAD_EXIT_FROM_EXEC */
	"Bad exit from exec() for printer %s: %d\n",

/* BAD_EXIT_FROM_INTERFACE */
	"Bad exit from interface program for printer %s: %d\n",

/* RECEIVED_SIGNAL */
	"Received unexpected signal %d, terminated\n",

/* GARBAGE_IN_FIFO */
	"WARNING: %d bytes of garbage (%d bad messages) discarded from\n         the print service message pipe, %s\n",

/* ILLEGAL_OPTION */
	"%s: illegal option -- %c\n",

};
