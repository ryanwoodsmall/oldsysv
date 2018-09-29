/* @(#)proc.h	1.1 */
/*
 * One structure allocated per active process. It contains all data needed
 * about the process while the process may be swapped out.
 * Other per process data (user.h) is swapped with the process.
 */

struct	proc {
	char	p_stat;
	char	p_flag;
	char	p_pri;		/* priority, negative is high */
	char	p_time;		/* resident time for scheduling */
	char	p_cpu;		/* cpu usage for scheduling */
	char	p_nice;		/* nice for cpu usage */
	ushort	p_uid;		/* real user id */
	ushort	p_suid;		/* set (effective) user id */
	short	p_pgrp;		/* name of process group leader */
	short	p_pid;		/* unique process id */
	short	p_ppid;		/* process id of parent */
	ushort	p_addr;		/* address of swappable image */
	ushort	p_size;		/* size of swappable image (clicks) */
	long	p_sig;		/* signals pending to this process */
	caddr_t	p_wchan;	/* event process is awaiting */
	struct text *p_textp;	/* pointer to text structure */
	struct proc *p_link;	/* linked list of running processes */
	int	p_clktim;	/* time to alarm clock signal */
};

extern struct proc proc[];	/* the proc table itself */

/* stat codes */
#define	SSLEEP	1		/* awaiting an event */
#define	SWAIT	2		/* (abandoned state) */
#define	SRUN	3		/* running */
#define	SIDL	4		/* intermediate state in process creation */
#define	SZOMB	5		/* intermediate state in process termination */
#define	SSTOP	6		/* process being traced */
#define	SXBRK	7		/* process being xswapped */
#define	SXSTK	8		/* process being xswapped */

/* flag codes */
#define	SLOAD	01		/* in core */
#define	SSYS	02		/* scheduling process */
#define	SLOCK	04		/* process cannot be swapped */
#define	SSWAP	010		/* process is being swapped out */
#define	STRC	020		/* process is being traced */
#define	SWTED	040		/* another tracing flag */
#define	STEXT	0100		/* text pointer valid */
#define	SSPART	0200		/* process is partially swapped out */

/*
 * Structure used to access saved times and status
 * of a dead process, by the parent.
 * Overlays the proc structure.
 */
struct	xproc {
	char	xp_stat;
	char	xp_flag;
	char	xp_pri;		/* priority, negative is high */
	char	xp_time;	/* resident time for scheduling */
	char	xp_cpu;		/* cpu usage for scheduling */
	char	xp_nice;	/* nice for cpu usage */
	ushort	xp_uid;		/* real user id */
	ushort	xp_suid;	/* set (effective) user id */
	short	xp_pgrp;	/* name of process group leader */
	short	xp_pid;		/* unique process id */
	short	xp_ppid;	/* process id of parent */
	short	xp_xstat;	/* Exit status for wait */
	time_t	xp_utime;	/* user time, this proc */
	time_t	xp_stime;	/* system time, this proc */
};
