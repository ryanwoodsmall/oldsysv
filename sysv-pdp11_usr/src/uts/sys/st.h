#define STCWAIT (HZ / 3)
#define STFUDGE	16
#define NOSLP	0
#define STSP	36
#define SOH	0001
#define	STX	0002
#define	ETX	0003
#define	EOT	0004
#define	ENQ	0005
#define	BEL	0007
#define	DLE	0020
#define	DC1	0021
#define	DC2	0022
#define	DC3	0023
#define	NAK	0025
#define	SYN	0026
#define	ETB	0027
#define	ESC	0033
#define	GS	0035
#define	EFLG	0200	/* invalid address character flag */
#define	ROWLEN	80	/* columns per row */
#define	ROWCNT	24	/* rows per screen */

struct strca {
	char	a_row;
	char	a_col;
};

struct stbaa {
	char	a_baddr1;
	char	a_baddr2;
};

#define MAXWCSZ 150
#define MAXRCSZ 100
#define MAXWRSZ 100
#define MAXRRSZ 150
#define BLKSZ	256	/* minimum XMT buffer size */
#define	BLKFDG	64	/* block size fudge factor */
#define	FAC	' '	/* field attribute character */
#define	PT	'\011'	/* program tab order */
#define EWC	'5'	/* erase write command */
#define WC	'1'	/* write command */
#define WCC	'3'	/* write control character, no print */
#define PWCC	'#'	/* write control character, start printer */
#define	NSTCHQ	8	/* size of user RCV queue */
#define	NXSTCHQ	8	/* size of user reXMT queue */

/*
 *	Synchronous terminal parameter structure.
 *	Set up in space.h.
 */
struct ststat {
	unsigned	ibsz;	/* input buffer size */
	unsigned	obsz;	/* output buffer size */
	int		ihbuf;	/* # of input buffer headers */
	int		ohbuf;	/* # of output buffer headers */
	int		nprnt;	/* # of print channels */
};

/*
 *	Message header structure.
 *	One per outstanding message.  Others on free chain.
 */
struct	stbhdr {
	struct csibd	sm_bd;		/* buffer descriptors */
	char		sm_flags;	/* buffer flags */
};
/*
 * sm_flags definitions.
 */
#define	STRCV	1	/* receive buffer */
#define	STXMT	2	/* transmit buffer */
#define	STWAK	4	/* issue wakeup when buffer returned */
/*
 * sm_type definitions.
 */
#define	FRVI	0x01	/* XMT failed, RVI'ed */
#define	FNAK	0x02	/* XMT failed, NAK'ed */
#define	FWAK	0x04	/* XMT failed, WAK'ed */
#define	FTO	0x08	/* XMT failed, timed out */
#define	FGBG	0x10	/* XMT failed, garbage response */
#define	FEOT	0x20	/* XMT failed, EOT'ed */
#define FBLOCK	1	/* 1st block in a RCV message */
#define LBLOCK	2	/* last block in a RCV message */
#define	CONREPT	4	/* controller connected report */

/*
 *	Message queue header structure.
 *	One RCV queue & one reXMT queue per user channel.
 */
struct stmsghdr {
	short	s_max;			/* max length of queue */
	short	s_act;			/* number of current queue members */
	struct  stbhdr *s_last;		/* pointer to last queue member */
	struct	stbhdr *s_first;	/* pointer to message queue */
};

/*
 *	Synchronous terminal control header.
 *	One per synchronous line.
 */
struct stc {
	ushort	flags;			/* flags */
	ushort	ttyid;			/* current input terminal id */
	char	csidev;		/* CSI interface device */
};
/*
 *	flags definitions.
 */
#define	STACTIVE	0x010	/* channel connected */
#define STRUN		0x020	/* script is running */
#define VPMERR		0x040	/* script terminated */
#define STLOAD		0x080	/* script loaded (but not yet started) */
#define STATTACH	0x100	/* device attached */
#define STHANGUP	0x200	/* device in HANGUP state */

/*
 *	Synchronous terminal user header.
 *	One per user printer or terminal channel.
 */
struct st {
	ushort		s_ttyid;	/* terminal identifier */
	short		s_pgrp;		/* process group */
	char		s_row;		/* cursor row of last field read */
	char		s_col;		/* cursor col of last field read */
	char		s_orow;		/* cursor row on last send */
	char		s_ocol;		/* cursor col on last send */
	char		s_srow;		/* row save area */
	char		s_scol;		/* col save area */
	char		s_prow;		/* cursor row of prompt protect FAC */
	char		s_pcol;		/* cursor col of prompt protect FAC */
	char		s_tab;		/* change to tab on input */
	char		s_aid;		/* send key code */
	char		s_ss1;		/* status and sense byte 1 */
	char		s_ss2;		/* status and sense byte 2 */
	ushort		s_pcount;	/* XMT buffer size request */
	ushort		s_roffset;	/* read offset into s_rbuf */
	ushort		s_imode;	/* input modes */
	ushort		s_omode;	/* output modes */
	ushort		s_lmode;	/* local modes */
	ushort		s_size;		/* size of current XMT buffer */
	short		s_free;		/* free bytes in current XMT buffer */
	short		s_pfree;	/* s_free at prompt protect field */
	struct stbhdr	*s_rbuf;	/* ptr to current RCV buffer */
	struct stbhdr	*s_xbuf;	/* ptr to current XMT buffer */
	char		*s_ptr;		/* ptr into current XMT buffer */
	struct stc	*s_ctrlp;	/* ptr to associated control struct */
	long		s_flags;	/* state of channel */
	struct stmsghdr	s_hdr;		/* RCV queue header */
	struct stmsghdr	s_xhdr;		/* reXMT queue header */
};
/*
 *	s_flags definitions.
 */
#define	CH_OPEN		0x00001L	/* channel open */
#define	CH_AVAIL	0x00002L	/* channel not assigned to device */
#define WOUT		0x00004L
#define WWOUT		0x00008L
#define NOUT		0x00010L	/* wait for entry before next output to term */
#define CTLD		0x00020L	/* EOF indication entered */
#define STRFLG		0x00040L	/* a reader is waiting for input */
#define STAWR		0x00080L
#define STSOH		0x00100L	/* a status and sense message was received */
#define STTO		0x00200L
#define STFRD		0x00400L
#define RESET		0x00800L
#define DCF		0x01000L
#define DCF2		0x02000L
#define EOR		0x04000L
#define STEOT		0x08000L
#define STFMT		0x10000L
#define STCLR		0x20000L/* CLEAR key hit, reformat screen */
#define	STRVI		0x40000L/* device temporarily unavailable */
#define	STWIP		0x80000L/* write in progress */
#define	STWWAIT		0x100000L/* writer waiting for writer to complete */

#define	STIGNTAB	'\377'	/* stty code for undefined control character */
#define PA1		'%'	/* code for PA1 key */
#define PA2		'>'	/* code for PA2 key */
#define PF12		'@'	/* code for PF12 key */
#define CLEAR		'_'	/* code for CLEAR key */

/* Operation status flags for stcfl(). */
#define NOTEOW		0	/* flush because buffer full */
#define EOW		1	/* flush because end of write system call */
#define REOW		2	/* flush before read request */
#define LEOW		4	/* flush at end of output page */
#define	PEOW		8	/* protect prompt before flush */

#define HDRSZ		7	/* protocol hdr size in RCV buffer */
#define TERMHSZ		7	/* protocol hdr size in terminal XMT buffer */
#define PRNTHSZ		10	/* protocol hdr size in printer XMT buffer */
#define RCVLEN		4	/* # of RCV buffers per line */
#define	SMSGSZ		8	/* status message size */
#define	DEVCOD		" ABCDEFGHI[.<(+!&JKLMNOPQR]$*);^"
				/* valid synchronous device codes */
#define STMBSZ		256	/* RCV buffer size */
#define	STMPRI	(PZERO + 1)		/* allow interrupted reads */
#define stid(X)	(ushort)((X->sm_bd.d_octet2 << 8) | X->sm_bd.d_octet3)

#ifdef pdp11
#define moveio(loc, cnt, rw)	pimove((long)(unsigned)loc, cnt, rw)
#else
#define moveio(loc, cnt, rw)	iomove((caddr_t)loc, cnt, rw)
#endif
/* <@(#)st.h	1.5> */
