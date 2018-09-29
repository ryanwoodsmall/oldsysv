/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/proc.h	10.10.2.12"
/*
 * One structure allocated per active process.  It contains all
 * data needed about the process while the process may be swapped
 * out.  Other per-process data (user.h) may be swapped with the
 * process.
 */

typedef struct	proc {
	char	p_stat;			/* status of process */
	char	p_pri;			/* priority, negative is high */
	char	p_cpu;			/* cpu usage for scheduling */
	char	p_nice;			/* nice for cpu usage */
	uint	p_flag;			/* flags defined below */
	ushort	p_uid;			/* real user id */
	ushort	p_suid;			/* saved (effective) uid from exec */
	short	p_pgrp;			/* name of process group leader */
	short	p_pid;			/* unique process id*/
	short	p_ppid;			/* process id of parent*/
	ushort	p_sgid;			/* saved (effective) gid from exec */
	int	p_sig;			/* signals pending to this process */
	struct	proc	*p_flink;	/* forward link */
	struct	proc	*p_blink;	/* backward link */
	caddr_t	p_wchan;		/* wait addr for sleeping processes */
	struct	proc	*p_parent;	/* ptr to parent process */
	struct	proc	*p_child;	/* ptr to first child process */
	struct	proc	*p_sibling;	/* ptr to next sibling proc on chain */
	uint	p_srama[2];		/* SDT addresses for sections 2 and 3 */
	SRAMB	p_sramb[2];		/* SRAMB entries for sections 2 and 3 */
	int	p_clktim;		/* time to alarm clock signal */
	uint	p_size;			/* size of swappable image in pages */
	time_t	p_utime;		/* user time, this process */
	time_t	p_stime;		/* system time, this process */
	struct  proc *p_mlink;		/* linked list of processes sleeping
					 * on memwant or swapwant
					 */
	pde_t	p_ubptbl[USIZE + 7];	/* page table for the u-block */
	preg_t	*p_region;		/* process regions */
	ushort	p_mpgneed;		/* number of memory pages needed in
					 * memwant.
					 */
	char	p_time;			/* resident time for scheduling */
	unchar	p_cursig;		/* current signal */
	short	p_epid;			/* effective pid; normally same as
					 * p_pid; for servers, the system that
					 * sent the msg
					 */
	sysid_t p_sysid;		/* normally same as sysid; for servers,
					 * the system that sent the msg
					 */
	struct	rcvd  *p_minwd;		/* server msg arrived on this queue */
	struct	proc  *p_rlink;		/* linked list for server */
	int	p_trlock;
	struct	inode *p_trace;		/* pointer to /proc inode */
	long	p_sigmask;		/* tracing signal mask for /proc */
	int	p_hold;			/* hold signal bit mask */
	int	p_chold;		/* deferred signal bit mask; sigset(2)
					 * turns these bits on while signal(2)
					 * does not.
					 */
	short	p_xstat;		/* exit status for wait */
	dbd_t	p_ubdbd[USIZE];		/* DBDs for ublock when swapped out */
	ushort	p_whystop;		/* Reason for process stop */
	ushort	p_whatstop;		/* More detailed reason */
} proc_t;

#define	p_link	p_flink

extern struct proc proc[];		/* the proc table itself */

/* stat codes */

#define	SSLEEP	1		/* awaiting an event */
#define	SRUN	2		/* running */
#define	SZOMB	3		/* process terminated but not waited for */
#define	SSTOP	4		/* process stopped by debugger */
#define	SIDL	5		/* intermediate state in process creation */
#define	SONPROC	6		/* process is being run on a processor */
#define SXBRK	7		/* process being xswapped */

/* flag codes */

#define	SSYS	0x00000001	/* system (resident) process */
#define	STRC	0x00000002	/* process is being traced */
#define	SWTED	0x00000004	/* ptraced process has been given to parent
				 * by wait(2); don't return this process to
				 * parent again until it runs first.
				 */
#define	SNWAKE	0x00000008	/* process cannot be awakened by a signal */
#define	SLOAD	0x00000010	/* in core */
#define	SLOCK   0x00000020	/* process cannot be swapped */
#define	SRSIG	0x00000040	/* set when signal goes remote */
#define	SPOLL	0x00000080	/* process in stream poll */
#define	SPRSTOP	0x00000100	/* process is being stopped via /proc */
#define	SPROCTR	0x00000200	/* signal or syscall tracing via /proc */
#define	SPROCIO	0x00000400	/* doing I/O via /proc, so don't run */
#define	SSEXEC	0x00000800	/* stop on exec */
#define	SPROPEN	0x00001000	/* process is open via /proc */
#define	SULOAD  0x00002000	/* u-block in core */
#define	SRUNLCL	0x00004000	/* set process running on last /proc close */
#define	SNOSTOP	0x00008000	/* proc asleep, stop not allowed */
#define	SPTRX	0x00010000	/* process is exiting via ptrace(2) */
#define	SASLEEP	0x00020000	/* proc is stopped within a call to sleep() */
#define	SUSWAP	0x00040000	/* u-block is being swapped in or out */
#define	SUWANT	0x00080000	/* waiting for u-block swap to complete */

#define PTRACED(p)	((p)->p_flag&(STRC|SPROCTR|SSEXEC|SPROPEN))

/*
 * The following macro is required because a page table must
 * be on a double word boundary and it may not be in the
 * proc table.
 */

#define	ubptbl(PP) \
	(&(PP)->p_ubptbl[((0x20 - (((int)(PP)->p_ubptbl) & 0x1f)) & 0x1f) >> 2])

/* Flags for newproc() */

#define NP_FAILOK	0x1	/* don't panic if cannot create process */
#define NP_NOLAST	0x2	/* don't use last process slot */
#define	NP_SYSPROC	0x4	/* system (resident) process */

/* Reasons for stopping (values of p_whystop) */

#define	REQUESTED	1
#define	SIGNALLED	2
#define	SYSENTRY	3
#define	SYSEXIT		4

/* Macro to reduce unnecessary calls to issig() */

#define	ISSIG(p, why) \
  ((p)->p_cursig || (((p)->p_sig || ((p)->p_flag & SPRSTOP)) && issig(why)))

/* Reasons for calling issig() */

#define	FORREAL		0	/* Usual side-effects */
#define	JUSTLOOKING	1	/* Don't stop the process */
