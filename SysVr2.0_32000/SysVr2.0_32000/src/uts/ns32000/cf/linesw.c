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
/*	@(#)linesw.c	6.4	*/
#include "config.h"
#include <sys/conf.h>

/*
 * Line Discipline Switch Table
 */

extern nulldev();
extern ttopen(), ttclose(), ttread(), ttwrite(), ttioctl(), ttin(), ttout();
#ifdef SXT_0
extern sxtin(), sxtout(), sxtrwrite();
#endif
#ifdef XT_0
extern xtin(), xtout();
#endif

/* order:	open close read write ioctl rxint txint modemint */

struct linesw linesw[] = {
	ttopen,		ttclose,	ttread,		ttwrite,
		ttioctl,	ttin,		ttout,		nulldev

#ifdef SXT_0
	,nulldev,	nulldev,	nulldev,	sxtrwrite,
		nulldev,	sxtin,		sxtout,		nulldev
#endif
#ifdef XT_0
	,nulldev,	nulldev,	nulldev,	nulldev,
		nulldev,	xtin,		xtout,		nulldev
#endif
};

/* number on entries in linesw */
int	linecnt = sizeof(linesw) / sizeof(struct linesw);

/*
 * Virtual Terminal Switch Table
 */

#ifdef VT_VT100
extern	vt100input(), vt100output(), vt100ioctl();
#endif
#ifdef VT_HP45
extern	hp45input(), hp45output(), hp45ioctl();
#endif
struct termsw termsw[] = {
/*0*/	nulldev,	nulldev,	nulldev,	/* tty */
/*1*/
#ifdef VT_VT100
	vt100input,	vt100output,	vt100ioctl,	/* VT100 */
#else
	nulldev,	nulldev,	nulldev,
#endif
/*2*/
#ifdef VT_HP45
	hp45input,	hp45output,	hp45ioctl,	/* HP45 */
#else
	nulldev,	nulldev,	nulldev,
#endif
};

/* number of entries in termsw */
int termcnt = 3;
