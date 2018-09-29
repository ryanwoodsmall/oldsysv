/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
/* @(#)space.h	6.13 */

#define	KERNEL
#include "sys/acct.h"
struct	acct	acctbuf;
struct	inode	*acctp;

#include "sys/tty.h"
struct	cblock	cfree[NCLIST];

#include "sys/buf.h"
struct	buf	bfreelist;	/* head of the free list of buffers */
struct	pfree	pfreelist;	/* Head of physio header pool */
struct	buf	pbuf[NPBUF];	/* Physical io header pool */

struct	hbuf	hbuf[NHBUF];	/* buffer hash table */

#include "sys/file.h"
struct	file	file[NFILE];	/* file table */

#include "sys/inode.h"
struct	inode	inode[NINODE];	/* inode table */

#include "sys/proc.h"
struct	proc	proc[NPROC];	/* process table */


#include "sys/map.h"
struct map sptmap[SPTMAP] = {mapdata(SPTMAP)};

#include "sys/callo.h"
struct callo callout[NCALL];

#include "sys/mount.h"
struct mount mount[NMOUNT];

#include "sys/elog.h"
#include "sys/err.h"
struct	err	err = {
	NESLOT,
};

#include "sys/sysinfo.h"
struct sysinfo sysinfo;
struct syswait syswait;
struct syserr syserr;

#include "sys/opt.h"

#include "sys/var.h"
struct var v = {
	NBUF,
	NCALL,
	NINODE,
	(char *)(&inode[NINODE]),
	NFILE,
	(char *)(&file[NFILE]),
	NMOUNT,
	(char *)(&mount[NMOUNT]),
	NPROC,
	(char *)(&proc[1]),
	0,
	(char *)0,
	REGIONS,
	NCLIST,
	NSABUF,
	MAXUP,
	0,
	NHBUF,
	NHBUF-1,
	NPBUF,
	0,
	SPTMAP,
	VHNDFRAC,
	MAXPMEM
};


#include "sys/page.h"
#include "sys/region.h"
reg_t	region[REGIONS];
reg_t	ractive;
reg_t	rfree;

#include "sys/pfdat.h"
struct pfdat	phead;		/* Head of free page list. */
struct pfdat	*pfdat;		/* Page frame database. */
struct pfdat	*phash;		/* Page hash access to pfdat. */
struct pfdat	ptfree;		/* Head of page table free list. */
int		phashmask;	/* Page hash mask. */

#include	"sys/tuneable.h"
tune_t	tune = {
		GETPGSLO,	/* t_gpgslo - get pages low limit.	*/
		GETPGSHI,	/* t_gpgshi - get pages high limit.	*/
		AGERATE,	/* t_age */
		VHANDR,		/* t_handr - vhand wakeup rate.		*/
		VHANDL,		/* t_vhandl - vhand steal limit.	*/
		MAXSC,		/* t_maxsc - max contiguous swap cnt.	*/
		PREPAGE,	/* t_prep - max swap/file pre page factor */
};

#include	"sys/swap.h"
swpt_t	swaptab[MSFILES];
int	nextswap;
int	swapwant;

#include "sys/init.h"

#ifndef	PRF_0
prfintr() {}
int	prfstat;
#endif

#include "sys/iobuf.h"
#ifdef	TRACE_0
#include "sys/trace.h"
struct trace trace[TRACE_0];
#endif

#if MESG==1
#include	"sys/ipc.h"
#include	"sys/msg.h"

struct map	msgmap[MSGMAP];
struct msqid_ds	msgque[MSGMNI];
struct msg	msgh[MSGTQL];
struct msginfo	msginfo = {
	MSGMAP,
	MSGMAX,
	MSGMNB,
	MSGMNI,
	MSGSSZ,
	MSGTQL,
	MSGSEG
};
#endif

#if SEMA==1
#	ifndef IPC_ALLOC
#	include	"sys/ipc.h"
#	endif
#include	"sys/sem.h"
struct semid_ds	sema[SEMMNI];
struct sem	sem[SEMMNS];
struct map	semmap[SEMMAP];
struct	sem_undo	*sem_undo[NPROC];
#define	SEMUSZ	(sizeof(struct sem_undo)+sizeof(struct undo)*SEMUME)
int	semu[((SEMUSZ*SEMMNU)+NBPW-1)/NBPW];
union {
	short		semvals[SEMMSL];
	struct semid_ds	ds;
	struct sembuf	semops[SEMOPM];
}	semtmp;

struct	seminfo seminfo = {
	SEMMAP,
	SEMMNI,
	SEMMNS,
	SEMMNU,
	SEMMSL,
	SEMOPM,
	SEMUME,
	SEMUSZ,
	SEMVMX,
	SEMAEM
};
#endif

#if SHMEM==1
#	ifndef	IPC_ALLOC
#	include	"sys/ipc.h"
#	endif
#include	"sys/shm.h"
struct shmid_ds	*shm_shmem[NPROC*SHMSEG];
struct shmid_ds	shmem[SHMMNI];	
/*
struct	shmpt_ds	shm_pte[NPROC*SHMSEG];
*/
struct	shminfo shminfo = {
	SHMMAX,
	SHMMIN,
	SHMMNI,
	SHMSEG,
	SHMBRK,
	SHMALL
};
#endif

struct minfo minfo;

/*	Each process has 3 pregions (text, data, and stack) plus
 *	enough for the maximum number of shared memory segments.
 *	We also need one extra null pregion to indicate the end
 *	of the list.
 */

int	pregpp = 3 + SHMSEG + 1;

/* file and record locking */
#include <sys/flock.h>
struct flckinfo flckinfo = {
	FLCKREC,
	FLCKFIL,
	0,
	0,
	0,
	0
} ;

struct filock flox[FLCKREC];		/* lock structures */
struct flino flinotab[FLCKFIL];	/* inode to lock assoc. structures */

int	maxpmem = MAXPMEM;	/* Max physical memory to use.  Used for */
				/* system performance measurement purposes */
