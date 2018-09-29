/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:swinc/sys/types.h	1.1"
typedef	struct { int r[1]; } *	physadr;
typedef	long		daddr_t;	/* <disk address> type */
typedef	char *		caddr_t;	/* ?<core address> type */
typedef	unsigned short	ushort;
typedef	ushort		ino_t;		/* <inode> type */
typedef short		cnt_t;		/* ?<count> type */
typedef	long		time_t;		/* <time> type */
typedef	int		label_t[10];
typedef	short		dev_t;		/* <device number> type */
typedef	long		off_t;		/* ?<offset> type */
typedef	long		paddr_t;	/* <physical address> type */
typedef	int		key_t;		/* IPC key type */
typedef unsigned int	uint;

typedef struct _SDE {			/*  segment descriptor  */
/*                                                                            */
/*  +--------+--------------+--+--------+ +--------------------------------+  */
/*  | access |    maxoff    |  |  flags | |             address            |  */
/*  +--------+--------------+--+--------+ +--------------------------------+  */
/*       8          14        2     8                      32                 */
/*                                                                            */
/*                                        +--------------------------+-+-+-+  */
/*                         (V0):          |                          |N|W|S|  */
/*                                        +--------------------------+-+-+-+  */
/*                                                     29             1 1 1   */
/*                                                                            */
    unsigned int access   :  8;			/* Access rights	*/
    unsigned int maxoff   : 14;			/* Segment's max offset	*/
    unsigned int          :  2;			/* Reserved		*/
    unsigned int flags    :  8;			/* Descriptor flags	*/
    union {
	unsigned int address;
	struct {
	    unsigned int     	 : 29;
	    unsigned int lock  	 :  1;		/* "N" bit  */
	    unsigned int shmswap :  1;		/* "W" bit  */
	    unsigned int alloc 	 :  1;		/* "S" bit  */
	} V0;
    } wd2;
} sde_t;		/* old name: SDE     */

/*
 * MAC32 Data Typedef's
 */
typedef struct psw {		/* program status word */
		unsigned int	rsvd	:6;	/* Unused bits             */
 		unsigned int	CSH_F_D	:1;	/* Cache Flush Disable     */
		unsigned int	QIE	:1;	/* Quick Interrupt Enable  */
		unsigned int	CSH_D	:1;	/* Cache Disable	   */
		unsigned int	OE	:1;
		unsigned int	NZVC	:4;
		unsigned int	TE	:1;
		unsigned int	IPL	:4;
		unsigned int	CM	:2;
		unsigned int	PM	:2;
		unsigned int	R	:1;
		unsigned int	I	:1;
		unsigned int	ISC	:4;
		unsigned int	TM	:1;
		unsigned int	FT	:2;
	      } PSW;

typedef int SRAMA;			/* SRAMA Area			*/

typedef struct _SRAMB {			/* SRAMB Area			*/
/*							*/
/*  +--------------+------------+----------------+	*/
/*  |   Reserved   |   SDTlen   |    Reserved    |	*/
/*  +--------------+------------+----------------+	*/
/*         9             13             10		*/
/*							*/
    unsigned int        :  9;		/* Reserved			*/
    unsigned int SDTlen : 13;		/* Number of segments in section*/
    unsigned int        : 10;		/* Reserved			*/
} SRAMB;


/* MMU specific definitions */

struct psram			/* MMu srama and sramb entries for process */
	{
	SRAMA	sram2st;	/* section 2 SRAMA */
	SRAMA	sram3st;	/* section 3 SRAMA */
	};

struct mmuproc		/* process segment descriptor table */
	{
				/* The "137" below is really NUSEGD
					from param.h, and should
					track it
				*/
		sde_t seg[137];
	} ;
