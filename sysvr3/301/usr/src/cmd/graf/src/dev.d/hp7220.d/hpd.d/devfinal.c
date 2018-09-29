/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/hp7220.d/hpd.d/devfinal.c	1.2"
#include <stdio.h>
#include "hpd.h"
#include "dev.h"
devfinal()
{
/*
 *  devfinal restores pen and sets plotoff condition
 */
	PENSEL; sbn(0);
	PLOTOFF;
}
