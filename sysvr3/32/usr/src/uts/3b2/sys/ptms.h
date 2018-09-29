/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/ptms.h	10.2"
/*
 *	         Copyright (c) 1987 AT&T
 *		   All Rights Reserved
 *
 *     THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
 *   The copyright notice above does not evidence any actual
 *   or intended publication of such source code.
 *
 ================================================================
 =   WARNING!!!! This source is not supported in future source  =
 =   releases.                                                  =
 ================================================================
 */

/*
 * Structures and definitions supporting the pseudo terminal
 * drivers.
 */
struct pt_ttys {
	unsigned pt_state;	/* state of master/slave pair */
	queue_t *ptm_wrq; 	/* master's write queue pointer */
	queue_t *pts_wrq; 	/* slave's write queue pointer */
	mblk_t *pt_bufp;        /* ptr. to zero byte msg. blk. */
	short tty;	        /* controlling tty pid */
};

/*
 * pt_state values
 */
#define PTLOCK	01	/* master/slave pair is locked */
#define PTMOPEN 02  	/* master side is open */
#define PTSOPEN 04	/* slave side is open */

/*
 * ioctl commands
 */
#define ISPTM	(('P'<<8)|1)	/* query for master */
#define UNLKPT	(('P'<<8)|2)	/* unlock master/slave pair */

