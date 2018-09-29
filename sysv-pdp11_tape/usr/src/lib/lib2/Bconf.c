/*	@(#)Bconf.c	1.2	*/

#include <stand.h>

int	_nullsys();
int	_gdstrategy();
int	_rkstrategy();
int	_tmstrategy(), _tmclose();

struct devsw _devsw[] = {
/* 0: rp */	_gdstrategy,	_nullsys,	_nullsys,
/* 1: rk */	_rkstrategy,	_nullsys,	_nullsys,
/* 2: tm */	_tmstrategy,	_nullsys,	_tmclose,
};

struct dtab _dtab[NDEV];
struct iob _iobuf[NFILES];
struct mtab _mtab[NMOUNT];
int errno;

int	_devcnt = 3;
