/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/fs/s5rmdir.c	1.2"

#include "errno.h"

#define	RMDIR	"rmdir"

extern char		*malloc();

extern void		free();

extern int		strlen();

/**
 ** s5rmdir() - SIMULATE "rmdir()" ON ``s5'' FILESYSTEM
 **/

/*
 * This should work on ANY filesystem, but should be used only
 * when SVR3 is not available, to minimize the use of fork/exec.
 */

int			s5rmdir (path)
	register char		*path;
{
	register char		*cmd	= malloc(
		sizeof(RMDIR)-1 + 1 + strlen(path) + 1
	);

	register int		ret;


	if (!cmd) {
		errno = ENOMEM;
		return (-1);
	}

	(void)sprintf (cmd, "%s %s", RMDIR, path);
	ret = (system(cmd) < 0? -1 : 0);
	free (cmd);

	return (ret);
}
