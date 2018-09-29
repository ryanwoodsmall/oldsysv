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
 *  	processor status
 *	bits are shifted left 16 for use in psr_mod
 */

#define	PS_C	0x0010000	/* carry flag */
#define	PS_T	0x0020000	/* trace flag */
#define	PS_L	0x0040000	/* low flag */
#define	PS_F	0x0200000	/* flag flag */
#define	PS_Z	0x0400000	/* zero flag */
#define	PS_N	0x0800000	/* negative flag */
#define PS_U	0x1000000	/* user mode flag */
#define PS_S	0x2000000	/* stack flag */
#define PS_P	0x4000000	/* trace trap pending flag */
#define PS_I	0x8000000	/* interrupt enable flag */
