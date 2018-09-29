/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident "@(#)lp:destlist.c	3.2"
/* routines for manipulating destlist structures */

#include	"lp.h"
#include	"lpsched.h"


/* newdl -- returns pointer to new, empty destlist structure */

struct destlist *
newdl()
{
	struct destlist *dl;

	if((dl = (struct destlist *) malloc(sizeof(struct destlist))) == NULL)
		fatal(CORMSG, 1);

	dl->dl_dest = NULL;
	dl->dl_next = dl->dl_prev = dl;

	return(dl);
}

/* insertl(d1, d2) -- inserts d2 at the end of d1's class list */

insertl(d1, d2)
struct dest *d1, *d2;
{
	struct destlist *new, *head;

	if((new = (struct destlist *) malloc(sizeof(struct destlist))) == NULL)
		fatal(CORMSG, 1);

	head = d1->d_class;
	new->dl_dest = d2;
	new->dl_next = head;
	new->dl_prev = head->dl_prev;
	head->dl_prev = new;
	(new->dl_prev)->dl_next = new;

}
