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
/*
 *	gwd(wkdir) -- place name of working directory in wkdir
 *	returns 0 for success, -1 for failure
 */

#include	"lp.h"

SCCSID("@(#)gwd.c	3.1")

gwd(wkdir)
char *wkdir;
{
	FILE *fp, *popen();
	char *c;

	if ((fp = popen("pwd 2>/dev/null", "r")) == NULL) {
		*wkdir = '\0';
		return(-1);
	}
	if (fgets(wkdir, FILEMAX, fp) == NULL) {
		pclose(fp);
		*wkdir = '\0';
		return(-1);
	}
	if (*(c = wkdir + strlen(wkdir) - 1) == '\n')
		*c = '\0';
	pclose(fp);
	return(0);
}
