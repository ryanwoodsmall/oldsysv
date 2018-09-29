/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/gutil.d/bel.c	1.4"
#include	"stdio.h"

/*
	Program to issue bel character.
*/

main()
{
	putchar('\07');
	exit(0);
}
