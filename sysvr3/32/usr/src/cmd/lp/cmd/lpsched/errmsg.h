/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpsched/errmsg.h	1.7"
#define	MEMORY_ALLOC_FAILED		1
#define	NEED_IPC_PKG			2
#define	MSG_IPC_ALLOC_FAILED		3
#define	MSG_FIFO_ALLOC_FAILED		4
#define	CANNOT_DETERMINE_USERNAME	5
#define	NOT_LP_ADMIN			6
#define	CANNOT_OPEN_FILE		7
#define READ_FILE_FAILED		8
#define	LPSCHED_ALREADY_ACTIVE		9
#define	FILE_LOCK_FAILED		10
#define	PRINT_SERVICES_STARTED		11
#define	PRINT_SERVICES_STOPPED		12
#define	LP_USAGE_MESSAGE		13
#define	NO_LP_ADMIN			14
#define	EXEC_FAILED			15
#define	EXEC_NOPEN			16
#define	CHILD_FORK_FAILED		17
#define	BAD_EXIT_FROM_EXEC		18
#define	BAD_EXIT_FROM_INTERFACE		19
#define	RECEIVED_SIGNAL			20
#define	GARBAGE_IN_FIFO			21
#define ILLEGAL_OPTION			22

extern char		*logmsg[];
