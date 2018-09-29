/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/rbuf.h	10.1.2.3"
/*
 *  	New parameters and macros for RFS network cache. rbuf structure
 *	defines contents of buffer header when buffer contains remote (RFS)
 *	server file data.
 *
 *      WARNING:  do not change size of the buf structure (sys/buf.h) without 
 *	making a corresponding size change in the the rbuf structure 
 *	(sys/rbuf.h), and vice versa.  The two structures define the two 
 *	possible formats of the buffer header (the format used depends on 
 *	whether the buffer contains local data or RFS data).  
 *
 *      This overlay of the two structures is an interim solution
 *	that is expected to change in a future release.  Users
 *	should be aware that the rbuf structure will probably go
 *	away at that time.
 */

#define	RBSIZE		FsBSIZE(SBUFSIZE)
#define RBSHIFT		FsBSHIFT(SBUFSIZE)
#define	RBMASK		FsBMASK(SBUFSIZE)
#define LGET		0
#define	RGET		1
#define BUFAGE		300	/* Maximum buffer age time for NREMOTE minimum*/

#define off_to_blkoff(x)     ((unsigned long)(x) >> (RBSHIFT))
					/*convert offset to 1024 block-size*/
#define ct_to_blkct(x,y)     ((x)<=0?0:((off_to_blkoff((x)-1+(y))-off_to_blkoff(y))+1))
					/*no. of blocks in read/write request*/

typedef struct	rbuf
{
	int	b_flags;		
	struct	rbuf *b_forw;		/* position in RFS block hash table */
	struct	rbuf *b_back;		/*  "  */
	struct	rbuf *av_forw;		/* position on RFS free list, */
	struct	rbuf *av_back;		/*     if not BUSY*/
	struct	rbuf *f_forw;		/* position in RFS file hash table */
	struct	rbuf *f_back;		/*  "  */
	union {
	    caddr_t b_addr;		/* low order core address */
	} b_un;

	daddr_t	b_blkno;		/* logical block # (computed from offset)*/
	long   		b_fhandle;	/* buffer remote file ID */
	struct queue	*b_queue;       /* buffer remote server ID */
	index_t		b_mntindx;      /* remote mount index */
	unsigned long	b_vcode;        /* remote file version code */
	unsigned long	b_reltime;      /* previous release time */
} rbuf_t;

/*  
 * Union defining dependency between buf and rbuf structure sizes
 * (this union is not actually used in kernel code because of dependencies of
 * old source code on buf structure).
 */
union unibuf {
	struct	buf	loc;
	struct	rbuf	rem;
} ;

extern	struct	rbuf	rbfreelist;	/* head of available list */
extern  int		rcache_enable;
extern  unsigned long	rfs_vcode;
extern  unsigned long	rbuf_ct;
extern  unsigned long	lbuf_ct;
extern	int		rhmask;
extern	int		nrhbuf;
extern	unsigned long	nlbuf;
extern	unsigned long	nrbuf;
extern	unsigned long	rcache_time;
extern	int		rc_time;
extern	unsigned long	nremote;
extern	unsigned long	nlocal;
extern	unsigned long	maxbufage;
struct 	rbuf 		*rget_cache();
struct 	rbuf 		*rgeteblk();
struct 	rbuf 		*chk_rlist();
uint			rcache_out();


/*
 *	Fast access to buffers in cache by hashing.
 */
#define rbhash(d,b)	((struct rbuf *)&rhbuf[((int)d+(int)b)&rhmask])
/*
 * Key value for rfhash is fhandle (file handle), which is a full-word aligned
 * memory address.  It therefore needs to be shifted right two places
 * before using it as a hash key.
 */
#define rfhash(d)	((struct rbuf *)&rhbuf[((int)(d>>2))&rhmask])

struct	rhbuf
{
	int	b_flags;
	struct	rbuf *b_forw;
	struct	rbuf *b_back;
	struct	rbuf *av_forw;
	struct	rbuf *av_back;	
	struct	rbuf *f_forw;			
	struct	rbuf *f_back;		
};

extern	struct	rhbuf	rhbuf[];

/*
 * Unlink a buffer from the remote freelist list and mark it busy.
 * Spls not needed since interrupts should
 * never return buffers to remote freelist.
 */
#define remnotavail(bp) \
{\
	register s;\
\
	bp->av_back->av_forw = bp->av_forw;\
	bp->av_forw->av_back = bp->av_back;\
	bp->b_flags |= B_BUSY;\
}
