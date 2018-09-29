/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
/* isdest -- predicate which returns TRUE if the specified name is
	     a legal lp destination, FALSE if not.		*/

#include	"lp.h"

SCCSID("@(#)isdest.c	3.1")

isdest(name)
char *name;
{
	char dest[FILEMAX];

	if(*name == '\0' || strlen(name) > DESTMAX)
		return(FALSE);

	/* Check request directory */

	sprintf(dest, "%s/%s/%s", SPOOL, REQUEST, name);
	return(eaccess(dest, ACC_R | ACC_W | ACC_X | ACC_DIR) != -1);
}
