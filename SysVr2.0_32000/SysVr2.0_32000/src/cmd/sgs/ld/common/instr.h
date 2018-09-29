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
 * static char ID_instrh[] = "@(#) instr.h: 1.1 1/6/82";
 */

extern long	SDP_read, SDP_write,
		LD_lock, LD_unlock;

extern long	noutsyms,
		maxrelocsms;

extern long	numldsyms, numldaux, nwalks, nfwalks, ncolisns, maxchain;

extern long	allocspc,
		maxalloc;

extern long ttime;
extern struct tbuffer {
	long proc_user_time;
	long proc_system_time;
	long child_user_time;
	long child_system_time;
	} ptimes;
