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
/*
 * disk configuration
 */

#define DMAGIC	0x6d737000		/* The disk magic number */

struct disktab {
	long	dt_magic;	/* the magic number */
	short	dt_ploz;	/* formatting data for sector zero */
	short	dt_plonz;	/* formatting data for non-zero sectors */
	short	dt_nsec;	/* # of sectors per track */
	short	dt_ntr;	/* # of tracks per cylinder */
	short	dt_ncyl;	/* # of cylinders per unit */
	short	dt_spcyl;	/* # of sectors per cylinder */
	long	dt_spunit;	/* # of sectors per unit */
	struct partition {	/* the partition table */
		int	nbl;/* number of sectors in partition */
		int	cyloff;	/* starting cylinder offset for partition */
	}dt_part[8];
	char	dt_name[8];	/* a buffer for the name of the disk */
};
