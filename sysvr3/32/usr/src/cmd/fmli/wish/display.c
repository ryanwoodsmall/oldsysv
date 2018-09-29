/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.wish:src/wish/display.c	1.1"

#include <stdio.h>
#include "wish.h"
#include "typetab.h"

int
glob_display (path)
char	*path;
{
	char	*vpath;
	char	title[PATHSIZ];
	struct	ott_entry *path_to_ott();
	char	*path_to_vpath();
	char	*path_to_title();
	struct	ott_entry *ott, *path_to_ott();

	if ((vpath = path_to_vpath(path)) == NULL) {
		mess_temp("Object is not displayable");
		return(FAIL);
	}
	sprintf(title, "%s/%s - %s", path_to_title(parent(path), NULL), 
				(ott = path_to_ott(path))->dname, ott->display);
	return(objop("OPEN", "TEXT", "$VMSYS/OBJECTS/Text.disp", 
				vpath, title, NULL));
}
