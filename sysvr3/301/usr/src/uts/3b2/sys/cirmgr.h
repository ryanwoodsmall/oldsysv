/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/cirmgr.h	10.15"

#define MAXTOKLEN sizeof(struct token)		/* maximum token length in bytes */

struct gdpmisc {
	int hetero;
	int version;
};

struct token {
	int	t_id;	 /* token id for differentiating multiple ckts	*/
	char	t_uname[MAXDNAME]; /* full domain name of machine		*/
};


struct gdp {
	struct queue *queue;	/* pointer to associated stream head	*/
	struct file *file;	/* file pointer to stream head we stole */
	short mntcnt;		/* number of mounts on this stream	*/
	short sysid;
	short flag;		/* connection info */		
	int hetero;		/* need to canonicalize messages	*/
	int version;		/* DU version at the other end of circuit */
	long time;		/* time delta */
	struct token token;	/* circuit identification		*/
	char	*idmap[2];	/* 0=uid=UID_DEV, 1=gid=GID_DEV		*/
};

extern int maxgdp;
extern struct gdp gdp[];
#define get_sysid(x)       ((struct gdp *)(x)->sd_queue->q_ptr)->sysid
#define	GDP(x)		((struct gdp *)(x)->q_ptr)


/* GDP circuit state flags */
#define GDPRECOVER	0x004
#define GDPDISCONN	0x002
#define GDPCONNECT	0x001
#define GDPFREE		0x000
