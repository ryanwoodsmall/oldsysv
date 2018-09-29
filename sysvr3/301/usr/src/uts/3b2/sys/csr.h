/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/csr.h	10.2"
/*
 *	Control and Status Register
 *
 *  Write-only status register mapping
 *
 *				       7      0
 *				      +--------+
 *	(struct	wcsr *)	-> pir9c      |	       |
 *				      +--------+
 *
 *			-> pir9s
 *			     :
 *			-> sanityc
 *
 */
struct wcsr {
	int :24; unsigned char c_sanity;
	int :24; unsigned char c_parity;
	int :24; unsigned char s_reqrst;
	int :24; unsigned char c_align;
	int :24; unsigned char s_led;
	int :24; unsigned char c_led;
	int :24; unsigned char s_flop;
	int :24; unsigned char c_flop;
	int :24; unsigned char s_timers;
	int :24; unsigned char c_timers;
	int :24; unsigned char s_inhibit;
	int :24; unsigned char c_inhibit;
	int :24; unsigned char s_pir9;
	int :24; unsigned char c_pir9;
	int :24; unsigned char s_pir8;
	int :24; unsigned char c_pir8;
};


/*
 *  Read-only status register mapping
 *
 *				      15              0
 *				      +----------------+
 *	short rcsr 		      |	               |
 *				      +----------------+
 */

#define CSRTIMO	0x8000
#define CSRPARE	0x4000
#define CSRRRST	0x2000
#define CSRALGN	0x1000
#define CSRLED	0x0800
#define CSRFLOP	0x0400
#define CSRITIM	0x0100
#define CSRIFLT	0x0080
#define CSRCLK	0x0040
#define CSRPIR8	0x0020
#define CSRPIR9	0x0010
#define CSRUART	0x0008
#define CSRDISK	0x0004
#define CSRDMA	0x0002
#define CSRIOF	0x0001

#define	OCSR		0x00044000L
#define SBDRCSR		(((struct r16 *)OCSR)->data)
#define SBDWCSR		((struct wcsr *)OCSR)
#define Rcsr		(((struct r16 *)&sbdrcsr)->data)
#define Wcsr		((struct wcsr *)&sbdwcsr)
