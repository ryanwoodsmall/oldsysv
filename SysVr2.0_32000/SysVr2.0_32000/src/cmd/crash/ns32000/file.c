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
/* @(#)file.c	1.5 */
#include	"crash.h"
#include	"sys/file.h"
#include	"sys/inode.h"

prfile(c, all)
	register  int  c;
	int	all;
{
	struct	file	fbuf;

	if(c == -1)
		return;
	if(c >= (int)v.ve_file) {
		printf("%4d  out of range\n", c);
		return;
	}

	if(readmem(&fbuf, (long)SYM_VALUE(File)+c*sizeof(fbuf), sizeof fbuf) != sizeof fbuf) {
		printf(" %4d read error on file table\n", c);
		return;
	}
	if(!all && fbuf.f_count == 0)
		return;
	printf(fbuf.f_inode ? "%4d  %3d   %3d  " : "%4d  %3d   -   ",
		 c, fbuf.f_count, ((unsigned)fbuf.f_inode - Inode->n_value)
		/ sizeof (struct inode) );
	printf("%s%s%s%s\n", fbuf.f_flag & FREAD ? " read" : "",
			     fbuf.f_flag & FWRITE ? " write" : "",
			     fbuf.f_flag & FAPPEND ? " append" : "",
			     fbuf.f_flag & FNDELAY ? " ndelay" : "");
}
