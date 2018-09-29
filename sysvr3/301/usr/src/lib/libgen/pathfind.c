/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libgen:pathfind.c	1.2"
/*
	pathfind.c	1.2	1/24/85 10:03:43
*/

#include <sys/types.h>
#include <sys/stat.h>
#include "libgen.h"

/*
 * Mode bit definitions -- see mknod(2)
 * Names of flags duplicate those of test(1)
 */

#define OPTSTRING	"rwxfbcdpugks"

/* File type: 0170000 */
#define FFLAG	0100000		/* normal file - also type 0 */
#define BFLAG	060000		/* block special */
#define CFLAG	020000		/* character special */
#define DFLAG	040000		/* directory */
#define PFLAG	010000		/* fifo */

#define UFLAG	04000		/* setuid */
#define GFLAG	02000		/* setgid */
#define KFLAG	01000		/* sticky bit */

/*
 * Perms: 0700 user, 070 group, 07 other
 * Note that pathfind uses access(2), so no need to hassle
 * with shifts and such
 */
#define RFLAG	04		/* read */
#define WFLAG	02		/* write */
#define XFLAG	01		/* execute */

	char *
pathfind(path, name, mode)
	register char *path, *name;
	char *mode;	/* any combination from OPTSTRING */
{
	static char cpath[256];
	register char *cp;
	int imode;
	int nzflag;

	/* Build imode */
	imode = 0; nzflag = 0;
	if (mode == ((char *) 0))
		mode = "";
	for (cp = mode; *cp; cp++) {
		switch (*cp) {
		case 'r':
			imode |= RFLAG;
			break;
		case 'w':
			imode |= WFLAG;
			break;
		case 'x':
			imode |= XFLAG;
			break;
		case 'b':
			imode |= BFLAG;
			break;
		case 'c':
			imode |= CFLAG;
			break;
		case 'd':
			imode |= DFLAG;
			break;
		case 'f':
			imode |= FFLAG;
			break;
		case 'p':
			imode |= PFLAG;
			break;
		case 'u':
			imode |= UFLAG;
			break;
		case 'g':
			imode |= GFLAG;
			break;
		case 'k':
			imode |= KFLAG;
			break;
		case 's':
			nzflag = 1;
			break;
		default:
			return 0;
		}
	}
		
	if (name[0] == '/' || path == ((char *) 0) || *path == '\0')
		path = ":";
	while (*path) {
		for (cp = cpath; *cp = *path; cp++) {
			path++;
			if (*cp == ':')
				break;
		}
		if (cp != cpath)
			*cp++ = '/';
		*cp = '\0';
		strcat (cp, name);
		if (access (cpath, imode&07) == 0 &&
			fullck (cpath, imode, nzflag))
			return cpath;
	}

	return ((char *) 0);
}

	static
fullck (name, mode, nzflag)
	char *name;
	int mode;
	int nzflag;
{
	struct stat sbuf;
	int xor;

	if ((mode & 0177000) == 0 && nzflag == 0)	/* no special info wanted */
		return 1;
	stat (name, &sbuf);
	xor = (sbuf.st_mode ^ mode) & 077000;	/* see mknod(2) */
	if (mode & 0170000 == 0)
		xor &= ~070000;
	if ((mode & 07000) == 0)
		xor &= ~07000;
	if (xor)
		return 0;
	if (nzflag && sbuf.st_size == 0)
		return 0;
	return 1;
}

