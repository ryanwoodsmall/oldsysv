/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/ct_lla.h	10.4"
/*
 * CTC board control block
 */

struct ctboard {
	union {
	    struct brdflgs {
		unsigned usused:    24;
		unsigned pumping:    1;
		unsigned streaming:  1;
		unsigned opnpending: 1;
		unsigned opn_sleep:  1;
		unsigned ciotype:    1;
		unsigned isinit:     1;
		unsigned issysgen:   1;
		unsigned is_up:      1;
	    } bits;
	    int brd_state;
	} brd_flgs;
	struct iobuf  ctutab;				/* CT unit IO table */

	struct ctjobstat {

	    struct ctjobstat *b_link;			     /* back link ptr */

	    unsigned long procid;		    /* UNIX process id number */

	    union {
	        struct jobstat {
			unsigned opcode:   8;  /* opcode of job (6 bits used) */
			unsigned brd_id:   3;	       /* board number of job */
			unsigned sub_dev:  1;	     /* sub-device no. of job */
			unsigned splitjob: 1;		/* r/w split job flag */
			unsigned io_done:  1; /* fw int received for job flag */
			unsigned unused:  10;
			unsigned err_code: 8;		   /* UNIX error code */
		} bits;
		unsigned long lng_val;
	    } jobstat;

	    struct ctjobstat *f_link;			  /* forward link ptr */

	} ctjobs[(RQSIZE*2)];

	struct ctjobstat jobq_frehdr;		     /* job queue free header */
	struct ctjobstat jobq_acthdr;	     	   /* job queue active header */
	int           jobq_busy;		 /* flag to mark queue access */
	int           jobq_wanted;	     /* flag to mark request to queue */

	struct ctsub {
	    union {
		struct sdev_flgs {
		    unsigned unused:    29;
		    unsigned subopen:    1;
		    unsigned specl_opn:  1;
		    unsigned vtoc_pres:  1;
		} bits;
		int sub_state;
	    } sdev_flgs;
	    unsigned short ct_otyp[OTYPCNT];      /* sub-dev open types flags */
	    unsigned short ct_verify;	     /* sub-device part.s verify flag */
	    struct vtoc ct_vtoc;		     /* sub-device VTOC info. */
	    struct pdsector ct_pdsect;		     /* sub-device parameters */
	    struct iotime ct_stat;		    /* drive status structure */
	} ct_sub[NUM_DEVS];

	int           time_id;			  /* global driver timeout id */
	int           full_q;		      /* board full Q request counter */
	caddr_t       slp_add;		       /* sleep address for timeout() */
	int           tapepas;			     /* tape media pass count */
	unsigned long cntrtim;		       /* controller time usage count */
	RQUEUE        ct_rq;			       /* board request queue */
	CQUEUE        ct_cq;			    /* board completion queue */
};

extern struct ctboard ct_board[];

/*
 * brd_flgs (board state) flags
 *
 * IS_UP;	ISSYSGEN + ISINIT ... set in ctinit(), cleared in ctclr()
 *              used to indicate a power failure.
 * ISSYSGEN;	board sysgen'd successfully ... set in ctint()
 * ISINIT;	driver executed ctinit() successfully ... set in ctinit()
 * CIOTYPE;	CIO command is currently executing on CTC
 * OPENPNDING;	Open request for this board is being processed.
 * OPN_SLEEP;	Open request(s) put to sleep pending current open completion.
 * STREAMING;	Stream request bit, set via ctioctl() call
 * PUMPING;	Pump request bit.
 */

#define	IS_UP		0x00000001
#define	ISSYSGEN	0x00000002
#define	ISINIT		0x00000004
#define	CIOTYPE		0x00000008
#define	OPN_SLEEP	0x00000010
#define	OPENPNDING	0x00000020
#define	STREAMING	0x00000040
#define	PUMPING		0x00000080

#define	BRD_FLGS(b)	ct_board[b].brd_flgs

#define	IOBUF(b)		ct_board[b].ctutab
#define	CTJOBQ_NODE(b,i)	ct_board[b].ctjobs[i]
#define	CTJOBQ_FREEHDR(b)	ct_board[b].jobq_frehdr
#define	CTJOBQ_ACTVHDR(b)	ct_board[b].jobq_acthdr
#define	CTJOBQ_BUSY(b)		ct_board[b].jobq_busy
#define	CTJOBQ_WANTED(b)	ct_board[b].jobq_wanted
#define	TIME_ID(b)		ct_board[b].time_id
#define	CTIMEUSE(b)		ct_board[b].cntrtim

/*
 * ct job queue status bit defines
 */

#define	CTSPLITJOB	0x00080000L
#define	CTIO_DONE	0x00040000L
#define	CTIO_ERROR	0x00020000L

#define	CTCLRSBITS	0xfff1ff00L

/*
 * These macros are defined to improve the readability of the code.
 */

#define	CTJOB_BRDID	ctjob->jobstat.bits.brd_id
#define	CTJOB_SUBDEV	ctjob->jobstat.bits.sub_dev

/*
 * sdev_flgs (sub-device state) flags
 *
 * VTOC_PRES;	vtoc information is present
 * SPECL_OPN;	special ioctl() open.
 * SUBOPEN;	sub-device opened flag.
 */

#define	VTOC_PRES	0x00000001
#define	SPECL_OPN	0x00000002
#define SUBOPEN		0x00000004

#define	SDEV_FLGS(b,s)	 ct_board[b].ct_sub[s].sdev_flgs

#define	CT_OTYP(b,s)	ct_board[b].ct_sub[s].ct_otyp
#define	CT_VERIFY(b,s)	ct_board[b].ct_sub[s].ct_verify
#define	VTOC(b,s)	ct_board[b].ct_sub[s].ct_vtoc
#define	PDSECT(b,s)	ct_board[b].ct_sub[s].ct_pdsect
#define	IOTIME(b,s)	ct_board[b].ct_sub[s].ct_stat

#define	PMNT_CHK(b,s,p)	(VTOC(b,s).v_part[p].p_flag & V_UNMNT)

struct lla_rwcmd {
	unsigned short nobytes;		/* number of bytes to be transferred */
	char brd_id;			/* local driver cmd formation var. */
	char sub_dev;			/* sub-device number */
	unsigned char opcode;		/* CTREAD or CTWRITE FW opcode */

	paddr_t bufaddr;		/* address of UNIX bp data area */

	unsigned long blkno;		/* physical block no. on media */
	daddr_t jqaddr;			/* local job queue entry pointer */
	time_t utime;			/* UNIX lbolt FW start time */
};

/*
 * Defines for Minor Device Number Usage
 *        7     4 3     0
 *       +-+-+-+-+-+-+-+-+
 *       |C|C|C|N|P|P|P|P|  ct minor device number
 *       +-+-+-+-+-+-+-+-+
 *        CCC = Controller Number
 *          N = Sub-Device Number
 *       PPPP = Sub-Device Partition Number (16 possible)
 */

#define	CNTRLR		0xe0
#define	SUBDNO		0x10
#define	SUBID		0x1f
#define	PARTS		0x0f

#define	CT_BRD(m)	((m & CNTRLR) >> 5)
#define	SUB_DEV(m)	((m & SUBDNO) >> 4)
#define	SUB_ID(m)	(m & SUBID)
#define	PARTNUM(m)	(m & PARTS)

#define	CTMAX_MINOR	23
/*
 * Sub-device defines
 */

#define	TAPE_DEV	0
#define	FLPY_DEV	1

/*
 * Default Floppy Drive Configuration
 */

struct dfd_conf {
	daddr_t nblocks;	/* number of blocks in disk parition */
	int     sblock;		/* starting cylinder # of parition */
};

/*
 * Low Level Access Request/Completion Queue Macros
 *
 * Due to multiple boards and multiple queues, these are a modification
 * of those macros found in <sys/lla.h>.
 */

#define	R_ADD(b)	ct_board[b].ct_rq
#define	C_ADD(b)	ct_board[b].ct_cq

#define	RULOAD(b)	R_ADD(b).queue[0].p_queues.bit16.unload
#define	RLOAD(b)	R_ADD(b).queue[0].p_queues.bit16.load
#define	CULOAD(b)	C_ADD(b).queue.p_queues.bit16.unload
#define	CLOAD(b)	C_ADD(b).queue.p_queues.bit16.load

#define	RQFULL(b)	(((RULOAD(b)/sizeof(RENTRY) + RQSIZE - RLOAD(b)/sizeof(RENTRY)) % RQSIZE) == 1)
#define	NEW_RLPTR(b)	((1 + (RLOAD(b)/sizeof(RENTRY))) % RQSIZE)

#define	CQEMPTY(b)	(CULOAD(b) == CLOAD(b))
#define	NEW_CUPTR(b)	((1 + CULOAD(b)/sizeof(CENTRY)) % CQSIZE)

/*
 * These macros are defined to improve the readability of the code.
 */

#define	RQ_BYTCNT(b,r) ct_board[b].ct_rq.queue[0].entry[r].common.codes.bytes.bytcnt
#define	RQ_SEQBIT(b,r) ct_board[b].ct_rq.queue[0].entry[r].common.codes.bits.seqbit
#define	RQ_CSBIT(b,r)  ct_board[b].ct_rq.queue[0].entry[r].common.codes.bits.cmd_stat
#define	RQ_SUBDEV(b,r) ct_board[b].ct_rq.queue[0].entry[r].common.codes.bits.subdev
#define	RQ_OPCODE(b,r) ct_board[b].ct_rq.queue[0].entry[r].common.codes.bytes.opcode
#define	RQ_CADDR(b,r)  ct_board[b].ct_rq.queue[0].entry[r].common.addr

#define	ARQ_BLKNO(b,r)     ct_board[b].ct_rq.queue[0].entry[r].appl.blkno
#define	ARQ_JID(b,r)       ct_board[b].ct_rq.queue[0].entry[r].appl.jid
#define	ARQ_JIO_START(b,r) ct_board[b].ct_rq.queue[0].entry[r].appl.jio_start

/* Pump defines */

#define	PU_BLOCK	1024
#define	PU_LBLOCK	(PU_BLOCK/4)

#define	SG_PTR	*((long *) DBLK_PTR)

#define	SET	1	/* bit set state = 1 */
#define	CLEAR	0	/* bit clear state = 0 */

/* Board RAM boundaries */

#define	RAM_START	0
#define	RAM_END		0x1ffff

/* Time parameters to wait until dead board assumed */

#define	CTSG_TIME	1500	/* timeout for sysgen change */
#define	CTCIO_TIME	1500	/* timeout for CIO LLA commands to complete */
#define	CTFLG_TIME	3300	/* timeout for default brd flag reset */
#define	CTJOB_TIME	33000	/* timeout for reg. cmnd to complete */
#define	CTFMT_TIME	360000	/* timeout for format command to complete */

#define	SIXTY4_K	65536
#define	ONE28K		131072
#define	CTSEG_SIZE	15872
#define	CTSEG_BLKS	31
#define	CTBYTESCT	512
