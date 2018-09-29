/* @(#)seg.h	6.3 */
/*
 * Memory management addresses and bits
 */

#define	RO	PG_URKR		/* access abilities */
#define	RW	PG_UW

#define SEG_RO		1
#define SEG_WO		2
#define SEG_RW		3
#define SEG_CW		4
