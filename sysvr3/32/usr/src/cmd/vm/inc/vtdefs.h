/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 *
 */

#ident	"@(#)vm:inc/vtdefs.h	1.1"

#define VT_UPARROW	32
#define VT_DNARROW	64
#define VT_NONUMBER	16384
#define VT_NOBORDER	32768

#define VT_UNDEFINED	((vt_id) -1)

/* indicates cost function to use when creating a new vt */
#define VT_NOOVERLAP	0
#define VT_CENTER	1
#define VT_COVERCUR	2
#define VT_NOCOVERCUR	3
#define NUMCOSTS	4
#define VT_COSTS	3	/* AND off the COST part of the flags */

#define STATUS_WIN	0
#define CMD_WIN		1
#define MESS_WIN	2

/* "funny" characters */
#define MENU_MARKER	'\1'	/* RIGHT ARROW */
