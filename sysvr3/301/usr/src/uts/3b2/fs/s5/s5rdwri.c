/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)kern-port:fs/s5/s5rdwri.c	10.10"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/fstyp.h"
#include "sys/fs/s5macros.h"
#include "sys/fs/s5inode.h"
#include "sys/inode.h"
#include "sys/mount.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/sbd.h"
#include "sys/immu.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/buf.h"
#include "sys/conf.h"
#include "sys/file.h"
#include "sys/fcntl.h"
#include "sys/flock.h"
#include "sys/systm.h"
#include "sys/debug.h"

/*
 * Read the file corresponding to
 * the inode pointed at by the argument.
 * The actual read arguments are found
 * in the variables:
 *	u_base		core address for destination
 *	u_offset	byte offset in file
 *	u_count		number of bytes to read
 *	u_segflg	read to kernel/user/user I
 */
s5readi(ip)
register struct inode *ip;
{
	register struct buf *bp;
	register struct s5inode *s5ip;
	register long bsize;
	register unsigned on, n;
	register daddr_t bn;
	register type;
	register dev_t dev;

	if (ip->i_ftype == IFREG && !s5access(ip, IMNDLCK)) {
		s5chklock(ip, FREAD);
		if (u.u_error)
			return;
	}
	if (u.u_count == 0)
		return;
	if (u.u_offset < 0) {
		u.u_error = EINVAL;
		return;
	}
	s5ip = (struct s5inode *)ip->i_fsptr;
	ASSERT(s5ip != NULL);
	type = s5ip->s5i_mode & IFMT;
	switch (type) {
	case IFCHR:
		dev = (dev_t)ip->i_rdev;
		ip->i_flag |= IACC;
		if (cdevsw[major(dev)].d_str)
			strread(ip);
		else
			(*cdevsw[major(dev)].d_read)(dev);
		break;

	case IFIFO:
		/* While the pipe (FIFO) is empty ... */
		while (ip->i_size == 0) {
			if (s5ip->s5i_fwcnt == 0)
				return;
			if (u.u_fmode&FNDELAY)
				return;
			s5ip->s5i_fflag |= IFIR;
			prele(ip);
			sleep((caddr_t)&s5ip->s5i_frcnt, PPIPE);
			plock(ip);
		}
		u.u_offset = s5ip->s5i_frptr;
	case IFBLK:
	case IFREG:
	case IFDIR:
		do {
			bn = s5bmap(ip, B_READ);
			if (u.u_error)
				break;
			on = u.u_pboff;
			if ((n = u.u_pbsize) == 0)
				break;
			dev = u.u_pbdev;
			bsize = u.u_bsize;
			if ((long)bn < 0) {
				if (type != IFREG)
					break;
				bp = geteblk();
				clrbuf(bp);
			} else if (u.u_rablock)
				bp = breada(dev, bn, u.u_rablock, bsize);
			else
				bp = bread(dev, bn, bsize);
			if (bp->b_resid)
				n = 0;
			if (n != 0) {
				if (u.u_segflg != 1) {
					if (copyout(bp->b_un.b_addr+on,
					  u.u_base, n)) {
						u.u_error = EFAULT;
						goto bad;
					}
				} else
					bcopy(bp->b_un.b_addr+on, u.u_base, n);
				u.u_offset += n;
				u.u_count -= n;
				u.u_base += n;
			bad:
				;
			}
			if (type == IFIFO) {
				ip->i_size -= n;
				if (u.u_offset >= PIPSIZ)
					u.u_offset = 0;
				if ((on + n) == bsize
				  && ip->i_size < (PIPSIZ - bsize))
					bp->b_flags &= ~B_DELWRI;
			}
			brelse(bp);
			ip->i_flag |= IACC;
		} while (u.u_error == 0 && u.u_count != 0 && n != 0);
		if (type == IFIFO) {
			if (ip->i_size)
				s5ip->s5i_frptr = u.u_offset;
			else
				s5ip->s5i_frptr = s5ip->s5i_fwptr = 0;
			if (s5ip->s5i_fflag&IFIW) {
				s5ip->s5i_fflag &= ~IFIW;
				curpri = PPIPE;
				wakeup((caddr_t)&s5ip->s5i_fwcnt);
			}
		}
		break;

	default:
		u.u_error = ENODEV;
	}
}

/*
 * Write the file corresponding to
 * the inode pointed at by the argument.
 * The actual write arguments are found
 * in the variables:
 *	u_base		core address for source
 *	u_offset	byte offset in file
 *	u_count		number of bytes to write
 *	u_segflg	write to kernel/user/user I
 */
s5writei(ip)
register struct inode *ip;
{
	register struct buf *bp;
	register struct s5inode *s5ip;
	register long bsize;
	register unsigned n, on;
	register dev_t dev;
	register daddr_t bn;
	register type;
	unsigned int usave;

	if (ip->i_ftype == IFREG && !s5access(ip, IMNDLCK)) {
		s5chklock(ip, FWRITE);
		if (u.u_error)
			return;
	}
	if (u.u_error)
		return;
	if (u.u_offset < 0) {
		u.u_error = EINVAL;
		return;
	}

	s5ip = (struct s5inode *)ip->i_fsptr;
	ASSERT(s5ip != NULL);
	type = s5ip->s5i_mode & IFMT;

	switch (type) {
	case IFCHR:
		dev = (dev_t)ip->i_rdev;
		ip->i_flag |= IUPD|ICHG;
		if (cdevsw[major(dev)].d_str)
			strwrite(ip);
		else
			(*cdevsw[major(dev)].d_write)(dev);
		break;

	case IFIFO:
	floop:
		usave = 0;
		while ((u.u_count+ip->i_size) > PIPSIZ) {
			if (s5ip->s5i_frcnt == 0)
				break;
			if ((u.u_count > PIPSIZ) && (ip->i_size < PIPSIZ)) {
				usave = u.u_count;
				u.u_count = PIPSIZ - ip->i_size;
				usave -= u.u_count;
				break;
			}
			if (u.u_fmode & FNDELAY)
				return;
			s5ip->s5i_fflag |= IFIW;
			prele(ip);
			sleep((caddr_t)&s5ip->s5i_fwcnt, PPIPE);
			plock(ip);
		}
		if (s5ip->s5i_frcnt == 0) {
			u.u_error = EPIPE;
			psignal(u.u_procp, SIGPIPE);
			break;
		}
		u.u_offset = s5ip->s5i_fwptr;
	case IFBLK:
	case IFREG:
	case IFDIR:
		n = 0;
		while (u.u_error == 0 && u.u_count != 0) {
			bn = s5bmap(ip, B_WRITE);
			if (u.u_error) {
				if (n)
					u.u_error = 0;
				break;
			}
			on = u.u_pboff;
			n = u.u_pbsize;
			dev = u.u_pbdev;
			bsize = u.u_bsize;
			if (n == bsize) 
				bp = getblk(dev, bn, bsize);
			else if (type == IFIFO && on == 0
			  && ip->i_size < (PIPSIZ - bsize))
				bp = getblk(dev, bn, bsize);
			else
				bp = bread(dev, bn, bsize);
			if (u.u_segflg != 1) {
				if (copyin(u.u_base, bp->b_un.b_addr+on, n)) {
					u.u_error = EFAULT;
					goto bad;
				}
			} else
				bcopy(u.u_base, bp->b_un.b_addr+on, n);
			u.u_offset += n;
			u.u_count -= n;
			u.u_base += n;
		bad:
			if (u.u_error)
				brelse(bp);
			else if (u.u_fmode & FSYNC)
				bwrite(bp);
			else if (type == IFBLK) {
				bp->b_flags |= B_AGE;
				bawrite(bp);
			} else
				bdwrite(bp);
			if (type == IFREG || type == IFDIR) {
				if (u.u_offset > ip->i_size) {
					if (s5ip->s5i_map)
						s5freemap(ip);
					ip->i_size = u.u_offset;
				}
			} else if (type == IFIFO) {
				ip->i_size += n;
				if (u.u_offset == PIPSIZ)
					u.u_offset = 0;
			}
			ip->i_flag |= IUPD|ICHG;
		}
		if (type == IFIFO) {
			s5ip->s5i_fwptr = u.u_offset;
			if (s5ip->s5i_fflag & IFIR) {
				s5ip->s5i_fflag &= ~IFIR;
				curpri = PPIPE;
				wakeup((caddr_t)&s5ip->s5i_frcnt);
			}
			if (u.u_error == 0 && usave != 0) {
				u.u_count = usave;
				goto floop;
			}
		}
		break;

	default:
		u.u_error = ENODEV;
	}
}

/*
 * Enforce record locking protocol on regular file ip.
 */
s5chklock(ip, mode)
register struct inode *ip;
int mode;
{
	register int i;
	struct flock bf;

	bf.l_type = (mode & FWRITE) ? F_WRLCK : F_RDLCK;
	bf.l_whence = 0;
	bf.l_start = u.u_offset;
	bf.l_len = u.u_count;
	i = (u.u_fmode & FNDELAY) ? INOFLCK : SLPFLCK|INOFLCK;
	if ((i = reclock(ip, &bf, i, 0, u.u_offset)) || bf.l_type != F_UNLCK)
		u.u_error = i ? i : EAGAIN;
}
