/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:swinc/sys/param.h	1.1"
/*
 * fundamental variables
 * don't change too often
 */

#define	NOFILE	20		/* max open files per process */
#define	MAXPID	30000		/* max process id */
#define	MAXUID	60000		/* max user id */
#define	MAXLINK	1000		/* max links */

#define	SSIZE	1		/* initial stack size (*2048 bytes) */
#define	SINCR	1		/* increment of stack (*2048 bytes) */
#define	USIZE	2		/* size of user block (*4096 bytes) */

#define	CANBSIZ	256		/* max size of typewriter line	*/
#define	HZ	100		/* 100 ticks/second of the clock */
#define	NCARGS	5120		/* # characters in exec arglist */

				/****************************************
				 *	If the value of NUSEGD changes, *
				 *	the 137 must also be changed in *
				 *	the definition of mmuproc       *
				 *	in types.h                      *
				 ****************************************/
#define NUSEGD  (NS2SDE + NS3SDE)	/* number of sde's per user process */
#define NS2SDE	116		/* number of section 2 sde's */
#define NS3SDE  21		/* number of section 3 sde's */
#define SEC2    0
#define SEC3	(SEC2 + NS2SDE)

/*
 * User processes are limited to eight segments
 * to avoid the 32A RESTORE bug.
 */

#define TSEG	(SEC2 + 64)	/* text starting segment descriptor number */
#define DSEG	(TSEG + 4)	/* data starting segment descriptor number */
#define DESEG	(DSEG + 4)	/* data ending segment descriptor number */
#define USEG	(SEC3 + 0)	/* ublock segment descriptor number */
#define STKSEG	(SEC3 + 1)	/* stack segment descriptor number */
#define SMSEG	(SEC3 + 5)	/* shared memory starting segment descriptor */



#define	PREFETCH	12	/* minimum padding for text to reduce prefetch errors */

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
#define	NICFREE	50		/* number of superblock free blocks */
#define	NULL	0
#define	CMASK	0		/* default mask for file creation */
#define	CDLIMIT	(1L<<11)	/* default max write address */
#define	NODEV	(dev_t)(-1)
#define	ROOTINO	((ino_t)2)	/* i number of all roots */
#define	SUPERB	((daddr_t)1)	/* block number of the super block */
#define	DIRSIZ	14		/* max characters per directory */
#define	NICINOD	100		/* number of superblock inodes */

/*
 * filesystem parameters
 */

#ifndef FsTYPE
#define FsTYPE	3
#endif


#if FsTYPE==1
/* Original 512 byte file system */
#define	BSIZE		512		/* size of secondary block (bytes) */
#define SBUFSIZE	512		/* system buffer size */
#define	BSHIFT		9		/* log2(BSIZE) */
#define	NINDIR		(BSIZE/sizeof(daddr_t))	/* BSIZE/sizeof(daddr_t) */
#define	BMASK		0777		/* BSIZE-1 */
#define INOPB		8		/* BSIZE/sizeof(struct dinode) */
#define INOSHIFT	3		/* log2(INOPB) */
#define	NMASK		0177		/* NINDIR-1 */
#define	NSHIFT		7		/* log2(NINDIR) */
#define NDPC		4		/* number of blocks/click */
#define Fs2BLK		0x8000		/* large block flag in dev */
#define FsBSIZE(dev)	BSIZE
#define FsBSHIFT(dev)	BSHIFT
#define FsNINDIR(dev)	NINDIR
#define FsBMASK(dev)	BMASK
#define FsINOPB(dev)	INOPB
#define FsLTOP(dev, b)	b
#define FsPTOL(dev, b)	b
#define FsNMASK(dev)	NMASK
#define FsNSHIFT(dev)	NSHIFT
#define FsITOD(dev, x)	(daddr_t)(((unsigned)x+(2*INOPB-1)) >> INOSHIFT)
#define FsITOO(dev, x)	(daddr_t)(((unsigned)x+(2*INOPB-1)) & (INOPB-1))
#define FsINOS(dev, x)	((x&~07)+1)
#endif

#if FsTYPE==2
/* New 1024 byte file system */
#define	BSIZE		1024		/* size of secondary block (bytes) */
#define SBUFSIZE	1024		/* system buffer size */
#define	BSHIFT		10		/* log2(BSIZE) */
#define	NINDIR		(BSIZE/sizeof(daddr_t))	/* BSIZE/sizeof(daddr_t) */
#define	BMASK		01777		/* BSIZE-1 */
#define INOPB		16		/* BSIZE/sizeof(struct dinode) */
#define INOSHIFT	4		/* log2(INOPB) */
#define	NMASK		0377		/* NINDIR-1 */
#define	NSHIFT		8		/* log2(NINDIR) */
#define NDPC		4		/* number of blocks per click */
#define Fs2BLK		0x8000		/* large block flag in dev */
#define FsBSIZE(dev)	BSIZE
#define FsBSHIFT(dev)	BSHIFT
#define FsNINDIR(dev)	NINDIR
#define FsBMASK(dev)	BMASK
#define FsINOPB(dev)	INOPB
#define FsLTOP(dev, b)	(b<<1)
#define FsPTOL(dev, b)	(b>>1)
#define FsNMASK(dev)	NMASK
#define FsNSHIFT(dev)	NSHIFT
#define FsITOD(dev, x)	(daddr_t)(((unsigned)x+(2*INOPB-1)) >> INOSHIFT)
#define FsITOO(dev, x)	(daddr_t)(((unsigned)x+(2*INOPB-1)) & (INOPB-1))
#define FsINOS(dev, x)	((x&~017)+1)
#endif

#if FsTYPE==3
/* Dual file system */
#define	BSIZE		512		/* size of secondary block (bytes) */
#define SBUFSIZE	1024		/* system buffer size */
#define	BSHIFT		9		/* log2(BSIZE) */
#define	NINDIR		128		/* BSIZE/sizeof(daddr_t) */
#define	BMASK		0777		/* BSIZE-1 */
#define INOPB		8		/* BSIZE/sizeof(struct dinode) */
#define INOSHIFT	3		/* log2(INOPB) */
#define	NMASK		0177		/* NINDIR-1 */
#define	NSHIFT		7		/* log2(NINDIR) */
#define NDPC		4
#define Fs2BLK		0x8000		/* large block flag in dev */
#define FsLRG(dev)	(dev&Fs2BLK)
#define FsBSIZE(dev)	(FsLRG(dev) ? 1024 : 512)
#define FsBSHIFT(dev)	(FsLRG(dev) ? 10 : 9)
#define FsNINDIR(dev)	(FsLRG(dev) ? 256 : 128)
#define FsBMASK(dev)	(FsLRG(dev) ? 01777 : 0777)
#define FsINOPB(dev)	(FsLRG(dev) ? 16 : 8)
#define FsLTOP(dev, b)	(FsLRG(dev) ? (b<<1) : b)
#define FsPTOL(dev, b)	(FsLRG(dev) ? (b>>1) : b)
#define FsNMASK(dev)	(FsLRG(dev) ? 0377 : 0177)
#define FsNSHIFT(dev)	(FsLRG(dev) ? 8 : 7)
#define FsITOD(dev, x)	(daddr_t)(FsLRG(dev) ? \
	((unsigned)x+31)>>4 : ((unsigned)x+15)>>3)
#define FsITOO(dev, x)	(daddr_t)(FsLRG(dev) ? \
	((unsigned)x+31)&017 : ((unsigned)x+15)&07)
#define FsINOS(dev, x)	(FsLRG(dev) ? \
	((x&~017)+1) : ((x&~07)+1))
#endif

#define SUPERBOFF	512	/* superblock offset */
#define	UMODE	3		/* current Xlevel == user */
#define	USERMODE(psw)	(psw.CM == UMODE)
#define	BASEPRI(psw)	(psw.IPL > 1)

#define	lobyte(X)	(((unsigned char *)&X)[1])
#define	hibyte(X)	(((unsigned char *)&X)[0])
#define	loword(X)	(((ushort *)&X)[1])
#define	hiword(X)	(((ushort *)&X)[0])

/*
 *  Interrupt stack size in STKENT units
 */
#define ISTKSZ	200
