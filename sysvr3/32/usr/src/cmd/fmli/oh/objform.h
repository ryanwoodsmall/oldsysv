/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.oh:src/oh/objform.h	1.2"

typedef struct {
	int flags;
	int curpage;
	int lastpage;
	int curfield;
	char **holdptrs;
	char **mulvals;
	struct fm_mn fm_mn;
	int *visible;
	int *slks;
} forminfo;
