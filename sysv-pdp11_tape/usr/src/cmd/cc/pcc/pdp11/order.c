/*	@(#)order.c	1.3	*/
# include "mfile2"

int fltused = 0;

stoasg( p, o ) register NODE *p; {
	/* should the assignment op p be stored,
	   given that it lies as the right operand of o
	   (or the left, if o==UNARY MUL) */
	p = p->in.left;
	return( shltype(p->in.op, p ) );

	}

deltest( p ) register NODE *p; {
	/* should we delay the INCR or DECR operation p */
# ifndef MULTILEVEL
	if( p->in.op == INCR && p->in.left->in.op == REG && spsz( p->in.left->in.type, p->in.right->tn.lval ) ){
		/* STARREG */
		return( 0 );
		}
# else
	if( mlmatch(p,DEFINCDEC,INCR) && spsz( p->in.left->in.type, p->in.right->tn.lval ) ){
		/* STARREG */
		return( 0 );
		}
# endif

	p = p->in.left;
/* the OREG case and the UMUL case are wrong try p += (*p)--
	if( p->in.op == UNARY MUL ) p = p->in.left;
	return( p->in.op == NAME || p->in.op == OREG || p->in.op == REG );
*/
	return( p->in.op == NAME || p->in.op == REG );
	}

mkadrs(p) NODE *p; {
	register NODE *l, *r, *ll;

	l = p->in.left;
	ll = l->in.left;
	r = p->in.right;

	if( asgop(p->in.op) ){
		if( l->in.su >= r->in.su ){
			if( l->in.op == UNARY MUL ){
				if( l->in.su > 0 )
					SETSTO( ll, INTEMP );
				else {
					if( r->in.su > 0 ) SETSTO( r, INTEMP );
					else cerror( "store finds both sides trivial" );
					}
				}
			else if( l->in.op == FLD && ll->in.op == UNARY MUL ){
				SETSTO( ll->in.left, INTEMP );
				}
			else { /* should be only structure assignment */
				SETSTO( l, INTEMP );
				}
			}
		else SETSTO( r, INTEMP );
		}
	else {
		if( l->in.su > r->in.su ){
			SETSTO( l, INTEMP );
			}
		else {
			SETSTO( r, INTEMP );
			}
		}
	}

#ifndef notoff
notoff( t, r, off, cp) TWORD t; CONSZ off; char *cp; {
	/* is it legal to make an OREG or NAME entry which has an
	/* offset of off, (from a register of r), if the
	/* resulting thing had type t */

	/* return( 1 );  /* NO */
	return(0);  /* YES */
	}
#endif

# define max(x,y) ((x)<(y)?(y):(x))
# define min(x,y) ((x)<(y)?(x):(y))


# define ZCHAR 01
# define ZLONG 02
# define ZFLOAT 04

zum( p, zap ) register NODE *p; {
	/* zap Sethi-Ullman number for chars, longs, floats */
	/* in the case of longs, only STARNM's are zapped */
	/* ZCHAR, ZLONG, ZFLOAT are used to select the zapping */

	register su;

	su = p->in.su;

	switch( p->in.type ){

	case CHAR:
	case UCHAR:
		if( !(zap&ZCHAR) ) break;
		if( su == 0 ) p->in.su = su = 1;
		break;

	case LONG:
	case ULONG:
		if( !(zap&ZLONG) ) break;
		if( p->in.op == UNARY MUL && su == 0 ) p->in.su = su = 2;
		break;

	case FLOAT:
		if( !(zap&ZFLOAT) ) break;
		if( su == 0 ) p->in.su = su = 1;

		}

	return( su );
	}

sucomp( p ) register NODE *p; {

	/* set the su field in the node to the sethi-ullman
	   number, or local equivalent */

	register nr;
	register sul, sur, o, ty;

	ty = optype( o=p->in.op);
	nr = szty( p->in.type );
	p->in.su = 0;

	if( ty == LTYPE ) {
		if( p->in.type==FLOAT ) p->in.su = 1;
		return;
		}
	else if( ty == UTYPE ){
		switch( o ) {
		case UNARY CALL:
		case UNARY STCALL:
			p->in.su = fregs;  /* all regs needed */
			return;

		case UNARY MUL:
			if( shumul( p->in.left ) ) return;

		default:
			p->in.su = max( p->in.left->in.su, nr);
			return;
			}
		}


	/* If rhs needs n, lhs needs m, regular su computation */

	sul = p->in.left->in.su;
	sur = p->in.right->in.su;

	if( o == ASSIGN ){
		asop:  /* also used for +=, etc., to memory */
		if( sul==0 ){
			/* don't need to worry about the left side */
			p->in.su = max( sur, nr );
			}
		else {
			/* right, left address, op */
			if( sur == 0 ){
				/* just get the lhs address into a register, and mov */
				/* the `nr' covers the case where value is in reg afterwards */
				p->in.su = max( sul, nr );
				}
			else {
				/* right, left address, op */
				p->in.su = max( sur, nr+sul );
				}
			}
		return;
		}

	if( o == CALL || o == STCALL ){
		/* in effect, takes all free registers */
		p->in.su = fregs;
		return;
		}

	if( o == STASG ){
		/* right, then left */
		p->in.su = max( max( sul+nr, sur), fregs );
		return;
		}

	if( logop(o) ){
		/* do the harder side, then the easier side, into registers */
		/* left then right, max(sul,sur+nr) */
		/* right then left, max(sur,sul+nr) */
		/* to hold both sides in regs: nr+nr */
		nr = szty( p->in.left->in.type );
		sul = zum( p->in.left, ZLONG|ZCHAR|ZFLOAT );
		sur = zum( p->in.right, ZLONG|ZCHAR|ZFLOAT );
		p->in.su = min( max(sul,sur+nr), max(sur,sul+nr) );
		return;
		}

	if( asgop(o) ){
		/* computed by doing right, doing left address, doing left, op, and store */
		switch( o ) {
		case INCR:
		case DECR:
			/* do as binary op */
			break;

		case ASG DIV:
		case ASG MOD:
		case ASG MUL:
			if( p->in.type!=FLOAT && p->in.type!=DOUBLE ) nr = fregs;
			goto gencase;

		case ASG PLUS:
		case ASG MINUS:
		case ASG AND:  /* really bic */
		case ASG OR:
		case ASG ER:
				/*  keep LONG different from INT? */
				/*  try *long += int */
			if( p->in.type == INT || p->in.type == UNSIGNED || ISPTR(p->in.type) ) goto asop;
			if( p->in.type == LONG || p->in.type == ULONG )
				if( p->in.left->in.op != UNARY MUL && p->in.right->in.op != UNARY MUL ) goto asop;

		gencase:
		default:
			sur = zum( p->in.right, ZCHAR|ZLONG|ZFLOAT );
			if( sur == 0 ){ /* easy case: if addressable,
				do left value, op, store */
				if( sul == 0 ) p->in.su = nr;
				/* harder: left adr, val, op, store */
				else p->in.su = max( sul, nr+1 );
				}
			else { /* do right, left adr, left value, op, store */
				if( sul == 0 ){  /* right, left value, op, store */
					p->in.su = max( sur, nr+nr );
					}
				else {
					p->in.su = max( sur, max( sul+nr, 1+nr+nr ) );
					}
				}
			return;
			}
		}

	switch( o ){
	case ANDAND:
	case OROR:
	case QUEST:
	case COLON:
	case COMOP:
		p->in.su = max( max(sul,sur), nr);
		return;
		}

	if( ( o==DIV || o==MOD || o==MUL )
	    && p->in.type!=FLOAT && p->in.type!=DOUBLE ) nr = fregs;
	if( o==PLUS || o==MUL || o==OR || o==ER ){
		/* AND is ruined by the hardware */
		/* permute: get the harder on the left */

		register rt, lt;

		if( istnode( p->in.left ) || sul > sur ) goto noswap;  /* don't do it! */

		/* look for a funny type on the left, one on the right */


		lt = p->in.left->in.type;
		rt = p->in.right->in.type;

		if( rt == FLOAT && lt == DOUBLE ) goto swap;

		if( (rt==CHAR||rt==UCHAR) && (lt==INT||lt==UNSIGNED||ISPTR(lt)) ) goto swap;

		if( lt==LONG || lt==ULONG ){
			if( rt==LONG || rt==ULONG ){
				/* if one is a STARNM, swap */
				if( p->in.left->in.op == UNARY MUL && sul==0 ) goto noswap;
				if( p->in.right->in.op == UNARY MUL && p->in.left->in.op != UNARY MUL ) goto swap;
				}
			else if( p->in.left->in.op == UNARY MUL && sul == 0 ) goto noswap;
			else goto swap;  /* put long on right, unless STARNM */
			}

		/* we are finished with the type stuff now; if one is addressable,
			put it on the right */
		if( sul == 0 && sur != 0 ){

			NODE *s;
			int ssu;

		swap:
			ssu = sul;  sul = sur; sur = ssu;
			s = p->in.left;  p->in.left = p->in.right; p->in.right = s;
			}
		}
	noswap:

	sur = zum( p->in.right, ZCHAR|ZLONG|ZFLOAT );
	if( sur == 0 ){
		/* get left value into a register, do op */
		p->in.su = max( nr, sul );
		}
	else {
		/* do harder into a register, then easier */
		p->in.su = max( nr+nr, min( max( sul, nr+sur ), max( sur, nr+sul ) ) );
		}
	}

int radebug = 0;

mkrall( p, r ) register NODE *p; {
	/* insure that the use of p gets done with register r; in effect, */
	/* simulate offstar */

	if( p->in.op == FLD ){
		p->in.left->in.rall = p->in.rall;
		p = p->in.left;
		}

	if( p->in.op != UNARY MUL ) return;  /* no more to do */
	p = p->in.left;
	if( p->in.op == UNARY MUL ){
		p->in.rall = r;
		p = p->in.left;
		}
	if( p->in.op == PLUS && p->in.right->in.op == ICON ){
		p->in.rall = r;
		p = p->in.left;
		}
	rallo( p, r );
	}

rallo( p, down ) register NODE *p; {
	/* do register allocation */
	register down1, down2, type, ty;

	if( radebug ) printf( "rallo( %o, %o )\n", p, down );

	down2 = NOPREF;
	p->in.rall = down;
	down1 = ( down &= ~MUSTDO );

	ty = optype( p->in.op );
	type = p->in.type;


	if( type == DOUBLE || type == FLOAT ){
		if( p->in.op == FORCE ) down1 = FR0|MUSTDO;
		++fltused;
		}
	else switch( p->in.op ) {
	case ASSIGN:	
		down1 = NOPREF;
		down2 = down;
		break;

	case ASG MUL:
	case ASG DIV:
	case ASG MOD:
		/* keep the addresses out of the hair of (r0,r1) */
		if(fregs == 2 ){
			/* lhs in (r0,r1), nothing else matters */
			down1 = R1|MUSTDO;
			down2 = NOPREF;
			break;
			}
		/* at least 3 regs free */
		/* compute lhs in (r0,r1), address of left in r2 */
		p->in.left->in.rall = R1|MUSTDO;
		mkrall( p->in.left, R2|MUSTDO );
		/* now, deal with right */
		if( fregs == 3 ) rallo( p->in.right, NOPREF );
		else {
			/* put address of long or value here */
			p->in.right->in.rall = R3|MUSTDO;
			mkrall( p->in.right, R3|MUSTDO );
			}
		return;

	case MUL:
	case DIV:
	case MOD:
		rallo( p->in.left, R1|MUSTDO );

		if( fregs == 2 ){
			rallo( p->in.right, NOPREF );
			return;
			}
		/* compute addresses, stay away from (r0,r1) */

		p->in.right->in.rall = (fregs==3) ? R2|MUSTDO : R3|MUSTDO ;
		mkrall( p->in.right, R2|MUSTDO );
		return;

	case CALL:
	case STASG:
	case EQ:
	case NE:
	case GT:
	case GE:
	case LT:
	case LE:
	case NOT:
	case ANDAND:
	case OROR:
		down1 = NOPREF;
		break;

	case FORCE:	
		down1 = R0|MUSTDO;
		break;

	case AND: /* it helps for the case of return( x & y ) */
		down2 = R1;
		break;

/* if the optimizer could remove the "mov r1,r1" this could save a little
	case SCONV:
		if( type == LONG || type == ULONG ) down1 = R1;
		break;
* but keep it out for now */

		}

	if( ty != LTYPE ) rallo( p->in.left, down1 );
	if( ty == BITYPE ) rallo( p->in.right, down2 );

	}

offstar( p, cook ) register NODE *p; {
	/* handle indirections */

	if( p->in.op == UNARY MUL ){/* walk to lowest U* and start there */
		while( p->in.left->in.op == UNARY MUL ) p = p->in.left;
		if( !canaddr(p) ) p = p->in.left;
		}

	if( p->in.op == PLUS || p->in.op == MINUS ){
		if( p->in.right->in.op == ICON ){
			order( p->in.left , INTAREG|INAREG );
			return;
			}
		}
	if( p->in.op == ASG PLUS || p->in.op == ASG MINUS )/* could match more */
		order( p, INTAREG|INAREG|SOREG|SNAME|SCON );
	else
		order( p, INTAREG|INAREG );
	}

#ifndef setincr
setincr( p, cook ) NODE *p; {
	return( 0 );	/* for the moment, don't bother */
	}
#endif

niceuty( p ) register NODE *p; {
	register TWORD t;

	return( p->in.op == UNARY MUL && (t=p->in.type)!=CHAR &&
		t!= UCHAR && t!= FLOAT &&
		shumul( p->in.left) != STARREG );
	}
setbin( p, cook ) register NODE *p; {
	register NODE *r, *l;

	r = p->in.right;
	l = p->in.left;

	if( r->in.su == 0 ){ /* rhs is addressable */
		if( logop( p->in.op ) ){
			if( l->in.op == UNARY MUL && l->in.type != FLOAT && shumul( l->in.left ) != STARREG ) offstar( l, cook );/* let offstar drop the U* */
			else order( l, INAREG|INTAREG|INBREG|INTBREG|INTEMP );
			return( 1 );
			}
		if( !istnode( l ) ){
			if( l->in.op == UNARY MUL && l->in.type != FLOAT && !shumul( l->in.left ) ){
				offstar( l, cook );/* let offstar drop the U* */
			}
			else order( l, INTAREG|INTBREG );
			return( 1 );
			}
		/* rewrite */
		return( 0 );
		}
	/* now, rhs is complicated: must do both sides into registers */
	/* do the harder side first */

	if( logop( p->in.op ) ){
		/* relational: do both sides into regs if need be */

		if( r->in.su > l->in.su ){
			if( niceuty(r) ){
				offstar( r, cook );/* let offstar drop the U* */
				return( 1 );
				}
			else if( !istnode( r ) ){
				order( r, INTAREG|INAREG|INTBREG|INBREG|INTEMP );
				return( 1 );
				}
			}

		/* do the lhs then the rhs */
		if( niceuty(l) ){
			offstar( l, cook );/* let offstar drop the U* */
			return( 1 );
			}
		else if( !istnode( l ) ){
			order( l, INTAREG|INAREG|INTBREG|INBREG|INTEMP );
			return( 1 );
			}
		else if( niceuty(r) ){
			offstar( r, cook );/* let offstar drop the U* */
			return( 1 );
			}
		if( !istnode( r ) ){
			order( r, INTAREG|INAREG|INTBREG|INBREG|INTEMP );
			return( 1 );
			}
		cerror( "setbin can't deal with %s", opst[p->in.op] );
		}

	/* ordinary operator */

	if( !istnode(r) && r->in.su > l->in.su ){
		/* if there is a chance of making it addressable, try it... */
		if( niceuty(r) ){
			offstar( r, cook );/* let offstar drop the U* */
			return( 1 );  /* hopefully, it is addressable by now */
			}
		order( r, INTAREG|INAREG|INTBREG|INBREG|INTEMP );  /* anything goes on rhs */
		return( 1 );
		}
	else {
		if( !istnode( l ) ){
			order( l, INTAREG|INTBREG );
			return( 1 );
			}
		/* rewrite */
		return( 0 );
		}
	}

	/* setstr puts the address of both strutures in registers	*/
	/* this will match the shapes in table.c and zzzcode will	*/
	/* produce the correct number of "mov	(rx)+,(ry)+"		*/
setstr( p, cook ) register NODE *p; { /* structure assignment */
	register NODE *l,*r;

	order( p->in.right, INTAREG ); /* put the right in a temp register
					* no matter what it is		*/
	l = p->in.left;
	if( l->in.op != OREG ){
		if( l->in.op == NAME ){
			l->in.op = ICON;
			l->in.type |= PTR;
			order( l, INTAREG );
			}
		else if( l->in.op == UNARY MUL ){
			order( l->in.left, INTAREG ); /* put left in register */
			p->in.left = l->in.left; /* put REG on left */
			l->in.op = FREE; /* get rid of U MUL */
			}
		else cerror( "bad setstr" );
		}
	else {
		CONSZ val;

		val = l->tn.lval;
		l->in.op = REG;		/* OREG becomes REG */
		l->tn.type |= PTR;
		if( val != 0 ) { /* it is an auto or a param so rebuild
				the tree and compute INTAREG */
			p->in.left = talloc();	/* new NODE on left of STASG */
			p = p->in.left;		/* let p point to new NODE */
			*p = *l;		/* copy info from old NODE */
			p->in.left = l;		/* put REG on left */
			r = talloc();
			p->in.right = r;	/* put new NODE on right */
			p->in.op = ( val < 0L ) ? MINUS : PLUS ; /* set OP */
			r->tn.op = ICON;	/* put ICON on the right */
			r->tn.lval = ( val < 0L ) ? -(val) : val ;
			r->tn.type = INT;
			r->tn.name[0] = '\0';
			}
		else {
			p = l;	/* reset p so the call to order will work */
			}
		order( p, INTAREG );
		}
	return( 1 );
	}

setasg( p, cook ) register NODE *p; {
	/* setup for assignment operator */
	register NODE *l,*r;

	l = p->in.left;
	r = p->in.right;

	if( r->in.su != 0 && r->in.op != REG ) {
		if( r->in.op == UNARY MUL )
			offstar( r, INAREG|INBREG );/* the cookie is a dummy arg to offstar */
		else
			order( r, INAREG|INBREG|SOREG|SNAME|SCON );
		return(1);
		}
	if( r->in.op != REG && ( p->in.type == FLOAT || p->in.type == DOUBLE ) ) {
		order( r, INBREG );
		return(1);
		}
	if( l->in.op == UNARY MUL && !tshape( l, STARREG|STARNM ) ){
		offstar( l, INAREG|INBREG );/* the cookie is a dummy arg to offstar */
		return(1);
		}
	if( l->in.op == FLD && l->in.left->in.op == UNARY MUL ){
		offstar( l->in.left, INAREG|INBREG );/* the cookie is a dummy arg to offstar */
		return(1);
		}
	/* if things are really strange, get rhs into a register */
	if( r->in.op != REG ){
		order( r, INAREG|INBREG );
		return( 1 );
		}
	return(0);
	}

setasop( p, cook ) register NODE *p; {
	/* setup for =ops */
	register NODE *q, *p2;
	register sul, sur;

	sul = p->in.left->in.su;
	sur = p->in.right->in.su;

	switch( p->in.op ){

	case ASG PLUS:
	case ASG OR:
	case ASG MINUS:
		if( p->in.type != INT && p->in.type != UNSIGNED && !ISPTR(p->in.type) ) break;
		if( p->in.right->in.type == CHAR || p->in.right->in.type == UCHAR ){
			order( p->in.right, INAREG );
			return( 1 );
			}
		break;

	case ASG ER:
		if( sul == 0 || p->in.left->in.op == REG ){
			if( p->in.left->in.type == CHAR || p->in.left->in.type == UCHAR ) goto rew;  /* rewrite */
			order( p->in.right, INAREG|INBREG );
			return( 1 );
			}
		goto leftadr;
		}

	if( sur == 0 ){

	leftadr:
		/* easy case: if addressable, do left value, op, store */
		if( sul == 0 ) goto rew;  /* rewrite */

		/* harder; make aleft address, val, op, and store */

		if( p->in.left->in.op == UNARY MUL ){
			offstar( p->in.left, INAREG|INBREG );/* the cookie is a dummy arg to offstar */
			return( 1 );
			}
		if( p->in.left->in.op == FLD && p->in.left->in.left->in.op == UNARY MUL ){
			offstar( p->in.left->in.left, INAREG|INBREG );/* the cookie is a dummy arg to offstar */
			return( 1 );
			}
	rew:	/* rewrite, accounting for autoincrement and autodecrement */

		q = p->in.left;
		if( q->in.op == FLD ) q = q->in.left;
		if( q->in.op != UNARY MUL || shumul(q->in.left) != STARREG ) return(0); /* let reader.c do it */

		/* mimic code from reader.c */

		p2 = tcopy( p );
		p->in.op = ASSIGN;
		reclaim( p->in.right, RNULL, 0 );
		p->in.right = p2;

		/* now, zap INCR on right, ASG MINUS on left */

		if( q->in.left->in.op == INCR ){
			q = p2->in.left;
			if( q->in.op == FLD ) q = q->in.left;
			if( q->in.left->in.op != INCR ) cerror( "bad incr rewrite" );
			}
		else if( q->in.left->in.op != ASG MINUS )  cerror( " bad -= rewrite" );

		q->in.left->in.right->in.op = FREE;
		q->in.left->in.op = FREE;
		q->in.left = q->in.left->in.left;

		/* now, resume reader.c rewriting code */

		canon(p);
		rallo( p, p->in.rall );
		order( p2->in.left, INTBREG|INTAREG );
		order( p2, INTBREG|INTAREG );
		return( 1 );
		}

	/* harder case: do right, left address, left value, op, store */

	if( p->in.right->in.op == UNARY MUL ){
		offstar( p->in.right, INAREG|INBREG );/* the cookie is a dummy arg to offstar */
		return( 1 );
		}
	/* sur> 0, since otherwise, done above */
	if( p->in.right->in.op == REG ) goto leftadr;  /* make lhs addressable */
	order( p->in.right, INAREG|INBREG );
	return( 1 );
	}

int crslab = 10000;

getlab(){
	return( crslab++ );
	}

deflab( l ){
	printf( "L%d:", l );
	}

genargs( p) register NODE *p; {
	register size;

	/* generate code for the arguments */

	/*  first, do the arguments on the right (last->first) */
	while( p->in.op == CM ){
		genargs( p->in.right );
		p->in.op = FREE;
		p = p->in.left;
		}

	if( p->in.op == STARG ){ /* structure valued argument */

		size = p->stn.stsize;
		if( p->in.left->in.op == ICON ){
			/* make into a name node */
			p->in.op = FREE;
			p = p->in.left;
			p->in.op = NAME;
			}
		else {
			/* make it look beautiful... */
			p->in.op = UNARY MUL;
			canon( p );  /* turn it into an oreg */
			if( p->in.op != OREG ){
				offstar( p->in.left, INAREG|INBREG );/* the cookie is a dummy arg to offstar */
				canon( p );
				if( p->in.op != OREG ) cerror( "stuck starg" );
				}
			}

		p->tn.lval += size;  /* end of structure */
		/* put on stack backwards */
		for( ; size>0; size -= 2 ){
			p->tn.lval -= 2;
			expand( p, RNOP, "	mov	AR,Z-\n" );
			}
		reclaim( p, RNULL, 0 );
		return;
		}

	/* ordinary case */

	order( p, FORARG );
	}

argsize( p ) register NODE *p; {
	register t;
	t = 0;
	if( p->in.op == CM ){
		t = argsize( p->in.left );
		p = p->in.right;
		}
	if( p->in.type == DOUBLE || p->in.type == FLOAT ){
		SETOFF( t, 2 );
		return( t+8 );
		}
	else if( p->in.type == LONG || p->in.type == ULONG ) {
		SETOFF( t, 2);
		return( t+4 );
		}
	else if( p->in.op == STARG ){
		SETOFF( t, p->stn.stalign );  /* alignment */
		return( t + p->stn.stsize );  /* size */
		}
	else {
		SETOFF( t, 2 );
		return( t+2 );
		}
	}
