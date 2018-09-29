/* @(#)psl.h	1.1 */
/*
 *  	processor status
 */

#define	PS_C	0x1		/* carry bit */
#define	PS_V	0x2		/* overflow bit */
#define	PS_Z	0x4		/* zero bit */
#define	PS_N	0x8		/* negative bit */
#define	PS_T	0x10		/* trace enable bit */
#define	PS_IPL	0340		/* interrupt priority level */
#define	PS_PRV	0030000		/* previous mode */
#define	PS_CUR	0140000		/* current mode */
