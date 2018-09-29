/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:common/instab.c	1.4"
#include "systems.h"
#include "symbols.h"
#include <instab.h>
#include <parse.h>
instr instab[] = {
#include "ops.out"
0
};
