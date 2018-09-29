/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sa:sys/sd00_dep.h	1.2"
/*
 * Copyright (c)  1986 AT&T
 * All Rights Reserved.
 *
 * Bit Structure of the External Device Number:
 *      
 *           MAJOR           MINOR      
 *      -------------------------------
 *      |  tttttttt  |  x lll pppp    |
 *      -------------------------------
 *      
 *         x = don't care
 *         t = target driver index (0-255)
 *         l = logical unit number (0-7)
 *         p = partition number (0-15)
 *
 * Bit Structure of the Internal Device Number:
 *      
 *           MAJOR           MINOR      
 *      -----------------------------
 *      |  ssssssss  |  iiiiiiii    |
 *      -----------------------------
 *      
 *         s = switch table index (0-255)
 *         i = target controller index (0-255)
*/

#define DEBUGFLG	73
#define DEBUGNOP	74
#define DEBUGRESET	75
#define DEBUGABORT	76
#define DEBUGFLUSH	77

#define SECTORSZ	512

#define T_LIMIT		5000

#define	MAX_LUN		4

#define	PDSECTOR	0

#define	VTOCBLK		1

#define SUCCESS		0

#define MASK_FF		0xFF

#define	FALSE		0
#define	TRUE		(~FALSE)

#define	LU(t,l)		sd00_tc[t].lu[l]
#define VTOC(t,l)	sd00_tc[t].lu[l].vtp
#define	PT(t,l,p)	sd00_tc[t].lu[l].vtp.v_part[p]

#define	NEXT(t,l)	sd00_tc[t].lu[l].next
#define	CURRENT(t,l)	sd00_tc[t].lu[l].current

#define	EMULEX		1		/* Target controller type      */

#define	OFFLINE		0		/* Logical unit state          */
#define	ONLINE		1		/* Logical unit state          */

#define	DISK0		1		/* Disk drive type             */

#define SD00_TCN(emj)	(pdi_iminor (emj))
#define SD00_LUN(emn)	(((emn) >> 4) & 0x7)
#define SD00_PTN(emn)	((emn) & 0xF)

#define VTOC_UPDATE	-1	/* VTOC Needs to be reread from the disk */

struct sd00_ctrl {
	unsigned short	type;		/* target controller type */
	unsigned short	lu_cnt;		/* logical unit count */

	struct lu_ctrl {
		unsigned long	pad;
		unsigned char	state;		/* logical unit state    */
		unsigned char	type;		/* logical unit type     */
		unsigned short	logicalst;	/* lu logical start      */
		int		jobcnt;		/* no. of jobs           */
		pdi_buf_t	*next;
		pdi_buf_t	*current;
		struct vtoc	vtp;		/* lu vtoc               */
		char		lu_serial[12];	/* serial no. of LU      */
		pdi_iotime_t	lustat;		/* for performance data  */
	} lu[MAX_LUN];
};

