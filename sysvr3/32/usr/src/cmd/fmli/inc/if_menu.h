/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */

#ident	"@(#)fmli:inc/if_menu.h	1.2"

#define MAX_PATHS	(4)	/* max number of OBJPATH's */

#ifndef WISH
#define MAX_MENUS	(14)	/* max number of menu lines */
#define MENU_LABSIZ	(32)
#else
#define MAX_MENUS	(50)	/* WISH allows more */
#define MENU_LABSIZ	(60)	/* WISH needs more */
#endif

struct menu_entry {
#ifdef WISH
	char *label;
#else
	char label[MENU_LABSIZ];
#endif
	char *command;
};
