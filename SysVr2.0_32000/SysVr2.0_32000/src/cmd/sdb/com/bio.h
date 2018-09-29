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
	/* @(#) bio.h: 1.6 5/17/83 */

#define BRSIZ	512
struct brbuf {
	int	nl, nr;
	char	*next;
	char	b[BRSIZ];
	int	fd;
};
long lseek();
