/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/omnisubr.h	10.3"


#define LPNO 		64		/* Number of Local Ports available  */
#define NIMAP		(3*LPNO)	/* Size of NI mapping table */
#define WORDBRY		0xfffffffc	/* word boundary value  */
#define OMNI_PRI_LEV 	PZERO		/* Priority Level */
#define ENITIM		208		/* TIMEOUT ERROR  */
#define OMNITIMOUT	500		/* Length */
#define OMNISLEEP(a)	sleep(a,OMNI_PRI_LEV)

#define SAMEID(x,y)	(x[0] == y[0]&& \
			  x[1] == y[1]&& \
			  x[2] == y[2]&& \
			  x[3] == y[3]&& \
			  x[4] == y[4]&& \
			  x[5] == y[5] \
)

struct port_ctl {
	short	port;
	short	open;
	long	type;		/* read, write, or both */
	long	que_size;
	long	buf_size;
	char	*buf_add;
	int  	time_id;	/* timeout id used with timeout () */
	caddr_t slp_ad;		/* sleep address for timeout() */
	char	*load_ptr;
	char	*unload_ptr;
	short	reset;		/* reset state of port */
	long 	key;		/* "magic cookie" key from user */
	short 	link;		/* link to next group member */
	short 	grpindex;	/* index into group list */
	long	sleep;		/* group sleep/wakeup pattern */
};

struct grpinfo {		/* group info list element  */
	long	sleep;		/* group sleep/wakeup pattern */
	short	portindex;	/* index into port_ctl array to */
				/*   first group member   */
	short	count;		/* count of group members  */
};

struct pack_buf {
	short	used;
	short	size;
	short	start;
	char	*next_buf;
	char	pack;
	char	null[3];	/* cluge for word boundary */
};

struct ni_ctl {
	struct map nimap[NIMAP];
	paddr_t segp;
	paddr_t ubmd;
	paddr_t	bubm;
	struct buf ubuf;
};

typedef char SIXCH[6];
