/*	x25.h 1.12 of 4/14/82
	@(#)x25.h	1.12
 */

#define X25LCH		7		/* size of per-link hash table */
#define MAXPKT		1024		/* maximum packet size */
#define HIRECVQ		4		/* high-water mark for slot recvq */
#define LORECVQ		2		/* low-water mark for slot recvq */
#define HIXMTQ		4		/* high-water mark for tab xmtq */
#define LOXMTQ		2		/* low-water mark for tab xmtq */
#define HIL2Q		4		/* high-water mark for Level 2 queue */
#define X25EBQMAX	4		/* max for empty buffer queue */
#define WINDOW		2		/* window size */
#define X25SEQNSZ	8		/* modulo value for seq number range */
#define X25HDRSZ	3		/* length of level 3 packet header */

#define MAXRS		1	/* max for retry counter l_rscount for Ts timer */
#define MAXRL		1	/* max for retry counter t_rlcount for Tl timer */
#define MAXRD		1	/* max for retry counter t_rdcount for Td timer */
#define MAXRJ		1	/* max for retry counter t_rjcount for Tj timer */
#define MAXRC		2	/* max for retry counter l_ccount for changeover */

struct x25info {
	int nslots;	/* number of slots configured */
	int ntabs;	/* number of tabs configured */
	int nlinks;	/* number of links configured */
	int nbd;	/* number of buffer descriptors configured */
	int nxbytes;	/* size of buffer area in bytes */
};

/* Layout of items used per link */

struct x25link{
	struct x25tab	*l_x25chan[X25LCH];	/* array of ptrs to tab structures */
	struct x25tab	*l_begloq;		/* pointer to beginning of low priority queue */
	struct x25tab	*l_endloq;		/* pointer to end of low priority queue */
	struct x25tab	*l_beghiq;		/* pointer to beginning of high priority queue */
	struct x25tab	*l_endhiq;		/* pointer to end of high priority queue */
	unsigned short	l_pktsz;		/* packetsize for the link */
	unsigned char	l_rststate;		/* restart state of link */
	unsigned char	l_rscount;		/* RS counter for TS timer */
	unsigned char	l_ccount;		/* RC counter for TC timer */
	struct l_state {
	 unsigned short	devset:1,	/* set if primary device attached */
			bkset:1,	/* set if backup link attached */
			rstonl2:1,	/* set if RST pkt on Level 2 queue */
			l2full:1,	/* set if lev 2 q at high-water mark */
			l3run:1,	/* 1 if level 3 running */
			run:1,		/* indicates where lev 3 is running */
					/* 0:primary, 1:backup */
			bkstart:1,	/* 1 if backup started by nc driver */
			trychg:1,	/* set if trying to change over */
			statchg:2,	/* status of last changeover 0:none, */
					/* 1:CHGOCC, 2:CHGFAIL, 3:CHGDEAD */
			nchgfails:6;	/* number of times changeover failed */
	} l_state;
	struct l_line {
		unsigned short idev;	/* csi device index */
		char l2sync;		/* set if lev 2 synchronized */
		unsigned char nl2fails;	/* numb of times N2 ctr at max */
		short errcode;		/* errcode sent from PCD */
	} l_line[2];
};

/*
 *	Layout of items used per logical channel.
 *	There must be one of these structures assigned for
 *	each logical channel that may be assigned on a link group.
 */

struct x25tab  {
	struct csillist	t_xmtq;		/* transmit queue for data packets */
	struct csibd	*t_unack[X25SEQNSZ];	/* for unacknowledged packets */
	struct x25tab	*t_nextab;	/* pointer to next tab for hashing */
	struct x25tab	*t_nexthi;	/* pointer to next tab structure on high priority queue */
	struct x25tab	*t_nextlo;	/* pointer to next tab structure on low priority queue */
	struct x25slot	*t_slotp;	/* ptr to slot connected to this tab */
	short	t_chno;			/* logical channel number on link */
	unsigned char	t_link;		/* index of link structure x25link */
	unsigned char	t_dgnsend;	/* diagnostic code for sending RES or RST pkt*/
	unsigned char	t_dgnrecv;	/* diagnostic code for received RES or RST packet */
	unsigned char	t_rlcount;	/* Rl counter for Tl timer */
	unsigned char	t_rdcount;	/* Rd counter for Td timer */
	unsigned char	t_rjcount;	/* Rj counter for Tj timer */
	unsigned char	t_seqsent;	/* bit position set means data with that seqno is being xmitted */
	struct {
	 unsigned short	r_ps:3,		/* next expected receive P(S) */
			s_pr:3,		/* last sent value of P(R) */
			s_ps:3,		/* next send P(S) */
			r_pr:3,		/* last received value of P(R) */
			osdbit:4;	/* seq # of unack pkt with Dbit set */
	} t_seq;
	struct {
	 unsigned short	res:1,		/* send reset packet */
			resc:1,		/* send reset conf packet */
			inte:1,		/* send interrupt packet */
			intc:1,		/* send interrupt conf packet */
			rnr:1,		/* send receiver-not-ready packet */
			rr:1,		/* send receiver-ready packet */
			data:1,		/* send data packet */
			rej:1,		/* send reject packet */
			wa:1,		/* send window advancement packet - rr,rnr or data */
			onhiq:1,	/* set if tab is on high priority queue */
			onloq:1,	/* set if tab is on low priority queue */
			rejneed:1,	/* need to schedule a REJ pkt */
			rejxmit:1,	/* transmitted a REJ pkt and have not received DATA with PS=PR */
			rejrec:1,	/* received REJ & have not sent out DATA with PS = PR */
			:2;
	} t_flags;
	struct t_state {
	 unsigned short	state:4,	/* state of channel - R1,R2,R3,D1,D2,D3 */
			intstate:2,	/* interrupt state within D1 - I2,I3,I4 */
			sending:1,	/* set if channel can send data */
			rcving:1,	/* set if channel can receive data */
			unconfint:1,	/* set if an interrupt is unconfirmed */
			delconf:1,	/* set if delivery conf is requested */
			isreset:1,	/* set if chan was just reset */
			faropen:1,	/* set if far end open */
			dbitset:1,	/* set when writer sets Dbit */
			:3;
	} t_state;
};

/* Layout of items used per slot structure */

struct x25slot {
	struct	csillist s_recvq;	/* receive queue */
	struct	x25tab *s_tabp;		/* ptr to tab connected to this slot */
	short	s_brdcnt;		/* num chars read out of buf on header recvq */
	struct s_flags {
		unsigned short  
			openrd:1,	/* open for reading */
			openwr:1,	/* open for writing */
			wrsleep:1,	/* writers should sleep - xmitq full */
			messcont:1,	/* message will be cont'd next read */
			sesspr:2,	/* session setup protocol */
			excl:1,		/* exclusive use */
			ndelay:1,	/* no delay */
			:8;
	} s_flags;
};

/* Per-channel timers */

#define TL	0
#define TD	1
#define TF	2
#define TJ	3
#define NTIMER	4
#define ALLTIMER (-1)

/* Per-link timers */

#define TSTIMER	0
#define TCTIMER	1
#define TKTIMER	2
#define TWTIMER	3
#define NLNTIMER	4

/* Timer initial values */

#define TLVAL	60
#define TDVAL	180
#define TFVAL	30
#define TJVAL	40
#define TSVAL	45
#define TCVAL	60
#define TKVAL	30
#define TWVAL	10

/* timer list entry - one for each tab */
struct x25timer {
	struct x25timer *tnext;
	short	chno;
	unsigned char tval[NTIMER];
};

/* per link timers */
struct x25lntimer {
	unsigned char lntval[NLNTIMER];
};

/* packet types */

#define	RST	1
#define	RSTC	2
#define	RES	3
#define	RESC	4
#define	INT	5
#define	INTC	6
#define	RNR	7
#define	RR	8
#define	WA	9
#define REJ	10
#define	DATA	11
#define INV_PKT	12
#define INV_LEN	13

/* channel states */

#define HALTED	0	/* halted state */
#define R1	1	/* flow control ready */
#define R2	2	/* processing restart indication */
#define R3	3	/* awaiting restart confirmation */

#define D1	4	/* data - flow control ready */
#define D2	5	/* processing reset indication */
#define D3	6	/* awaiting reset confirmation */

/* interrupt states within D1 */

#define I1	0	/* not interrupted */
#define I2	1	/* processing interrupt */
#define I3	2	/* awaiting interrupt confirmation */
#define I4	3	/* transmitted/received interrupt */

/* priorities */

#define HI	1
#define LOW	2

#define PR_OK	0
#define PR_OW	1
#define PR_SAME 2
#define PS_OK	0
#define PS_OW	1

#define PRIM	0
#define BKUP	1

#define SETSEQ(BITS, SEQN) (BITS |= (01<<SEQN))
#define UNSETSEQ(BITS, SEQN) (BITS &= ~(01<<SEQN))
#define ISSEQSET(BITS, SEQN) ((BITS>>SEQN)&01)

#ifdef CBUNIX
#define FNDELAY 04
#define FEXCL 0
#endif

#define CHECKRRDY(SLOTP) if (SLOTP->s_recvq.l_ql <= LORECVQ) x25rrdy(SLOTP->s_tabp)

#define X25PRI	(PZERO+5)
#define X25HIPRI (PZERO-1)

#define TOBUF	1
#define FROMBUF	2

/* buffer statistics and header statistics */
struct x25m {
	int	bufused;	/* bytes of buffer space used */
	int	bufwrite;	/* bytes used for user write buffers */
	int	bufrcv;		/* bytes of buffer space used for rcv */
	int	bdused;		/* buffer desc. used */
	int	bdwrite;	/* buffer desc. used for user writes */
	int	bdrcv;		/* buffer desc. used for rcv buffers */
	int	wantbd;		/* set if sleeping for buffer space */
};


/* usage for buffer descriptor: rcv data, control packet, xmt data */
#define RCVDBD	0
#define CTLBD	1
#define XMTDBD	2

#ifdef pdp11
#define OCTET1(BDP)	BDP->d_octet1
#define OCTET2(BDP)	BDP->d_octet2
#define OCTET3(BDP)	BDP->d_octet3

#define HDPTR(BDP)	(struct level_3 *)&BDP->d_octet1

#define HDROFFSET	0

#else

#define OCTET1(BDP)	*BDP->d_adres
#define OCTET2(BDP)	*(BDP->d_adres+1)
#define OCTET3(BDP)	*(BDP->d_adres+2)

#define HDPTR(BDP)	(struct level_3 *)BDP->d_adres

#define HDROFFSET	X25HDRSZ
#endif

#define RUNIND		linkp->l_state.run
#define STNDIND		linkp->l_state.run?0:1

#define NOCHG	0	/* no changeover requested */
#define CHGOCC	1	/* changeover occurred */
#define CHGFAIL 2	/* changeover failed - Lev 2 of standby not sync */
#define CHGDEAD 3	/* changeover failed - Lev 2 of standby dead */

#define RUNDEAD	4	/* arg to x25chnge if changeover must occur */
			/* because running line is dead */

#ifdef CBUNIX
#define MINOR(DEV)	DEV = minor(DEV)
#else
#define MINOR(DEV)
#endif
