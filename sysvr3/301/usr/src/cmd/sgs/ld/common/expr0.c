/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ld:common/expr0.c	1.8"
#include "system.h"

#include <stdio.h>
#include "structs.h"
#include "extrns.h"
#include "sgsmacros.h"
#include "y.tab.h"
#include "ldmacros.h"
/*eject*/
ENODE *
buildtree(op,l,r)
int op;
ENODE *l,*r; 
{
 
/*
 * Build a node in the expression tree and
 * return a pointer to it
 */

	register ENODE *p;

	p = (ENODE *) myalloc(sizeof(ENODE));

	p->gnode.exop = op;
	p->gnode.exleft = l;
	p->gnode.exright = r;
	return(p);
}
/*eject*/
ENODE *
symnode(symp)
char *symp;
{

/*
 * Build an expression node for NAME or DOT
 */

	register ENODE *p;
	SYMENT sym;

	p = (ENODE *) myalloc(sizeof(ENODE));

	if( symp == NULL ) {		/* special DOT symbol */
		p->nnode.exop = DOT;
		p->nnode.exsymptr = 0;
		}
	else {
		p->nnode.exop = NAME;
		zero( (char *) &sym, SYMESZ );
#if FLEXNAMES
		if (strlen(symp) > 8) {
			sym.n_zeroes = 0L;
			sym.n_nptr = symp;
			}
		else
#endif
			copy(sym.n_name,symp,8);
		sym.n_type = T_INT;
		sym.n_sclass = C_EXT;
		p->nnode.exsymptr = (makesym(&sym, NULL))->smmyacid;
		}

	return(p);
}


/*
 *	Build an expression node for SIZEOF(name) or ADDR(name)
 */

ENODE *
sectnode( op, name )
	int op;
	char *name;
{
	register ENODE *p;

	p = (ENODE *) myalloc(sizeof(ENODE));

	p->pnnode.exop = op;
	copy( p->pnnode.symbol, name, 8 );

	return(p);
}
