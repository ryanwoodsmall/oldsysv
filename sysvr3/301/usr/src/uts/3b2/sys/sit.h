/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/sit.h	10.1"
/*
 *	Programmable Interval Timer (8253)
 *
 *				     7	    0
 *				    +--------+
 *	(struct	sit *) -> count0    |	     |
 *				    +--------+
 *
 *				     7	    0
 *				    +--------+
 *		       -> count1    |	     |
 *				    +--------+
 *
 *				     7	    0
 *				    +--------+
 *	               -> count2    |	     |
 *				    +--------+
 *
 *				     7	    0
 *				    +--------+
 *	               -> command   |	     |
 *				    +--------+
 *
 * 16 bit counters, loaded and read 1 byte at a time.
 *
 */

struct	sit {
	int :24; unsigned char count0;
	int :24; unsigned char count1;
	int :24; unsigned char count2;
	int :24; unsigned char command;
	int :24; unsigned char c_latch;
	};

/*
 *	control word format:
 *		  7   6   5   4  3  2  1   0
 *		SC1 SC0 RL1 RL0 M2 M1 M0 BCD
 */
#define SITSC	0xc0				/* SC mask */
#define SITRL	0x30				/* RL mask */
#define SITMD	0x0e				/* M mask */

#define SITCT0	0x00				/* select counter (SC) */
#define SITCT1	0x40
#define SITCT2	0x80
#define SITILL	0xc0

#define SITLAT	0x00				/* read/load (RL) */
#define SITLSB	0x10
#define SITMSB	0x20
#define SITLMB	0x30

#define SITMD0	0x00				/* mode (M) */
#define SITMD1	0x02
#define SITMD2	0x04
#define SITMD3	0x06
#define SITMD4	0x08
#define SITMD5	0x0a

#define SITBIN	0x00				/* BCD */
#define SITBCD	0x01

#define MAXMS	0x00				/* maximum time - 640ms */

#define ITINIT	SITCT1|SITLMB|SITMD2|SITBIN	/* init interval timer */
#define ITLSB	0xe8		    		/* least sig byte - 10ms */
#define ITMSB	0x03		    		/* most  sig byte - 10ms */

#define STINIT	SITCT0|SITLMB|SITMD4|SITBIN	/* init sanity timer */
#define STLSB	MAXMS				/* least sig byte - 640ms */
#define STMSB	MAXMS				/* most  sig byte - 640ms */

#define SITST	0x00;				/* sanity timer - sublev0 */
#define SITIT	0x05;				/* interval timer - sublev5 */

#define	OSIT		0x00042000L
#define	SBDSIT		((struct sit *)OSIT)
