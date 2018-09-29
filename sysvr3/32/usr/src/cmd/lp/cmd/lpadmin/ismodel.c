/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpadmin/ismodel.c	1.4"

#include "lp.h"
#include "lpadmin.h"

extern int		Access();

int			ismodel (name)
	char			*name;
{
	if (!name || !*name)
		return (0);

	return (Access(makepath(Lp_Model, name, (char *)0), 04) != -1);
}
