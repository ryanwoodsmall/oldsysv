/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 *
 */

#ident	"@(#)vm.inc:src/inc/terror.h	1.1"

extern char	nil[];

#define warn(what, name)	_terror(0, what, name, __FILE__, __LINE__)
#define error(what, name)	_terror(TERR_LOG, what, name, __FILE__, __LINE__)
#define fatal(what, name)	_terror(TERR_LOG | TERR_EXIT, what, name, __FILE__, __LINE__)

#define TERR_CONT	0
#define TERR_LOG	1
#define TERR_EXIT	2

#define TERRLOG		"/tmp/TERRLOG"

/*
 * These values are indices into the What array in terrmess.c
 * If you want to add a new error, the procedure is as follows:
 *  add the message for it to the end of the What array.
 *  add a define for it to this group of defines.
 *  add one to the value of TS_NERRS in this file.
 */
#define NONE		0
#define NOFORK		0
#define NOMEM		0
#define NOPEN		1
#define BADARGS		2
#define MUNGED		3
#define MISSING		4
#define SWERR		5
#define NOEXEC		6
#define LINK		7
#define VALID		8

#define TS_NERRS	9
