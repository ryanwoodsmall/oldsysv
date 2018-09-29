/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/hp7220.d/hpd.d/newcolor.c	1.2"
#include <stdio.h>
#include "dev.h"
#include "hpd.h"
extern int curcol;

newcolor(col)
int col;
{
	if(col < 1 || col > 4){PENSEL; sbn(1);}
	else {PENSEL; sbn(col);}
	curcol = col;
	return(SUCCESS);
}
