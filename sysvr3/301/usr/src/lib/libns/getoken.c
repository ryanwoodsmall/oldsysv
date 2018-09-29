/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libns:getoken.c	1.3"

#include "nserve.h"
#include "sys/cirmgr.h"

int
getoken(s)
struct token *s;
{
	char *sp;

	s->t_id = 0;
	sp = (char *)&s->t_uname[0];
	return(netname(sp));
}
