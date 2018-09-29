/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:include/lp.h	1.10"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

/**
 ** Types:
 **/

typedef struct SCALED {
	float		val; /* value of number, scaled according to "sc" */
	char		sc;  /* 'i' inches, 'c' centimeters, ' ' lines/cols */
}			SCALED;

typedef struct FALERT {
	char		*shcmd; /* shell command used to perform the alert */
	int		Q;	/* # requests queued to activate alert */
	int		W;	/* alert is sent every "W" minutes */
}			FALERT;

/**
 ** Places:
 **/

#if	!defined(SPOOLDIR)
#define SPOOLDIR	"/usr/spool/lp"
#endif

#if	!defined(PUBDIR)
#define PUBDIR		"/usr/spool/lppublic"
#endif

#if	!defined(TERMINFO)
#define	TERMINFO	"/usr/lib/terminfo"
#endif

#define	LPUSER		"lp"
#define ROOTUSER	"root"

#define	ADMINSDIR	"admins"
# define CLASSESDIR	"classes"
# define FORMSDIR	"forms"
# define INTERFACESDIR	"interfaces"
# define PRINTERSDIR	"printers"
# define PRINTWHEELSDIR	"pwheels"
#define BINDIR		"bin"
#define LOGSDIR		"logs"
#define MODELSDIR	"model"
#define NETWORKDIR	"network"
#define FIFOSDIR	"fifos"
# define PRIVFIFODIR	"private"
# define PUBFIFODIR	"public"
#define REQUESTSDIR	"requests"
#define SYSTEMDIR	"system"
#define TEMPDIR		"temp"

#define SCHEDLOCK	"SCHEDLOCK"
#define FIFO		"FIFO"

#define	FILTERTABLE	"filter.table"
#define	FILTERTABLE_I	"filter.table.i"

#define DESCRIBEFILE	"describe"
#define ALIGNFILE	"align_ptrn"
#define COMMENTFILE	"comment"
#define ALLOWFILE	"allow"
#define DENYFILE	"deny"
#define ALERTSHFILE	"alert.sh"
#define ALERTVARSFILE	"alert.vars"
#define CONFIGFILE	"configuration"
#define FALLOWFILE	"forms.allow"
#define FDENYFILE	"forms.deny"
#define UALLOWFILE	"users.allow"
#define UDENYFILE	"users.deny"
#define DEFAULTFILE	"default"
#define STATUSFILE	"status"
#define USERSFILE	"users"
#define NAMEFILE	"name"
#define XFERFILE	"transfer"
#define EXECFILE	"execute"
#define PSTATUSFILE	"pstatus"
#define CSTATUSFILE	"cstatus"
#define ALERTPROTOFILE	"alert.proto"
#define REQLOGFILE	"requests"

#define STANDARD	"standard"
#define SLOWFILTER	"slow.filter"
#define LPSCHED_JR	"lpsched.jr"

/**
 ** Names and phrases:
 **/

/*
 * If you change these from macros to defined (char *) strings,
 * be aware that in several places the lengths of the strings
 * are computed using "sizeof()", not "strlen()"!
 */
#define	NAME_ALL	"all"
#define	NAME_ANY	"any"
#define NAME_NONE	"none"
#define	NAME_TERMINFO	"terminfo"
#define	NAME_SIMPLE	"simple"
#define NAME_HOLD	"hold"
#define	NAME_RESUME	"resume"
#define NAME_IMMEDIATE	"immediate"
#define NAME_CONTINUE	"continue"
#define NAME_BEGINNING	"beginning"
#define NAME_WAIT	"wait"
#define NAME_MAIL	"mail"
#define	NAME_WRITE	"write"
#define NAME_QUIET	"quiet"
#define NAME_LIST	"list"
#define NAME_ON		"on"
#define NAME_OFF	"off"
#define NAME_ALWAYS	"Always"
#define NAME_UNKNOWN	"unknown"
#define NAME_RFS	"rfs"
#define NAME_REJECTING	"rejecting"
#define NAME_ACCEPTING	"accepting"
#define NAME_DISABLED	"disabled"
#define NAME_ENABLED	"enabled"
#define NAME_DIRECT	"direct"
#define NAME_PICA	"pica"
#define NAME_ELITE	"elite"
#define NAME_COMPRESSED	"compressed"
#define NAME_ALLOW	"allow"
#define NAME_DENY	"deny"
#define NAME_ONCE	"once"

/**
 ** Common messages:
 **/

#define CUZ_NEW_PRINTER		"new printer"
#define CUZ_NEW_DEST		"new destination"
#define CUZ_STOPPED		"stopped with printer fault"
#define CUZ_FAULT		"printer fault"
#define CUZ_LOGIN_PRINTER	"disabled by Spooler: login terminal"
#define CUZ_MOUNTING		"mounting a form"
#define CUZ_NOFORK		"can't fork"

#define TIMEOUT_FAULT \
"Timed-out trying to open the printer port.\n"

#define OPEN_FAULT \
"Failed to open the printer port.\n"

/*
 * Currently, this message is marked to identify which process detected
 * the hangup:
 *
 *	. - lp.cat
 *	? - standard interface (but not lp.cat)
 *	! - the Spooler
 */
#define HANGUP_FAULT \
"The connection to the printer dropped; perhaps the printer went off-line!\n"

#define INTERRUPT_FAULT	\
"Received an interrupt from the printer. The reason is unknown,\nalthough a common cause is that the baud rate is too high.\n"

#define PIPE_FAULT \
"The output ``port'', a FIFO, was closed before all output was written.\n"

#define EXIT_FAULT \
"The interface program returned with a reserved exit code.\n"

/**
 ** Lp-errno #defines, etc.
 **/

#define LP_EBADSDN	1
#define LP_EBADINT	2
#define LP_EBADNAME	3
#define LP_EBADARG	4
#define LP_ETRAILIN	5
#define LP_ENOCMT	6
#define LP_EBADCTYPE	7
#define LP_ENOALP	8
#define LP_ENULLPTR	9
#define LP_EBADHDR	10
#define LP_ETEMPLATE	11
#define LP_EKEYWORD	12
#define LP_EPATTERN	13
#define LP_ERESULT	14
#define LP_ENOMEM	99

extern int		lp_errno;

/**
 ** Misc. Macros
 **/

#define	LP_WS		" "	/* Whitespace (also list separator) */
#define	LP_SEP		","	/* List separator */
#define LP_QUOTES	"'\""

#define MAIL		"mail"
#define WRITE		"write"

#define STATUS_BREAK	"=========="

#define	STREQU(A,B)	(strcmp((A), (B)) == 0)
#define	STRNEQU(A,B,N)	(strncmp((A), (B), (N)) == 0)
#define	CS_STREQU(A,B)	(cs_strcmp((A), (B)) == 0)
#define	CS_STRNEQU(A,B,N)	(cs_strncmp((A), (B), (N)) == 0)
#define STRSIZE(X)	(sizeof(X) - 1)

/*
 * Almost STREQU but compares null pointers as equal, too.
 */
#define	SAME(A,B)	((A) == (B) || (A) && (B) && STREQU((A), (B)))

#define	PRINTF		(void)printf
#define SPRINTF		(void)sprintf
#define FPRINTF		(void)fprintf

#define	NB(X)		(X? X : "")

#define	WHEN		((
#define	USE		)?(
#define	OTHERWISE	):(
#define	NEHW		))

#define PERROR		(errno < sys_nerr? sys_errlist[errno] : "unknown")

/**
 ** Alert macros:
 **/

/*
 * Type of alert to be S_QUIET'd
 */
#define	QA_FORM		1
#define	QA_PRINTER	2
#define	QA_PRINTWHEEL	3

/**
 ** File modes:
 ** (The "NO" prefix is relative to ``others''.)
 **/

#define	MODE_READ	0664
#define MODE_NOREAD	0660
#define MODE_EXEC	0775
#define MODE_NOEXEC	0770
#define MODE_DIR	0775
#define MODE_NODIR	0770

/**
 ** Routines:
 **/

/*
 * File access:
 */

#if	defined(BUFSIZ)
extern FILE		*open_lpfile();

extern int		close_lpfile(),
			chown_lppath(),
			mkdir_lpdir(),
			rmfile(),
			dumpstring();
#define dumpline(X)	dumpstring(X)

extern char		*loadstring(),
			*loadline(),
			*sop_up_rest();
#endif

/*
 * List manipulation routines:
 */

#define search_list	searchlist

extern int		addlist(),
			dellist(),
			lenlist(),
			printlist(),
			searchlist(),
			printlist_qsep;

extern char		**duplist(),
			**getlist(),
			**dashos(),
			*sprintlist(),
			**wherelist(),
			*search_cslist();

extern void		freelist(),
			printlist_setup(),
			printlist_unsetup();

/*
 * Scaled decimal number routines:
 */

extern void		printsdn(),
			printsdn_setup(),
			printsdn_unsetup();

/*
 * File name routines:
 */

extern char		*makepath(),
			*getspooldir(),
			*getrequestfile(),
			*getprinterfile(),
			*getclassfile(),
			*getfilterfile();

extern void		getpaths(),
			getadminpaths();

extern char		*Lp_Spooldir,
			*Lp_Admins,
			*Lp_Bin,
			*Lp_FIFO,
			*Lp_Logs,
			*Lp_ReqLog,
			*Lp_Model,
			*Lp_Private_FIFOs,
			*Lp_Public_FIFOs,
			*Lp_Requests,
			*Lp_Schedlock,
			*Lp_System,
			*Lp_Temp,
			*Lp_Users,
			*Lp_Slow_Filter,
			*Lp_Lpsched_Jr,
			*Lp_A,
			*Lp_A_Classes,
			*Lp_A_Forms,
			*Lp_A_Interfaces,
			*Lp_A_Logs,
			*Lp_A_Printers,
			*Lp_A_PrintWheels,
			*Lp_A_Filters;

/*
 * Additional string manipulation routines:
 */

extern int		cs_strcmp(),
			cs_strncmp();

/*
 * Syntax checking routines:
 */

extern int		syn_name(),
			syn_text(),
			syn_comment(),
			syn_machine_name(),
			syn_option();

/*
 * Alert management routines:
 */

extern int		putalert(),
			delalert();

extern FALERT		*getalert();

extern void		printalert();

/*
 * Terminfo Database Inquiry Tool
 */

extern int		tidbit();

/*
 * Auto-restarting system calls:
 * The two versions are here to reduce the chance of colliding
 * with similar names in standard libraries (e.g. dial(3C) uses
 * Read/Write).
 */

extern int		_Access(),
			_Chdir(),
			_Chmod(),
			_Chown(),
			_Close(),
			_Creat(),
			_Fcntl(),
			_Fstat(),
			_Link(),
			_Mknod(),
			_Open(),
			_Read(),
			_Stat(),
			_Unlink(),
			_Wait(),
			_Write();

#define Access	_Access
#define Chdir	_Chdir
#define Chmod	_Chmod
#define Chown	_Chown
#define Close	_Close
#define Creat	_Creat
#define Fcntl	_Fcntl
#define Fstat	_Fstat
#define Link	_Link
#define Mknod	_Mknod
#define Open	_Open
#define Read	_Read
#define Stat	_Stat
#define Unlink	_Unlink
#define Wait	_Wait
#define Write	_Write

/*
 * Misc. routines:
 */

extern int		isterminfo(),
			isprinter(),
			isrequest(),
			isnumber();

extern char		*getname(),
			*makestr(),
			*strip();

extern void		sendmail();

extern char		*sys_errlist[];

extern int		sys_nerr,
			errno;

/**
 ** malloc()/free() trace routines:
 **/

#if	defined(NEW_TRACE_MALLOC)

extern char		*malloc(),
			*(*p_malloc)(),
			*realloc(),
			*(*p_realloc)(),
			*strdup(),
			*(*p_strdup)();

extern void		free(),
			(*p_free)(),
			mark_m_use();

#define	malloc	(mark_m_use(__FILE__,__LINE__,"malloc"),  (*p_malloc))
#define	realloc	(mark_m_use(__FILE__,__LINE__,"realloc"), (*p_realloc))
#define	free	(mark_m_use(__FILE__,__LINE__,"free"),    (*p_free))
#define	strdup	(mark_m_use(__FILE__,__LINE__,"strdup"),  (*p_strdup))

#endif
