/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/proc.h	10.10"
/*	One structure allocated per active process. It contains all
**	data needed about the process while the process may be swapped
**	out.  Other per process data (user.h) may swapped with the
**	process but in fact it is not.
*/

typedef struct	proc	{
	char	p_stat;			/* Status of process.	     */
	char	p_pri;			/* priority, negative is high */
	char	p_cpu;			/* cpu usage for scheduling */
	char	p_nice;			/* nice for cpu usage */
	uint	p_flag;			/* Flags defined below.	     */
	ushort	p_uid;			/* real user id */
	ushort	p_suid;			/* saved (effective) user id */
					/* from exec.		     */
	short	p_pgrp;			/* name of process group     */
					/* leader		     */
	short	p_pid;			/* unique process id */
	short	p_ppid;			/* process id of parent */
	ushort	p_sgid;			/* saved (effective) group   */
					/* id from exec.	     */
	int	p_sig;			/* signals pending to this   */
					/* process		     */
	struct	proc	*p_flink;	/* linked list of processes */
	struct	proc	*p_blink;	/* linked list of processes */
	caddr_t	p_wchan;		/* Wait addr for sleeping   */
					/* processes.		    */
	struct	proc	*p_parent;	/* ptr to parent process    */
	struct	proc	*p_child;	/* ptr to first child process */
	struct	proc	*p_sibling;	/* ptr to next sibling	    */
					/* process on chain	    */

	uint	p_srama[2];		/* SDT addresses for sections */
					/* 2 and 3.		      */
	SRAMB	p_sramb[2];		/* Sramb entries for sections */
					/* 2 and 3.		      */

	int	p_clktim;		/* time to alarm clock signal */
	uint	p_size;			/* size of swappable image  */
					/* in pages.		    */
	time_t	p_utime;		/* user time, this proc */
	time_t	p_stime;		/* system time, this proc */
	struct  proc *p_mlink;		/* link list of processes    */
					/* sleeping on memwant or    */
					/* swapwant.	  	     */
	pde_t	p_ubptbl[USIZE + 7];	/* Page table for the u-block. */

	preg_t	*p_region;		/* process regions */
	ushort	p_mpgneed;		/* number of memory pages    */
					/* needed in memwant.	     */
	char	p_time;			/* resident time for scheduling */
	short	p_epid;			/* effective pid             */
					/* normally - same as p_pid  */
					/* if server - p_pid that sent msg */
	sysid_t p_sysid;		/* normally - same as sysid */
					/* if server - system that sent msg */
	struct	rcvd  *p_minwd;		/* server msg arrived on this queue */
	struct	proc  *p_rlink;		/* linked list for server */
	int	p_trlock;
	struct inode *p_trace;		/* pointer to /proc inode */
	long	p_sigmask;		/* tracing signal mask for /proc */
	int	p_hold;			/* hold signal bit mask */
	int	p_chold;		/* defer signal bit mask */
					/* sigset turns on this bit */
					/* signal does not turn on this bit */
	short	p_xstat;		/* exit status for wait */

} proc_t;

#define	p_link	p_flink

extern struct proc proc[];		/* the proc table itself */

/* stat codes */

#define	SSLEEP	1		/* Awaiting an event.		*/
#define	SRUN	2		/* Running.			*/
#define	SZOMB	3		/* Process terminated but not	*/
				/* waited for.			*/
#define	SSTOP	4		/* Process stopped by signal	*/
				/* since it is being traced by	*/
				/* its parent.			*/
#define	SIDL	5		/* Intermediate state in	*/
				/* process creation.		*/
#define	SONPROC	6		/* Process is being run on a	*/
				/* processor.			*/
#define SXBRK	7		/* process being xswapped       */

/* flag codes */

#define	SSYS	0x0001		/* System (resident) process.	*/
#define	STRC	0x0002		/* Process is being traced.	*/
#define	SWTED	0x0004		/* Stopped process has been	*/
				/* given to parent by wait	*/
				/* system call.  Don't return	*/
				/* this process to parent again	*/
				/* until it runs first.		*/
#define SNWAKE	0x0008		/* Process cannot wakeup by	*/
				/* a signal.			*/
#define SLOAD	0x0010		/* in core                      */
#define SLOCK   0x0020		/* Process cannot be swapped.	*/
#define	SRSIG	0x0040		/* Set when signal goes remote	*/
#define SPOLL	0x0080		/* Process in stream poll	*/
#define SPRSTOP	0x0100		/* process is being stopped via /proc */
#define SPROCTR	0x0200		/* signal tracing via /proc */
#define SPROCIO	0x0400		/* doing I/O via /proc, so don't swap */
#define SSEXEC	0x0800		/* stop on exec */
#define SPROPEN	0x1000		/* process is open via /proc */

#define PTRACED(p)	((p)->p_flag&(STRC|SPROCTR|SSEXEC|SPROPEN))

/*	The following macro is required because a page table must
**	be on a double word boundary and it may not be in the
**	proc table.
*/

#define	ubptbl(PP) \
	(&(PP)->p_ubptbl[((0x20 - (((int)(PP)->p_ubptbl) & 0x1f)) & 0x1f) >> 2])

