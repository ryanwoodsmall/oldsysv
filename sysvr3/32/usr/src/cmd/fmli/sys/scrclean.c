/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.sys:src/sys/scrclean.c	1.1"

#include <stdio.h>

screen_clean()
{
	slk_clear();
	vt_close_all();
	copyright();
	vt_flush();
	endwin();
	putchar('\n');
}
