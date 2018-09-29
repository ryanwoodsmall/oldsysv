/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/td.d/devfinal.c	1.3"
/* <: t-5 d :> */
#include <stdio.h>
#include "td.h"
#include "dev.h"
#define SMALLFONT	printf("\033;")
devfinal()
{
/*
 *  devfinal resets the tek terminal after a display
 */
	fflush(stdout); sleep(1); /* wait for output to complete */
	devcursor(0,763);
	SMALLFONT;
	return;
}
