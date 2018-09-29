static char	sccsid[] = "@(#)Aconf.c	1.5";

#include "stand.h"

int	_nullsys();
int	_gdstrategy();
int	_hmstrategy();
#ifdef vax
int	_gtstrategy(), _gtclose();
int	_tsstrategy(), _tsclose();
#else
int	_htstrategy(), _htclose();
#endif

struct devsw _devsw[] = {
/* 0: gd */	_gdstrategy,	_nullsys,	_nullsys,
#ifdef vax
/* 1: gt */	_gtstrategy,	_nullsys,	_gtclose,
/* 2: ts */	_tsstrategy,	_nullsys,	_tsclose,
#else
/* 1: ht */	_htstrategy,	_nullsys,	_htclose,
#endif
};

struct dtab _dtab[NDEV];
struct iob _iobuf[NFILES];
struct mtab _mtab[NMOUNT];
int errno;

#ifdef vax
int	_devcnt = 3;
#else
int	_devcnt = 2;
#endif
