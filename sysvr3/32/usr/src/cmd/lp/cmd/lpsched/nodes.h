/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpsched/nodes.h	1.8"
typedef struct alert_node	ALERT;
typedef struct cstat_node	CSTATUS;
typedef struct exec_node	EXEC;
typedef struct flt_node		FLT;
typedef struct form_node	_FORM;
typedef struct fstat_node	FSTATUS;
typedef struct pstat_node	PSTATUS;
typedef struct pwstat_node	PWSTATUS;
#if	defined(USE_FIFOS)
typedef struct mque_node	MQUE;
typedef	struct client_node	NODE;
#else
typedef	struct que_node		NODE;
#endif
typedef struct rstat_node	RSTATUS;
typedef struct secure_node	SECURE;
struct alert_node
{
    short	active;			/* Non-zero if triggered     */
    EXEC	*exec;			/* Index into EXEC table     */
    char	*msgfile;
};

struct cstat_node
{
    short	status;
    char	*rej_reason;
    time_t	rej_date;
    CLASS	*class;
};

struct exec_node
{
    int		pid;			/* process-id of exec		*/
    int		status;			/* low order bits from wait	*/
    long	key;			/* private key for security	*/
    short	errno;			/* copy of child's errno	*/
    short	type;			/* type of exec, EX_...		*/
    ushort	flags;			/* flags, EXF_...		*/
    union ex
    {
	RSTATUS		*request;
	FSTATUS		*form;
	PWSTATUS	*pwheel;
	PSTATUS		*printer;
    } ex;
};

#define	EX_INTERF	1	/* exec interface for ex.printer	*/
#define	EX_SLOWF	2	/* exec slow filter for ex.request	*/
#define	EX_ALERT	3	/* exec alert for ex.printer		*/
#define	EX_FALERT	4	/* exec alert for ex.form		*/
#define	EX_PALERT	5	/* exec alert for ex.pwheel		*/
#define	EX_NOTIFY	6	/* exec notification for ex.request	*/

#define	EXF_RESTART	0x0001	/* restart the exec			*/
#define	EXF_KILLED	0x0002	/* terminate() has killed the exec	*/
#if	defined(CHECK_CHILDREN)
#define	EXF_GONE	0x0004	/* child has disappeared		*/
#endif	/* CHECK_CHILDREN */

struct flt_node
{
    int type;			/* type of ``fault'', FLT_...		*/
    FLT *next;
    
    /*
    **    These variables are used in do_flt_acts().  Variables may be added
    **    as needed.  Be sure to update free_flt() if you add or delete any
    **    variables from this structure.
    */

    char        *s1;
    int         i1;
    RSTATUS     *r1;
    PSTATUS     *p1;
};

/*
**	Possible values for <type>
*/
#define        FLT_FILES       1	/* remove alloc'd files		*/
#define        FLT_CHANGE      2	/* clear RS_CHANGING for .r1	*/

struct fstat_node
{
    _FORM	*form;
    ALERT	*alert;
    short	requests;		/* Number of events thus far */
    short	requests_last;		/* # when alert last sent */
    short	trigger;		/* Trigger when this value   */
    short	mounted;		/* # times currently mounted */
    char	**users_allowed;
    char	**users_denied;
    char	*cpi;
    char	*lpi;
    char	*plen;
    char	*pwid;
};

struct pstat_node
{
    short	status;			/* Current Status of printer */
    RSTATUS	*request;
    PRINTER	*printer;
    ALERT	*alert;
    EXEC	*exec;
    FSTATUS	*form;
    char	*pwheel_name;
    PWSTATUS	*pwheel;
    char	*dis_reason;
    char	*rej_reason;
    char	**users_allowed;
    char	**users_denied;
    char	**forms_allowed;
    char	**forms_denied;
    char	*cpi;
    char	*lpi;
    char	*plen;
    char	*pwid;
    time_t	dis_date;
    time_t	rej_date;
    short	last_dial_rc;		/* last exit from dial() */
    short	nretry;			/* number of dial attempts */
};

struct pwstat_node
{
    PWHEEL	*pwheel;
    ALERT	*alert;
    short	requests;
    short	requests_last;		/* # when alert last sent */
    short	trigger;
    short	mounted;
};

#if	defined(USE_FIFOS)

#define CLIENT_NEW		1
#define CLIENT_PROTOCOL		2
#define CLIENT_TALKING		3

struct mque_node
{
    char		*msgbuf;
    unsigned int	size;
    struct mque_node	*next;
};

struct client_node
{
    ushort		uid;		/* User id of client		*/
    ushort		gid;		/* Group id of client		*/
    char		admin;		/* Non-zero if client is admin	*/
    char		state;		/* (CLIENT_... value)		*/
    FLT			*flt;		/* Linked list of fault actions */
    char		*fifo;		/* Name of client's fifo	*/
    char		*system;	/* Name of client's system	*/
    int			fd;		/* File-descriptor of open fifo	*/
    char		authcode[HEAD_AUTHCODE_LEN];
					/* ``Password'' to verify client*/
    MQUE		*mque;		/* Linked list of pending msgs.	*/
};

#else

struct que_node
{
    ushort		uid;		/* User id of user             */
    ushort		gid;		/* Group id of user            */
    char		admin;		/* Non-zero if user is admin   */
    char		master;		/* Non-zero if primary queue   */
    FLT			*flt;		/* Linked list of fault actions */
    int			ident;		/* Message queue identifier    */
    char		*system;	/* Name of client's system	*/
    int			pid;		/* Process id of originator    */
    NODE		*next;
    NODE		*prev;
};

#endif

struct rstat_node
{
    char	*req_file;
    char	*slow;
    char	*fast;
    short	copies;		/* # copies interface is to make */    
    short	reason;		/* reason for failing _validate() */

    SECURE	*secure;
    REQUEST	*request;
    PSTATUS	*printer;
    FSTATUS	*form;
    char	*pwheel_name;
    PWSTATUS	*pwheel;
    EXEC	*exec;		/* Pointer to running filter or notify */

    RSTATUS	*next;
    RSTATUS	*prev;
};

struct secure_node
{
    ushort	uid;
    ushort	gid;
    off_t	size;
    time_t	date;
    char	*system;	/* set only if not local user */
    char	*user;
    char	*req_id;
};

struct form_node
{
    SCALED	plen;
    SCALED	pwid;
    SCALED	lpi;
    SCALED	cpi;
    int	np;
    char	*chset;
    short	mandatory;
    char	*rcolor;
    char	*comment;
    char	*conttype;
    char	*name;
    FALERT	alert;
};

#define BEGIN_WALK_LOOP(PRS, CONDITION) \
	{ \
		register RSTATUS	*pnext; \
		for (PRS = Request_List; PRS; PRS = pnext) { \
			pnext = PRS->next; \
			if (CONDITION) {

#define	END_WALK_LOOP \
			} \
		} \
	}

#define BEGIN_WALK_BY_PRINTER_LOOP(PRS, PPS)	\
	BEGIN_WALK_LOOP(PRS, PRS->printer == PPS)

#define BEGIN_WALK_BY_DEST_LOOP(PRS, DEST) \
	BEGIN_WALK_LOOP(PRS, STREQU(PRS->request->destination, DEST))

#define BEGIN_WALK_BY_FORM_LOOP(PRS, PFS) \
	BEGIN_WALK_LOOP(PRS, PRS->form == PFS)

#define	BEGIN_WALK_BY_PWHEEL_LOOP(PRS, NAME) \
	BEGIN_WALK_LOOP(PRS, PRS->pwheel_name && STREQU(PRS->pwheel_name, NAME))
