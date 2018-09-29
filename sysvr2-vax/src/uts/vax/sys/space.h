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

#ifdef	VP_0
#include "sys/vp.h"
#endif

#include "sys/iobuf.h"
#ifdef	DISK_0
#define	RM05_0
#define	RP06_0
#define	RM80_0
#define	RP07_0
#ifndef	DISK_1
#define	DISK_1	0
#endif
#ifndef	DISK_2
#define	DISK_2	0
#endif
#ifndef	DISK_3
#define	DISK_3	0
#endif
#define	DISKS	(DISK_0+DISK_1+DISK_2+DISK_3)
struct iobuf gdtab[DISKS];
struct iobuf gdutab[DISKS*8];
int	gdindex[DISKS*8];
short	gdtype[DISKS*8];
struct iotime gdstat[DISKS*8];
#endif

#ifdef	TRACE_0
#include "sys/trace.h"
struct trace trace[TRACE_0];
#endif

#ifdef  CSI_0
#define MAXKMC 16
#include "sys/csi.h"
#include "sys/csihdw.h"
struct csi csi_csi[CSI_0];
int csibnum = CSIBNUM;
struct csibuf *csibpt[CSIBNUM];
#ifndef DMK_0
unsigned short kmc_dmk[MAXKMC];
#endif
#endif

#ifdef	VPM_0
#include "sys/vpmt.h"
struct vpmt vpmt[VPM_0];
struct csibd vpmtbd[VPM_0*(XBQMAX + EBQMAX)];
struct csibuf vpmtbmt =
	{ { {0, }, }, 0, 0, 0,VPMNEXUS, };
struct vpminfo vpminfo =
	{XBQMAX, EBQMAX, VPM_0*(XBQMAX + EBQMAX)};
int vpmbsz= VPMBSZ;
#endif

#ifdef	DMK_0
#define MAXKMC 16
#define MAXDMK 16
#include	"sys/dmk.h"
struct dmksave dmksave[MAXDMK];
unsigned short kmc_dmk[MAXKMC];
#endif

#ifdef	X25_0
#include "sys/x25.h"
struct x25slot x25slot[X25_0];
struct x25tab x25tab[X25_0];
struct x25timer x25timer[X25_0];
struct x25link x25link[X25LINKS];
struct x25timer *x25thead[X25LINKS];
struct x25lntimer x25lntimer[X25LINKS];
struct csibd x25bd[X25BUFS];
struct csibuf x25buf=
	{ { {0, }, }, 0, 0, 0,X25NEXUS, };
struct x25info x25info =
	{X25_0, X25_0, X25LINKS, X25BUFS, X25BYTES};
#endif

#ifdef BX25S_0
#include "sys/bx25.space.h"
#endif

#ifdef PCL11B_0
#include "sys/pcl.h"
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
#ifdef	NSC_0
#include "sys/nscdev.h"
#endif
#ifdef ST_0
#include "sys/st.h"
struct stbhdr	stihdrb[STIHBUF];
struct stbhdr	stohdrb[STOHBUF];
struct ststat	ststat = {
	STIBSZ,	/* input buffer size */
	STOBSZ,	/* output buffer size */
	STIHBUF,	/* # of buffer headers */
	STOHBUF,	/* # of buffer headers */
	STNPRNT	/* # of printer channels */
};
struct csibuf   stibuf =
	{ { {0, }, }, 0, 0, 0,STNEXUS, };
struct csibuf   stobuf =
	{ { {0, }, }, 0, 0, 0,STNEXUS, };
#endif

#ifdef EM_0
#include <sys/em.h>
#ifndef EMBHDR
#define EMBHDR	(EM_0 * 6)	/* Default Buffer Headers */
#endif
struct csibd	embd[EMBHDR];	/* Buffer Descriptors */
int embhdr = EMBHDR;		/* No. of Buffer Headers */
int emtbsz = EMTBSZ;		/* Transmit Buffer Space */
int emrbsz = EMRBSZ;		/* Receive Buffer Space */
int emrcvsz = EMRCVSZ;		/* Receive Buffer Size */
struct csibuf embmt =
	{ { {0, }, }, 0, 0, 0,EMNEXUS, };
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
