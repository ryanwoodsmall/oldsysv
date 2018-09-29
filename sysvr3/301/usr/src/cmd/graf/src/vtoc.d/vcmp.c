/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/vtoc.d/vcmp.c	1.1"
extern char *nodename;
#include	"vtoc.h"
vcmp(nd1,nd2)
	struct node **nd1,**nd2;
{
	return(idcmp((*nd1)->id,(*nd2)->id));
}
