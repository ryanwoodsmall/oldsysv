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
/*	send9.c		Trap handling code	*/

#include	"send.h"

static char Sccsid[] = "@(#)send9.c	1.1";

struct	trap	*tchain;

rstrp(strp)
struct	trap	*strp;
{
	tchain = strp->str;
}

dotrp()
{
	if(tchain)
		longjmp(tchain->save);
}
