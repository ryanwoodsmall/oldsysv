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
/*	@(#)lseek.c	1.1	*/

# include <stand.h>

off_t
lseek(fdesc, off, sbase)
int fdesc;
off_t off;
int sbase; {
	register struct iob *io;

	fdesc -= 3;
	if (fdesc < 0
	    || fdesc >= NFILES
	    || ((io = &_iobuf[fdesc])->i_flgs&F_ALLOC) == 0) {
		errno = EBADF;
		return (-1);
	}

	if (sbase == 1)
		off += io->i_offset;
	else if (sbase == 2)
		off += io->i_ino.i_size;
	else if (sbase != 0) {
		errno = EINVAL;
		return (-1);
	}
	if (off < 0) {
		errno = EINVAL;
		return (-1);
	}
	io->i_offset = off;
	io->i_cc = 0;
	return (off);
}
