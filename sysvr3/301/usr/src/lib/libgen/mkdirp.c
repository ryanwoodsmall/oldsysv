/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libgen:mkdirp.c	1.4"

/* Creates directory and it's parents if the parents do not
** exist yet.
**
** Returns -1 if fails for reasons other than non-existing
** parents.
** Does NOT compress pathnames with . or .. in them.
*/

#include <stdio.h>
#include <errno.h>
#include <string.h>

extern int errno;
char *strrchr(), *strchr();
static int pperm(), makedir();

int
mkdirp(d, mode)
char *d;
int mode;
{
	char  *endptr, *ptr, *p, *slash;
	int i;

	ptr = d;


		/* Try to make the directory */

	if ((i=makedir(d, mode)) == 0)
		return(0);
	if (errno != ENOENT)
		return(-1);
	endptr = strrchr(d, '\0');
	ptr = endptr;
	slash = strrchr(d, '/');

		/* Search upward for the non-existing parent */

	while (slash != NULL) {

				/* Skip extra slashes */

		if ((p = (slash+1)) == ptr) {
			ptr = slash;
			p = slash;
			*p = '\0';
			slash = strrchr(d, '/');
			*p = '/';
			continue;
		} else
			ptr = slash;
		*ptr = '\0';

			/* If reached an existing parent, break */

		if (access(d, 00) ==0)
			break;

			/* If non-existing parent*/

		else {
			slash = strrchr(d,'/');

				/* If under / or current directory, make it. */

			if (slash  == NULL || slash== d) {
				if (makedir(d, mode))
					return(-1);
				break;
			}
		}
	}
		/* Create directories starting from upmost non-existing parent*/

	while ((ptr = strchr(d, '\0')) != endptr){
		*ptr = '/';
		if (makedir(d, mode))
			return(-1);
	}
	return(0);
}

/* Check write permission of parent of dir */

static int
pperm(dir)
char *dir;
{
	char *dp, *pp, *malloc();

	if ((dp=malloc((unsigned)strlen(dir) +4)) == NULL)
		return(-1);
	strcpy(dp, dir);
	if ((pp=strrchr(dp,'/')) == NULL) {
		if (access(".",02))
			return(-1);
		return(0);
	}
	if (pp == dp) {
		if (access("/", 02))
			return(-1);
		return(0);
	}
	*pp = '\0';
	if (access(dp, 02))
		return(-1);
	return(0);
}

/* Make directory if permitted, and change uid and gid to real uid and gid */

static int
makedir(dir, mode)
char *dir;
int mode;
{
	if (pperm(dir))
		return(-1);
	if (setuid(getuid()) || setgid(getgid()))
		return(-1);
	if (mkdir(dir,mode))
		return(-1);
	return(0);
}
