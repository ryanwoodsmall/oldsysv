/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/param.h	10.10"
/*
 * fundamental variables
 * don't change too often
 */
#include "sys/fs/s5param.h"

#define	MAXPID	30000		/* max process id */
#define	MAXUID	60000		/* max user id */
#define	MAXLINK	1000		/* max links */

#define	SSIZE	1		/* initial stack size (*2048 bytes) */
#define	SINCR	1		/* increment of stack (*2048 bytes) */
#define	USIZE	3		/* size of user block (*2048 bytes) */

#define	CANBSIZ	256		/* max size of typewriter line	*/
#define	HZ	100		/* 100 ticks/second of the clock */

#define	NCARGS	5120		/* # characters in exec arglist */
				/*   must be multiple of NBPW.  */

/*	The following define is here for temporary compatibility
**	and should be removed in the next release.  It gives a
**	value for the maximum number of open files per process.
**	However, this value is no longer a constant.  It is a
**	configurable parameter, NOFILES, specified in the kernel
**	master file and available in v.v_nofiles.  Programs which
**	include this header file and use the following value may
**	not operate correctly if the system has been configured
**	to a different value.
*/

#define	NOFILE	20

/*	The following represent the minimum and maximum values to
**	which the paramater NOFILES in the kernel master file may
**	be set.
*/

#define	NOFILES_MIN	 20
#define	NOFILES_MAX	100

/*	The following defines apply to the kernel virtual
**	address space.
*/

#define	SYSSEGSZ 1024	/* The size of the kernel segment	*/
			/* sysseg in pages.  The starting	*/
			/* address comes from the vuifile.	*/
#define	NS0SDE	0x120	/* Nbr of kernel segments in section 0.	*/
#define	NS1SDE	0x38	/* Nbr of kernel segments in section 1.	*/
#define	NS2SDE	0x0	/* Nbr of kernel segments in section 2.	*/
#define	NS3SDE	0x1	/* Nbr of kernel segments in section 3.	*/

/*	To avoid prefetch errors at the end of a region, it must
**	be padded with the following number of bytes.
*/

#define	PREFETCH	12

/*
 * priorities
 * should not be altered too much
 */

#define	PMASK	0177
#define	PCATCH	0400
#define	PSWP	0
#define	PINOD	10
#define	PRIBIO	20
#define	PZERO	25
#define PMEM	0
#define	NZERO	20
#define	PPIPE	26
#define	PWAIT	30
#define	PSLEP	39
#define	PUSER	60
#define	PIDLE	127

/*
 * fundamental constants of the implementation--
 * cannot be changed easily
 */

#define NBPS	0x20000		/* Number of bytes per segment */
#define	NBPW	sizeof(int)	/* number of bytes in an integer */
#define	NCPS	64		/* Number of clicks per segment */
#define	NBPC	2048		/* Number of bytes per click */
#define	BPCSHIFT	11	/* LOG2(NBPC) if exact */
#define	NULL	0
#define	CMASK	0		/* default mask for file creation */
#define	CDLIMIT	(1L<<11)	/* default max write address */
#define	NODEV	(dev_t)(-1)
#define	NBPSCTR		512	/* Bytes per disk sector.	*/
#define SCTRSHFT	9	/* Shift for BPSECT.		*/

#define	UMODE	3		/* current Xlevel == user */
#define	USERMODE(psw)	(psw.CM == UMODE)
#define	BASEPRI(psw)	(psw.IPL > 8)

#define	lobyte(X)	(((unsigned char *)&X)[1])
#define	hibyte(X)	(((unsigned char *)&X)[0])
#define	loword(X)	(((ushort *)&X)[1])
#define	hiword(X)	(((ushort *)&X)[0])

/*
 *  Interrupt stack size in STKENT units
 */
#define QSTKSZ	1000
#define ISTKSZ	200

#define	MAXSUSE	255

/* REMOTE -- whether machine is primary, secondary, or regular */
#define SYSNAME 9		/* # chars in system name */
#define PREMOTE 39
