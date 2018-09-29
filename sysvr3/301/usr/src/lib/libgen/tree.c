/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libgen:tree.c	1.2"
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 *
 *	The algorithms in this file are
 *	derived, in part, from Knuth Volume 3
 *
 * ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

#include <stdio.h>
#include "hash.h"

#define	TRUE		1
#define	FALSE		0

#define	LLINK(T)	T->L
#define	RLINK(T)	T->R
#define	LINK(A,T)	*(A==1?&(T->R):&(T->L))

#define	DATA(T)		T->data
#define	ROOT(H)		(H==0?default_root:H)
#define	TREE(H)		H->head
#define	COMPARE(A, B)	(theroot->compare)(A, B)

#define BALANCE(T)	T->balance_factor
#define	DELETED(T)	T->deleted

#define NUMNODES(H)     H->numnodes
#define	HEIGHT(H)	H->height

static	T_HEAD	*default_root, *theroot;
static	T_NODE	*p, *q, *r, *s, *t;
static	T_NODE	*mknode();
extern	float	*malloc();


T_HEAD	*
tcreate(root, compare)
T_HEAD	**root;
int	(*compare)();
{
	theroot = (T_HEAD *)malloc(sizeof(*theroot));
	TREE(theroot) = NULL;
	HEIGHT(theroot) = 0;
	NUMNODES(theroot) = 0;
		
	if (compare == NULL)
		return NULL;
	theroot->compare = compare;

	if (root == NULL) {
		if (default_root != NULL)
			return NULL;
		default_root = theroot;
	} else
		*root        = theroot;
	return theroot;
}


static	int
adjust_balance(data)
int	*data;
{
	int	i;

	/* adjust balance factors */
	i = COMPARE(data, DATA(s));
	if (i < 0)
		r = p = LLINK(s);
	else if (i > 0)
		r = p = RLINK(s);
	else return FALSE;

	for (i=COMPARE(data, DATA(p)); i != 0; i=COMPARE(data, DATA(p))) 
		if (i < 0) {
			BALANCE(p) = -1;
			p = LLINK(p);
		} else if (i > 0) {
			BALANCE(p) = 1;
			p = RLINK(p);
		} else
			return FALSE;
	return TRUE;
}

static	int
balancing_act(data)
int	*data;
{
	int	i, a;
	
	i = COMPARE(data, DATA(s));
	if (i < 0)
		a = -1;
	else if (i > 0)
		a = 1;
	else
		return FALSE;
	if (BALANCE(s) == 0) {
		/* The tree has grown higher */
		BALANCE(s) = a;
		HEIGHT(theroot)++;
	} else if (BALANCE(s) == -a) {
		/* The tree has become more balanced */
		BALANCE(s) = 0;
	} else if (BALANCE(s) == a) {

		/* The tree has gotten out of balance */
		if (BALANCE(r) == a) {

			/* single rotation */
			p = r;
			LINK(a, s) = LINK(-a, r);
			LINK(-a, r) = s;
			BALANCE(s) = BALANCE(r) = 0;
		} else if (BALANCE(r) == -a) {	

			/* double rotation */
			p = LINK(-a, r);
			LINK(-a, r) = LINK(a, p);
			LINK(a, p) = r;
			LINK(a, s) = LINK(-a, p);
			LINK(-a, p) = s;
			if (BALANCE(p) == a) {
				BALANCE(s) = -a;
				BALANCE(r) = 0;
			} else if (BALANCE(p) == 0) {
				BALANCE(s) = BALANCE(r) = 0;
			}else if (BALANCE(p) == -a) {
				BALANCE(s) = 0;
				BALANCE(r) = a;
			} else
				return FALSE;
			BALANCE(p) = 0;
		} else	
			return FALSE;
	
		/* finishing touch */
		if (t == NULL) { 
			/* then t~root */
			if (s == TREE(theroot))
				TREE(theroot) = p;
		} else
			if (s == RLINK(t))
				RLINK(t) = p;
			else
				LLINK(t) = p;
	} else
		return FALSE;
	return TRUE;
}


int
tinsert(root, data)
T_HEAD	*root;
int	*data;
{
	int	i;

	/* initialize */
	t = NULL;
	theroot = ROOT(root);
	if (theroot == NULL)
		return FALSE;
	s = p = TREE(theroot);

	/* check for empty tree */
	if (p == NULL) { 
		/* add first node to tree */
		/* set number of nodes in header */
		NUMNODES(theroot) = 1;
		HEIGHT(theroot) = 1;
		TREE(theroot) = mknode(data);
		return TRUE;
	}

	/* compare */
	for (i=COMPARE(data, DATA(p)); i != 0; i=COMPARE(data, DATA(p))) {
		if (i > 0) {
			/* move right */
			q = RLINK(p);
			if (q == NULL) {
				/* add new node */
				RLINK(p) = q = mknode(data);

				if (!adjust_balance(data)) {
					fprintf(stderr, "Error adjusting balance\n");
					return FALSE;
				}
				if (!balancing_act(data)) {
					fprintf(stderr, "Error balancing tree\n");
					return FALSE;
				}
				NUMNODES(ROOT(root))++;
				return TRUE;
			} else {
				/* do some book keeping, to determine probable
				   place for re-balancing, then go again */
				if (BALANCE(q) != 0) {
					t = p;
					s = q;
				}
				p = q;
			}
		} else {
			/* move left  */
			q = LLINK(p);
			if (q == NULL) { 
				/* add new node */
				LLINK(p) = q = mknode(data);

				if (!adjust_balance(data)) {
					fprintf(stderr, "Error adjusting balance\n");
					return FALSE;
				}
				if (!balancing_act(data)) {
					fprintf(stderr, "Error balancing tree\n");
					return FALSE;
				}
				NUMNODES(ROOT(root))++;
				return TRUE;
			} else {
				/* do some book keeping, to determine the
				   probable place for re-balancing	*/
				if (BALANCE(q) != 0) {
					t = p;
					s = q;
				}
				p = q;
			}
		} /* of comparison step */
	} /* of for loop */
	/* check to see if node was logically deleted */
	if (!DELETED(p))
		/* node already existed & was not logically deleted */
		return FALSE;

	/* move the data into the node */
	DATA(p) = data;
	DELETED(p) = FALSE;
	NUMNODES(ROOT(root))++;
	return TRUE;
}


int	*
tlookup(root, data)
T_HEAD	*root;
int	*data;
{
	int	i;

	/* initialize */
	if (ROOT(root) == NULL)
		return NULL;
	s = p = TREE(ROOT(root));
	if (s == NULL)
		return NULL;
	/* compare */
	for (i=COMPARE(data, DATA(p)); i != 0; i=COMPARE(data, DATA(p))) {
		if (i > 0) {
			/* move right */
			p = RLINK(p);
			if (p == NULL) 
				return NULL;
		} else {
			/* move left  */
			p = LLINK(p);
			if (p == NULL) 
				return NULL;
		} /* of comparison step */
	} /* of for loop */
	/* check for logical deletion of the node */
	if (DELETED(p))
		return NULL;
	return DATA(p);
}


int	
tdelete(root, data)
T_HEAD	*root;
int	*data;
{
	int	i;

	/* initialize */
	if (ROOT(root) == NULL)
		return FALSE;
	s = p = TREE(ROOT(root));
	if (s == NULL)
		return FALSE;

	/* compare */
	for (i=COMPARE(data, DATA(p)); i != 0; i=COMPARE(data, DATA(p))) {
		if (i > 0) {
			/* move right */
			p = RLINK(p);
			if (p == NULL) 
				return FALSE;
		} else {
			/* move left  */
			p = LLINK(p);
			if (p == NULL) 
				return FALSE;
		} /* of comparison step */
	} /* of for loop */
	/* check for logical deletion of the node */
	if (DELETED(p))
		return FALSE;
	/* delete the node */
	NUMNODES(ROOT(root))--;
	free (DATA(p));
	DELETED(p) = TRUE;
	return TRUE;
}


int
tscan(root, datalist, listsize)
T_HEAD	*root;
int	*datalist[];
int	listsize;
{
	if (ROOT(root) == NULL)
		return 0;
	return trscan(datalist, listsize, TREE(ROOT(root)));
}

static	int
trscan(datalist, listsize, node)
int	*datalist[];
int	listsize;
T_NODE	*node;
{
	int	added;

	if ((node == NULL) || (listsize <= 0))
		return 0;

	if (!DELETED(node)) {
		*datalist = DATA(node);
		added = 1;
	} else
		added = 0;

	added+=trscan(datalist+added, listsize-added, LLINK(node));
	added+=trscan(datalist+added, listsize-added, RLINK(node));
	return added;
}


static	T_NODE	*
mknode(data)
int	*data;
{
	T_NODE	*new_node;

	/* get the space for the new node */
	new_node = (T_NODE *)malloc(sizeof(*new_node));

	/* initialize the new node */
	DATA(new_node) = data;
	LLINK(new_node) = RLINK(new_node) = (T_NODE *)NULL;
	BALANCE(new_node) = 0;
	DELETED(new_node) = FALSE;
	
	return new_node;
}
int
tdestroy(root)
T_HEAD	*root;
{
	if (ROOT(root) != NULL) {
		trdestroy(TREE(ROOT(root)));
		if (root == NULL) {
			free(default_root);
			default_root = NULL;
		}
		return TRUE;
	}
	else
		return FALSE;
}
static	trdestroy(node)
T_NODE	*node;
{
	if (node != NULL) {
		trdestroy(LLINK(node));
		trdestroy(RLINK(node));
		free(node);
	}
}


static	int
tdump(root)
T_HEAD	*root;
{
	if (ROOT(root) != NULL) {
		trdump(TREE(ROOT(root)));
	}
}

static	trdump(node)
T_NODE	*node;
{
	if (node != NULL) {
		printf("data:%x\tdelete:%d\tbalance:%d\tllink:%x\trlink:%x\n",
			DATA(node), DELETED(node), BALANCE(node), LLINK(node), RLINK(node));
		trdump(LLINK(node));
		trdump(RLINK(node));
	}
}
