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
/* @(#)versys.c	1.6 */
#include "uucp.h"


#define SNAMESIZE 7

/*
 * verify system name
 *	name	-> system name
 * returns:  
 *	0	-> success
 *	FAIL	-> failure
 */
versys(name)
char *name;
{
	register FILE *fp;
	register char *iptr;
	char line[300];
	char s1[SNAMESIZE + 1];
	char myname[SNAMESIZE + 1];
	char *strchr(), *strpbrk();

	sprintf(myname, "%.6s", Myname);
	sprintf(s1, "%.6s", name);
	if (strncmp(s1, myname, SYSNSIZE) == SAME)
		return(0);
	fp = fopen(SYSFILE, "r");
	if (fp == NULL)
		return(FAIL);
	
	while (fgets(line, sizeof(line) ,fp) != NULL) {
		if((line[0] == '#') || (line[0] == ' ') || (line[0] == '\t') || 
			(line[0] == '\n'))
			continue;
		if ((iptr=strpbrk(line, " \t")) == NULL) continue;
		*iptr = '\0';
		line[SYSNSIZE] = '\0';
		if (strncmp(s1, line, SYSNSIZE) == SAME) {
			fclose(fp);
			return(0);
		}
	}
	fclose(fp);
	return(FAIL);
}
