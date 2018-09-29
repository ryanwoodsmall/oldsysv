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
