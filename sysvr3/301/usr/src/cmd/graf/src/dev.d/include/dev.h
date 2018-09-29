/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/include/dev.h	1.2"
#include "debug.h"
#include "errpr.h"
#include "util.h"
#include "gsl.h"
#include "gpl.h"
struct device{
	struct area v;
	int htext;
	int harc;
	int hfill;
	int hbuff;
};
struct window{
	double utodrat;
	struct area w;
};
