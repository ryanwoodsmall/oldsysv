/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident "@(#)lp:isclass.c	3.2"
/* isclass -- predicate which returns TRUE if the specified name is
	     a legal lp class, FALSE if not.		*/

#include	"lp.h"


isclass(name)
char *name;
{
	char class[FILEMAX];

	if(*name == '\0' || strlen(name) > DESTMAX)
		return(FALSE);

	/* Check class directory */

	sprintf(class, "%s/%s/%s", SPOOL, CLASS, name);
	return(eaccess(class, ACC_R | ACC_W | ACC_DIR) != -1);
}
