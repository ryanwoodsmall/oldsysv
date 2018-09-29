/* @(#)space.h	1.8 */
#include "sys/acct.h"
struct	acct	acctbuf;
struct	inode	*acctp;

#include "sys/tty.h"
struct	cblock	cfree[NCLIST];

#include "sys/buf.h"
struct	buf	bfreelist;	/* head of the free list of buffers */
struct	buf	buf[NBUF+NSABUF];	/* buffer headers */
char	sabuf	[NSABUF][BSIZE];
struct	pfree	pfreelist;	/* Head of physio header pool */
struct	buf	pbuf[NPBUF];	/* Physical io header pool */

struct	hbuf	hbuf[NHBUF];	/* buffer hash table */

#include "sys/file.h"
struct	file	file[NFILE];	/* file table */

#include "sys/inode.h"
struct	inode	inode[NINODE];	/* inode table */
daddr_t	iblk[NIBLOCK][NADDR];	/* Inode block cache */

#include "sys/proc.h"
struct	proc	proc[NPROC];	/* process table */

#include "sys/text.h"
struct	text text[NTEXT];	/* text table */

#include "sys/map.h"
struct map coremap[CMAPSIZ] = {mapdata(CMAPSIZ)};
struct map swapmap[SMAPSIZ] = {mapdata(SMAPSIZ)};

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

#include "sys/init.h"
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
	NTEXT,
	(char *)(&text[NTEXT]),
	NCLIST,
	NSABUF,
	MAXUP,
	CMAPSIZ,
	SMAPSIZ,
	NHBUF,
	NHBUF-1,
	NPBUF,
	NIBLOCK
};

#ifndef	PRF_0
prfintr() {}
int	prfstat;
#endif

#ifdef	PCL11B_0
#include "sys/pcl.h"
#endif

#ifdef	DISK_0
#define	RM05_0
#define	RP06_0
#ifndef	DISK_1
#define	DISK_1	0
#endif
#ifndef	DISK_2
#define	DISK_2	0
#endif
#ifndef	DISK_3
#define	DISK_3	0
#endif
#include "sys/iobuf.h"
#define	DISKS	(DISK_0+DISK_1+DISK_2+DISK_3)
struct iobuf gdtab[DISKS];
struct iobuf gdutab[DISKS*8];
int	gdindex[DISKS*8];
short	gdtype[DISKS*8];
struct iotime gdstat[DISKS*8];
#endif

#ifdef	DA11B_0
#include "sys/da.h"
#endif

#ifdef	DU11_0
#include "sys/du.h"
#endif

#ifdef	VP_0
#include "sys/vp.h"
#endif


int	mauscore;		/* Start of MAUS segments */
int	mausend;		/* Last core address of MAUS regions */
int	nmausent;		/* Number of entries in mausmap */



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

#ifdef	NSC_0
#include "sys/nscdev.h"
#endif

#ifdef	TRACE_0
#include "sys/trace.h"
struct trace trace[TRACE_0];
#endif

#ifdef CSI_0
#include "sys/csi.h"
#include "sys/csihdw.h"
struct csi csi_csi[CSI_0];
#endif

#ifdef	VPM_0
#include "sys/vpmt.h"
struct vpmt vpmt[VPM_0];
struct csibd vpmtbd[VPM_0*(XBQMAX + EBQMAX)];
struct vpminfo vpminfo =
	{XBQMAX, EBQMAX, VPM_0*(XBQMAX + EBQMAX)};
int vpmbsz = VPMBSZ;
#endif

#ifdef DMK_0
#define MAXDMK 4
#include "sys/dmk.h"
struct dmksave dmksave[MAXDMK];
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
struct csibuf x25buf;
struct x25info x25info =
	{X25_0, X25_0, X25LINKS, X25BUFS, X25BYTES};
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

#endif
