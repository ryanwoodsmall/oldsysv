/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/vtoc.d/setkids.c	1.1"
extern char *nodename;
/* <:t-5 d:> */
#include "vtoc.h"
extern struct node *nd[];
extern int lastnode;

setkids(){
	struct node *sptr,*tmp[MAXNODE-1];
	int i,n,j;
/*
 *  the following section sequentially examines structure elements
 *  associating parents with kids.  [Kid = Level of Parent + 1]
 */
  	for(n=0,nd[n]->par=nd[n]; n <= lastnode; n++) {
		sptr=nd[n+1];
		for	(i=0,j=1; n+j <= lastnode
			 && sptr->level > nd[n]->level; sptr=nd[n+(++j)]){
  			if (sptr->level == (nd[n]->level)+1) {
  				tmp[i] = sptr;	/*  input ptrs to kids  */
				sptr->par = nd[n];	/*  input ptr to parent  */
  				i++;
  			}
		}
  		nd[n]->numkid = i;
		if	((nd[n]->kid=(struct node **) calloc(i+1,sizeof(*tmp))) == NULL){
			ERRPR1(No room for %s\'s children,nd[n]->id);
			return(FAIL);
		}
		else	;
		for	(j=0; j < i; j++)
			nd[n]->kid[j]=tmp[j];
		nd[n]->kid[j]=NULL;
	}
	return(SUCCESS);
}
