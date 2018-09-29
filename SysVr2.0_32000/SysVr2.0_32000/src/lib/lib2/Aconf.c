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
static char	sccsid[] = "@(#)Aconf.c	1.5";

#include "stand.h"

int	_nullsys();
#ifdef vax
int	_gdstrategy(), _hmstrategy();
int	_gtstrategy(), _gtclose();
int	_tsstrategy(), _tsclose();
#endif
#ifdef pdp11
int	_gdstrategy(), _hmstrategy();
int	_htstrategy(), _htclose();
#endif
#ifdef ns32000
int	_dcopen(), _dcstrategy();
int	_tcopen(), _tcclose(), _tcstrategy();
#endif

struct devsw _devsw[] = {
#ifdef vax
/* 0: gd */	_gdstrategy,	_nullsys,	_nullsys,
/* 1: gt */	_gtstrategy,	_nullsys,	_gtclose,
/* 2: ts */	_tsstrategy,	_nullsys,	_tsclose,
#endif
#ifdef pdp11
/* 0: gd */	_gdstrategy,	_nullsys,	_nullsys,
/* 1: ht */	_htstrategy,	_nullsys,	_htclose,
#endif
#ifdef ns32000
/* 0: dc */	_dcstrategy,	_dcopen,	_nullsys,
/* 1: tc */	_tcstrategy,	_tcopen,	_tcclose,
#endif
};

struct dtab _dtab[NDEV];
struct iob _iobuf[NFILES];
struct mtab _mtab[NMOUNT];
int errno;

#ifdef vax
int	_devcnt = 3;
#endif
#ifdef pdp11
int	_devcnt = 2;
#endif
#ifdef ns32000
int	_devcnt = 2;
#endif
