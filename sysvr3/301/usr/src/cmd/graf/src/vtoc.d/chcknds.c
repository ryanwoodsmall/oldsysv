/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/vtoc.d/chcknds.c	1.1"
extern char *nodename;
#include	"vtoc.h"

extern	struct node *nd[];
extern	int	lastnode;

/*
	chcknds will scan the nodes to find any inproper id's
or nodes below the root level.

*/

chcknds()
{
	int	seqno;		/* counter */
	int	return_code=0;

	nd[0]->seq=0;
	for	(seqno=1; seqno <= lastnode; seqno++){
		if(nd[seqno]->level <= nd[0]->level) {
			ERRPR0(Level below root level\, tree truncated.);
			lastnode=seqno-1;
			return_code=1;
			break;
		}
		if(nd[seqno]->level >nd[seqno-1]->level &&
			nd[seqno]->level != nd[seqno-1]->level+1) {
			ERRPR0(Node missing\, tree truncated.);
			lastnode=seqno-1;
			return_code=1;
			break;
		}
		nd[seqno]->seq=seqno;
	}
	return(return_code);
}
