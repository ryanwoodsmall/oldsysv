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
/* @(#)gnamef.c	1.2 */
#include "uucp.h"
#include <sys/types.h>
#include <sys/dir.h>



/*
 * get next file name from directory
 *	p	 -> file description of directory file to read
 *	filename -> address of buffer to return filename in
 *		    must be of size DIRSIZ+1
 * returns:
 *	FALS	-> end of directory read
 *	TRUE	-> returned name
 */
gnamef(p, filename)
register char *filename;
FILE *p;
{
	register int i;
	register char *s;
	static struct direct dentry;

	while (1) {
		if (fread((char *)&dentry,  sizeof(dentry), 1, p) != 1)
			return(FALSE);
		if (dentry.d_ino != 0)
			break;
	}

	for (i = 0, s = dentry.d_name; i < DIRSIZ; i++)
		if ((filename[i] = *s++) == '\0')
			break;
	filename[DIRSIZ] = '\0';
	return(TRUE);
}

