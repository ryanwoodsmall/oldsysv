/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.sys:src/sys/exit.c	1.2"

#include	<stdio.h>
#include	"wish.h"
#include	"var_arrays.h"

char	*_tmp_ptr;
char	*Home;
char	*Filecabinet;
char	*Wastebasket;
char	*Filesys;
char	*Oasys;
char	*Progname;
char	**Remove;
int	(**Onexit)();
char	nil[] = "";
int	_Debug;

void
exit(n)
int	n;
{
	register int	i;
	int	lcv;

	lcv = array_len(Onexit);
	for (i = 0; i < lcv; i++)
		(*Onexit[i])(n);
	lcv = array_len(Remove);
	for (i = 0; i < lcv; i++)
		unlink(Remove[i]);
	_cleanup();
	_exit(n);
}
