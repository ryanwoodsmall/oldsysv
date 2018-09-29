/* @(#)trap.h	1.1 */
/*
 * Trap type values
 */

#define	RSADFLT	0	/* reserved addressing fault */
#define	PRIVFLT	1	/* privileged instruction fault */
#define	BPTFLT	2	/* bpt instruction fault */
#define	IOTFLT	3	/* iot instruction fault */
#define	EMTFLT	5	/* emt instruction fault */
#define	SYSCALL	6	/* trap instruction (syscall trap) */
#define	ARTHTRP	8	/* arithmetic trap */
#define	SEGFLT	9	/* segmentation fault */
#define	PRTYFLT	10	/* memory parity fault */
#define	RESCHED	12	/* software level 1 trap (reschedule trap) */
