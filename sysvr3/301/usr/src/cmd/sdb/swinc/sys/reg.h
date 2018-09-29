/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:swinc/sys/reg.h	1.1"
/*
 * Location of the users' stored registers relative to R0.
 * Usage is u.u_ar0[XX].
 */
#define	R0	0
#define	R1	1
#define	R2	2
#define	R3	8
#define	R4	9
#define	R5	10
#define	R6	11
#define	R7	12
#define R8	13
#define FP	7
#define AP	6
#define PS	-3
#define PC	-4
#define SP	3

#define SIG_PS	-1
#define SIG_PC	-2

#define TRAP_RET 5
