/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:boot/olboot/findfs.c	10.2"

#include	"sys/param.h"
#include	"sys/types.h"
#include	"sys/inode.h"
#include	"sys/filsys.h"
#include	"sys/iobuf.h"
#include	"sys/elog.h"
#include	"sys/firmware.h"
#include	"sys/boot.h"
#include	"sys/lboot.h"

extern ndisk_acs();

/*
 *	findfs - Find root file system.
 *
 *	Expand the inode for the root directory into Dinode.
 */

unsigned short
findfs()
{

	/* Get inode for root directory */

	Dinode.i_number = ROOTINO;
	liread(&Dinode);
	return(1);
}

/*
 *	readsb() - read superblock
 *
 */
readsb()
{
	struct filsys *fsptr;

	fsptr = (struct filsys *)IOBASE;

	ndisk_acs(Fso+1, IOBASE);

	if (fsptr->s_magic == FsMAGIC) {
		switch (fsptr->s_type) {
		case Fs1b:
			fstype = 0;
			break;
		case Fs2b:
			fstype = Fs2BLK;
			break;
		default:
			break;
		}
	}
	else
		PRINTF("Not FsMAGIC:  fsptr->s_magic is %d   FsMAGIC is %d\n", fsptr->s_magic, FsMAGIC);
}
