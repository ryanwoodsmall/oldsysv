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

#ident	"@(#)vm.inc:src/inc/menudefs.h	1.2"

#define MENU_UNDEFINED	(-1)
#define MENU_MRK	(1)

struct menu_line {
	char	*highlight;
	char	*lininfo;
	char	*description;
	short	flags;
};
