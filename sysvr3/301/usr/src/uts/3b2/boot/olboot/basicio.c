/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:boot/olboot/basicio.c	10.2"

#include	"sys/param.h"
#include	"sys/types.h"
#include	"sys/inode.h"
#include	"sys/ino.h"
#include	"sys/iobuf.h"
#include	"sys/elog.h"
#include	"sys/sysmacros.h"
#include	"sys/firmware.h"
#include	"sys/boot.h"
#include	"sys/lboot.h"


/*
**	This file contains basic I/O routines to unpack inodes and to read
**	a block in a file given the inode.
*/


/*
**	lbmap - Little bmap.
**
**	This routine maps a block number within a file to a block number within
**	a file system.  Zero is returned if the block is not allocated.
**
**	This code is a stripped version of bmap(os/subr.c).
*/

lbmap(ip, bn)
struct inode	*ip;	/* inode of file being referenced */
register int	bn;	/* block # */
{
	register int	nb,	/* next block wanted */
			j;	/* indirect block indicator */
	static int	Ind1b,	/* current block # in IND1 */
			Ind2b,	/* current block # in IND2 */
			Ind3b;	/* current block # in IND3 */
	int		sh,	/* shift count */
			*bap;	/* ptr to indirect block */

	/* 0..NADDR-4 are direct blocks. */
	if (bn < NADDR - 3)
		return(ip->i_addr[bn]);

	/* Addresses NADDR-3, NADDR-2, and NADDR-1 have single, double, and
		triple indirect blocks.  Determine how many levels. */
	sh = 0;
	nb = 1;
	bn -= NADDR - 3;
	for (j = 3; j > 0; j--) {
		sh += FsNSHIFT(fstype);
		nb <<= FsNSHIFT(fstype);
		if (bn < nb)
			break;
		bn -= nb;
	}
	if (j == 0) {
		/* File too big. */
                PRINTF("FILE TOO BIG!\n");
                return;
        }

	/* Fetch through the indirect blocks. */
	for (nb = ip->i_addr[NADDR - j]; j <= 3; j++) {
		if (nb == 0)
			break;
		switch (j) {
		case 3:

			/* Single indirect. */
			if (Ind1b != nb) {
				Ind1b = nb;
				if (FsBSIZE(fstype) == 512)
					ndisk_acs(Fso + nb, IND1);
				else {
					ndisk_acs(Fso + 2*nb, IND1);
					ndisk_acs(Fso + 2*nb + 1, IND1 + 512);
				}
			}
			bap = (int *)(IND1);
			break;
		case 2:

			/* Double indirect. */
			if (Ind2b != nb) {
				Ind2b = nb;
				if (FsBSIZE(fstype) == 512)
					ndisk_acs(Fso + nb, IND2);
				else {
					ndisk_acs(Fso + 2*nb, IND2);
					ndisk_acs(Fso + 2*nb + 1, IND2 + 512);
				}
			}
			bap = (int *)(IND2);
			break;
		case 1:

			/* Triple indirect. */
			if (Ind3b != nb) {
				Ind3b = nb;
				if (FsBSIZE(fstype) == 512)
					ndisk_acs(Fso + nb, IND3);
				else {
					ndisk_acs(Fso + 2*nb, IND3);
					ndisk_acs(Fso + 2*nb + 1, IND3 + 512);
				}
			}
			bap = (int *)(IND3);
			break;
		}
		sh -= FsNSHIFT(fstype);
		nb = bap[(bn >> sh) & FsNMASK(fstype)];
	}
	return(nb);
}

/*
**	liread - Little iread.
**
**	This routine reads in and unpacks the inode pointed to by ip.  The
**	root file system is assumed.  The inode # is taken from the inode.
**	Other fields are filled in from disk.
**
**	This is a stripped version of iread(os/iget.c).
*/

liread(ip)
register struct inode	*ip;	/* ptr to inode to fill in */
{
	register struct dinode	*dp;	/* ptr to disk version of inode */
	static int		Inodeb;	/* current block # in INODE */
	register int		i;	/* loop control */
	register char		*p1,	/* pointers for unpacking address */
				*p2;	/*	fields of inode		  */
	int			b;	/* inode block # */

	/* Get inode block. */
	b = FsITOD(fstype, ip->i_number);
	if (b != Inodeb) {
		Inodeb = b;
		if (FsBSIZE(fstype) == 512)
			ndisk_acs(Fso + b, INODE);
		else {
			ndisk_acs(Fso + 2*b, INODE);
			ndisk_acs(Fso + 2*b + 1, INODE + 512);
		}
	}

	/* Set up ptr to the disk copy of the inode. */
	dp = (struct dinode *)(INODE);
	dp += FsITOO(fstype, ip->i_number);

	/* Unpack the inode. */
	ip->i_mode = dp->di_mode;
	ip->i_nlink = dp->di_nlink;
	ip->i_size = dp->di_size;
	p1 = (char *)ip->i_addr;
	p2 = (char *)dp->di_addr;
	for (i = 0; i < NADDR; i++) {
		*p1++ = '\0';
		*p1++ = *p2++;
		*p1++ = *p2++;
		*p1++ = *p2++;
	}
}

/*
**	rb - Read a block.
**
**	This routine reads the block from the inode pointed to by ip that
**	contains file byte offset faddr into the I/O buffer given by maddr.
*/

unsigned short
rb(maddr, faddr, ip)
paddr_t		maddr;	/* I/O buffer address */
off_t		faddr;	/* file byte offset */
struct inode	*ip;	/* ptr to inode */
{
	register int	bn,	/* file system block # */
			*addr,	/* physical buffer address */
			i;	/* loop control */

	if (bn = lbmap(ip, faddr >> FsBSHIFT(fstype))) {
		if (FsBSIZE(fstype) == 512)
			ndisk_acs(Fso + bn, maddr);
		else {
			ndisk_acs(Fso + 2*bn, maddr);
			ndisk_acs(Fso + 2*bn + 1, maddr + 512);
		}
	}
	else {
		PRINTF("BLOCK NOT MAPPED  offset = %x, buffer addr = %x\n", faddr, maddr);
		return(CFAIL);
	}
	return(CPASS);
}
ndisk_acs(nblock,addr)
	int nblock;
	unsigned long addr;
 	{
	switch (P_CMDQ->b_dev)
	{

	case HARDDISK0:
	case HARDDISK1:

	HD_ACS(P_CMDQ->b_dev - HARDDISK0, nblock, addr, DISKRD);

	case FLOPDISK:
	
	FD_ACS(nblock, addr, DISKRD, NOCHANGE);

	default:

	IOBLK_ACS(P_CMDQ->b_dev, nblock, addr, DISKRD);

	}
}
