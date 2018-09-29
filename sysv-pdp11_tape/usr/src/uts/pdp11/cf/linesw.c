/* @(#)linesw.c	1.1 */
#include "sys/conf.h"

/*
 * Line Discipline Switch Table
 */

extern nulldev();
extern ttopen(), ttclose(), ttread(), ttwrite(), ttioctl(), ttin(), ttout();

/* order:	open close read write ioctl rxint txint modemint */

struct linesw linesw[] = {
/*0*/	ttopen,		ttclose,	ttread,		ttwrite,
		ttioctl,	ttin,		ttout,		nulldev,

	0
};

/* number on entries in linesw */
int	linecnt = 1;

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
