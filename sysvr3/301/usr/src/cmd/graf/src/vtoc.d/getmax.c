/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/vtoc.d/getmax.c	1.1"
extern char *nodename;
#include	"vtoc.h"
extern int		lastnode;
extern struct node	*nd[];

getmax(level_deep,level_wide)
	int	*level_deep,*level_wide;
{
	int	i;
	int	max_rx,min_rx;
	int	max_level,min_level;

	max_rx=min_rx=nd[0]->rx;
	max_level=min_level=nd[0]->level;

	for	(i=1; i <= lastnode; i++){
		max_rx=MAX(max_rx,nd[i]->rx);
		min_rx=MIN(min_rx,nd[i]->rx);
		max_level=MAX(max_level,nd[i]->level);
	}

	*level_deep=max_level-min_level;
	*level_wide=max_rx-min_rx;
	return(0);
}
