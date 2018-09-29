/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:fs/s5/s5blklist.c	10.6"
#include	"sys/types.h"
#include	"sys/sema.h"
#include	"sys/param.h"
#include	"sys/fs/s5macros.h"
#include	"sys/sysmacros.h"
#include	"sys/immu.h"
#include	"sys/fs/s5dir.h"
#include	"sys/signal.h"
#include	"sys/fs/s5inode.h"
#include	"sys/fstyp.h"
#include	"sys/inode.h"
#include	"sys/mount.h"
#include	"sys/psw.h"
#include	"sys/pcb.h"
#include	"sys/user.h"
#include	"sys/buf.h"
#include	"sys/region.h"
#include 	"sys/proc.h"
#include	"sys/pfdat.h"
#include	"sys/errno.h"
#include	"sys/cmn_err.h"
#include	"sys/debug.h"
#include	"sys/conf.h"

/*
 *  Allocate and build the block address map
 */

s5allocmap(ip)
register struct inode *ip;
{
	register int	i;
	register int	*bnptr;
	register struct s5inode *s5ip;
	register int	blkspp;
	register int	bsize;
	register int	nblks;

	s5ip = (struct s5inode *)ip->i_fsptr;
ASSERT(s5ip != NULL);
	if (s5ip->s5i_map) 
		return(1);

	/*	Get number of blocks to be mapped.
	 */

	ASSERT(s5ip->s5i_map == 0);
	bsize = FsBSIZE((ip->i_mntdev)->m_bsize);
	nblks = (ip->i_size + bsize - 1)/bsize;
	blkspp = NBPP/bsize;


	/*	Round up the file size in block to an
	 *	integral number of pages.  Allocate
	 *	space for the block number list.
	 */

	i = ((nblks + blkspp - 1) / blkspp) * blkspp;
	bnptr = s5ip->s5i_map = (int *)uptalloc(ctos(i));
	if (bnptr == NULL)
		return(0);

	/*	Build the actual list of block numbers
	 *	for the file.
	 */

	s5bldblklst(bnptr, ip, nblks);

	/*	If the size is not an integral number of
	 *	pages long, then the last few block
	 *	number up to the next page boundary are
	 *	made zero so that no one will try to
	 *	read them in.  See code in fault.c/vfault.
	 */

	while (i%blkspp != 0) {
		*bnptr++ = -1;
		i++;
	}
	return(1);
}

/*	Read page from a file
 *
 *	return # of bytes read if no error occurs
 *	return -1 - when read error occurs
 */
s5readmap(ip, offset, size, vaddr, segflg)
register struct inode *ip;
off_t offset;
int size;
caddr_t vaddr;
int segflg;
{
	register struct buf	*bp;
	register int	*bnptr;
	register struct s5inode *s5ip;
	register dev_t	edev;
	register int bsize;
	register int	i;
	int ret, on, n;
	struct buf *bread(), *breada();

	/*	Get the number of blocks to read and
	 *	a pointer to the block number list.
	 */

	s5ip = (struct s5inode *)ip->i_fsptr;
ASSERT(s5ip != NULL);
	ASSERT(s5ip->s5i_map != 0);
	if (offset > ip->i_size) {
		u.u_error = EINVAL;
		printf("s5readpg: offset > ip->i_size\n");
		printf("s5readpg: offset=%x ip->i_size == %x\n", offset, ip->i_size);
		return(-1);
	}
	if (offset+size > ip->i_size) {
		size = ip->i_size - offset;
	}
	ret = size;
	edev = ip->i_dev;
	bsize = FsBSIZE((ip->i_mntdev)->m_bsize);
	i = offset/bsize;
	bnptr = &s5ip->s5i_map[i];
	on = offset - (i*bsize);

	while (size > 0) {
		if (*bnptr == -1) 
			break;

		if (*bnptr) {
			if ((size > bsize)  &&  *(bnptr + 1))
				bp = breada(edev, *bnptr, *(bnptr+1), bsize);
			else
				bp = bread(edev, *bnptr, bsize);
		} else {
			bp = geteblk();
			clrbuf(bp);
		}
		bnptr++;

		bp->b_flags |= B_AGE;
		if (bp->b_flags&B_ERROR) {
			prdev("page read error", edev);
			brelse(bp);
			return(-1);
		}

		n = bsize - on;
		if (n > size) n = size;
		if (segflg != 1) {
			if (copyout(bp->b_un.b_addr + on, vaddr, n)) {
				u.u_error = EFAULT;
				brelse(bp);
				printf("s5readpg:  failure in copyout\n");
				return(-1);
			}
		} else
			bcopy(bp->b_un.b_addr+on, vaddr, n);
		brelse(bp);
		size -= n;
		offset += n;
		vaddr += n;
		on = 0;
	}

	return(ret);
}

/*	Build the list of block numbers for a file.  This is used
 *	for mapped files.
 */

s5bldblklst(lp, ip, nblks)
register int		*lp;
inode_t			*ip;
register int		nblks;
{
	register int	lim;
	register int	*eptr;
	register int	i;
	register struct s5inode *s5ip;
	int		*s5bldindr();

	/*	Get the block numbers from the direct blocks first.
	 */

	eptr = &lp[nblks];
	if (nblks < 10)
		lim = nblks;
	else
		lim = 10;
	
	s5ip = (struct s5inode *)ip->i_fsptr;
ASSERT(s5ip != NULL);
	for (i = 0  ;  i < lim  ;  i++)
		*lp++ = s5ip->s5i_addr[i];
	
	if (lp >= eptr)
		return(1);
	
	while (lp < eptr) {
		lp = s5bldindr(ip, lp, eptr, effdev(ip), s5ip->s5i_addr[i], i - 10);
		if (lp == 0)
			return(0);
		i++;
	}
	return(1);
}

int  *
s5bldindr(ip, lp, eptr, edev, blknbr, indlvl)
struct inode *ip;
register int		*lp;
register int		*eptr;
register int		edev;
int			blknbr;
int			indlvl;
{
	register struct buf *bp;
	register int	*bnptr;
	int		cnt;
	struct buf 	*bread();

	bp = bread(edev, blknbr, FsBSIZE((ip->i_mntdev)->m_bsize));
	if (u.u_error) {
		brelse(bp);
		return((int *) 0);
	}
	bnptr = bp->b_un.b_words;
	cnt = FsNINDIR((ip->i_mntdev)->m_bsize);
	
	ASSERT(indlvl >= 0);
	while (cnt--  &&  lp < eptr) {
		if (indlvl == 0) {
			*lp++ = *bnptr++;
		} else {
			lp = s5bldindr(ip, lp, eptr, edev, *bnptr++, indlvl-1);
			if (lp == 0) {
				brelse(bp);
				return((int *) 0);
			}
		}
	}

	brelse(bp);
	return(lp);
}

/*	Free the block list attached to an inode.
 */

s5freemap(ip)
register inode_t	*ip;
{
	register int	nblks;
	register int	blkspp;
	register struct s5inode *s5ip;
	register	bsize;

	ASSERT((ip->i_flag & ITEXT) == 0);
	ASSERT(ip->i_flag & ILOCK);
	ASSERT(ip->i_count <= 1);
	
	s5ip = (struct s5inode *)ip->i_fsptr;
ASSERT(s5ip != NULL);

	if (ip->i_ftype != IFREG || s5ip->s5i_map == NULL)
		return;

	/* Call paging routine to flush page cache */
	flushpgch(ip);

	bsize = FsBSIZE((ip->i_mntdev)->m_bsize);
	nblks = (ip->i_size + bsize - 1)/bsize;
	blkspp = NBPP/bsize;

	/* Round up the file size (in blocks) to an integral */
	/* number of pages */
	nblks = ((nblks + blkspp - 1) / blkspp) * blkspp;
	uptfree(s5ip->s5i_map, ctos(nblks));
	s5ip->s5i_map = NULL;
}
