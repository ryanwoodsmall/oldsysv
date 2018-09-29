/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sa:sys/pdi.h	1.2"
/*	Copyright (c) 1986 AT&T
 *	  All Rights Reserved
 *
 *	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
 *	The copyright notice above does not evidence any
 *	actual or intended publication of such source code.
 *
 *	@(#)pdi.h	1.0
 */
#ifndef NULL
#define NULL 0
#endif

#define	pdi_procp_t	caddr_t

/*
 *  The portable buffer header structure
 */
typedef struct pdi_buf
{
	long	b_flags;	/* status of I/O */
	long	unavail1;	/* unavailable */
	long	unavail2;	/* unavailable */
 	struct pdi_buf *b_forw;	/* driver queue pntrs */
	struct pdi_buf *b_back;
	short	unavail3;	/* unavailable */
	unsigned long	b_bcount;  /* transfer count */
	caddr_t b_addr;		/* buffer virtual address */
	daddr_t	b_blkno;	/* block # on device (in 512 bytes)*/
	char	b_error;	/* returned after I/O */
	unsigned long	b_resid;  /* words not transferred after error */
	time_t	b_start;	/* request start time */
	pdi_procp_t	b_proc;	/* Pointer to process */
} pdi_buf_t;

/*
 *	These values are kept in b_flags.
 */
#define PDI_B_WRITE   0x0000	/* non-read pseudo-flag */
#define PDI_B_READ    0x0001	/* read when I/O occurs */
#define PDI_B_DONE    0x0002	/* transaction finished */
#define PDI_B_ERROR   0x0004	/* transaction aborted  */
#define PDI_B_BUSY    0x0008	/* buffer busy          */
#define PDI_B_PHYS    0x0010	/* physical I/O         */

/*
 * System accounting logging structures
 */

typedef struct pdi_iostat {
	long	io_ops;		/* number of read/writes */
	long	io_misc;	/* number of "other" operations */
	long	io_qcnt;	/* number of jobs assigned to drive */
	ushort io_unlog;	/* number of unlogged errors */
} pdi_iostat_t;

#define	NTRACK		10	/* # of entries in drive's perf. queue */

/* performance data gathering queue */

typedef struct	pdi_df_ptrack
{
	long		b_blkno;	/* start disk block address */
	pdi_buf_t	*bp;		/* pointer to user buffer header */
} pdi_ptrk_t;

/*
 * structure for system accounting
 */
typedef struct pdi_iotime {
	pdi_iostat_t 	ios; 	/* iostat sub structure */
	long	io_bcnt;	/* total blocks transferred */
	time_t	io_resp;	/* total block response time in ms */
	time_t	io_act;		/* cumulative use in ms */
	long	maj;		/* major number of device */
	long	min;		/* minor number of device */
	long	tnrreq;		/* total number of read requests */
	long	tnwreq;		/* total number of write requests */
	long	cumqlen;	/* cumulative queue length */
	long	maxqlen;	/* max queue length */
	long	minqlen;	/* minimum queue length */
	long	cumseekd;	/* cumulative seek distance */
	time_t	io_liact;	/* time drive active for last interval in ms */
	time_t	io_intv;	/* drive perf reporting interval in ms */
	pdi_ptrk_t	*pttrack;	/* start of queue */
	pdi_ptrk_t	*endptrack;	/* last entry in queue */
	pdi_ptrk_t	ptrackq[NTRACK];	/* queue */
} pdi_iotime_t;

/*
 *  dma function tuple structure 
 */
typedef struct pdi_dma_tuple {
	paddr_t physaddr;
	long	count;
} pdi_dma_t;

/*
 *  portable device number structure 
 */
typedef struct pdi_dev {
	long	maj;
	long	min;
} pdi_dev_t;

#define	io_cnt	ios.io_ops
#define io_qc ios.io_qcnt
#define pdi_brelse	brelse
#define pdi_clrbuf(bp)	(clrbuf((struct buf*)bp))
#define pdi_copyin	copyin
#define pdi_copyout	copyout
#define pdi_iodone	iodone
#define pdi_iowait	iowait
#define pdi_physck	physck

#define pdi_sleep	sleep
#define PDI_PCATCH	0400
#define PDI_PRIBIO	20
#define PDI_PZERO	25

#define pdi_splblk	spl6
#define pdi_splchar	spl5
#define pdi_splhi	splhi
#define pdi_splx	splx
#define pdi_subyte	subyte
#define pdi_suword	suword
#define pdi_untimeout	untimeout
#define pdi_wakeup	wakeup
/*
 * PDI external major/minor macros
 */
#define pdi_emajor(x)	(long)(((unsigned)(x)>>8)&0x7F)
#define pdi_eminor(x)	(long)((x)&0xFF)

#define pdi_bemajor(x)	(pdi_emajor(x->unavail3))
#define pdi_beminor(x)	(pdi_eminor(x->unavail3))
#define pdi_bdevset(buf,maj,min) (buf->unavail3=(dev_t)(((maj)<<8) | (min)))

/*
 * PDI Functions
 */
extern caddr_t		pdi_base();
extern time_t		pdi_btime();
extern void		pdi_cmn_err();
#define	PDI_CE_CONT	0
#define	PDI_CE_NOTE	1
#define	PDI_CE_WARN	2
#define	PDI_CE_PANIC	3

extern uint		pdi_count();
extern void		pdi_dmafreelist();
extern pdi_dma_t	*pdi_dmamakelist();
extern void		pdi_error();
extern long		pdi_euid();
extern int		pdi_freemem();
#define	PDI_ROOTID	0

extern pdi_buf_t	*pdi_getrbuf();
extern void		pdi_freerbuf();

extern pdi_buf_t	*pdi_geteblk();
extern caddr_t		pdi_getmem();
extern void		pdi_hdelog();
#define PDI_SERLEN		12	/* length of serial number */
#define PDI_HDECRC		1	/* for CRC data checking */
#define PDI_HDEECC		2	/* for ECC data processing */
#define PDI_HDEMARG		1	/* for marginal blocks */
#define PDI_HDEUNRD		2	/* for unreadable blocks */

extern void		pdi_hdeeqd();
#define	PDI_EQD_ID	0
#define	PDI_EQD_IF	1
#define	PDI_EQD_TAPE	2
#define	PDI_EQD_EHDC	3
#define	PDI_EQD_EFC	4
#define	PDI_EQD_OROM	5
#define	PDI_EQD_ORW	6
#define	PDI_EQD_OWORM	7

extern long		pdi_itoemaj();
extern void		pdi_logberr();
extern long		pdi_imajor();
extern long		pdi_iminor();
extern off_t		pdi_offset();
extern void		pdi_physio();
extern pdi_procp_t	pdi_procp();
extern time_t		pdi_time();
extern int		pdi_timeout();
extern void		pdi_ubase();
extern void		pdi_ucount();
extern void		pdi_uoffset();
extern void		pdi_ureturn();
extern paddr_t		pdi_vtop();
