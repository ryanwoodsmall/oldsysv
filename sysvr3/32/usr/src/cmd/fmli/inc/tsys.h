/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */

#ident	"@(#)fmli:inc/tsys.h	1.1"

#define YES	(1)
#define NO	(0)
#define TRUE	(1)
#define FALSE	(0)

#define STRSIZ	(256)
#define PATHSIZ	(128)

#define CTRL(X)	('X' & ' ' - 1)

#ifndef NOBOOL
typedef int	bool;
#endif

#define TERROR(sev, err) { fprintf(stderr, "FAILED IN %s, LINE %d\n", __FILE__, __LINE__); exit(R_ERR); }
#define Min(X, Y)	((X) > (Y) ? (Y) : (X))
#define Max(X, Y)	((X) > (Y) ? (X) : (Y))

#ifndef NDEBUG
extern int	_Debug;
#define _debug	(!_Debug) ? 0 : fprintf
#endif
