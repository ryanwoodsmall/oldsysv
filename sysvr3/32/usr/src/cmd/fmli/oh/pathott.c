/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.oh:src/oh/pathott.c	1.3"

#include <stdio.h>
#include "typetab.h"

extern int Vflag;

struct ott_entry *
path_to_ott(path)
char	*path;
{
	register char	*name;
	register struct ott_entry	*entry;
	struct ott_entry	*name_to_ott();
	struct ott_entry	*dname_to_ott();
	char	*filename();
	char	*parent();
	char	*nstrcat();
	char	*path_to_title();

	if (make_current(parent(path)) == O_FAIL) {
		if (Vflag)
			mess_temp(nstrcat("Could not open folder ", path_to_title(parent(path), NULL), NULL));
		else
			mess_temp("Command unknown, please try again");
		return(NULL);
	}
	if ((entry = name_to_ott(name = filename(path))) == NULL &&
			(entry = dname_to_ott(name)) == NULL) {
		mess_temp(nstrcat("Could not access object ", name, NULL));
		return(NULL);
	}
	return(entry);
}
