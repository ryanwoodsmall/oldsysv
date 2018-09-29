/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/vtoc.d/levcal.c	1.1"
extern char *nodename;
#include "vtoc.h"
levcal(id)
	char *id; {
	int groups;
/*
 *  levcal computes level from the number of
 *  digit-groups in the id.
 */
	if(*id == '0') return(0);
	for(groups = 0; *id != '\0';groups++) {
	while(isdigit(*id)) id++;
	if	(*id != '\0')
		id++;
	}
	return(groups);
}

