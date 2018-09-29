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
 * Location of the users' stored registers relative to R0.
 * Usage is u.u_ar0[XX].
 */
#define	R0	0
#define	R1	(-2)
#define	R2	(-3)
#define	R3	(-4)
#define	R4	(-8)
#define	R5	(-9)
#define	R6	(-10)
#define	R7	(-11)

#define	FP	(-6)
#define	SP	(-1)
#define	PSR_MOD	3
#define	PC	2
