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
/*	@(#)umount.c	1.1	*/

# include <stand.h>

umount(dn)
char *dn; {
	register struct mtab *mp;
	char dname[NAMSIZ];
	extern free();

	_cond(dn, dname);

	for (mp = &_mtab[0]; mp < &_mtab[NMOUNT]; mp++) {
		if (mp->mt_name == 0)
			continue;
		if (strcmp(dname, mp->mt_dp->dt_name) == 0) {
			free(mp->mt_name);
			mp->mt_name = 0;
			return (0);
		}
	}
	errno = EINVAL;
	return (-1);
}
