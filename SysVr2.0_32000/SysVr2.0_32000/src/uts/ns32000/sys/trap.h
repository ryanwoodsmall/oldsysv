/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
/*
 * Trap type values
 */

#define	NVIFLT	0	/* non-vectored interrupt */
#define	NMIFLT	1	/* non-maskable interrupt */
#define SEGFLT	2	/* abort, but not protection only */
#define FPUFLT	3	/* fpu trap */
#define ILLFLT	4	/* illegal operation */
#define	SYSCALL	5	/* svc */
#define	DVZFLT	6	/* divide by zero */
#define	FLGFLT	7	/* flag */
#define BPTFLT	8	/* breakpoint */
#define	TRCTRAP	9	/* trace trap */
#define UNDFLT	10	/* undefined instruction */
#define RESCHED	15	/* Reschedule pseudo-trap */
#define PROTFLT	16	/* protection abort */
#define PGRDFLT	17	/* read error during page fault */
#define ASTTRAP	18	/* user level prof pseudo-trap */
