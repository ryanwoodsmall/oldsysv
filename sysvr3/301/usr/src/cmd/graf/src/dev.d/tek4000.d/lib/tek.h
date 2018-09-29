/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/lib/tek.h	1.4"
#include "../../../../include/gsl.h"
#include "../../../../include/gpl.h"
#include "../../../../include/debug.h"
#define UNSET      -1
#define BOLDWIDTH 2.5

struct tekaddr {
	int yh,yl,xh,xl;
	int ch;
};

#include "../include/gedstructs.h"
