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
/* @(#)gnsys.c	1.6 */
#include "uucp.h"


#define LSIZE 30	/* number of systems to store */
#define WSUFSIZE 7	/* work file name suffix size */

char *strcpy();

/*
 * return the next system -name which has
 * work to be done.
 * pre is the prefix for work files.
 * dir is the directory to search.
 * sname is a string of size DIRSIZ - WSUFSIZE.
 * to return name of called system
 * returns:
 *	0	-> no more names
 *	1	-> name returned in sname
 *	FAIL 	-> bad directory
 */
int	rotindx;
gnsys(sname, dir, pre)
char *sname, *dir, pre;
{
	register FILE *fp;
	char *p1;
	char px[3];
	static char *list[LSIZE];
	static int nitem=0, n=0;
	char sysname[NAMESIZE], filename[NAMESIZE];
	long	time();
	void free();

	px[0] = pre;
	px[1] = '.';
	px[2] = '\0';
	if (nitem == 0) {

		/*
		 * get list of systems with work
		 */
		fp = fopen(dir, "r");
		ASSERT(fp != NULL, "BAD DIRECTRY", dir, 0);
		{
			register int i;

			for (i = 0; i < LSIZE; i++)
			list[i] = NULL;
		}
		while (gnamef(fp, filename) == TRUE) {
DEBUG(4,"gns %s\n",filename);
			if (prefix(px, filename) == FALSE)
				continue;
			{
				register char *s, *p, *p2;
				p2 = filename + strlen(filename)
					- WSUFSIZE;
				p1 = filename + strlen(px);
				for(s = sysname; p1 <= p2; p1++)
					*s++ = *p1;
				*s = '\0';
			}
DEBUG(4,"gns1 %s\n",sysname);
			if (sysname[0] == '\0')
				continue;
			if (callok(sysname) != 0)
				continue;
			if (mlock(sysname) != 0)
				continue;
			else delock(sysname);
			nitem = srchst(sysname, list, nitem);
			if (LSIZE <= nitem) break;
		}

		/*
		 * Random selextion of system to call
		 * When debugging you don't want to do this
		 */
#ifndef NOROT
		if(Debug == 0){
			if(nitem){
				rotindx = (time((long *)0))%(long)nitem;
			}else
				rotindx = 0;
		}else
#endif
			rotindx = 0;
		fclose(fp);
	}

	if (nitem == 0)
		return(FALSE);
	while(nitem > n) {
		if(rotindx >= nitem)
			rotindx = 0;
		strcpy(sname, list[rotindx]);
		rotindx++;n++;
		if (callok(sname) == 0)
			return(TRUE);
	}

	{
		register int i;

		for (i = 0; i < nitem; i++)
			if (list[i] != NULL)
				free(list[i]);
	}
	nitem = n = 0;
	return(FALSE);
}

/*
 * do a linear search of list (list)
 * to find name (name).
 * If the name is not found, it is added to the
 * list.
 * The number of items in the list (n) is
 * returned (incremented if a name is added).
 *	name	-> system name
 *	list	-> array of item
 *	n	-> # of items in list
 * return:
 *	n	-> the number of items in the list
 */
srchst(name, list, n)
register char *name, **list;
int n;
{
	register int i;
	char *p;
	extern char *calloc();

DEBUG(4,"srch %s\n",name);
	for (i = 0; i < n; i++){
DEBUG(4, "srchst1 %s\n",list[i]);
		if (strcmp(name, list[i]) == SAME)
			break;
	}
	if (i >= n) {
		if ((p = calloc((unsigned) strlen(name) + 1, sizeof (char)))
			== NULL)
			return(n);
		strcpy(p, name);
		list[n++] = p;
	}
	return(n);
}
