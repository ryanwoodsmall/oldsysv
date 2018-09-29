/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ld:common/expr1.c	1.8"
#include "system.h"

#include <stdio.h>
#include "structs.h"
#include "extrns.h"
#include "sgsmacros.h"
#include "y.tab.h"
#include "ldmacros.h"

static int expsect;
/*eject*/
ADDRESS
eval(p)
ENODE *p;
{

/*
 * Evaluate an expression node, returning the (long) value of the
 * expression.
 *
 * The extern variable "cur_dot" is used when processing exprs
 * inside of an output section specification, and is set to
 * -1L after allocation, when all sections have been relocated.
 */

	ADDRESS val1, val2;
	SYMTAB *symp;
	OUTSECT *s;
	ADDRESS findnext();

	switch (p->gnode.exop) {
	case INT:
		return( p->vnode.exvalue );
	case DOT:
		/*
		 * Assign to DOT, if not a misuse of the assignment
		 */
		if( p->nnode.exsymptr == 0L ) {
			if( cur_dot == -1L ) {
				lderror(1, lineno, curfilnm,
					"misuse of DOT symbol in assignment instruction");
				return (0L);
				}
			return(cur_dot);
			}
		else {
			/*
			 * Use of dot in assign to symbol
			 */
			symp = getsym(p->nnode.exsymptr);
			s = (OUTSECT *) symp->smscnptr;
			val1 = symp->sment.n_value + s->oshdr.s_paddr;
			if ( expsect == 0 )
				expsect = s->ossecnum;
			return (val1);
			}
	case NAME:
		symp = getsym(p->nnode.exsymptr);
		val1 = symp->smnewval;
		if (symp->sment.n_scnum > 0) {
			if(expsect == 0)
				expsect = symp->sment.n_scnum;
			else
				expsect = -1;
			}
		else if( symp->sment.n_scnum == 0 )
			lderror(1, lineno, curfilnm,
				"undefined symbol in expression");
		return (val1);
	case ALIGN:
		if( cur_dot == -1L ) {
			lderror(1, lineno, curfilnm,
				"ALIGN illegal in this context");
		return(0L);
		}
		val2 = eval(p->gnode.exleft);
		val1 = (cur_dot + val2 - 1) & (~(val2-1));
		return(val1);

	case PHY:
		/*
		 * If (cur_dot != -1L) here, then eval() was
		 * called from pboslist(), which is processing
		 * the output section list.  Moreover, we have
		 * an assignment to DOT, because eval() is not
		 * called otherwise.  The statement is
		 *	. = ... PHY(...) ...
		 * which is invalid.
		 */
		if( cur_dot != -1L ) {
			lderror(1, lineno, curfilnm,
				"illegal assignment of physical address to DOT");
			return (0L);
			}
		expsect = -1;
		switch( p->gnode.exleft->gnode.exop ) {
		case NAME:
			symp = getsym(p->gnode.exleft->nnode.exsymptr);
			val1 = symp->smnewval;
			if ( symp->smscnptr == NULL ) {
				switch ( symp->sment.n_scnum ) {
				case -1 :	/* absolute */
					lderror(1, lineno, curfilnm,
						"phy of absolute symbol %s is illegal",
						SYMNAME(symp->sment));
					break;
				case 0 :	/* undefined */
					lderror(1, lineno, curfilnm,
						"symbol %s is undefined",
						SYMNAME(symp->sment));
					break;
				}
				}
			return ( val1 );
		case DOT:
			if( p->gnode.exleft->nnode.exsymptr == 0L ) {
				lderror(1,lineno, curfilnm,
					"internal error: no symtab entry for DOT");
				return (0L);
				}
			symp = getsym(p->gnode.exleft->nnode.exsymptr);
			val1 = symp->sment.n_value;
			val1 += ((OUTSECT *) symp->smscnptr)->oshdr.s_paddr;

			return ( val1 );
		default:
			lderror(1, lineno, curfilnm,
				"operand of PHY must be a name");
			return(0L);
		}


	case UMINUS:	val1 = eval(p->gnode.exleft);
			return(-val1);
	case NOT:	val1 = eval(p->gnode.exleft);
			return(!val1);
	case BNOT:	val1 = eval(p->gnode.exleft);
			return(~val1);
	case AND:	val1 = eval(p->gnode.exleft);
			val2 = eval(p->gnode.exright);
			return(val1 & val2);
	case ANDAND:	val1 = eval(p->gnode.exleft);
			val2 = eval(p->gnode.exright);
			return(val1 && val2);
	case DIV:	val1 = eval(p->gnode.exleft);
			val2 = eval(p->gnode.exright);
			return(val1 / val2);
	case EQEQ:	val1 = eval(p->gnode.exleft);
			val2 = eval(p->gnode.exright);
			return(val1 == val2);
	case GE:	val1 = eval(p->gnode.exleft);
			val2 = eval(p->gnode.exright);
			return(val1 >= val2);
	case GT:	val1 = eval(p->gnode.exleft);
			val2 = eval(p->gnode.exright);
			return(val1 > val2);
	case LE:	val1 = eval(p->gnode.exleft);
			val2 = eval(p->gnode.exright);
			return(val1 <= val2);
	case LSHIFT:	val1 = eval(p->gnode.exleft);
			val2 = eval(p->gnode.exright);
			return(val1 << val2);
	case LT:	val1 = eval(p->gnode.exleft);
			val2 = eval(p->gnode.exright);
			return(val1 < val2);
	case MINUS:	val1 = eval(p->gnode.exleft);
			val2 = eval(p->gnode.exright);
			return(val1 - val2);
	case MULT:	val1 = eval(p->gnode.exleft);
			val2 = eval(p->gnode.exright);
			return(val1 * val2);
	case NE:	val1 = eval(p->gnode.exleft);
			val2 = eval(p->gnode.exright);
			return(val1 != val2);
	case OR:	val1 = eval(p->gnode.exleft);
			val2 = eval(p->gnode.exright);
			return(val1 | val2);
	case OROR:	val1 = eval(p->gnode.exleft);
			val2 = eval(p->gnode.exright);
			return(val1 || val2);
	case PC:	val1 = eval(p->gnode.exleft);
			val2 = eval(p->gnode.exright);
			return(val1 % val2);
	case PLUS:	val1 = eval(p->gnode.exleft);
			val2 = eval(p->gnode.exright);
			return(val1 + val2);
	case RSHIFT:	val1 = eval(p->gnode.exleft);
			val2 = eval(p->gnode.exright);
			return(val1 >> val2);
	case SIZEOF:
	case ADDR:
		if (cur_dot != -1L)
		{
			lderror(1, lineno, curfilnm,
				"SIZEOF/ADDR illegal in this context" );
			return( 0L );
		}
		if ((s = fndoutsec( p->pnnode.symbol )) == NULL)
		{
			lderror(1, lineno, curfilnm,
				"output section %.8s not found",
				p->pnnode.symbol );
			return( 0L );
		}

		if (p->pnnode.exop == SIZEOF)
			return( s->oshdr.s_size );
		else
		{
			if (s->oshdr.s_paddr == -1L)
			{
				lderror( 1, lineno, curfilnm,
					"section %.8s not yet allocated",
					p->pnnode.symbol );
				return( 0L );
			}
			return( s->oshdr.s_paddr );
		}

	case NEXT:
		if (cur_dot != -1L)
		{
			lderror(1, lineno, curfilnm,
				"NEXT illegal in this context" );
			return( 0L );
		}
		return( findnext( (ADDRESS) p->vnode.exvalue ));

	default:
		lderror(1, lineno, curfilnm, "illegal operator in expression");
		return(0L);
	}
}
/*eject*/
pexlist()
{

/*
 * After allocation, process the "expression list"
 * to define any user-specified symbols.
 */

	cur_dot = -1L;
	curexp = (ACTITEM *) explist.head;

	while( curexp ) {
		lineno = curexp->evexpr.aiinlnno;
		curfilnm = curexp->evexpr.aiinflnm;
		pexitem();
		clrexp( curexp );
		curexp = curexp->evexpr.ainext;
		}
}
/*eject*/
pexitem()
{

/*
 * Process one assignment instruction
 */

	long val;
	register SYMTAB *sp;
	register OUTSECT *oscn;

	expsect = 0;
	val = eval(curexp->evexpr.aiexptr->gnode.exright);
	sp = getsym(curexp->evexpr.aiexptr->gnode.exleft->nnode.exsymptr);

/*
 * If the section number of the symbol we
 * are assigning a value is not zero ( = undefined),
 * the user is redefining the value of the symbol.
 */

	if( sp->sment.n_scnum )
		if( sp->smscnptr )
			lderror(0, curexp->evexpr.aiinlnno, curexp->evexpr.aiinflnm,
				"symbol %s from file %s being redefined",
				SYMNAME(sp->sment), sp->smscnptr->isfilptr->flname);
		else
			lderror(0, curexp->evexpr.aiinlnno, curexp->evexpr.aiinflnm,
				"absolute symbol %s being redefined",
				SYMNAME(sp->sment));

	if( expsect == 0 )
		sp->sment.n_scnum = -1;		/* absolute symbol */
	else
		sp->sment.n_scnum = expsect;

	if( (oscn = findoscn(sp->sment.n_scnum)) != NULL )
		sp->smnewvirt = val - oscn->oshdr.s_paddr + oscn->oshdr.s_vaddr;
	else
		sp->smnewvirt = val;
	sp->smnewval = val;
	sp->sment.n_sclass = C_EXT;

}
/*eject*/
clrexp(a)
ACTITEM *a;
{

/*
 * Free up all memory assigned to the assignment instruction pointed
 * to by "a"
 */
	ENODE *eptr;

	eptr = a->evexpr.aiexptr;

	switch( (unsigned) eptr->gnode.exop ) {
	case INT:
	case DOT:
	case NAME:
	case ADDR:
	case SIZEOF:
	case NEXT:
		break;
	default:
		if (eptr->gnode.exleft)
			clrlimb(eptr->gnode.exleft);
		if (eptr->gnode.exright)
			clrlimb(eptr->gnode.exright);
		break;
	}

	free( eptr );
	free( a );
}




clrlimb(eptr)
ENODE *eptr;
{

/*
 * Free up a non-root node of an expression tree
 */

	switch( (unsigned) eptr->gnode.exop ) {
	case INT:
	case DOT:
	case NAME:
	case SIZEOF:
	case ADDR:
	case NEXT:
		break;

	default:
		if (eptr->gnode.exleft)
			clrlimb(eptr->gnode.exleft);
		if (eptr->gnode.exright)
			clrlimb(eptr->gnode.exright);
		break;
	}

	free( eptr );
}



ADDRESS
findnext( boundary )
	ADDRESS boundary;
{
	ANODE *sap;
	ADDRESS next_addr;

	next_addr = boundary;
	for (sap = (ANODE *) avlist.head; sap; sap = sap->adnext)
	{
		if (sap->adtype != ADAVAIL)
			continue;
		while (sap->adpaddr > next_addr)
			next_addr += boundary;

		if (sap->adpaddr + (ADDRESS)sap->adsize > next_addr)
			break;
	}

	if (sap == NULL)
		lderror( 2, lineno, curfilnm,
			"boundary %d not available in configured memory",
			boundary );
	else
		return (next_addr);
}
