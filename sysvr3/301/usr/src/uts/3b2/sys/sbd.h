/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/sbd.h	10.5"
/*
 *  3B2 Internal System Board Data Definitions
 */

/*
 *	Internal ROM
 */
#define	SBDROM		0x0L
#define	ROMSIZE		0x10000L

/*
 *	Memory Size Register
 */

#define DRDEVL		0x01		/* large DRAM devices on memory array */
#define MBNK2		0x02		/* number of banks equipped on array */
#define DRSIZL		0x100000 	/* large devs in one bank total 1 M */
#define DRSIZS		0x40000  	/* small devs in one bank total 256 K */
#define	OMEMSIZ		0x0004C003L
#define	MEMSIZ		*((char *)OMEMSIZ)

/*
 *	Mainstore Memory Space
 */
#define	MAINSTORE	0x2000000L
#define SPMEM		0x2004000L
#define EPMEM		0x2100000L
/*
 *  User address space offsets
 *
 *****************************  NOTE - NOTE  *********************************
 *
 *	 ANY CHANGES THE THE FOLLOWING DEFINES, NEED TO BE REFLECTED IN
 *	    EITHER ml/misc.s, OR ml/ttrap.s, OR BOTH.
 */

#define UVBASE		((unsigned)0x80000000L)
#define UVTEXT		((unsigned)0x80800000L)
#define UVUBLK		((unsigned)0xc0000000L)
#define UVSTACK		((unsigned)0xc0020000L)
#define UVSHM		((unsigned)0xc1000000L)


struct r8	/* generalized 8-bit register */
	{
	int :16; int :8;	unsigned char data;
	};

struct r16	/* generalized 16-bit register */
	{
	int :16;	unsigned short data;
	};
