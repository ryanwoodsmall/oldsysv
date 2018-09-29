/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm:oh/externoot.c	1.1"

#include "tsys.h"

char *
externoot(obj)
char *obj;
{
	char *extdir = "/info/OH/externals/";
	extern char *Oasys;
	static char fname[PATHSIZ];

	strcpy(fname, Oasys);
	strcat(fname, extdir);
	strcat(fname, obj);
	return(fname);
}

