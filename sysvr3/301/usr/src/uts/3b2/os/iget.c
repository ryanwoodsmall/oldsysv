/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/iget.c	10.16"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/immu.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/mount.h"
#include "sys/fs/s5dir.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/inode.h"
#include "sys/fstyp.h"
#include "sys/file.h"
#include "sys/ino.h"
#include "sys/stat.h"
#include "sys/var.h"
#include "sys/conf.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/open.h"
#include "sys/debug.h"
#include "sys/cmn_err.h"

extern struct mount	*pipemnt;
/*
 *	inode hashing
 */

#define	NHINO	128
#define ihash(X)	(&hinode[(int) (X) & (NHINO-1)])

struct	hinode	{
	struct	inode	*i_forw;
	struct	inode	*i_back;
} hinode[NHINO];

struct ifreelist ifreelist;

/*
 * initialize inodes
 */

inoinit()
{
	register struct inode *ip;
	register int i;

	ifreelist.av_forw = ifreelist.av_back =
		(struct inode *) &ifreelist;
	for (i = 0; i < NHINO; i++) {
		hinode[i].i_forw = hinode[i].i_back =
			(struct inode *) &hinode[i];
	}
	for (i = 0, ip = inode; i < v.v_inode; i++, ip++) {
		ip->i_forw = ip->i_back = ip;
		(ifreelist.av_forw)->av_back = ip;
		ip->av_forw = ifreelist.av_forw;
		ifreelist.av_forw = ip;
		ip->av_back = (struct inode *) &ifreelist;
	}
}

/*
 * Look up an inode by mount point (device) and inumber.
 * If it is in core (in the inode structure), honor the locking
 * protocol.
 * If it is not in core, read it in from the specified device.
 * If the inode is mounted on, perform the indicated indirection.
 * In all cases, a pointer to a locked inode structure is returned.
 *
 * printf warning: inode table overflow -- if the inode structure is
 *					   full.
 * panic: no imt -- if the mounted filesystem is not in the mount table.
 *	"cannot happen"
 */

#ifndef DEBUG
struct inode *
iget(mp, ino)
register struct mount *mp;
register ino;
#else
struct inode *
iget(mp, ino, caller)
register struct mount *mp;
register ino;
int *caller;
#endif
{
	register struct inode *ip;
	register struct hinode *hip;
	register short fstyp;
	register struct inode *tip;

	sysinfo.iget++;
	fstyp = mp->m_fstyp;
loop:
	hip = ihash(ino);
	for (ip = hip->i_forw; ip != (struct inode *) hip; ip = ip->i_forw)
		if (ino == ip->i_number && mp == ip->i_mntdev)
			goto found;

	ip = ifreelist.av_forw;
	while ((ip != (struct inode *)&ifreelist)) {
		/*
		 * If inode locked, assume it has been found in the cache
		 * by another process, so march on, march on...
		 */
		if (!(ip->i_flag & ILOCK))
			break;
		ip = ip->av_forw;
	}
	if (ip == (struct inode *)&ifreelist) {
		cmn_err(CE_WARN, "iget - inode table overflow");
		syserr.inodeovf++;
		u.u_error = ENFILE;
		return(NULL);
	}

	ASSERT(ip == ip->av_back->av_forw);
	ASSERT(ip == ip->av_forw->av_back);
	ip->av_back->av_forw = ip->av_forw;	/* remove from free list */
	ip->av_forw->av_back = ip->av_back;

	/* Remove from old hash list. */

	ip->i_back->i_forw = ip->i_forw;
	ip->i_forw->i_back = ip->i_back;
	ip->i_flag = ILOCK;
	ASSERT(addilock(ip) == 0);
	ASSERT(ip->i_count == 0);

	/*
	 * If a block number list was allocated for this file
	 * (because it is a 413), then free the space now since
	 * we are going to reuse this table slot for a different
	 * inode.  See the code in mmgt/region.c/mapreg.
	 * Also, dispose of the left-over data structures associated with
	 * an old inode whose slot we are about to re-use.  If FS_NOICACHE
	 * is set for this fstyp then old inodes don't remain in
	 * the cache and this disposal is taken care of at the time of
	 * last reference to the inode in iput().  We also assume that
	 * this disposal need not be done at all unless a new fstyp
	 * is being assigned to the inode.
	 */

	if (ip->i_fstyp) {
		FS_FREEMAP(ip);
		if ((fsinfo[ip->i_fstyp].fs_flags & FS_NOICACHE) == 0
	  	  && ip->i_fstyp != fstyp)
			FS_IPUT(ip);
	}

	hip->i_forw->i_back = ip;	/* insert into new hash list */
	ip->i_forw = hip->i_forw;
	hip->i_forw = ip;
	ip->i_back = (struct inode *) hip;

	ip->i_mntdev = mp;
	ip->i_mnton = NULL;
	ip->i_dev = mp->m_dev;
	ip->i_fstyp = fstyp;
	ip->i_number = ino;
	ip->i_count = 1;
#ifdef FSPTR
	ip->i_fstypp = &fstypsw[fstyp];
#endif
#ifdef DEBUG
	ilog(ip, 0, caller);
#endif
	tip = FS_IREAD(ip);
	/*
	 * fs_iread returns NULL to indicate an error.
	 * If NULL is returned, no fs dependent inode has been
	 * assigned to the fs independent inode.
	 * In this case the newly allocated fs independent
	 * inode should be deallocated, that is, returned to the
	 * free list and taken off the hash list. If it stayed
	 * on the hash list it could be "found" and its use would
	 * cause a panic because it has no fs specific portion.
	 */
	if (tip == NULL) {
		ip->i_back->i_forw = ip->i_forw; /* remove from hash list */
		ip->i_forw->i_back = ip->i_back;
		ip->i_forw = ip->i_back = ip;
		ifreelist.av_back->av_forw = ip;
		ip->av_forw = (struct inode *) &ifreelist;
		ip->av_back = ifreelist.av_back;
		ifreelist.av_back = ip;
		ip->i_fsptr = NULL;
		ip->i_count = 0;
		ip->i_fstyp = 0;
#ifdef DEBUG
		/*
		 * Use inline code (temporarily) here instead of a call to
		 * prele() to allow use of an otherwise-bogus assertion in
		 * prele().
		 */
		ip->i_flag &= ~ILOCK;
		if (ip->i_flag & IWANT) {
			ip->i_flag &= ~IWANT;
			wakeup((caddr_t)ip);
		}
		ASSERT(rmilock(ip) == 0);
#else
		prele(ip);
#endif
	}
	return(tip);
found:
	if (ip->i_flag & ILOCK) {
		ip->i_flag |= IWANT;
		sleep(ip, PINOD);
		goto loop;
	}

	if (ip->i_flag & (IMOUNT|IRMOUNT)) {
		mp = ip->i_mnton;
		ASSERT(mp != NULL);
		/*
		 * If MINTER is set it means that the file system
		 * is in the process of being unmounted. The
		 * fs-dependent umount routine flushed all of the
		 * inodes (via iflush()) and cleared m_mount.  The
		 * fs-independent umount has not yet done an iput()
		 * on the "mounted on" inode.
		 */
		if (mp->m_flags&MINTER) {
			u.u_error = EAGAIN;
			return(NULL);
		}
		if (mp->m_inodp == ip) {
			if (ip->i_flag & IMOUNT) {	/* local case */
				ip = mp->m_mount;
				if (ip == NULL) {
					cmn_err(CE_WARN, "Null m_mount in iget mp: %x\n", mp);
					u.u_error = EAGAIN; 
					return(NULL);
				}
				ino = ip->i_number;
				fstyp = ip->i_fstyp;
				goto found;
			} else {
				ip = remote_call(mp->m_mount);
				return(ip);
			}
		}
		cmn_err(CE_PANIC,
			"iget - mounted on inode not in mount table.");
	} else if ((ip->i_flag & ILBIN) && server() && !riget(ip))
		return(NULL);

	if (ip->i_count == 0) {
		/* remove from freelist */
		ASSERT(ip->av_back->av_forw == ip);
		ASSERT(ip->av_forw->av_back == ip);

		ip->av_back->av_forw = ip->av_forw;
		ip->av_forw->av_back = ip->av_back;
	}
	ip->i_count++;
	ip->i_flag |= ILOCK;
	ASSERT(addilock(ip) == 0);
#ifdef DEBUG
	ilog(ip, 1, caller);
#endif
	return(ip);
}

/*
 * Decrement reference count of an inode structure.
 * On the last reference, write the inode out and if necessary,
 * truncate and deallocate the file.
 */
#ifndef DEBUG
iput(ip)
register struct inode *ip;
#else
iput(ip, caller)
register struct inode *ip;
int *caller;
#endif
{
#ifdef DEBUG
	ilog(ip, 2, caller);
#endif
	ASSERT(ip->i_flag & ILOCK);

	/* Free the incore slot only if reference count is 1.	*/

	if (ip->i_count == 1) {

		ip->i_flag &= ~ITEXT;
		if (ip->i_ftype == IFCHR) ip->i_sptr = NULL;

		FS_IPUT(ip);

		/*
		 * Don't take ip off its current hash list unless the
		 * fstyp does not wish to make use of the inode cache. 
		 * In that case FS_NOICACHE will be set. If the cache is
		 * used an inode stays on this hash list, 
		 * with its inumber filled in until:
		 * (1) iget needs that inumber and re-uses it, or
		 * (2) iget allocates it off the freelist. 
		 * In case (1) iget removes it off the freelist. 
		 * In case (2) iget removes it off the freelist, 
		 * and also moves it from its current hash list 
		 * to the new hash list.
		 */
		if (fsinfo[ip->i_fstyp].fs_flags & FS_NOICACHE) {
			/* Remove this inode from its hash list. */
			ip->i_back->i_forw = ip->i_forw;
			ip->i_forw->i_back = ip->i_back;
			ip->i_forw = ip->i_back = ip;
			ip->i_fstyp = 0;
		}

		/* put inode on freelist */
		ASSERT(ip->av_back->av_forw != ip);
		ASSERT(ip->av_forw->av_back != ip);

		ifreelist.av_back->av_forw = ip;
		ip->av_forw = (struct inode *) &ifreelist;
		ip->av_back = ifreelist.av_back;
		ifreelist.av_back = ip;

		ip->i_count--;
#ifdef DEBUG
		/* Temporary. */
		ip->i_flag &= ~ILOCK;
		if (ip->i_flag & IWANT) {
			ip->i_flag &= ~IWANT;
			wakeup((caddr_t)ip);
		}
		ASSERT(rmilock(ip) == 0);
#else
		prele(ip);
#endif
		return;
	}

	ASSERT(ip->i_count > 1);
	ip->i_count--;
	prele(ip);
}

/*
 * Search inodes for those on dev
 * Purge any cached inodes.
 * Returns -1 if an active inode (desides root) is
 * found, otherwise 0
 * Warning: This code assumes that it runs on a simplex machine.
 * That is, it assumes that the inode table will not change while
 * it is being scanned. Let's hope that no drivers are changing 
 * the inode table at interrupt level!!!!
 */
iflush(mp)
register struct mount *mp;
{
	register dev_t dev;
	register inode_t	*ip;
	register inode_t	*rip;
	register		i;


	dev = mp->m_dev;
	rip = mp->m_mount;
	ASSERT(rip != NULL);
	for (i = 0, ip = inode; i < v.v_inode; i++, ip++) {
		if (ip->i_dev == dev) {
			if (ip == rip) {
				if (ip->i_count > 1)
					return(-1);
				continue;
			}
			if (ip->i_count == 0) {
				/*
				 * Remove from hash list.
				 * i_number is left filled in by iput,
				 * so it can be used to locate the hash
				 * chain ip is on.
				 */

				if (ip->i_flag & ILOCK)
					return(-1);

				ip->i_back->i_forw = ip->i_forw;
				ip->i_forw->i_back = ip->i_back;
				ip->i_forw = ip->i_back = ip;
			} else
				return(-1);
		}
	}
	return(0);
}

/*
 * Remove an inode form its hash list
 */
iunhash(ip)
register struct inode *ip;
{
	ip->i_back->i_forw = ip->i_forw;
	ip->i_forw->i_back = ip->i_back;
	ip->i_forw = ip->i_back = ip;
}

/*
 * Check how many inodes are still active for the device.
 * Return 0 if only 1---assumed to be root inode.
 * Return -1 if more.
 * DO NOT REMOVE--THIS IS USED BY RFS.
 */
icheck(mp)
register struct mount *mp;
{
	register int i, count;
	register struct inode *ip;

	count = 0;

	for (i = 0, ip = inode;   i < v.v_inode;   i++, ip++)
	{
		if ((ip->i_mntdev == mp) && ip->i_count)
		{
			count += ip->i_count;
			if (count > 1)
				return(-1);
		}
	}

	return(0);
}

/*
 *	iinit is called once (from main) very early in initialization.
 *	It reads the root's super block and initializes the current date
 *	from the last modified date.
 *
 */
iinit()
{
	(*bdevsw[bmajor(rootdev)].d_open)(minor(rootdev), 
					FREAD|FWRITE,OTYP_LYR);
	(*bdevsw[bmajor(pipedev)].d_open)(minor(pipedev), 
					FREAD|FWRITE,OTYP_LYR);
	srmountfun(1);
}

#ifdef DEBUG

struct ilog {
	struct inode	*il_addr;
	struct inode	*il_forw;
	struct inode	*il_back;
	struct inode	*il_avforw;
	struct inode	*il_avback;
	int		*il_fsptr;
	long		il_number;
	struct rcvd	*il_rcvd;
	short		il_fstyp;
	dev_t		il_dev;
	cnt_t		il_count;
	short		il_op;
	int		*il_caller;
};

extern int ilogsize;
extern struct ilog ilogs[];
int ilogindex = 0;
int ilogwraps = 0;

ilog(ip, op, caller)
register struct inode *ip;
int op;
int *caller;
{
	register struct ilog *ilp;

	if (ilogsize <= 0)
		return;
	ilp = &ilogs[ilogindex];
	ilp->il_addr = ip;
	ilp->il_forw = ip->i_forw;
	ilp->il_back = ip->i_back;
	ilp->il_avforw = ip->av_forw;
	ilp->il_avback = ip->av_back;
	ilp->il_fsptr = ip->i_fsptr;
	ilp->il_number = ip->i_number;
	ilp->il_rcvd = ip->i_rcvd;
	ilp->il_fstyp = ip->i_fstyp;
	ilp->il_dev = ip->i_dev;
	ilp->il_count = ip->i_count;
	ilp->il_op = op;
	ilp->il_caller = caller;
	if (++ilogindex >= ilogsize) {
		ilogindex = 0;
		ilogwraps++;
	}
}

#endif

#ifdef DEBUG

/*
 * Routines to track inode logging in the u-block.
 */

#define ULSIZE 9

struct ulk {
	int	u_cnt;
	struct 	inode *u_ip[ULSIZE];
};
int	ilkmax = 0;
extern short ipid[];

/*
 * Add inode to list of locked inodes.  Return 0 if it wasn't already there,
 * 1 if it was or if space runs out.
 */

int
addilock(ip)
register struct inode *ip;
{
	register int i;
	register struct ulk *up = (struct ulk *)u.u_filler1;

	for (i = 0; i < up->u_cnt; i++) {
		if (up->u_ip[i] == ip)
			return(1);
	}
	if (up->u_cnt >= ULSIZE)
		return(1);
	up->u_ip[up->u_cnt++] = ip;
	if (up->u_cnt > ilkmax)
		ilkmax = up->u_cnt;
	if (ip >= &inode[0] && ip < &inode[v.v_inode])
		ipid[ip-inode] = u.u_procp->p_pid;
	return(0);
}

/*
 * Remove an inode from the list of locked inodes.  Return 0 if it was
 * there, 1 if it wasn't.
 */

int
rmilock(ip)
register struct inode *ip;
{
	register int i, j;
	register struct ulk *up = (struct ulk *)u.u_filler1;

	for (i = 0; i < up->u_cnt; i++) {
		if (up->u_ip[i] == ip) {
			up->u_cnt--;
			for (j = i; j < up->u_cnt; j++)
				up->u_ip[j] = up->u_ip[j+1];
			up->u_ip[j] = NULL;
			return(0);
		}
	}
	return(1);
}

/*
 * Return 0 if there are no locked inodes in the list, 1 if there are.
 */

int
noilocks()
{
	register struct ulk *up = (struct ulk *)u.u_filler1;

	return(up->u_cnt == 0 ? 0 : 1);
}

/*
 * Clear the list.
 */

clrilocks()
{
	register struct ulk *up = (struct ulk *)u.u_filler1;
	register int i;

	up->u_cnt = 0;
	for (i = 0; i < ULSIZE; i++)
		up->u_ip[i] = NULL;
}

#endif
