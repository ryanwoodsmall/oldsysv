/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/vtoc.d/squish.c	1.1"
extern char *nodename;
#include "vtoc.h"
#define MAXLEVD 15
#define MAXLEVW 25
#define RELLEV(sptr)	sptr->level - nd[0]->level - 1
#define CENTER(nptr)		nptr->rx = ( (nptr->kid[0])->rx + \
		(((nptr->kid[(nptr->numkid)-1])->rx - (nptr->kid[0])->rx)/2) )
#define RIGHTKID(nptr)		nptr = nptr->kid[nptr->numkid-1]
#define LEFTKID(nptr)		nptr = nptr->kid[0]
extern struct node *nd[];
extern int lastnode;
int **adjacent,lw;
struct node *nextcompare(), *neighbor();

squish(level_deep,level_wide)
	int level_deep,level_wide;
{
	int i,j,*lptr,k,l,m;
/*
 *  squish resets rx (relative horizontal) values in the tree
 *  to compact the VTOC horizontally
 */
	lw=level_wide;
	if	((adjacent=(int **) calloc(level_deep+1,sizeof(*adjacent))) == NULL)
		exit(1);
	else	;
	for	(i=0; i <= level_deep; i++)
		if	((adjacent[i]=(int *) calloc(level_wide+2,sizeof(int))) == NULL)
			exit(1);
		else	;
	for(k=1;k <= lastnode;k++) {
		adjacent[RELLEV(nd[k])][nd[k]->rx-1]=nd[k]->seq;
	}

	for(l = lastnode;l >= 0; l--) {
		if(nd[l]->numkid >=2) {
			CENTER(nd[l]);
			for(i=0;(nd[l]->kid[i])->rx <= nd[l]->rx &&
				i < nd[l]->numkid; i++);
			for(j=i;j>0;j--) {
				squishpair(nd[l],j-1);
			}
			for(i--; i>=0 && i< (nd[l]->numkid)-1;i++){
				squishpair(nd[l],i);
			}
			CENTER(nd[l]);
		}
		else {
			if(nd[l]->numkid == 1) nd[l]->rx = (nd[l]->kid[0])->rx;
		}
	}
}
squishpair(sptr,i)
	struct node *sptr;
	int i;{
	int displ,m;
/*
 *  squishpair determines if there is space between two kids of
 *  sptr:  the ith kid and the i+1 kid.  One or both of the
 *  subtrees are moved toward the center, or parent's RX position.
 */
	if(displ = brdispl(sptr->kid[i],sptr->kid[i+1])) {
		for	(m=0; nd[m] != sptr->kid[i] && m <= lastnode; m++)
			;
		if(((sptr->kid[i+1])->rx ) <= sptr->rx) 	{
					/* rt kid left of, or at center  */
			brmv('r',m,displ);
		}
		else  {
			for	(m=0;nd[m] != sptr->kid[i+1] && m <= lastnode; m++)
				;
			brmv('l',m,displ);
		}
	}
}
brdispl(kptr1,kptr2)
	struct node *kptr1,*kptr2; {
	int displ,term1,term2;
	struct node *root,*ptr1,*ptr2;
/*
 *  brdispl determines displacement between two adjacent subtrees
 */
	root = kptr1->par;
	if( (displ = (kptr2->rx - kptr1->rx)) <= 2 ) return(0);
	for(term1=term2=0;;) {
		if(!term1 && !term2) {
			if((ptr1 = nextcompare('l',kptr1,root)) == NULL)
				term1 = 1;
			if((ptr2 = nextcompare('r',kptr2,root)) == NULL)
				term2 = 1;
			if(term1 && term2) break;
			if(term1 || term2) continue;
			kptr1 = ptr1;
			kptr2 = ptr2;
		}
		if(term1 && term2) break;
		if(term1) {			/* subtree 1 has terminated  */
			if((kptr2 = nextcompare('r',kptr2,root)) == NULL) {
				term2 = 1;
				break;
			}
			if((kptr1 = neighbor('l',kptr2)) == NULL) {
				term2 = 1;
				break;
			}
		}
		if(term2) {			/*  subtree 2 has terminated  */
			if((kptr1 = nextcompare('l',kptr1,root)) == NULL ) {
				term1 = 1;
				break;
			}
			if((kptr2 = neighbor('r',kptr1)) == NULL){
				term1 = 1;
				break;
			}
		}
		displ = MIN(displ,(kptr2->rx-kptr1->rx));
		if(displ <= 2) return(0);
	}
	return(displ-2);
}
struct node
*nextcompare(direction,nptr,root)
	char direction;
	struct node *root,*nptr;{
/*
 *  nextcompare returns next level compare node for non-terminated
 *  subtrees.  If tree terminates on given level a NULL pointer 
 *  is returned.
 */
	if(nptr->numkid == 0) {
		while ((nptr = neighbor(direction,nptr)) 
		!= NULL && insubtree(nptr,root)) {
			if(nptr->numkid > 0) break;
		}
		if(nptr == NULL || nptr->numkid == 0) return(NULL);
	}
	if(direction == 'l') return(RIGHTKID(nptr));
	else return(LEFTKID(nptr));
}
struct node
*neighbor(direction,nptr)
char direction;
struct node *nptr;
{	int *beginptr,*endptr,i,j;
/*
 *  neighbor returns a pointer to right or left neighbor 
 *  using adjacent matrix values.  if neighbor is  on right or 
 *  left borders a NULL pointer is returned.
 */
	for(i=RELLEV(nptr),j=0;adjacent[i][j]!=nptr->seq && j<=lw;j++);
	if(j>lw) return(NULL);
	if(direction=='r'){
		for(j++;adjacent[i][j]==0 && j<=lw;j++);
		if(j>lw)return(NULL);
		else return(nd[adjacent[i][j]]);
	}
	if(direction=='l'){
		for(j--;adjacent[i][j]==0 && j>=0;j--);
		if(j<0)return(NULL);
		else return(nd[adjacent[i][j]]);
	}
}

insubtree (nptr,root)
	struct node *nptr, *root; {
	int dif;
/*
 *  insubtree returns 1 if node pointed to by nptr is
 *  part of the subreee defined by root.  else 0 is returned
 */
	for(dif = nptr->level - root->level; dif >0; dif--) {
		nptr = nptr->par;
	}
	if (nptr == root) return(1);
	else return (0);
}
brmv(direction,index,displ)
	char direction;
	int displ,index; {
	int endlev;
/*
 *  brmv changes RX values to move the branch
 */
	endlev = nd[index]->level;
	if (direction == 'r') nd[index]->rx += displ;
	if (direction == 'l') nd[index]->rx -= displ;
	for(index++; nd[index]->level > endlev && index <= lastnode; index++) {
		if (direction == 'r') nd[index]->rx += displ;
		if (direction == 'l') nd[index]->rx -= displ;
	}
	return;
}
