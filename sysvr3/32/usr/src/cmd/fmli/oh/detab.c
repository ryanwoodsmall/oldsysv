/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */

#ident	"@(#)vm.oh:src/oh/detab.c	1.1"

#include <stdio.h>
#include "tsys.h"
#include "typetab.h"
#include "detabdefs.h"

struct odft_entry Detab[MAXODFT];
