/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:include/fs.h	1.3"
/* EMACS_MODES: !fill, lnumb, !overwrite */

#include "dirent.h"

#define Opendir		opendir
#define Telldir		telldir
#define Seekdir		seekdir
#define	Rewinddir(dirp)	Seekdir(dirp, 0L)
#define Closedir	closedir

extern struct dirent	*Readdir();

extern int		Mkdir(),
			Rmdir();

extern char		*next_dir(),
			*next_file();
