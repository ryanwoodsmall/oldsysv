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
/*	trace.h 6.1 of 8/22/83
	@(#)trace.h	6.1
 */

/*
 * Per trace structure
 */
struct trace {
	struct 	clist tr_outq;
	short	tr_state;
	short	tr_chbits;
	short	tr_rcnt;
	unsigned char	tr_chno;
	char	tr_ct;
};
