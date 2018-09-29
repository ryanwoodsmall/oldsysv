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
 *    dcudisk.h:  definitions for dcutest
 */


#define DMAGIC	0x6d737000		/* The disk magic number */
#ifndef BSIZE
#define BSIZE	1024			/* the default block size */
#endif

/*
 *	The basic disk configuration data
 */

struct dcon {
	short	dc_nsectors;	/* # of sectors per track */
	short	dc_ntracks;	/* # of tracks per cylinder */
	short	dc_ncylinders;	/* # of cylinders per unit */
	short	dc_secpercyl;	/* # of sectors per cylinder */
	long	dc_secperunit;	/* # of sectors per unit */
};

/*
 *	The configuration data used by the disk drivers. This is a
 *	subset of the full configuration data stored on the disk, as
 *	the kernel driver doesn't need everything.
 */

typedef struct {
	struct dcon dc;		/* the configuration data */
	struct partition {	/* the partition table */
		int	nblocks;/* number of sectors in partition */
		int	cyloff;	/* starting cylinder offset for partition */
	}dd_partition[8];
}dcdata;

#define d_nsectors		dc.dc_nsectors
#define d_ntracks		dc.dc_ntracks
#define d_ncylinders		dc.dc_ncylinders
#define d_secpercyl		dc.dc_secpercyl
#define d_secperunit		dc.dc_secperunit

/*
 *	The full configuration data stored on block 0 of the disk
 */

struct disktab {
	long	dt_magic;	/* the magic number */
	short	dt_ploz;	/* formatting data for sector zero */
	short	dt_plonz;	/* formatting data for non-zero sectors */
	dcdata	dt_dd;		/* device driver data */
	char	dt_name[8];	/* a buffer for the name of the disk */
};

#define dt_dc		dt_dd.dc
#define dt_nsectors	dt_dc.dc_nsectors
#define dt_ntracks	dt_dc.dc_ntracks
#define dt_ncylinders	dt_dc.dc_ncylinders
#define dt_secpercyl	dt_dc.dc_secpercyl
#define dt_secperunit	dt_dc.dc_secperunit
#define dt_partition	dt_dd.dd_partition

/*
 *	Default configurations used by stand alone driver for
 *	auto-configure feature. An array of these exists in
 *	dcusaio.c, one entry per known drive type.
 */

struct unitdata {
	short	ud_ploz;	/* formatting data for sector zero */
	short	ud_plonz;	/* formatting data for non-zero sectors */
	struct	dcon dc;	/* the configuration data */
	struct	partition *ud_pt;/* pointer to disk partition table*/
	char   *ud_name;	/* name of disk type */
};

/*
 *	A convenience for assigning partition tables
 */

typedef struct {
	struct partition pt[8];
} partitions;
