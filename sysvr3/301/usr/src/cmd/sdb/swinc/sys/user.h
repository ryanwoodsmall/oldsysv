/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:swinc/sys/user.h	1.2"
#include "sys/pcb.h"
/*
 * The user structure.
 * One allocated per process.
 * Contains all per process data that doesn't need to be referenced
 * while the process is swapped.
 * The user block is USIZE*click bytes long; resides at virtual kernel
 * 0xc0000000(3B2);
 * contains the system stack per user for exec system call only;
 * is cross referenced with the proc structure for the same process.
 *
 ************************   NOTE  --  NOTE  ********************************
 *
 * 	WHEN THIS HEADER CHANGES, BOTH ml/misc.s AND ml/ttrap.s MUST BE
 *	   EXAMINED FOR ANY DEPENDENCIES UPON OFFSETS WITHIN THE UBLOCK.
 *	   IN PARTICULAR - .SET's ARE DONE ON:
 *				u_iop,
 *				u_pcb,
 *				u_stack,
 *				pr_base,
 *				pr_size,
 *				pr_off, and
 *				pr_scale
 *
 ***************************************************************************
 * 
 */
 
struct	user
{
	struct pcb u_pcb;		/* pcb, save area when switching */

	struct mmuproc u_sdt;		/* seg descriptors for this process */
	int *	u_ksbase;		/* boundary between kernel and user */
					/* portions of process stack.	    */

	int	u_shmcnt;		/* no. of shared segments attached  */
					/* to this process.		    */

	label_t	u_qsav;			/* label variable for quits and interrupts */

	char	u_dmm;			/*  0: no basic(1) support of double-mapped data */
					/* !0: data is double mapped with segments 76, 77, 78, and 79 */

	char	u_segflg;		/* IO flag: 0:user D; 1:system; 2:user I */
	char	u_error;		/* return error code */

	ushort	u_uid;			/* effective user id */
	ushort	u_gid;			/* effective group id */
	ushort	u_ruid;			/* real user id */
	ushort	u_rgid;			/* real group id */

	struct proc *u_procp;		/* pointer to proc structure */

	int	*u_ap;			/* pointer to arglist */

	union {				/* syscall return values */
		struct	{
			int	r_val1;
			int	r_val2;
		}r_reg;
		off_t	r_off;
		time_t	r_time;
	} u_r;

	caddr_t	u_base;			/* base address for IO */
	unsigned u_count;		/* bytes remaining for IO */
	off_t	u_offset;		/* offset in file for IO */
	short	u_fmode;		/* file mode for IO */
	ushort	u_pbsize;		/* Bytes in block for IO */
	ushort	u_pboff;		/* offset in block for IO */
	dev_t	u_pbdev;		/* real device for IO */
	daddr_t	u_rablock;		/* read ahead block address */
	short	u_errcnt;		/* syscall error count */

	struct inode *u_cdir;		/* pointer to inode of current directory */

	struct inode *u_rdir;		/* root directory of current process */
	caddr_t	u_dirp;			/* pathname pointer */
	struct direct u_dent;		/* current directory entry */
	struct inode *u_pdir;		/* inode of parent directory of dirp */

	struct file *u_ofile[NOFILE];	/* pointers to file structures of open files */
	char	u_pofile[NOFILE];	/* per-process flags of open files */

	int	u_arg[6];		/* arguments to current system call */

	unsigned u_tsize;		/* text size (clicks) */
	unsigned u_dsize;		/* data size (clicks) */
	unsigned u_ssize;		/* stack size (clicks) */

	int	u_signal[NSIG];		/* disposition of signals */

	/*
	 *    This offset must be reflected in misc.s and ttrap.s
	 */
	int	*u_iop;			/* secret 3B2 illegal opcode
					 * for floating point simulation
					 */

	time_t	u_utime;		/* this process user time */
	time_t	u_stime;		/* this process system time */
	time_t	u_cutime;		/* sum of childs' utimes */
	time_t	u_cstime;		/* sum of childs' stimes */

	int	*u_ar0;			/* address of users saved R0 */

/*
 *    The offsets of these elements must be reflected in ttrap.s and misc.s
 */
	struct {			/* profile arguments */
		short	*pr_base;	/* buffer base */
		unsigned pr_size;	/* buffer size */
		unsigned pr_off;	/* pc offset */
		unsigned pr_scale;	/* pc scaling */
	} u_prof;

	short  *u_ttyp;			/* pointer to pgrp in "tty" struct */
	dev_t	u_ttyd;			/* controlling tty dev */

	struct {			/* data extracted from executable file */

		long ux_tsize;		/* text size */
		long ux_toffset;	/* file offset to raw text */
		long ux_dsize;		/* data size */
		long ux_doffset;	/* file offset to raw data */
		long ux_bsize;		/* bss size */
		long ux_entloc;		/* entry location */

		/*
		 *   Common object file header
		 */

		struct {
			ushort	f_magic;	/* magic number
						 *
						 *   NOTE:   For 3b-5, the old values
						 *   of magic numbers will be in the
						 *   optional header in the structure
						 *   "aouthdr" (identical to old unix
						 *   aouthdr).
						 */
#define  M32MAGIC	0560

			ushort	f_nscns;	/* number of sections */
			long	f_timdat;	/* time & date stamp */
			long	f_symptr;	/* file pointer to symtab */
			long	f_nsyms;	/* number of symtab entries */
			ushort	f_opthdr;	/* sizeof(optional hdr) */
			ushort	f_flags;	/* flags
						 *
						 *	F_EXEC		file is executable
						 *		(i.e. no unresolved externel
						 *		references)
						 *	F_AR16WR	this file created on
						 *		AR16WR machine(e.g. 11/70)
						 *	F_AR32WR	this file created on
						 *		AR32WR machine(e.g. vax)
						 *	F_AR32W		this file created on
						 *		AR32W machine (e.g. 3b,maxi)
						 */

#define  F_EXEC		0000002
#define  F_AR16WR	0000200
#define  F_AR32WR	0000400
#define  F_AR32W	0001000

		} ux_filehdr;

		/*
		 *  Common object file section header
		 */
		struct {
			char	s_name[8];	/* section name */

#define _TEXT ".text"
#define _DATA ".data"
#define _BSS  ".bss"

			long	s_paddr;	/* physical address */
			long	s_vaddr;	/* virtual address */
			long	s_size;		/* section size */
			long	s_scnptr;	/* file ptr to raw data for section */
			long	s_relptr;	/* file ptr to relocation */
			long	s_lnnoptr;	/* file ptr to line numbers */
			ushort	s_nreloc;	/* number of relocation entries */
			ushort	s_nlnno;	/* number of line number entries */
			long	s_flags;	/* flags */


#define STYP_REG	0x00		/* "regular" section: allocated, relocated, loaded */

		} ux_scnhdr;

		/*
		 * Common object file optional unix header
		 */
		struct {
			short	o_magic;	/* magic number */
			short	o_stamp;	/* stamp */
			long	o_tsize;	/* text size */
			long	o_dsize;	/* data size */
			long	o_bsize;	/* bss size */
			long	o_entloc;	/* entry location */
			long	o_tstart;
			long	o_dstart;
		} ux_aouthdr;
	} u_exdata;

	char	u_comm[DIRSIZ];

	time_t	u_start;
	time_t	u_ticks;
	long	u_mem;
	long	u_ior;
	long	u_iow;
	long	u_iosw;
	long	u_ioch;
	char	u_acflag;

	short	u_cmask;		/* mask for file creation */
	daddr_t	u_limit;		/* maximum write address */

	short	u_lock;			/* process/text locking flags */

	char	u_rstrfix;		/*  set to one if a.out was built
					    with restore fix		*/

	long u_spop;			/* support processor being used */

#define U_SPOP_MAU	0x1L

	/*
	 * The offset of the following structure must be
	 * reflected in mau.c (the mau driver)
	 */

	struct mau_st {			/* structure to save mau regs */
		long asr;
		long dr[3];
		long fpregs[4][3];
	} u_mau;

	/*
	 *    This offset must be reflected in misc.s and ttrap.s
	 */
	int	u_stack[1];		/* kernel stack per user for exec
					 * system call;
					 * extends from here upward
					 * not to reach u + ctob(USIZE)
					 */
};

extern struct user u;

#define	u_rval1	u_r.r_reg.r_val1
#define	u_rval2	u_r.r_reg.r_val2
#define	u_roff	u_r.r_off
#define	u_rtime	u_r.r_time

/* ioflag values: Read/Write, User/Kernel, Ins/Data */
#define	U_WUD	0
#define	U_RUD	1
#define	U_WKD	2
#define	U_RKD	3
#define	U_WUI	4
#define	U_RUI	5

#define	EXCLOSE	01
