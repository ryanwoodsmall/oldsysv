/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:include/msgs.h	1.4"
/*
 * THE DISPATCH TABLE DEPENDS ON EACH R_... MESSAGE FOLLOWING
 * IMMEDIATELY AFTER ITS CORRESPONDING S_... COUNTERPART.
 * I.E R_... MESSAGE FOR A S_... MESSAGE IS (S_... + 1)
 */
# define	R_BAD_MESSAGE			0
# define	S_NEW_QUEUE			1
# define	R_NEW_QUEUE			2
# define	S_ALLOC_FILES			3
# define	R_ALLOC_FILES			4
# define	S_PRINT_REQUEST			5
# define	R_PRINT_REQUEST			6
# define	S_START_CHANGE_REQUEST		7
# define	R_START_CHANGE_REQUEST		8
# define	S_END_CHANGE_REQUEST		9
# define	R_END_CHANGE_REQUEST		10
# define	S_CANCEL_REQUEST		11
# define	R_CANCEL_REQUEST		12
# define	S_INQUIRE_REQUEST		13
# define	R_INQUIRE_REQUEST		14
# define	S_LOAD_PRINTER			15
# define	R_LOAD_PRINTER			16
# define	S_UNLOAD_PRINTER		17
# define	R_UNLOAD_PRINTER		18
# define	S_INQUIRE_PRINTER_STATUS	19
# define	R_INQUIRE_PRINTER_STATUS	20
# define	S_LOAD_CLASS			21
# define	R_LOAD_CLASS			22
# define	S_UNLOAD_CLASS			23
# define	R_UNLOAD_CLASS			24
# define	S_INQUIRE_CLASS			25
# define	R_INQUIRE_CLASS			26
# define	S_MOUNT				27
# define	R_MOUNT				28
# define	S_UNMOUNT			29
# define	R_UNMOUNT			30
# define	S_MOVE_REQUEST			31
# define	R_MOVE_REQUEST			32
# define	S_MOVE_DEST			33
# define	R_MOVE_DEST			34
# define	S_ACCEPT_DEST			35
# define	R_ACCEPT_DEST			36
# define	S_REJECT_DEST			37
# define	R_REJECT_DEST			38
# define	S_ENABLE_DEST			39
# define	R_ENABLE_DEST			40
# define	S_DISABLE_DEST			41
# define	R_DISABLE_DEST			42
# define	S_LOAD_FILTER_TABLE		43
# define	R_LOAD_FILTER_TABLE		44
# define	S_UNLOAD_FILTER_TABLE		45
# define	R_UNLOAD_FILTER_TABLE		46
# define	S_LOAD_PRINTWHEEL		47
# define	R_LOAD_PRINTWHEEL		48
# define	S_UNLOAD_PRINTWHEEL		49
# define	R_UNLOAD_PRINTWHEEL		50
# define	S_LOAD_USER_FILE		51
# define	R_LOAD_USER_FILE		52
# define	S_UNLOAD_USER_FILE		53
# define	R_UNLOAD_USER_FILE		54
# define	S_LOAD_FORM			55
# define	R_LOAD_FORM			56
# define	S_UNLOAD_FORM			57
# define	R_UNLOAD_FORM			58
# define	S_RFS_MOUNT			59
# define	R_RFS_MOUNT			60
# define	S_QUIET_ALERT			61
# define	R_QUIET_ALERT			62
# define	S_SEND_FAULT			63
# define	R_SEND_FAULT			64
# define	S_SHUTDOWN			65
# define	R_SHUTDOWN			66
# define	S_GOODBYE			67
# define	S_CHILD_DONE			68

/*
**      LAST_UMESSAGE is the last message the user can get their hands on.
**      LAST_IMESSAGE is the last message the scheduler can get its hands on.
**      The messages in between these are used internally by the spooler.
*/
# define	LAST_UMESSAGE			68

# define	I_GET_TYPE			69
# define	I_QUEUE_CHK			70

# define	LAST_IMESSAGE			70

/*
**      These are the possible status codes returned by the scheduler
*/
# define	MOK		 0
# define	MOKMORE		 1
# define	MOKREMOTE	 2
# define	MMORERR		 3
# define	MNODEST		 4
# define	MERRDEST	 5
# define	MDENYDEST	 6
# define	MNOMEDIA	 7
# define	MDENYMEDIA	 8
# define	MNOFILTER	 9
# define	MNOINFO		10
# define	MNOMEM		11
# define	MNOMOUNT	12
# define	MNOOPEN		13
# define	MNOPERM		14
# define	MNOSTART	15
# define	MUNKNOWN	16
# define	M2LATE		17
# define	MNOSPACE	18
# define	MBUSY		19

/*
**      Define some masks associated with opening/creating things
**      
**      U_MSG_MODE	-  Permissions for users message queue
**      S_MSG_MODE	-  Permissions on lpsched's message queue
**      S_SEM_MODE	-  Permissions on lpsched's public semaphore
*/
# define	U_MSG_MODE	0600
# define	S_MSG_MODE	0622
# define	S_SEM_MODE	0622

/*
**      BASE_MSG_SIZE(x)  returns the amount of memory needed to hold
**                        a message of size <x> when various system
**                        information is added.
*/
# define	BASE_MSG_SIZE(x)	(sizeof(struct msgbuf) + (unsigned)(x))

/**
 ** Offsets and lengths of the various elements of the message header.
 **/

/*
 * Don't be fooled by the word ``level'' in the following comments.
 * The lower levels know about the structure at the higher level,
 * although not vice versa. Thus if you change somthing in a higher
 * level protocol, check the code for the levels below it!
 */

/*
 * Lowest level: Message transport (fifo, IPC, etc.)
 */
#define	HEAD_RESYNC		(0)
#define HEAD_RESYNC_LEN			2
#define HEAD_MIDDLE_START	(HEAD_RESYNC_LEN)

/*
 * Middle level: Message msend/mrecv
 */
#define HEAD_AUTHCODE		(HEAD_MIDDLE_START)
#define HEAD_AUTHCODE_LEN		(sizeof(short) + sizeof(long))
#define HEAD_HIGHEST_START	(HEAD_AUTHCODE + HEAD_AUTHCODE_LEN)

/*
 * Top level: Message construction/parsing.
 */
#define HEAD_SIZE		(HEAD_HIGHEST_START)
#define HEAD_SIZE_LEN			4
#define HEAD_TYPE		(HEAD_SIZE + HEAD_SIZE_LEN)
#define HEAD_TYPE_LEN			4
#define HEAD_DATA		(HEAD_TYPE + HEAD_TYPE_LEN)

#define HEAD_LEN		HEAD_DATA

#define	TAIL_ENDSYNC_LEN		2
#define	TAIL_ENDSYNC(N)		(N - TAIL_ENDSYNC_LEN)
#define TAIL_CHKSUM_LEN			4
#define TAIL_CHKSUM(N)		(TAIL_ENDSYNC(N) - TAIL_CHKSUM_LEN)

#define	TAIL_LEN		(TAIL_CHKSUM_LEN + TAIL_ENDSYNC_LEN)

#define	CONTROL_LEN		(HEAD_LEN + TAIL_LEN)

/**
 ** Checksum:
 **/
#define CALC_CHKSUM(B,SZ,RC) \
if (SZ >= CONTROL_LEN) { \
	register unsigned char	*p = (unsigned char *)B, \
				*pend = p + SZ - TAIL_LEN; \
	RC = 0; \
	while (p < pend) \
		RC += *p++;  /* let it overflow */ \
} else \
	return ((errno = EINVAL, -1))

/*
**      Largest size permitted for any given message
*/
# define	MSGMAX		2048

/*
**      Possible values of the type field of S_QUIET_ALERT
*/
# define	QA_FORM		1
# define	QA_PRINTER	2
# define	QA_PRINTWHEEL	3

/*
**      Definitions for the rest of the world and lint
*/
extern int	mopen();
extern int	mclose();
extern int	msend(/* char * */);
extern int	mrecv(/* char *, int */);
extern int	nudge_sched();
extern int	getmessage(/* char*, short, ... */);
extern int	putmessage(/* char*, short, ... */);
extern short	stoh(/* char* */);
extern long	stol(/* char* */);
extern char	*htos(/* char*, short */);
extern char	*ltos(/* char*, long */);
