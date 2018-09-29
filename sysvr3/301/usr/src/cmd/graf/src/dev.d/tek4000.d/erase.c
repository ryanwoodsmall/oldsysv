/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/erase.c	1.3"
#include	"stdio.h"

/*
	This program will erase the screen of a tektronix 4010-1.
*/

main()
{
	printf("\033\014");
	fflush( stdout ); sleep(1);
	exit(0);
}
