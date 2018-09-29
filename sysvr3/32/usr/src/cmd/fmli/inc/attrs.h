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

#ident	"@(#)vm.inc:src/inc/attrs.h	1.2"

#define ATTR_NORMAL	0	/* normal video */
#define ATTR_HIDE	1	/* border of non-current window */
#define ATTR_HIGHLIGHT	2	/* border of current window */
#define ATTR_SELECT	3	/* attribute of "selector bar" */
#define ATTR_SHOW	4	/* something visible (errors, etc) */
#define ATTR_VISIBLE	5	/* the most annoying thing terminal can do */
#define ATTR_UNDERLINE	6	/* attribute of underline */
#define ATTR_MARK	7	/* attribute of "marked" items */
#define NUM_ATTRS	8
