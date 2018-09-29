/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/sys3b.h	10.2"

#ifndef DIRSIZ
#define	DIRSIZ	14
#endif

/*
 * Commands for sys3b system call (1-50)
 */

#define	S3BSWPI		1	/* General swap interface.		  */
#define	S3BSYM		2	/* acquire boot-built symbol table	  */
#define	S3BCONF		4	/* acquire boot-built configuration table */
#define	S3BBOOT		5	/* acquire timestamp and name of program  */
				/* booted				  */
#define S3BIOP		6	/* undocumented fast illegal op-code	  */
				/* handler interface			  */
#define S3BDMM		7	/* double-map data segment for		  */
				/* read/write/execute support		  */
#define	S3BAUTO		9	/* was an auto-config boot done?	  */
#define S3BEDT		10	/* copy contents of EDT to user		  */
#define	S3BSWAP		12	/* Declare swap space			  */
#define S3BFPHW		40	/* what (if any?) flting-pnt hardware */

/*
 * S3BSYM: Symbol table built by lboot
 */

struct	s3bsym {
	int	size;			/* total size of symbol table	*/
					/* in bytes			*/
	int	count;			/* number of symbols		*/

	/*
	 * each symbol is stored as:
	 *	char name[];  (padded with '\0' to next sizeof(long) boundry)
	 *	long value;   the symbol's value
	 */

	char	symbol[1];
};

/*	The following macro takes a pointer to a symbol name
**	in the table and returns its value.  Be sure you
**	have included "inline.h" to use this.
*/

#define	S3BSVAL(sp)	(*(uint *)((int)(strend(sp) + 1 + 3) & ~0x03))

/*	The following macro takes a pointer to a symbol name
**	in the table and returns a pointer to the next
**	succeeding entry.
*/

#define	S3BNXTSYM(sp)	((char *)(((int)(strend(sp) + 1 + 3) & ~0x03) + 4))


/*
 * S3BCONF: Configuration table built by lboot
 */
struct	s3bconf {
	int	count;				/* number of entries */
	struct	s3bc {
		long		timestamp;	/* f_timdat from file header  */
		char		name[DIRSIZ];	/* driver/module name */
		unsigned char	flag;		/* configuration information */
		unsigned char	board;		/* ELB/LBA address for a device */
	} driver[1];
};

#define	S3BC_IGN	0x80		/* EDT name[] is ignored */
#define	S3BC_DRV	0x40		/* name[] is a driver; board contains either
					 * the board code for a hardware device or
					 * zero for a software driver
					 */
#define	S3BC_MOD	0x20		/* name[] is a software module (not a driver) */


/*
 * S3BBOOT: Timestamp and path name of program booted
 */
struct	s3bboot {
	long	timestamp;		/* f_timdat from file header  */
	char	path[100];		/* path name */
};

/*
 * 3B2 specific sys3b system calls (51-?)
 */
#define GRNON		52	/* set green light to solid on state */
#define GRNFLASH	53	/* start green light flashing */
#define STIME 		54	/* set internal time */
#define SETNAME		56	/* rename the system */
#define RNVR 		58	/* read NVRAM */
#define WNVR 		59	/* write NVRAM */
#define RTODC		60	/* read time of day clock */
#define CHKSER		61	/* check soft serial number */
#define S3BNVPRT	62	/* print an xtra_nvr structure */
#define SANUPD		63	/* sanity update of kernel buffers */
#define S3BKSTR		64	/* make a copy of a kernel string */
#define S3BMEM		65	/* return the memory size of system */
#define	S3BTODEMON	66	/* Transfer control to firmware.	*/
#define	S3BCCDEMON	67	/* Control character access to demon.	*/
#define	S3BCACHE	68	/* Turn cache on and off.		*/
#define	S3BDELMEM	69	/* Delete available memory for testing.	*/
#define	S3BADDMEM	70	/* Add back deleted memory.		*/

#define TIMER		0x70
#define ENBTIMER 	0x04
#define TIME		0xF000
#define VRUNFLG  (*(((struct vectors *)(BASE+0x20000))->p_runflg))

struct todc {
	short htenths;
	short hsecs;
	short hmins;
	short hhours;
	short hdays;
	short hweekday;
	short hmonth;
	short hyear;
};

struct nvparams {
	char *addr;
	char *data;
	unsigned short cnt;
};
