/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/vtoc.d/position.c	1.1"
extern char *nodename;
#include"vtoc.h"
extern struct node *nd[];
extern int lastnode;
position() {
	int i,j;
	struct node *kptr;
/*
 *  position inserts node allocations and 
 *  relative position indices into the tree
 */
	for(i=0;i<=lastnode;i++)  {
		nd[i]->alw = allow(nd[i]); 
	}
	nd[0]->rx = nd[0]->alw;
	for(j=0;j <= lastnode;j++) {
		if(nd[j]->numkid != 0)
			for(i=0;(kptr = nd[j]->kid[i]) != NULL;i++)
				kptr->rx = pos(kptr);
	}
}
allow(parptr)
	struct node *parptr; {
	int i,n;
/*
 *  allow returns a space allowance for a parent and all it's kids
 *  If parent has no kids a 1 is returned, otherwise the sum
 *  of all kids' allowances is returned.
 */
	if (parptr->numkid == 0)
		return(1);
	else for(i=0,n=0;parptr->kid[i] != NULL;i++)
		n += allow(parptr->kid[i]);
	return (n);
}
pos(kptr)
	struct node *kptr; {
	struct node *parent,*sibl;
	int displ,i;
/*
 *  pos returns relative hor position of node pointed to by kptr.
 *  Rel hor position of kptr's parent must already be in tree
 *  pos = rx of par - allow of par + displ, where
 *  displ=sum of(2 * previous kid's allowances)+ kptr's own allowance
 */
	parent = kptr->par;
	for(displ=0,i=0;(sibl=parent->kid[i]) != kptr; i++)
		displ += 2*sibl->alw;
	displ += kptr->alw;
	return(parent->rx-(parent->alw) + displ);
}
