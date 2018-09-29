/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */

#ident	"@(#)vm.oh:src/oh/opt_rename.c	1.1"

#include <stdio.h>
#include <string.h>
#include "but.h"
#include "tsys.h"
#include "typetab.h"
#include "ifuncdefs.h"
#include "optabdefs.h"
#include "partabdefs.h"

bool
opt_rename(entry, newbase, allnames)
struct ott_entry *entry[MAXOBJPARTS+1];
char *newbase;
char allnames[MAXOBJPARTS][ONAMESIZ];
{
	char *part_construct();
	register int i = 0, n = 0;
	struct opt_entry *partab;
	int part_offset;
	char *base, *p;
	extern struct one_part  Parts[MAXPARTS];
	struct opt_entry *obj_to_parts();
	char *part_match();
	

	if ((partab = obj_to_parts(entry[0]->objtype)) == NULL)
		return(O_FAIL);
	part_offset = partab->part_offset;

	if (base = part_match(entry[0]->name, Parts[part_offset].part_template)) {
		strcpy(allnames[n++], 
			part_construct(newbase, Parts[part_offset+i].part_template));
		if (++entry == NULL)
			return(O_OK);
	} else
		return(O_FAIL);

	for (i = 1; i < partab->numparts; i++) {
		p = part_construct(base, Parts[part_offset+i].part_template);
		if (strcmp(entry[0]->name, p) == 0) {
			strcpy(allnames[n++], 
				part_construct(newbase, Parts[part_offset+i].part_template));
			if (++entry == NULL)
				return(O_OK);
		} else if (!(Parts[part_offset+i].part_flags & PRT_OPT) ) {
			return(O_FAIL);
		}
	}
	return(O_OK);
}
