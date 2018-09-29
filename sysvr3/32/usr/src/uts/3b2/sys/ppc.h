/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/ppc.h	10.2.2.2"	/* modified by gwb 10/14/87	*/

/*
 * b_state (board state) flags
 *
 *	ISSYSGEN:	board has been sysgen'ed and is operational
 *	CIOTYPE		common IO type command
 *	SYSGFAIL        indicated that SYSGEN has failed
 *	EXPRESS:	REQUEST EXPRESS entry "being used"
 */
#define ISSYSGEN	0x0001
#define CIOTYPE		0x0002
#define SYSGFAIL	0x0004
#define EXPRESS		0x8000


/*
 * t_dstat (special tty struc flags) flag definition
 *	SETOPT		a SET_OPTION command is in progress
 *	ISOFLUSH	flush output is in progress
 *	WENTRY		wait for xmit queue entry
 *      SUPBUF          have buffers been supplied for receive?
 *	CLDRAIN		wait for output to drain
 *	OPDRAIN		wait for output to drain
 *	WBREAK		wait for break send completion
 */
#define SETOPT		0x0001
#define ISOFLUSH	0x0002
#define WENTRY 		0x0004
#define SUPBUF		0x0008
#define CLDRAIN		0x0020
#define OPDRAIN		0x0040
#define WBREAK          0x0080


/*
 * Constants needed for determining the version
 * of the PORTS board, e. g. PORTS or HIPORTS
 *
 *	PPC_VERS	request version number of a ppc board (ioctl)
 *	O_VERS		oflag value for opening a board to read version number
 *	DEFAULTVER	default version number for boards without one
 *	HIPORTS		rom version number of the HIPORT board
 */

#define PPC_SETPGRP 987		/* ppc ioctl cmd for resetting pgrp of
				 * ports tty after a SIGHUP clears it
				 * gwb 10/14/87	*/

#define PPC_VERS (('v'<<8)|1)   
#define O_VERS  0200	       
#define DEFAULTVER 1           
#define HIPORTS  2              



extern char dstat[];
#define	QENTDRN		0x0010		/* wait for request entry to drain */

/*
 * PORTS board control block
 */
struct ppcboard {
	unsigned short b_state;	/* board state flags		     */
	short dcb;		/* desired nbr of cblocks for rqueue */
	short qcb;		/* actual nbr of allocated cblocks   */
	short retcode;		/* return code for lla commands	     */
 	short p_nbr[NUM_QUEUES];	/* bnr of entries in queue   */
	RQUEUE rqueue;
	CQUEUE cqueue;
	};

#define BSTATE(b)	pp_board[b].b_state
#define PNBR(b,p)	pp_board[b].p_nbr[p]



/*
 * Variables allocated by the operating system
 */

extern int pp_cnt;
extern struct tty pp_tty[];
extern char *pp_addr[];
extern struct ppcboard pp_board[];
extern int csbit[];
extern short ppcbid[];
extern short ppcpid[];
extern RENTRY savee[];
extern int maxsavexp;

/*
 Variables allocated by the driver
 */

extern int pp_bnbr;
