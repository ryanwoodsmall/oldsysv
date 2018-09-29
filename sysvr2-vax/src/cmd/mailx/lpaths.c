
/*
 * mailx -- a modified version of a University of California at Berkeley
 *	mail program
 */

/* static char *SccsId = "@(#)lpaths.c	1.3"; */

/*
 *	libpath(file) - return the full path to the library file
 *	binpath(file) - return the full path to the library file
 *
 *	If MAILXTEST is defined in the environment, use that.
 */

#include "uparm.h"

char *
libpath (file)
char *file;		/* the file name */
{
static char buf[100];	/* build name here */
char *envexlib;		/* the pointer returned by getenv */
extern char *getenv();

	if ( (envexlib = getenv ("MAILXTEST")) != 0) {
		strcpy (buf, envexlib);
		strcat (buf, "/lib/mailx");
	} else {
		strcpy (buf, LIBPATH);
	}
	strcat (buf, "/");
	strcat (buf, file);
	return (buf);
}

/*
 * --  same thing for bin
 */

char *
binpath (file)
char *file;		/* the file name */
{
static char buf[100];	/* build name here */
char *envexlib;		/* the pointer returned by getenv */
extern char *getenv();

	if ( (envexlib = getenv ("MAILXTEST")) != 0) {
		strcpy (buf, envexlib);
		strcat (buf, "/bin");
	} else {
		strcpy (buf, BINPATH);
	}
	strcat (buf, "/");
	strcat (buf, file);
	return (buf);
}
