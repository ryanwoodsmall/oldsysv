/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/reg.h	10.2"
/*
 * Location of the users' stored registers relative to R0.
 * Usage is u.u_ar0[XX].
 *
 * NOTE: all of these are valid for user-level registers.
 *	 only PC and PS are valid during fault/exception handling
 */

#define	R0	0
#define	R1	1
#define	R2	2
#define	R3	3
#define	R4	4
#define	R5	5
#define	R6	6
#define	R7	7
#define	R8	8
#define	FP	-1	/* FP == R9  */
#define	AP	-2	/* AP == R10 */
#define	PS	-7	/* PS == R11 */
#define	SP	-5	/* SP == R12 */
#define	PC	-6	/* PC == R15 */
