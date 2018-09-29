/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.qued:src/qued/syncfield.c	1.1"

#include <stdio.h>
#include "token.h"
#include "winp.h"
#include "fmacs.h"

syncfield(fld)
ifield *fld;
{
	savefield = Cfld;
	if (fld != NULL)
		Cfld = fld;
	/*
	 * syncronizes the visible window with the value 
	 * buffer (or scroll buffer for scrolling fields)
	 */
	Cfld = savefield;
}
