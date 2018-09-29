/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */

#ident	"@(#)vm:oh/nextpart.c	1.1"

#include <stdio.h>
#include <string.h>
#include "but.h"
#include "tsys.h"
#include "typetab.h"
#include "optabdefs.h"
#include "partabdefs.h"

struct one_part *
opt_next_part(entry)
struct opt_entry *entry;
{
	static int partsleft;
	static int curoffset;
	struct one_part *retval;
	extern struct one_part  Parts[MAXPARTS];

	if (entry) {
		partsleft = entry->numparts;
		curoffset = entry->part_offset;
	}
	if (partsleft > 0) {
		retval = Parts + curoffset++;
		partsleft--;
	} else
		retval = NULL;

	return(retval);
}
