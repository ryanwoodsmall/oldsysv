/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stincc:scommon/cost.c	1.7"

# include "mfile2.h"

# ifndef CSTORE
# define CSTORE(q) 2
# endif
# ifndef CLOAD
# define CLOAD(q) 2
# endif
# ifndef CCTEST
# define CCTEST(q) 1
# endif
# ifndef DFLT_STRATEGY
# define DFLT_STRATEGY	EITHER
# endif

/* Enough regs so there is always a free pair; to guarantee this,
** HREG is one more than half the registers available.  Note that,
** in fact, HREG guarantees three contiguous registers (really!),
** so there will an odd-even or an even-odd pair available.
*/
# define HREG ((1+NRGS)/2+1)

	/* used to identify subtrees */
# define NSUBTREES 10
int nsubtree;
NODE *subtree[NSUBTREES];
int subgoal[NSUBTREES];

commute( p ) NODE *p; {
	/* commute p in place */
	register NODE *q;
#ifndef NODBG
	if(odebug) printf("commute: .=%u l=%u r=%u\n",p,p->in.left,p->in.right);
#endif
	q = p->in.left;
	p->in.left = p->in.right;
	p->in.right = q;
	}

# ifndef NODBG
# define GETS(x,y) if( e2debug>1) printf( "	x gets %d\n", y );
# define GETSN(x,y) if( e2debug>1) printf( "	cst[%d] gets %d\n", x, y );
# else
# define GETS(x,y)
# define GETSN(x,y)
# endif


lhsok( p )
NODE *p;
{
	/* p appears on the lhs of an assignment op */
	/* is it an OK substitute for a TEMP? */

	switch( p->tn.op ) {

	case NAME:
	case VAUTO:
	case VPARAM:
	case TEMP:
	case REG:
		return( 1 );

	}
	return( 0 );
}

shpr(sp) register SHAPE *sp; {
	if (!sp) return;
	if( sp->op < 0 || sp->op > DSIZE ) cerror( "shape op %d\n", sp->op );
	printf(" %s", opst[sp->op]);
	shpr(sp->sl);
	shpr(sp->sr);
	}

pstrat( s ) {
	/* print a nice version of the strategy s */
	register i, flag;
	static char *stratnames[] = {
		"STORE",
		"LTOR",
		"RTOL",
		0 };
	flag = 0;
	for( i=0; stratnames[i]; ++i ){
		if( s & (1<<i) ) {
			if( flag ) putchar( '|' );
			printf( "%s", stratnames[i] );
			flag = 1;
			}
		}
	if( !flag ) printf( "0" );
	}

insout( p, i )
NODE *p;
{
	OPTAB *q;
	int o;

	/* generate the actual instructions */
	/* if the cost is infinite, try rewriting */
again:

	o = p->tn.op;

#ifndef NODBG
	if( odebug>1 ) printf( "insout(%d,%d)\n", p-node,i );
#endif

	/* handle COMOP specially */
	if( o == COMOP ) {
		q = match( p, (OPTAB *)0  );  /* had better match */
		if( !q ) cerror( "COMOP match fails" );
		return bprt( p, q, i );
	}


	if ( q = match(p, (OPTAB *) 0))
	{
		return bprt( p, q, i );
	}

	/* commuting may be in order here */

	if( o == PLUS  ||  o == MUL  ||  o == AND  ||  o == OR  ||  o == ER ) 
	{
		commute( p ); 
		if ( q=match(p, (OPTAB *) 0))
		{
			return bprt( p, q, i );
		}
	}

			/*ohoh- found no match!*/
			/*Try some rewrites*/
	if ( rewass(p)) return REWROTE; /*Major rewrite*/
	goto again;/*minor rewrite*/

}

SHAPE * leftshape;			/* these two filled in by match */
SHAPE * rightshape;

bprt( p, q, goal )
NODE *p;
OPTAB *q;
int goal;
{
	/* this routine is called to print out the actual instructions */
	/* it is called with a tree node p, a template q, and a goal goal */
	/* bprt calls bcost, and then captures the left and right shapes */
	/* it then uses findsub to determine the preconditions and goals */
	/* a local copy of this information must be made, since bprt can be
	/* called recursively */
	/* then, bprt calls insout to output the instructions that establish
	/* the preconditions.  Finally, it can output its own instruction */

	int haveregs;			/* registers available for tree */
	int curregs;			/* registers still available */
	int s, o, k;
	NODE *l, *r;
	int nn;
	int needregs;
	int mygoal[NSUBTREES];
	NODE *mysubs[NSUBTREES];
	int lstart, lend, subs;
	int rc;
	int savins;
	SHAPE * ls = leftshape;		/* make local copies */
	SHAPE * rs = rightshape;

	/* sets haveregs as well */
	if( goal < NRGS ) haveregs = goal;
	else haveregs = NRGS;
	l = getl( p );
	r = getr( p );
	if (q->rshape && !q->lshape)
		r = p;
			/*choose the strategy*/
	s = pickst(p);
	o = p->tn.op;
# ifndef NODBG
	if( odebug>1 ) {
		printf( "	matches %d, ls = %d(%s), rs = %d(%s),  s= ",
			q->stinline, ls-shapes,
			ls?opst[ls->op]:"SHNL",
			rs-shapes,
			rs?opst[rs->op]:"SHNL" );
		pstrat( s );
		printf( "\n" );
		}
# endif

#ifdef IN_LINE
	/* don't do anything with INCALL trees. 
	/* just stick it in inst[] and handle it in insprt()
	*/
	if( o==INCALL || o==UNARY INCALL ) {
	    if (goal < NRGS)
		return( OUTOFREG );	/* assume need all regs */
	    goto generate;
	}
#endif

	/* handle COMOP differently; this has more to do with the register
	/* allocation than the ordering */

	if( o == COMOP ) {
			/*COMOP's require all of the registers*/
			/*only time this is a problem is with GENLABs
			  (since commas can't rewrite past them)*/
		if ( goal < NRGS)
			return OUTOFREG;
		if (rc = insout( l, CEFF )) return rc;
		if (rc = insout( r, goal )) return rc;
		goto generate;
	}
			/*remember how far we have gotten incase we back up*/
	savins = nins;
			/*try the subtrees according to strategy s:*/
dosubs:
	nsubtree = lstart = lend = 0;
			/*lstart and lend bound the left subtrees*/
	if(rs && s == RTOL)
		findsub( r, rs );

	if( ls ) {
		lstart = nsubtree;
		findsub( l, ls );
		if( l->tn.op==REG && ls->op==REG && asgop(q->op) &&
				!asgop(o) ) {
			/* we must arrange to copy a reg variable on the lhs
			/* of a binary op, in some cases (cf. bcost) */
			subtree[nsubtree] = l;
			subgoal[nsubtree] = NRGS;
			++nsubtree;
			}
		lend = nsubtree;
		}
	if(rs && ((s == LTOR) || (s == EITHER) ) )
		findsub( r, rs );

	nn = nsubtree;

	/* make a local copy,counting registers */
	needregs = q->needs & NCOUNT;

	/* This is an unsatisfactory hack/guess at the number we
	** need for register pairs.
	** One other hack:  if only even-odd pairs are permitted,
	** we must be sure such a pair exists.  This is true for
	** two reasons:  either the current template will need such
	** a pair for its result, or, if due to $L or $R a non-register
	** double ends up being a sub-tree, we will need such a pair later
	** for cfix() to move the double into registers.
	** Make a special case of GENLAB (i.e., ignore it), because the
	** label itself requires no registers (we assume), only the
	** registers of its subtrees.
	*/
	if (   (q->needs & NPAIR) && needregs < HREG
#ifndef	ODDPAIR				/* must have even-odd pairs */
	    || (goal <= NRGS && szty(p->in.type) > 1 && o != GENLAB)
#endif
	    )
	    needregs = HREG;
	subs = 0;
	for( k=0; k<nn; ++k ) 
	{
		mygoal[k] = subgoal[k];
		mysubs[k] = subtree[k];
		if ( mygoal[k] == NRGS)
		{
			subs += szty(mysubs[k]->tn.type);
			/*Only count registers if they can't be shared*/
			if ( k >= lstart && k < lend )/*left subtree*/
			{
				if ( !(q->needs & LSHARE)) 
					needregs += szty(mysubs[k]->tn.type);
			}
			else	/*right subtree*/
			{
				if ( !(q->needs & RSHARE)) 
					needregs += szty(mysubs[k]->tn.type);
			}
		}
	}
			/*Need at least a reg for each subtree*/
	if ( needregs < subs)
		needregs = subs;
#ifndef NODBG
	if (e2debug)
	{
		if ( needregs > haveregs )
			printf("Out of registers: ");
		printf("%d needs %d regs, have %d regs\n",p-node,needregs,haveregs);
	}
#endif
	if (needregs > haveregs) {	/* We're out of registers */
	    if ( haveregs >= NRGS) {

	    /* we're out of REGs, even though we have access to all of them.
	    ** Choose a sub-tree to throw into a TEMP and try again.  If we
	    ** fail, panic.
	    ** For now, just rewrite the first suitable sub-tree.
	    ** We make the assumption (with crossed fingers) that a leaf
	    ** node is an inappropriate candidate for spilling to TEMP:
	    ** usually TEMPs, AUTOs, and PARAMs are equivalent as operands.
	    */
		for (k = 0; k<nn; ++k) {
		    NODE * subtree;

		    if (   mygoal[k] == NRGS
			&& optype((subtree = mysubs[k])->in.op) != LTYPE
			&& rewsto(subtree)
			) {
			if (odebug)
			    printf("Subtree %d spills to TEMP\n", subtree - node);
			return( REWROTE );	/* successful spill */
		    }
		}
	    }
	    return OUTOFREG;		/* let someone at higher level figure out
					** what to do
					*/
	}

# ifndef NODBG
	if( odebug>1 ) {  /* subtree matches are: */
		printf( "\t\t%d matches\n", nn );
		for( k=0; k<nn; ++k ) {
			printf( "\t\tnode %d, goal %d\n",mysubs[k]-node,
				mygoal[k] );
			}
		}
# endif

	/* do the subtrees */

	curregs = haveregs;
	for( k=0; k<nn; ++k ) {
		NODE * sub = mysubs[k];
# ifndef NODBG
		if( odebug>2 )
			printf( "\t\tcalling insout(%d,%d)\n", sub-node,
					curregs );
# endif

			/*generate the code for the subtree*/

		if( mygoal[k] == NRGS ) {
			if((rc = insout( sub, curregs )) == 0)
				curregs -= szty(sub->tn.type);
		}
		else {
			rc = insout( sub, mygoal[k] );
			}		

			/*did the subtree get into trouble?*/
			/*If rewritten, bubble all the way out*/

		if ( rc == REWROTE)
			return REWROTE;

			/*If out of registers, try to recover:*/

		if ( rc == OUTOFREG)
		{
#ifndef NODBG
			if (odebug)
				printf("Subtree %d out of regs\n",sub - node);
#endif

			/*Try the other ordering, if possible*/

			if ( s == EITHER)
			{
#ifndef NODBG
				if (odebug)
					printf("%d First order fails;try RTOL\n",
					p-node);
#endif
				s = RTOL;
				nins = savins;
				goto dosubs;
			}

			/*otherwise, rewrite to temps at highest level*/

			if ( haveregs != NRGS || mygoal[k] != NRGS)
				return OUTOFREG;
#ifndef NODBG
			if (odebug)
				printf("Subtree %d goes into TEMP\n",
					sub - node);
#endif
			/* rewrite sub-tree to TEMP, if possible */
			if (rewsto(mysubs[k]))
			    return( REWROTE );	/* success */
			else {
			    if (odebug)
				printf("-->> Couldn't rewrite %d to TEMP\n",
					sub-node);
			    return( OUTOFREG ); /* fail */
			}
		}
	}

	/* put onto the instruction string the info about the instruction */
    generate:
#ifndef NODBG
	if (odebug)
		printf("Generate #%d: node %d template %d\n",
			nins, p-node, q->stinline);
# endif
	if( nins >= NINS ) cerror( "too many instructions generated" );
	inst[nins].p = p;
	inst[nins].q = q;
	inst[nins].goal = goal;
	/* a special case: REG op= xxx, should be done as early as possible */
	if( asgop(o) && p->in.left->tn.op == REG && o != INCR && o != DECR
			&& goal!=CEFF && goal!=CCC && !istreg(p->in.left->tn.rval)){
		/* "istreg" guards against rewriting returns, switches, etc. */
		if (rewsto(p))
		    return( REWROTE );		/* indicate we rewrote this */
		else
		    inst[nins].goal = CTEMP;
	}
	++nins;
	return 0;
}

findsub( p, s )
NODE *p;
SHAPE *s;
{
	/* account for the costs of matching the shape s with the tree j */

	if( !s )
		return;

# ifndef NODBG
	if( e2debug>1 ) {
		printf( "\t\tfindsub( %d, %d )\n", p-node, s-shapes );
		}
# endif

	switch( s->op ) {

	case TEMP:
		/* leave j unchanged */
		if( p->tn.op == TEMP ) return;
		subtree[nsubtree] = p;
		subgoal[nsubtree] = CTEMP;
		++nsubtree;
		return;

	case FREE:
		subtree[nsubtree] = p;
		subgoal[nsubtree] = CEFF;
		++nsubtree;
		return;

	case CCODES:
		subtree[nsubtree] = p;
		subgoal[nsubtree] = CCC;
		++nsubtree;
		return;

	case REG:
		if( p->tn.op == REG ) return;  /* exact match */

		/* in general, look beneath */
		/* also, look here if a REG and rcst is 1 */

		subtree[nsubtree] = p;
		subgoal[nsubtree] = NRGS;
		++nsubtree;
		return;
		}

	if( s->op == p->tn.op ) {

		/* look at subtrees */
		if( s->sl ) findsub( getl(p), s->sl );
		if( s->sr ) findsub( getr(p), s->sr );
		return;
		}
	}


pickst(p)
NODE *p;
{
	register int s;
	register int o = p->in.op;


	/* determine the code generation strategy */

	s = LTOR;  /* default strategy */

	if( optype(o) == BITYPE ) {

		switch( o ) {

		case CALL:
		case STCALL:
		case FORTCALL:
# ifdef IN_LINE
		case INCALL:	/* handle asm calls like regular calls */
# endif
# ifndef LTORARGS
		case CM:  /* function arguments */
# endif
			s = RTOL;
			break;

		default:
# ifdef STACK
			if( asgop(o) ) s = RTOL;
			else s = LTOR;
# else
			s = DFLT_STRATEGY;
# endif
			break;
		}
	}
	return s;
}
