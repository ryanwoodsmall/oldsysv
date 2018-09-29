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
/* @(#)rdwri.c	6.4 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/inode.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include <sys/page.h>
#include "sys/buf.h"
#include "sys/conf.h"
#include "sys/file.h"
#include "sys/systm.h"
#include "sys/tty.h"

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
readi(ip)
register struct inode *ip;
{
	struct buf *bp;
	dev_t dev;
	daddr_t bn;
	register unsigned on, n;
	register type;
	register struct tty *tp;

	if (u.u_count == 0)
		return;
	if (u.u_offset < 0) {
		u.u_error = EINVAL;
		return;
	}
	type = ip->i_mode&IFMT;
	switch(type) {
	case IFCHR:
		dev = (dev_t)ip->i_rdev;
		ip->i_flag |= IACC;
		if (tp = cdevsw[major(dev)].d_ttys) {
			tp += minor(dev);
			(*linesw[tp->t_line].l_read)(tp);
		}
		else
			(*cdevsw[major(dev)].d_read)(minor(dev));
		break;

	case IFIFO:
		while (ip->i_size == 0) {
			if (ip->i_fwcnt == 0)
				return;
			if (u.u_fmode&FNDELAY)
				return;
			ip->i_fflag |= IFIR;
			prele(ip);
			sleep((caddr_t)&ip->i_frcnt, PPIPE);
			plock(ip);
		}
		u.u_offset = ip->i_frptr;
	case IFBLK:
	case IFREG:
	case IFDIR:
	do {
		bn = bmap(ip, B_READ);
		if (u.u_error)
			break;
		on = u.u_pboff;
		if ((n = u.u_pbsize) == 0)
			break;
		dev = u.u_pbdev;
		if ((long)bn<0) {
			if (type != IFREG)
				break;
			bp = geteblk();
			clrbuf(bp);
		} else if (u.u_rablock)
			bp = breada(dev, bn, u.u_rablock);
		else
			bp = bread(dev, bn);
		if (bp->b_resid) {
			n = 0;
		}
		if (n!=0)
			iomove(bp->b_un.b_addr+on, n, B_READ);
		if (type == IFIFO) {
			ip->i_size -= n;
			if (u.u_offset >= PIPSIZ)
				u.u_offset = 0;
			if ((on+n) == FsBSIZE(dev) && ip->i_size < (PIPSIZ-FsBSIZE(dev)))
				bp->b_flags &= ~B_DELWRI;
		}
		brelse(bp);
		ip->i_flag |= IACC;
	} while (u.u_error==0 && u.u_count!=0 && n!=0);
		if (type == IFIFO) {
			if (ip->i_size)
				ip->i_frptr = u.u_offset;
			else
				ip->i_frptr = ip->i_fwptr = 0;
			if (ip->i_fflag&IFIW) {
				ip->i_fflag &= ~IFIW;
				curpri = PPIPE;
				wakeup((caddr_t)&ip->i_fwcnt);
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
writei(ip)
register struct inode *ip;
{
	struct buf *bp;
	dev_t dev;
	daddr_t bn;
	register unsigned n, on;
	register type;
	unsigned int usave;
	register struct tty *tp;

	if (u.u_offset < 0) {
		u.u_error = EINVAL;
		return;
	}
	type = ip->i_mode&IFMT;
	switch (type) {
	case IFCHR:
		dev = (dev_t)ip->i_rdev;
		ip->i_flag |= IUPD|ICHG;
		if (tp = cdevsw[major(dev)].d_ttys) {
			tp += minor(dev);
			(*linesw[tp->t_line].l_write)(tp);
		}
		else
			(*cdevsw[major(dev)].d_write)(minor(dev));
		break;

	case IFIFO:
	floop:
		usave = 0;
		while ((u.u_count+ip->i_size) > PIPSIZ) {
			if (ip->i_frcnt == 0)
				break;
			if ((u.u_count > PIPSIZ) && (ip->i_size < PIPSIZ)) {
				usave = u.u_count;
				u.u_count = PIPSIZ - ip->i_size;
				usave -= u.u_count;
				break;
			}
			if (u.u_fmode&FNDELAY)
				return;
			ip->i_fflag |= IFIW;
			prele(ip);
			sleep((caddr_t)&ip->i_fwcnt, PPIPE);
			plock(ip);
		}
		if (ip->i_frcnt == 0) {
			u.u_error = EPIPE;
			psignal(u.u_procp, SIGPIPE);
			break;
		}
		u.u_offset = ip->i_fwptr;
	case IFBLK:
	case IFREG:
	case IFDIR:
	while (u.u_error==0 && u.u_count!=0) {
		bn = bmap(ip, B_WRITE);
		if (u.u_error)
			break;
		on = u.u_pboff;
		n = u.u_pbsize;
		dev = u.u_pbdev;
		if (n == FsBSIZE(dev)) 
			bp = getblk(dev, bn);
		else if (type==IFIFO && on==0 && ip->i_size < (PIPSIZ-FsBSIZE(dev)))
			bp = getblk(dev, bn);
		else
			bp = bread(dev, bn);

		iomove(bp->b_un.b_addr+on, n, B_WRITE);
		if (u.u_error)
			brelse(bp);
		else if (u.u_fmode&FSYNC)
			bwrite(bp);
		else if (type == IFBLK) {
			/* IFBLK not delayed for tapes */
			bp->b_flags |= B_AGE;
			bawrite(bp);
		} else
			bdwrite(bp);
		if (type == IFREG || type == IFDIR) {
			if (u.u_offset > ip->i_size) {
				if (ip->i_map)
					freeblklst(ip);
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
			ip->i_fwptr = u.u_offset;
			if (ip->i_fflag&IFIR) {
				ip->i_fflag &= ~IFIR;
				curpri = PPIPE;
				wakeup((caddr_t)&ip->i_frcnt);
			}
			if (u.u_error==0 && usave!=0) {
				u.u_count = usave;
				goto floop;
			}
		}
		break;

	default:
		u.u_error = ENODEV;
	}
}
