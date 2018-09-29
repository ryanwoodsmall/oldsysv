/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/vtoc.h	10.1"
/*
 * VTOC.H
 */

#define V_NUMPAR 		16		/* The number of partitions */

#define VTOC_SANE		0x600DDEEE	/* Indicates a sane VTOC */
#define V_VERSION		0x01		/* layout version number */

/* Partition identification tags */
#define V_BOOT		0x01		/* Boot partition */
#define V_ROOT		0x02		/* Root filesystem */
#define V_SWAP		0x03		/* Swap filesystem */
#define V_USR		0x04		/* Usr filesystem */
#define V_BACKUP	0x05		/* full disk */

/* Partition permission flags */
#define V_UNMNT		0x01		/* Unmountable partition */
#define V_RONLY		0x10		/* Read only */

/* driver ioctl() commands */
#define VIOC			('V'<<8)
#define V_PREAD			(VIOC|1)	/* Physical Read */
#define V_PWRITE		(VIOC|2)		/* Physical Write */
#define V_PDREAD		(VIOC|3)		/* Read of Physical Description Area */
#define V_PDWRITE		(VIOC|4)		/* Write of Physical Description Area */
#define V_GETSSZ		(VIOC|5)		/* Get the sector size of media */

/* ioctl() error return codes */
#define V_BADREAD		0x01
#define V_BADWRITE		0x02

/* Sanity word for the physical description area */
#define VALID_PD		0xCA5E600D

struct partition	{
	ushort p_tag;			/*ID tag of partition*/
	ushort p_flag;			/*permision flags*/
	daddr_t p_start;		/*start sector no of partition*/
	long p_size;			/*# of blocks in partition*/
};

struct vtoc {
	unsigned long v_bootinfo[3];		/*info needed by mboot*/
	unsigned long v_sanity;			/*to verify vtoc sanity*/
	unsigned long v_version;		/*layout version*/
	char v_volume[8];			/*volume name*/
	ushort v_sectorsz;			/*sector size in bytes*/
	ushort v_nparts;			/*number of partitions*/
	unsigned long v_reserved[10];		/*free space*/
	struct partition v_part[V_NUMPAR];	/*partition headers*/
};

struct pdinfo	{
	unsigned long driveid;		/*identifies the device type*/
	unsigned long sanity;		/*verifies device sanity*/
	unsigned long version;		/*version number*/
	char serial[12];		/*serial number of the device*/
	unsigned long cyls;		/*number of cylinders per drive*/
	unsigned long tracks;		/*number tracks per cylinder*/
	unsigned long sectors;		/*number sectors per track*/
	unsigned long bytes;		/*number of bytes per sector*/
	unsigned long logicalst;	/*sector address of logical sector 0*/
	unsigned long errlogst;		/*sector address of error log area*/
	unsigned long errlogsz;		/*size in bytes of error log area*/
	unsigned long mfgst;		/*sector address of mfg. defect info*/
	unsigned long mfgsz;		/*size in bytes of mfg. defect info*/
	unsigned long defectst;		/*sector address of the defect map*/
	unsigned long defectsz;		/*size in bytes of defect map*/
	unsigned long relno;		/*number of relocation areas*/
	unsigned long relst;		/*sector address of relocation area*/
	unsigned long relsz;		/*size in sectors of relocation area*/
	unsigned long relnext;		/*address of next avail reloc sector*/
};

struct pdsector	{
	struct pdinfo pdinfo;		/*disk physical info*/
	unsigned long reserved[10];	/*reserved for UNIX*/
	unsigned long devsp[97];	/*used for device specific info*/
					/*remaining words in sector 0*/
};

struct io_arg	{
	int retval;
	unsigned long sectst;
	unsigned long memaddr;
	unsigned long datasz;
} ;
