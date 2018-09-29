#ifndef vax
/*	@(#)tm.c	1.1	*/

/*
 * TM tape driver
 */

# include <stand.h>

struct device {
	int	tmer;
	int	tmcs;
	int	tmbc;
	caddr_t	tmba;
	int	tmdb;
	int	tmrd;
};

# define TMADDR	((struct device *) 0172520)
# define NUNIT	4

static daddr_t	_eof[NUNIT];

# define GO	01
# define RCOM	02
# define WCOM	04
# define WEOF	06
# define SFORW	010
# define SREV	012
# define WIRG	014
# define REW	016
# define FUNC	016
# define CRDY	0200
# define DENS	060000		/* 9-channel */

# define TUR	01
# define SELR	0100
# define EOF	040000

# define GSD	010000

static
_tcommand (io, com)
register struct iob *io;
register com; {
	register unit;

	unit = io->i_dp->dt_unit & 03;
	TMADDR->tmcs = (unit << 8) | (segflag << 4) | DENS | com | GO;
	while ((TMADDR->tmcs & CRDY) == 0);
	if (TMADDR->tmcs < 0) {			/* error bit */
		while (TMADDR->tmrd & GSD);	/* wait for gap shutdown */
		if (TMADDR->tmer & EOF) {
			_eof[unit]++;
			return (0);
		} else
			return (-1);
	}
	return (io->i_cc + TMADDR->tmbc);
}

_tmclose (io)
register struct iob *io; {
	register flag, unit;

	flag = io->i_flgs;
	unit = io->i_dp->dt_unit & 03;
	TMADDR->tmbc = 0;
	if (flag & F_WRITE) {
		_tcommand (io, WEOF);
		_tcommand (io, WEOF);
	}
	if ((io->i_dp->dt_unit & 04) == 0)
		_tcommand (io, REW);
	else if (flag & F_WRITE)
		_tcommand (io, SREV);
	else if (!_eof[unit])
		_tcommand (io, SFORW);
	_eof[unit] = 0;
}

_tmstrategy (io, func)
register struct iob *io; {
	register com, unit;
	int errcnt = 0;
	int ret;

	unit = io->i_dp->dt_unit & 03;
	TMADDR->tmcs = (unit << 8);
	if (func == READ)
		com = RCOM;
	else if (func == WRITE)
		com = WCOM;
	else
		com = func;
	if ((TMADDR->tmer & SELR) == 0) {
		_prs ("TU10 unit not ready\n");
		while ((TMADDR->tmer & SELR) == 0);
	}
	do {
		TMADDR->tmbc = -io->i_cc;
		TMADDR->tmba = io->i_ma;
		_eof[unit] = 0;
		while ((TMADDR->tmer & TUR) == 0);
		if ((ret = _tcommand (io, com)) >= 0)
			return (ret);
		TMADDR->tmbc = -1;
		while ((TMADDR->tmer & TUR) == 0);
		_tcommand (io, SREV);
		if (com == WCOM)
			com = WIRG;
	} while (++errcnt < 10);
	errno = EIO;
	return (-1);
}
#endif
