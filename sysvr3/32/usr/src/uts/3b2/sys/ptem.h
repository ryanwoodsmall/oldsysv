/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/ptem.h	10.1"
/*
 *	Copyright 1984 AT&T
 */

/*
 * Windowing structure to support JWINSIZE/TIOCSWINSZ/TIOCGWINSZ 
 */
struct winsize {
	unsigned short ws_row;       /* rows, in characters*/
	unsigned short ws_col;       /* columns, in character */
	unsigned short ws_xpixel;    /* horizontal size, pixels */
	unsigned short ws_ypixel;    /* vertical size, pixels */
};

/*
 * The ptem data structure used to define the global data
 * for the psuedo terminal emulation  streams module
 */
struct ptem
{
	unsigned short cflags;	/* copy of c_cflags */
	unsigned short state;	/* state of ptem entry, free or not */
	mblk_t *dack_ptr;	/* pointer to preallocated message block used to ACK disconnect */
	queue_t *q_ptr;		/* pointer to the ptem read queue */
	struct winsize wsz;	/* struct to hold the windowing info. */
	short ttypgid;		/* process group for controlling tty */
};
/*
 * state flags
 * if flag is zero then free then ptem entry is free to be allocated
 */
#define INUSE 	0x1
#define RDSIDE  0x2
#define WRTSIDE  0x4
