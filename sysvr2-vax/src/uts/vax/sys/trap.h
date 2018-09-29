/* @(#)trap.h	6.3 */
/*
 * Trap type values
 */

#define	RSADFLT	0	/* reserved addressing fault */
#define	PRIVFLT	1	/* privileged instruction fault */
#define	BPTFLT	2	/* bpt instruction fault */
#define	XFCFLT	3	/* xfc instruction fault */
#define	RSOPFLT	4	/* reserved operand fault */
#define	SYSCALL	5	/* chmk instruction (syscall trap) */
#define	ARTHTRP	6	/* arithmetic trap */
#define	RESCHED	7	/* software level 1 trap (reschedule trap) */
#define	SEGFLT	8	/* segmentation fault */
#define	PROTFLT	9	/* protection fault */
#define	TRCTRAP	10	/* trace trap */
#define	CMPTFLT	11	/* compatibility mode fault */
#define ASTTRAP	12	/* asynchronous trap (user level profiling) */
#define PGRDFLT 13	/* read error during page fault processing */
