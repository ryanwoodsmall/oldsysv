/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/cio_defs.h	10.2"
/*
 *	This header file contains common status and command
 *	defines for all 3b2 I/O applications and drivers.
 *	Also included are any common I/O firmware functions
 *	which are defined as macros.
 */

/*
	Return codes from CIO functions
*/

#define PASS	1	/* successful return */
#define FAIL	0	/* unsuccessful return */

/*
	Cmd/Status field of Common I/O queue entries
*/

#define CS_STATUS	0	/* queue entry is a status */
#define CS_COMMAND	1	/* queue entry is a command */

/*
	Opcodes of Common I/O queue entries
*/

#define DLM		1		/* CIO download memory		*/
#define ULM		2		/* CIO upload memory		*/
#define FCF		3		/* CIO force function call	*/
#define DOS		4		/* CIO determine op. status	*/
#define DSD		5		/* CIO determine sub devices	*/
#define ZOMB		6		/* CIO zombie 			*/
#define	DCONS		7		/* CIO determine Console device	*/
#define	IOGC		8		/* CIO I/O getchar		*/
#define	IOPC		9		/* CIO I/O putchar		*/
#define	IOBOOT		10		/* CIO Boot I/O device		*/
#define	IOBLKR		11		/* CIO I/O device read block	*/
#define	IOBLKW		12		/* CIO I/O device write block	*/
 
/*
	Status values of Common I/O queue entries
*/

#define	NORMAL		0		/* CIO NORMAL */
#define FAULT		1		/* CIO hardware fault */
#define QFAULT		2		/* CIO queue fault */
#define SYSGEN		3		/* CIO successful SYSGEN */

/*
  	Direction indicators for DMA transfers movoffb() movoffbw()
*/

#define TO_PER 	  0x0
#define TO_SYS 	  0x1

/*
	Location of the SYSGEN data block pointer on the SBD
*/

#define	DBLK_PTR	0x2000000

/*
  	Mask and unmask defines for attint() globint() and expint()
*/

#define UMSK 	  0x0
#define MSK 	  0x1

/* 
 *	putreq(x) - a macro for requesting jobs from the host;
 *		    its argument is a pointer to the completion
 *		    entry to be used as the request entry.
 */

#define CC_ENTRY(P) ((CENTRY *)P)
#define putreq(XX) CC_ENTRY(XX)->common.codes.bits.cmd_stat = 1; \
		  putcomp(XX)


/*
 *	swapshort(SHORT) - a function to swap bytes in a word
 *
 */

extern short	swapshort();	/* swap the bytes of a short */


/*
 *	swaplong(LONG) - a function to swap words and the bytes within them
 *	    	 	 in a 32 bit (long) field
 *
 */


extern long	swaplong();	/* swap the words and bytes of a long */
