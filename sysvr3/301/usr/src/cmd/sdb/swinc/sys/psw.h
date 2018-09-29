/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:swinc/sys/psw.h	1.1"
/*
 * MAC32A internal state code values for normal exceptions
 */

#define IZDFLT		0	/* integer zero divide fault */
#define TRCTRAP		1	/* TRACE TRAP	*/
#define	ILLOPFLT	2	/* illegal opcode fault */
#define RESOPFLT	3	/* reserved opcode fault */
#define IVDESFLT	4	/* invalid descriptor fault */
#define XMEMFLT		5	/* external memory fault	*/
#define GVFLT		6	/* gate vector fault	*/
#define ILCFLT		7 	/* illegal level change fault */
#define	RDTPFLT		8	/* reserved data type fault */
#define IOVFLT		9	/* integer overflow fault */
#define PRVOPFLT	10	/* priviliged opcode fault */
#define BPTRAP		14	/* break point trap */
#define PRVREGFLT	15	/* privileged register fault */

/*
 * Stack exception internal state codes
 */

#define STKBOUND	0	/* stack bounds exception */
#define STKFLT		1	/* stack fault	*/
#define INTIDFTCH	3	/* interrupt id fetch fault */

/* 
 * process exception internal state codes
 */

#define POPCBFLT	0	/* old pcb fault */
#define PGPCBFLT	1	/* gate pcb fault */
#define PNPCBFLT	4	/* new pcb fault */

/*
 * on reset internal state codes
 */

#define ROPCBFLT	0	/* reset, old pcb fault */
#define RSYSDATA	1	/* reset, system data fault */
#define RISTKFLT	2	/* reset, interrupt stack fault */
#define REXTRNL		3	/* reset, external */
#define RNPCBFLT	4	/* reset, new pcb fault */
#define RGVFLT		6 	/* reset, gate vector fault */

/*
 * MAC32A Fault Type Codes
 */

#define ON_RESET	0
#define ON_PROCESS	1
#define ON_STACK	2
#define ON_NORMAL	3

/*
 * MAC32A NZVC codes
 */

#define PS_C		1
#define PS_Z		2
#define PS_V		4
#define PS_N		8

/*
 * MAC-32 mode fields
 */

#define USER		3
#define KERNEL		0
