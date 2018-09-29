/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcc2:m32/optdir.c	1.8"
/* optdir.c
**
**	Optimization Director
**
**	The routines in this file monitor the information in
**	the machine independent portion of the compiler
**	and write comments in the assembly output informing
**      the machine dependent portion of the optimizer
**	that certain optimizations are possible.
**
*/

# include "mfile1.h"
# include "storclass.h"

extern int xdebug;

#ifdef IMPREGAL

int sizeopt = 0;	/* signal for size optimization.  set in local.c main() */

/* Register allocation
** Routines ra* direct the 'register allocation' optimization.
** The optimization attempts to assign selected quantities,
** i.e.,
** 	local variables that
**		- are simple (are not related to arrays, structures, 
**		  or unions)
**		- do not have their address taken explicitly with '&'
**	incoming arguments that
**		- are simple
**		- are in a procedure where no argument has its address taken
**		  expilitly with an '&'
** to available scratch or user registers.
**  
** An estimator is computed for each qualifying quantity by the formulas:
**
**   	local variables
**		RAPOEST * (FORWEIGHT or WHWEIGHT)**(loop depth)
**		-----------------------------------------------
**		    (BRWEIGHT)**(branch depth)
**
**	arguments
**		RAPOEST * (FORWEIGHT or WHWEIGHT)**(loop depth)
**		----------------------------------------------- - 9
**		    (BRWEIGHT)**(branch depth)
**	
** where '**' denotes exponentiation.
** The quantities are ordered by decreasing estimator and up to HIGHSZ
** are printed in the form
**	#REGAL	<estimator> <storage class> <ident> <length>
**
*/

#define RAPAYOFF	3	/* operand payoff estimator in cycles */
#define FORWEIGHT	10	/* weight to emphasize code in 'for' loops */
#define WHWEIGHT	5	/* weight to emphasize code in 'while' and
				** 'do-while' loops */
#define BRWEIGHT	10	/* weight to de-emphasize code in branches */
#define HIGHSZ		20	/* size of table for sorting estimators */

int	estimtab[SYMTSZ];	/* payoff estimator for each variable, 
				** set to -1 if address of variable is taken */
NODE 	*lastp;			/* pointer to last NAME node processed */
int	lastid;			/* index to symbol table for last NAME node */
int	dblflg = 0;		/* flag indicating presence of doubles */

struct hinode { /* ordered table of variables with highest estimators */
	int 	hiestim;		/* estimator of cycle payoff */
	int	hiscl;			/* scl of quant to put in reg */
	int	hiident;		/* identification of quantity:
					 * 	local		offset 
					 *	argument	offset*/
	int	hilen;			/* length in bytes of quantity */
} high[HIGHSZ];

/* table status */

#define	EMPTY	-2
#define	ADDRSD	-1

void
rainit()

{
	register int *p;
	struct hinode *hp;

	/* initialize estimator table */
	for( p = estimtab; p < estimtab + SYMTSZ; p++ ) *p = EMPTY;

	/* initialize hi table */
	for( hp = high; hp < high + HIGHSZ; hp++ ) hp->hiestim = EMPTY;

	/* clear double float */
	dblflg = 0;
}

void
radbl()
{
	dblflg = 1;
#ifndef NODBG
	if (xdebug > 1 && dblflg)
		printf( "radbl() sets dblflg\n");
#endif
}

void
raname(p)
NODE *p;

{
	int type, class;
	int old;

	/* check for simpleness */
	type = stab[idname].stype;
	if( !( (CHAR <= type && type <= LONG) ||
		(UCHAR <= type && type <= ULONG) ||
		(ULONG < type) ) ) return;
	if( ISFTN(type) || ISARY(type) ) return;
	if( stab[idname].slevel > 2 ) return;

	/* compute per reference payoff */
	old = estimtab[idname];
#ifndef NODBG
	if (xdebug)
	{
		printf( "raname(0x%lx)\tstab[%d] `%s' sclass %s cur.est %d\n",
			p, idname, stab[idname].sname,
			scnames(stab[idname].sclass), old);
	}
#endif
	/*
	* Don't count putting the PARAM into the REGISTER
	* for REGISTER PARAM's.
	*/
	if( blevel >= 2 && estimtab[idname] != ADDRSD ) 
		switch( stab[idname].sclass ) {
		int i, weight;

		case PARAM:
		case AUTO:
			if( estimtab[idname] == EMPTY ) estimtab[idname] = 0;
			weight = RAPAYOFF;
			for( i = 1; i <= fordepth; i++ ) {
				if( weight >= 10000 ) break;
				if (!sizeopt) weight *= FORWEIGHT;
			}
			for( i = 1; i <= whdepth; i++ ) {
				if( weight >= 10000 ) break;
				if (!sizeopt) weight *= WHWEIGHT;
			}
			for( i = 1; i <= brdepth; i++ ) {
				if (!sizeopt) weight /= BRWEIGHT;
			}
			estimtab[idname] += weight;
		}
	lastid = idname;
	lastp = p;
#ifndef NODBG
	if (xdebug > 1)
	{
		printf( "`%s' type=%d class=%d level=%d flags=%x offset=%d\n",
			stab[idname].sname, stab[idname].stype,
			stab[idname].sclass, stab[idname].slevel,
			stab[idname].sflags, stab[idname].offset);
		printf("\tfordepth=%d whdepth=%d brdepth=%d change=%d estimator=%d\n",
		fordepth, whdepth, brdepth, estimtab[idname]-old,
		estimtab[idname]);
	}
#endif
}

void
raua(p)
NODE *p;

{
	/* mark variables with address taken */
	if( p == lastp )  estimtab[lastid] = ADDRSD;
}

void
raftn()

{

	register int i, j, k, ei;
	struct hinode *hp;
	struct symtab *sp;

	/* check for end of function */
	if( blevel != 2 ) return;

	/* check for doubles */
	if( dblflg == 0 ) printf( "#REGAL\t%d\tNODBL\n", 0 );
	else return;

	/* check for args with address taken */
	for( i = 0; i < SYMTSZ; i++ ) {
		if( stab[i].sclass == PARAM && estimtab[i] == ADDRSD ) {
			for( j = 0; j < SYMTSZ; j++ ) 
				if( stab[j].sclass == PARAM )
					estimtab[j] = ADDRSD;
			break;
		}
	}

	/* insert entries from symbol table in decreasing order */
	for( i = 0; i < SYMTSZ; i++ ) {
		ei = estimtab[i];
		if( ei < 0 ) continue;
		sp = &stab[i];

		/* include cost for arg */
		if( sp->sclass == PARAM ) ei -= 9;
		if( ei < 0 ) continue;

		/* insertion sort */
		/* sort by estimator, then by AUTO or PARM, then by offset */
		for( j = 0; j < HIGHSZ; j++ ) {
			hp = &high[j];
			if( ei > hp->hiestim ||
			    ( ei == hp->hiestim &&
			      ( sp->sclass > hp->hiscl ||
				( sp->sclass == hp->hiscl && 
				   sp->offset > hp->hiident ) )))
			{
				for( k = 8; k >= j; k-- ) {
					hp = &high[k];
					(hp+1)->hiestim = hp->hiestim;
					(hp+1)->hiscl = hp->hiscl;
					(hp+1)->hiident = hp->hiident;
					(hp+1)->hilen = hp->hilen;
				}
				hp = &high[j];
				hp->hiestim = ei;
				hp->hiscl = sp->sclass;
				hp->hiident = sp->offset;
				switch( sp->stype ) {
				case CHAR:
				case UCHAR:
					hp->hilen = 1; break;
				case SHORT:
				case USHORT:
					hp->hilen = 2; break;
				default:
					hp->hilen = 4; break;
				}
				break;
			}
		}
	}

	/* print entries */
	for( i = 0; i < HIGHSZ; i++ ) {
		hp = &high[i];
		if( hp->hiestim == EMPTY ) break;
		printf( "#REGAL\t%d", hp->hiestim );
		switch( hp->hiscl ) {
		case AUTO:
			printf( "\tAUTO \t%d(%%fp)", 
				hp->hiident/SZCHAR );
			break;
		case PARAM:
			printf( "\tPARAM\t%d(%%ap)", hp->hiident/SZCHAR );
			break;
		}
		printf( "\t%d\n", hp->hilen );
	}
}
#endif /* IMPREGAL */
