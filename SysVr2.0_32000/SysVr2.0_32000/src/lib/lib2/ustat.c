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
/*	@(#)ustat.c	1.2	*/

# include "stand.h"
# include "ustat.h"
# include "sys/filsys.h"

ustat(device, buf)
register struct ustat *buf; {
	register struct mtab *mp;
	register i;
	int filep;
	struct filsys fs;

	for (mp = &_mtab[0]; mp < &_mtab[NMOUNT]; mp++)
		if (mp->mt_name && mp->mt_dp == &_dtab[device])
			break;

	if (mp == &_mtab[NMOUNT]) {
		errno = EINVAL;
		return (-1);
	}

	if ((filep = open(_dtab[device].dt_name, 0)) < 0)
		return (-1);
	
	lseek(filep, (long) BLKSIZ, 0);
	read(filep, (char *) &fs, sizeof(fs));
	buf->f_tfree = fs.s_tfree;
	buf->f_tinode = fs.s_tinode;
	for (i=0; i<6; i++) {
		buf->f_fname[i] = fs.s_fname[i];
		buf->f_fpack[i] = fs.s_fpack[i];
	}
	close(filep);
	return (0);
}
