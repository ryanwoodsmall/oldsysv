/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/include/tek.h	1.1"
#include "gsl.h"
#include "gpl.h"
#include "debug.h"
#define UNSET      -1
#define BOLDWIDTH 2.5

struct tekaddr {
	int yh,yl,xh,xl;
	int ch;
};

#include "../include/gedstructs.h"
