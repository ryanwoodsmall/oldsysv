/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpsched/lpsched.h	1.9"

#include "stdio.h"
#include "sys/types.h"
#include "memory.h"
#include "string.h"
#include "pwd.h"
#include "fcntl.h"
#include "errno.h"
#include "signal.h"
#include "unistd.h"

#include "lp.h"
#include "form.h"
#include "requests.h"
#include "filters.h"
#include "printers.h"
#include "class.h"
#include "users.h"
#include "msgs.h"

#include "nodes.h"
#include "errmsg.h"

/**
 ** Defines:
 **/

#if	defined(LPSCHED_JR)
# if	!defined(CHECK_CHILDREN)
#  define	CHECK_CHILDREN
# endif
#endif

/*
 * These are the fields in the ``secure'' request files,
 * found in the REQUESTDIR directory.
 */

#define SC_MAX	6
# define SC_REQID	0
# define SC_UID		1
# define SC_USER	2
# define SC_GID		3
# define SC_SIZE	4
# define SC_DATE	5

/*
 * These are the fields in the PSTATUS and CSTATUS files,
 * found in the SYSTEM directory.
 */

#define PST_MAX	8
# define PST_BRK	0
# define PST_NAME	1
# define PST_STATUS	2
# define PST_DATE	3
# define PST_DISREAS	4
# define PST_REJREAS	5
# define PST_PWHEEL	6
# define PST_FORM	7

#define CST_MAX	5
# define CST_BRK	0
# define CST_NAME	1
# define CST_STATUS	2
# define CST_DATE	3
# define CST_REJREAS	4

/*
 * Exit codes from child processes:
 *
 *    0 <= exit <= 0177 (127) are reserved for ``normal'' exits.
 * 0200 <= exit <= 0377 (255) are reserved for special failures.
 *
 * If bit 0200 is set, then we have three sets of special error
 * codes available, with 32 values in each set (except the first):
 *
 *	0201 - 0237	Printer faults
 *	0240 - 0277	Dial problems
 *	0300 - 0337	Port problems
 *	0340 - 0377	Exec problems
 *
 *	0200		Interface received SIGTERM
 */
#define EXEC_EXIT_OKAY	0	/* success */
#define EXEC_EXIT_USER	0177	/* user exit codes, 7 bits */
#define EXEC_EXIT_NMASK	0340	/* mask to uncover reason bits */
#define EXEC_EXIT_FAULT	0201	/* printer fault */
#define EXEC_EXIT_HUP	0202	/* got hangup early in exec */
#define EXEC_EXIT_INTR	0203	/* got interrupt early in exec */
#define EXEC_EXIT_PIPE	0204	/* got close of FIFO early in exec */
#define EXEC_EXIT_EXIT	0237	/* interface used reserved exit code */
#define EXEC_EXIT_NDIAL	0240	/* can't dial, low 5 bits abs(dial()) */
#define EXEC_EXIT_NPORT	0300	/* can't open port */
#define EXEC_EXIT_TMOUT	0301	/* can't open port in N seconds */
#define EXEC_EXIT_NOPEN	0340	/* can't open input/output file */
#define EXEC_EXIT_NEXEC	0341	/* can't exec */
#define EXEC_EXIT_NOMEM	0342	/* malloc failed */
#define EXEC_EXIT_NFORK	0343	/* fork failed, must try again */

/*
 * If killed, return signal, else 0.
 */
#define	KILLED(x) (!(x & 0xFF00)? (x & 0x7F) : 0)

/*
 * If exited, return exit code, else -1.
 */
#define	EXITED(x) (!(x & 0xFF)? ((x >> 8) & 0xFF) : -1)

/*
 * Events that can be scheduled:
 */
#define EV_SLOWF	1
#define	EV_INTERF	2
#define EV_NOTIFY	3
#define EV_LATER	4
#define EV_ALARM	5
#define	EV_MESSAGE	6
#if	defined(CHECK_CHILDREN)
#define	EV_CHECKCHILD	7
#endif	/* CHECK_CHILDREN */

/*
 * How long to wait before retrying an event:
 * (For best results, make CLOCK_TICK a factor of 60.)
 */
#define CLOCK_TICK	10		/* no. seconds between alarms	*/
#define MINUTE		(60/CLOCK_TICK)	/* number of ticks per minute	*/
#define WHEN_FORK	(MINUTE)	/* retry forking child process	*/
#define WHEN_PRINTER	(5*MINUTE)	/* retry faulted printer	*/
#define WHEN_MESSAGES	1		/* retry constipated client	*/
#if	defined(CHECK_CHILDREN)
#define	WHEN_CHECKCHILD	(1*MINUTE)
#endif	/* CHECK_CHILDREN */

/*
 * Alert types:
 */
#define	A_PRINTER	1
#define	A_PWHEEL	2
#define	A_FORM		3

/*
 * validate_request() - VERIFY REQUEST CAN BE PRINTED
 * evaluate_request() - TRY REQUEST ON A PARTICULAR PRINTER
 * reevaluate_request() - TRY TO MOVE REQUEST TO ANOTHER PRINTER
 */

#define validate_request(PRS,PREFIXP,MOVING) \
	_validate((PRS), (PSTATUS *)0, (PSTATUS *)0, (PREFIXP), (MOVING))

#define evaluate_request(PRS,PPS,MOVING) \
	_validate((PRS), (PPS), (PSTATUS *)0, (char **)0, (MOVING))

#define reevaluate_request(PRS,PPS) \
	_validate((PRS), (PSTATUS *)0, (PPS), (char **)0, 0)

/*
 * Request is ready to be slow-filtered:
 */
#define	NEEDS_FILTERING(PRS) \
	((PRS)->slow && !((PRS)->request->outcome & RS_FILTERED))

/*
 * Misc:
 */
#define	isadmin(ID)		(!(ID) || (ID) == Lp_Uid)
#define makereqerr(X)		makepath(Lp_Temp, getreqno(X), (char *)0)
#define	EVER			;;
#define	DEFAULT_SHELL		"/bin/sh"
#define	BINMAIL			"/bin/mail"
#define	BINWRITE		"/bin/write"

#if	defined(TRACE_MALLOC)
# define TRACE_ON(X)	trace_on(X)
# define TRACE_OFF(X)	trace_off()
#else
# define TRACE_ON(X)
# define TRACE_OFF(X)
#endif

/**
 ** External routines:
 **/

extern void		init_memory(),
			load_status(),
			dump_status(),
			dump_pstatus(),
			dump_cstatus(),
			add_flt_act(),
			do_flt_acts(),
			del_flt_act(),
			freeform(),
	                startup(),
			save_status(),
			alert(),
			notify(),
	                fail(),
	                process(),
	                lpshut(),
	                note(),
	                send(),
	                sendme(),
	                deallocr(),
	                freer(),
	                remover(),
	                insertr(),
	                background(),
			check_form_alert(),
			check_pwheel_alert(),
			queue_attract(),
			queue_check(),
			queue_form(),
			queue_pwheel(),
			unqueue_form(),
			unqueue_pwheel(),
			schedule(),
			rmfiles(),
			freerstatus(),
			check_request(),
			printer_fault(),
			dial_problem(),
			load_str(),
			load_sdn(),
			unload_str(),
			unload_list(),
			dowait(),
			terminate(),
			close_logfile(),
			qsort(),
	                free(),
	                exit();

extern short		_validate();

extern int		exec(),
			open_dialup(),
			open_direct(),
			Loadfilters(),
			putsecure(),
			qchk_form(),
			qchk_pwheel(),
			qchk_eligible(),
			qchk_waiting(),
			qchk_filter(),
			queue_repel(),
			one_printer_with_charsets(),
			cancel(),
			rsort();

extern long		getkey();

extern off_t		chfiles();

extern char		*getreqno(),
			*_alloc_files(),
			*malloc(),
	                *realloc(),
			*ctime();

extern FILE		*open_logfile();

extern _FORM		*Getform();

extern PRINTER		*Getprinter();

extern REQUEST		*Getrequest();

extern USER		*Getuser();

extern CLASS		*Getclass();

extern PWHEEL		*Getpwheel();

extern SECURE		*Getsecure(),
			*getsecure();

extern FSTATUS		*search_ftable(),
			*walk_ftable();

extern PSTATUS		*search_ptable(),
			*walk_ptable();

extern CSTATUS		*search_ctable(),
			*walk_ctable();

extern PWSTATUS		*search_pwtable(),
			*walk_pwtable();

extern RSTATUS		*allocr(),
	                *request_by_id(),
	                *request_by_pr(),
			*walk_req_by_printer(),
			*walk_req_by_form(),
			*walk_req_by_pwheel(),
			*walk_req_by_dest();

/*
 * Things that can't be passed as parameters:
 */

extern FSTATUS		*form_in_question;

extern char		*pwheel_in_question;

/**
 ** External tables, lists:
 **/

extern ALERT		*Alert_Table,	/* Printer fault alerts    */
			*FAlert_Table,	/* Form mount alerts       */
			*PAlert_Table;	/* PrintWheel mount alerts */

extern CLASS		*Class_Table;	/* Known classes           */

extern CSTATUS		*CStatus;	/* Status of classes       */

extern EXEC		*Exec_Table;	/* Running processes       */

extern _FORM		*Form_Table;	/* Known forms             */

extern FSTATUS		*FStatus;	/* Status of forms	   */

extern PRINTER		*Printer_Table;	/* Known printers          */

extern PSTATUS		*PStatus;	/* Status of printers      */

extern PWHEEL		*PrintWheel_Table;/* Known print wheels    */

extern PWSTATUS		*PWStatus;	/* Status of print wheels  */

extern RSTATUS		*Request_List,	/* Queue of print requests */
			*NewRequest;	/* Not in Request_List yet */

extern EXEC		*Exec_Slow,	/* First slow filter exec  */
			*Exec_Notify;	/* First notification exec */

extern int		CT_Size,	/* Size of class table		*/
			ET_Size,	/* Size of exec table		*/
			ET_SlowSize,	/* Number of filter execs  	*/
			ET_NotifySize,	/* Number of notify execs  	*/
			FT_Size,	/* Size of form table		*/
			PT_Size,	/* Size of printer table	*/
			PWT_Size;	/* Size of print wheel table	*/

#if	defined(DEBUG)
extern int		debug;
#endif

extern char		*Local_System,	/* Node name of local system	*/
			*SHELL;		/* value of $SHELL, or default	*/

extern FILE		*LockFp;

extern ushort		Lp_Uid,
			Lp_Gid;

extern int		Starting,
			OpenMax,
			Sig_Alrm,
			DoneChildren,
			am_in_background;

extern unsigned long	chkprinter_result;

#if defined(MDL)
#include	"mdl.h"
#endif /* MDL */
