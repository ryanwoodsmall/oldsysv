/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/comm.h	10.15"

#undef NOFILE
#define NOFILE 	v.v_nofiles

/*  parameters of this implementation  */

#define MINPKSIZE	64		/* minimum ethernet packet size */
#define	MAXNIBUF	(1024 + 64 * 2)	/*  size of the rcvq buffers	*/
#define	MSGBUFSIZE	MAXNIBUF	/*  the same as above		*/
#define DFRPORT		2		/*  default rcv port for machine*/
#define	PCOMM		PZERO		/*  priority of sleeping comm	*/
#define	NADDRLEN	6		/*  length of network address	*/
#define	RCVQEMP		-3		/*  receive queue empty		*/
#define NSQSIZE		10		/*  normal rcvd queue length	*/
#define SIGQSIZE	65535		/*  signal rcvd queue length	*/


#define	DU_UP	1	/* on network */
#define	DU_DOWN	0	/* not on network */
#define DU_INTER 2	/* RFS in an intermediate state */

#ifndef	TRUE
#define	TRUE	1
#define	FALSE	0
#endif

#ifndef SUCCESS
#define SUCCESS	1
#define	FAILURE	0	
#endif

#ifndef NULL
#define NULL	0
#endif


#define ROUND(X) ((X+3)&~3)	/*  round to full word  */
/* convert rcvd to an index */
#define	rdtoinx(x)	(x - rcvd)
/* convert index to a rcvd */
#define	inxtord(x)	(&rcvd[x])
/* is a receive queue empty? */
#define rcvdemp(RD) (RD->rd_rcvdq.qc_head == NULL)

/*  The network address is a NADDRLEN bytes long.
 */

typedef	struct  naddr  {
	ushort	length;
	char	address[NADDRLEN];
}	naddr_t;


typedef	int	csect_t;
#define	csect()		spl6()
#define	ecsect(s)	splx(s)
/*
 *  buffer queue control structure
 */

struct	bqctrl_st {
	struct	msgb *qc_head;		/* head pointer */
	struct	msgb *qc_tail;		/* tail pointer */
};


/*   receive descriptor structure
 */

typedef struct rcvd  {
	char	rd_stat;
	char	rd_qtype;		/*  unix - GENERAL or SPECIFIC	*/
	union	{
	struct	inode *rd_inodep;	/*  unix - pointer to inode	*/
	struct	file  *rd_filep;	/*  unix - pointer to file table*/
	}  r1;
	ushort	rd_act_cnt;		/*  unix - active server count	*/
	ushort	rd_max_serv;		/*  unix - maximum # servers	*/
	struct	rcvd *rd_next;		/*  unix - pointer to next rcvd	*/
	ushort	rd_qsize;		/*  recv queue size		*/
	ushort	rd_qcnt;		/*  how many msg queued		*/
	ushort	rd_refcnt;		/*  how many remote send desc	*/
	ushort	rd_connid;		/*  connection id 		*/
	struct	bqctrl_st rd_rcvdq;	/*  receive descriptor queue	*/
	struct	rd_user *rd_user_list;	/* one for each time RD is a gift */
	struct	sndd *rd_sdnack;	/* send desc list for NACK */
	sema_t	rd_qslp;		/*  recv desc queue sleep sema	*/
}	*rcvd_t;

#define rd_inode	r1.rd_inodep
#define	rd_file		r1.rd_filep

/* rd_stat */
#define	RDUSED		0x1
#define	RDUNUSED	0x2
#define	RDLINKDOWN	0x4

/* rd_qtype */
#define	GENERAL		0x1
#define SPECIFIC	0x2
#define RDLBIN		0x8
#define RDTEXT		0x40



/*  A send descriptor contains information to identify the destination
 *  of messages sent on this descriptor.
 *  It contains a pointer to the stream queue, which has a virtual circuit 
 *  setup to the remote side.
 *  The index tells the other side
 *  which receive descriptor queue to put messages in.  
 *  sd_refcnt is the refrence count of how many duplication of this
 *  send descriptor due to fork on the local machine.
 *  sd_mntindx is an index into the remote side's srmount table so that
 *  counts can be kept properly for umount.
 */

typedef struct sndd  {
	char	sd_stat;
	ushort	sd_refcnt;	/* how many duplication on local due to fork */
	index_t	sd_sindex;	/* index for the remote recv descriptor */
	ushort	sd_connid;	/* connect id for the remote recv descriptor*/
	index_t	sd_mntindx;	/* index to the remote srmount table */
	ushort	sd_copycnt;	/* how many copyout on this sd */
	struct	proc *sd_srvproc;	/* points to proc of dedicated server */
	struct	queue *sd_queue;	/* points to stream head queue 	*/
	struct	sndd *sd_next;
	int	sd_temp;		/* used for temp sd related oper */
	ushort	sd_mode;
}	*sndd_t;

#define	SDUSED		0x1
#define	SDUNUSED	0x2
#define	SDLINKDOWN	0x4
#define SDSERVE		0x8

extern	struct	sndd	sndd[];



/*
 * default size of receive queues.
 *	mount points will receive more
 *	simultaneous traffic than files.
 */
#define	MOUNT_QSIZE	10
#define	FILE_QSIZE	 5

extern	struct	rcvd	rcvd[];
/*
 *	Every time the refcnt of an RD is bumped, an rd_user
 *	structure is allocated to keep track of who it was
 *	bumped for and why.  This is the only way we have
 *	enough information to clean up after a crash.
 */

struct rd_user {
	struct	rd_user *ru_next;	/* next user */
	struct	queue *ru_queue;	/* which stream queue */
	index_t	ru_srmntindx;		/* which srmount entry */
	ushort ru_fcount;
	ushort ru_icount;
	ushort ru_frcnt;
	ushort ru_fwcnt;
};

extern struct rd_user rd_user[];


/*  
 *   The message structure is the header to every message.
 */

struct	message  {
	long	m_cmd;		/*  not used 				*/
	long	m_stat;		/*  see stat values below  		*/
	long	m_dest;		/*  destination index to which this goes*/
	long	m_connid;	/*  connection id			*/
	long	m_gindex;	/*  gift index				*/
	long	m_gconnid;	/*  gift connection id			*/
	long	m_size;		/*  size of this message		*/
	long	m_queue;	/*  queue message came in on 		*/
};

#define	PTOMSG(x)	(((struct message *)x) + 1)

/*  status values  */
#define GIFT	0x8		/*  are address & index real?	*/
#define SIGNAL	0x10		/*set for signal messages*/

/*  parameters of the implementation  */

extern	int	nrcvd; 		/*  number of receive descriptors*/
extern	int	nsndd;		/*  number of send descriptors	*/
extern	int	nrduser;	/*  number of rd_user entries	*/
extern  int	maxserve;
extern  int	minserve;

/*  subroutines available from the communications interface  */

extern	sndd_t	cr_sndd ();	/*  create a send descriptor		*/
extern		del_sndd ();	/*  delete a send descriptor		*/
extern	rcvd_t	cr_rcvd ();	/*  create a receive descriptor		*/
extern		del_rcvd ();	/*  delete a receive descriptor		*/
extern		sndmsg ();	/*  send a message			*/
extern	struct	msgb *alocbuf();/*  allocate a buffer to send later	*/
extern		freesbuf ();	/*  free a send buffer (didn't send)	*/
extern		dequeue ();	/*  dequeue a message from a rcv desc	*/
extern		freerbuf ();	/*  free a received message buffer	*/

