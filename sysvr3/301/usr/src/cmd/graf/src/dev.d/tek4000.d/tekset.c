/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/tekset.c	1.2"
#include	"stdio.h"
/*
	This program will reset the screen of a tektronix 4014-1.
*/

main()
{
	fprintf(stderr,"\033\014\033\073");
	exit(0);
}
