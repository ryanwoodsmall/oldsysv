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
/* @(#)blklist.c	1.4 */
#include	"sys/types.h"
#include	"sys/param.h"
#include	"sys/dir.h"
#include	"sys/signal.h"
#include	"sys/inode.h"
#include	"sys/user.h"
#include	"sys/buf.h"
#include	"sys/page.h"
#include	"sys/region.h"
#include	"sys/sysmacros.h"
#include	"sys/debug.h"


/*	Build the list of block numbers for a file.  This is used
 *	for mapped files.
 */

bldblklst(ip)
register struct inode	*ip;
{
	int		i;
	int		lim;
	register daddr_t	*lp;
	register daddr_t	*eptr;
	register daddr_t	nblks;
	extern daddr_t	*bldindr();


	ASSERT(ip->i_map == 0);

	/*	Get number of blocks to be mapped
	 *	and allocate a block list
	 */

	nblks = (ip->i_size + FsBSIZE(ip->i_dev) - 1)/FsBSIZE(ip->i_dev);
	ip->i_map = (daddr_t *)sptalloc(btop(nblks*sizeof(daddr_t)), PG_V | PG_KW, 0);

	lp = ip->i_map;
	eptr = &lp[nblks];

	/*	Get the block numbers from the direct blocks first.
	 */

	if(nblks < 10)
		lim = nblks;
	else
		lim = 10;
	
	for(i = 0  ;  i < lim  ;  i++)
		*lp++ = ip->i_addr[i];
	
	if(lp >= eptr)
		return(0);
	
	/*	Now do the indirect blocks
	 */

	while(lp < eptr){
		lp = bldindr(lp, eptr, ip->i_dev, ip->i_addr[i], i - 10);
		if(lp == (daddr_t *)-1)
			return(-1);
		i++;
	}
	return(0);
}

daddr_t  *
bldindr(lp, eptr, edev, blknbr, indlvl)
register daddr_t	*lp;
register daddr_t	*eptr;
register int	edev;
daddr_t		blknbr;
int		indlvl;
{
	register struct buf	*bp;
	register daddr_t	*bnptr;
	int			cnt;
	struct buf		*bread();

	bp = bread(edev, blknbr);
	if(u.u_error){
		brelse(bp);
		return((daddr_t *) -1);
	}
	bnptr = bp->b_un.b_daddr;
	cnt = FsNINDIR(edev);
	
	ASSERT(indlvl >= 0);
	while(cnt--  &&  lp < eptr){
		if(indlvl == 0){
			*lp++ = *bnptr++;
		} else {
			lp = bldindr(lp, eptr, edev, *bnptr++, indlvl-1);
			if(lp == (daddr_t *)-1){
				brelse(bp);
				return((daddr_t *) -1);
			}
		}
	}

	brelse(bp);
	return(lp);
}

/*	Free the block list attached to an inode.
 */

freeblklst(ip)
register struct inode	*ip;
{
	register daddr_t	*lp;
	register daddr_t	*eptr;
	dbd_t		dbd;
	reg_t		reg;
	daddr_t		nblks;

	ASSERT(ip->i_map);

	reg.r_iptr = ip;
	dbd.dbd_type = DBD_FILE;


	/*	Walk through block list removing cached pages
	 *	associated with the inode
	 */

	lp = ip->i_map;
	nblks = (ip->i_size + FsBSIZE(ip->i_dev) - 1)/FsBSIZE(ip->i_dev);
	eptr = &lp[nblks];

	while(lp < eptr) {
		dbd.dbd_blkno = *lp++;
		dbd.dbd_adjunct = DBD_FP;
		pbremove(&reg, &dbd);
		dbd.dbd_adjunct = 0;
		pbremove(&reg, &dbd);
	}

	/*	Free up block list
	 */
	sptfree(ip->i_map, btop(nblks * sizeof(daddr_t)), 1);
	ip->i_map = NULL;
}
