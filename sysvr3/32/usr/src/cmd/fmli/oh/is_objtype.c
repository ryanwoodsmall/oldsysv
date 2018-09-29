/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.oh:src/oh/is_objtype.c	1.1"

#include <stdio.h>
#include "wish.h"
#include "terror.h"
#include "typetab.h"
#include "detabdefs.h"

extern struct odft_entry Detab[MAXODFT];

bool
is_objtype(obj)
char *obj;
{
	register int i;

	for (i = 0; Detab[i].objtype[0]; i++)
		if (strCcmp(obj, Detab[i].objtype) == 0)
			return(TRUE);
	return(FALSE);
}
