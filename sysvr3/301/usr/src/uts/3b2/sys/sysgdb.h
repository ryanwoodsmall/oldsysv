/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/sysgdb.h	10.2"

/* sysgen data block descriptors */

struct	sgcom
{
	long	sgjid;	/* sysgen job id */
	unsigned char	sdbsize;	/* sysgen data block size in words */
	unsigned char	sgopc;	/* sysgen opcode */
	unsigned char	njrq;	/* number of job request queues */
	unsigned char	njcq;	/* number of job completion queues */
};

/* sysgen common job queue descriptor */

struct	sgjqd
{
	paddr_t	*jqsa;	/* queue start physical address */
	paddr_t	*jqldp;	/* queue load pointer phys addr. */
	paddr_t	*jquldp;	/* queue unload pointer phys. addr. */
	long	jqsize;	/* queue size in words */
	long	jqdsp1;	/* job queue descriptor spare */
};

