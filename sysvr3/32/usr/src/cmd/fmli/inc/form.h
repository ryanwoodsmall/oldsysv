/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 *
 */

#ident	"@(#)vm.inc:src/inc/form.h	1.3"

typedef struct {
	char *name;
	char *value;
	int frow;
	int fcol;
	int nrow;
	int ncol;
	int rows;
	int cols;
	int flags;
	char **ptr;
} formfield;

struct form {
	formfield (*display)();
	char *	  argptr;
	vt_id	  vid;
	int	  flags;
	int	  rows;
	int	  cols;
};

#define FORM_USED	1
#define FORM_DIRTY	2	/* contents of form changed */
#define FORM_ALLDIRTY	4	/* form has been reshaped or moved */

extern form_id		FORM_curid;
extern struct form	*FORM_array;
