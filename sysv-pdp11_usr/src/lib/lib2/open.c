/*	@(#)open.c	1.2	*/

# include "stand.h"

ino_t	_find();
char	*strchr(), *strrchr();

open(strx, how)
char *strx; {
	register char *cp1, *cp2;
	register struct iob *io;
	struct dtab *dp;
	struct mtab *mp;
	int fdesc;
	char c, str[NAMSIZ];
	ino_t n;

	if (strx == NULL || how < 0 || how > 2) {
		errno = EINVAL;
		return (-1);
	}

	for (fdesc = 0; fdesc < NFILES; fdesc++)
		if (_iobuf[fdesc].i_flgs == 0)
			goto gotfile;
	errno = EMFILE;
	return (-1);
gotfile:
	(io = &_iobuf[fdesc])->i_flgs |= F_ALLOC;

	_cond(strx, str);

	for (dp = &_dtab[0]; dp < &_dtab[NDEV]; dp++) {
#ifdef DEBUG
		printf("dt_name: %s, str: %s\n", dp->dt_name, str);
#endif
		if (dp->dt_name == 0 || strcmp(str, dp->dt_name) == 0)
			break;
	}

	if (dp->dt_name && dp < &_dtab[NDEV]) {
		io->i_ino.i_dev = dp->dt_devp - &_devsw[0];
		io->i_dp = dp;
		_devopen(io);
		io->i_flgs |= how+1;
		io->i_offset = 0;
		return (fdesc+3);
	}

	c = '\0';
	cp2 = strchr(str, '\0');

	for (;;) {
		for (mp = &_mtab[0]; mp < &_mtab[NMOUNT]; mp++) {
			if (mp->mt_name == 0)
				continue;
			if (strcmp(str, mp->mt_name) == 0)
				break;
		}
		if (mp != &_mtab[NMOUNT]) {
			dp = mp->mt_dp;
			goto gotname;
		}
		cp1 = strrchr(str, '/');
		*cp2 = c;
		if (cp1 == NULL)
			break;
		c = '/';
		*(cp2 = cp1) = '\0';
	}
	io->i_flgs = 0;
	errno = ENOENT;
	return (-1);
gotname:
	io->i_ino.i_dev = dp->dt_devp - &_devsw[0];
	io->i_dp = dp;
	_devopen(io);

	if ((n = _find(++cp2, io)) == 0) {
		io->i_flgs = 0;
		return (-1);
	}

	if (how != 0) {
		io->i_flgs = 0;
		errno = EACCES;
		return (-1);
	}

	_openi(n, io);
	io->i_offset = 0;
	io->i_cc = 0;
	io->i_flgs |= F_FILE | (how+1);
	return (fdesc+3);
}
