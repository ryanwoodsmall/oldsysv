/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/fs/next.c	1.2"

#include "fslib.h"

extern void		free();

/**
 ** next_dir() - GO TO NEXT DIRECTORY ENTRY UNDER PARENT DIRECTORY
 ** next_file() - GO TO NEXT FILE ENTRY UNDER PARENT DIRECTORY
 **/

static char		*next_x();

static int		is();

char			*next_dir (parent, lastdirp)
	char			*parent;
	long			*lastdirp;
{
	return (next_x(parent, lastdirp, S_IFDIR));
}

char			*next_file (parent, lastdirp)
	char			*parent;
	long			*lastdirp;
{
	return (next_x(parent, lastdirp, S_IFREG));
}

static char		*next_x (parent, lastdirp, what)
	char			*parent;
	long			*lastdirp;
	unsigned int		what;
{
	DIR			*dirp;
	register char		*ret = 0;
	struct dirent		*direntp;

	if (!(dirp = Opendir(parent)))
		return (0);

	if (*lastdirp != -1)
		Seekdir (dirp, *lastdirp);

	do
		direntp = Readdir(dirp);
	while (
		direntp
	     && (
			STREQU(direntp->d_name, ".")
		     || STREQU(direntp->d_name, "..")
		     || !is(parent, direntp->d_name, what)
		)
	);

	if (direntp)
	{
		if (!(ret = strdup(direntp->d_name)))
		    errno = ENOMEM;
		*lastdirp = Telldir(dirp);
	}
	else
	{
		errno = ENOENT;
		*lastdirp = -1;
	}

	Closedir (dirp);

	return (ret);
}

static int		is (parent, name, what)
	char			*parent,
				*name;
	unsigned int		what;
{
	char			*path;

	struct stat		statbuf;

	if (!(path = makepath(parent, name, (char *)0)))
		return (0);
	if (Stat(path, &statbuf) == -1) {
		free (path);
		return (0);
	}
	free (path);
	return ((statbuf.st_mode & S_IFMT) == what);
}
