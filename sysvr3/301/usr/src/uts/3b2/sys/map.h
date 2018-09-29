/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/map.h	10.2"
/*
 *		struct map	X[]	.m_size		.m_addr
 *				---	------------	-----------
 *				[0]	mapsize(X)	mapwant(X)
 *					# X[] unused	sleep value
 *
 *		  mapstart(X)->	[1]	# units		unit number
 *				 :	    :		  :
 *				[ ]	    0
 */

struct map
{
	unsigned long m_size;	/* number of units available */
	unsigned long m_addr;	/* address of first available unit */
};

extern struct map sptmap[];	/* Map for system virtual space.   */

#define	mapstart(X)	&X[1]		/* start of map array */
#define	mapwant(X)	X[0].m_addr
#define	mapsize(X)	X[0].m_size	/* number of empty slots \
					   remaining in map array */
#define	mapdata(X) {(X)-2, 0} , {0, 0}
#define	mapinit(X, Y)	X[0].m_size = (Y)-2

uint malloc();
