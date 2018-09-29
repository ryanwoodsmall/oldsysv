/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.vt:src/vt/lp.c	1.1"

#include <curses.h>
#include <term.h>

char *
lpon()
{
	return((char *)prtr_on);
}

char *
lpoff()
{
	return((char *)prtr_off);
}
