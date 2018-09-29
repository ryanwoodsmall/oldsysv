/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)comp:u3b/order.c	1.6.3.1"
# include "mfile2"

/*	@(#) order.c: 1.6.3.1 1/14/86	*/
int maxargs = { -1 };
# ifdef FLT
extern int fltused; /* if fltused>0 then storage space is needed
		   for intermediate floating point results */
# endif /*FLT*/

stoasg( p, o ) register NODE *p; {
	/* should the assignment op p be stored,
	 * given that it lies as the right operand of o
	 * (or the left, if o==UNARY MUL)
	 */
	}

deltest( p ) register NODE *p; {
	/* should we delay the INCR or DECR operation p
	 */
	p = p->in.left;
	return( p->in.op == REG || p->in.op == NAME || p->in.op == OREG );
	}

mkadrs(p) register NODE *p; {
	/* some intermediate result has to be stored on the stack;
	 * determine from which subtree this result should come
	 */
	register o;

	o = p->in.op;

	if( asgop(o) ){
		if( p->in.left->in.su >= p->in.right->in.su ){
			if( p->in.left->in.op == UNARY MUL ){
				SETSTO( p->in.left->in.left, INTEMP );
				}
			else if( p->in.left->in.op == FLD && p->in.left->in.left->in.op == UNARY MUL ){
				SETSTO( p->in.left->in.left->in.left, INTEMP );
				}
			else { /* should be only structure assignment */
				SETSTO( p->in.left, INTEMP );
				}
			}
		else SETSTO( p->in.right, INTEMP );
		}
	else {
		if( p->in.left->in.su > p->in.right->in.su ){
			SETSTO( p->in.left, INTEMP );
			}
		else {
			SETSTO( p->in.right, INTEMP );
			}
		}
	}

notoff( t, r, off, cp) TWORD t; CONSZ off; char *cp; {
	/* is it legal to make an OREG or NAME entry which has an
	 * offset of off, (from a register of r), if the
	 * resulting thing had type t
	 */
	return(0);  /* YES */
	}

# define max(x,y) ((x)<(y)?(y):(x))

sucomp( p ) register NODE *p; {

	/* set the su field in the node to the sethi-ullman
	 * number, or local equivalent
	 */

	register o, ty, sul, sur, r;

	o = p->in.op;
	ty = optype( o );
	p->in.su = szty( p->in.type );

	if( ty == LTYPE ){
		if( o == OREG ){
			r = p->tn.rval;
			/* oreg cost is (worst case) 1 + number of temp registers used */
# ifdef INDEX
			if( R2TEST(r) ){
				if( R2UPK1(r)!=RDUMMY && istreg(R2UPK1(r)) ) ++p->in.su;
				if( istreg(R2UPK2(r)) ) ++p->in.su;
				}
			else
# else
			{
# endif
				if( istreg( r ) ) ++p->in.su;
				}
			}
		if( p->in.su == szty(p->in.type) &&
		   (p->in.op!=REG || !istreg(p->tn.rval)) &&
		   (p->in.type==INT || p->in.type==UNSIGNED || p->in.type == DOUBLE) )
			p->in.su = 0;
		return;
		}

	else if( ty == UTYPE ){
		switch( o ){
		case UNARY CALL:
		case UNARY STCALL:
			p->in.su = fregs;  /* all regs needed */
			return;
		case UNARY MUL:
		case SCONV:
		    {
			register sz = szty(p->in.type);
			/* this assumes that you can share the register
			   used by the lhs, even for doubles
			   ... a relatively safe (?) assumption. */
			if (p->in.left->in.su > sz)  {
				p->in.su = p->in.left->in.su;
			}  else  {
				p->in.su = sz;
			}
			return;
		    }

		default:
			p->in.su =  p->in.left->in.su +
				(szty( p->in.type ) > 1 ? 2 : 0) ;
			return;
			}
		}


	/* If rhs needs n, lhs needs m, regular su computation */

	sul = p->in.left->in.su;
	sur = p->in.right->in.su;

	if( o == ASSIGN ){
		/* computed by doing right, then left (if not in mem), then doing it */
		p->in.su = max(sur, sul + szty(p->in.right->in.type));
		return;
		}

	if( o == CALL || o == STCALL ){
		/* in effect, takes all free registers */
		p->in.su = fregs;
		return;
		}

	if( o == STASG ){
		/* right, then left */
		p->in.su = max( max( 1+sul, sur), fregs );
		return;
		}

	if( asgop(o) ){
		/* computed by doing right, doing left address, doing left, op, and store */
		if( o == ASG LS || o == ASG RS )
			p->in.su = max(sur,sul+2);
		else
			p->in.su = max(sur, sul + szty(p->in.right->in.type));
		return;
		}

	switch( o ){
	case ANDAND:
	case OROR:
	case QUEST:
	case COLON:
	case COMOP:
		p->in.su = max( max(sul,sur), 1);
		return;

	case PLUS:
	case MUL:
	case OR:
	case ER:
		/* commutative ops; put harder on left */
		if( p->in.right->in.su > p->in.left->in.su && !istnode(p->in.left) ){
			register NODE *temp;
			temp = p->in.left;
			p->in.left = p->in.right;
			p->in.right = temp;
			}
#ifdef FORT
	case DIV:
	case MINUS:
		/* We must account for the fact that floats get 
		   promoted to doubles within and operation.  In
		   C, szty(float) = 2, which inflates the su #
		   sufficiently.  This is a bad idea in f77 where
		   floating point operations are frequently
		   confined to floats (reals) and overestimating
		   in this fashion could harm performance.

		   The problem only occurs when there is a float
		   and a double in an operation.  Furthermore,
		   in all cases I can construct, if the float is
		   on the left, coincidence inflates the su number
		   sufficiently.  So we adjust just one case.
		*/
		if ((p->in.left->in.type == DOUBLE) && 
				(p->in.right->in.type == FLOAT))
			sur += 2;
#endif
		}

	/* binary op, computed by left, then right, then do op */
	p->in.su = max(sul,szty(p->in.right->in.type)+sur);
	}

int radebug = 0;

rallo( p, down ) NODE *p; {
	/* do register allocation
	 */
	register o, down1, down2, ty;
# ifdef FLT
	register type;
# endif /*FLT*/

	if( radebug ) printf( "rallo( %o, %d )\n", p, down );

	down2 = NOPREF;
	p->in.rall = down;
	down1 = ( down &= ~MUSTDO );

	ty = optype( o = p->in.op );
# ifdef FLT
	type = p->in.type;

	if( type == DOUBLE || type == FLOAT )
		fltused++;
# endif /*FLT*/


	switch( o ){
	case ASSIGN:	
		down1 = NOPREF;
		down2 = down;
		break;

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

		}

	if( ty != LTYPE ) rallo( p->in.left, down1 );
	if( ty == BITYPE ) rallo( p->in.right, down2 );

	}

setregs(){
	/* set up temporary registers
	 */
	fregs = 4;	/* 4 free regs */
	}		/* used to be 3; need 4 for double calcs */

szty( t ) TWORD t; {
	/* size, in registers, needed to hold thing of type t
	 */
#ifdef FORT
	if (t == DOUBLE)
#else
	if (t == DOUBLE || t == FLOAT)
#endif
		return (2);
	else
		return (1);
	}

rewfld( p ) NODE *p; {
	/* can FLD that is not in lvalue context be rewritten;
	 * only called if no hardware support for fields
	 */
	return(1);
	}

callreg( p ) NODE *p; {
	/* register used to return function value
	 */
	    return( R0 );

	}
# ifdef INDEX
base( p ) register NODE *p; {
	/* determine whether node is valid base for indexed OREG;
	 * return register number packed to contain any indirection info
	 */
	register int o = p->in.op;

	if( (o==ICON && p->in.name[0] != '\0')) return( RDUMMY ); /* ie no base reg */
	if( o==REG ) return( p->tn.rval );
	if( (o==PLUS || o==MINUS) && p->in.left->in.op == REG && p->in.right->in.op==ICON)
		return( p->in.left->tn.rval );
	if( o==OREG && !R2TEST(p->tn.rval) && (p->in.type==INT || p->in.type==UNSIGNED || ISPTR(p->in.type)) )
		return( p->tn.rval + 0200*1 );
	return( -1 );
	}

offset( p, tyl ) register NODE *p; int tyl; {
	/* determine whether node is valid as index reg for indexed OREG;
	 * return register number
	 */
	if( tyl==1 && p->in.op==REG && (p->in.type==INT || p->in.type==UNSIGNED) ) return( p->tn.rval );
	if( (p->in.op==LS && p->in.left->in.op==REG && (p->in.left->in.type==INT || p->in.left->in.type==UNSIGNED) &&
	      (p->in.right->in.op==ICON && p->in.right->in.name[0]=='\0')
	      && (1<<p->in.right->tn.lval)==tyl))
		return( p->in.left->tn.rval );
	return( -1 );
	}

makeor2( p, q, b, o) register NODE *p, *q; register int b, o; {
	/* convert node into an indexed OREG
	 */
	register NODE *t;
	register int i;
	NODE *f;

	p->in.op = OREG;
	f = p->in.left; 	/* have to free this subtree later */

	/* init base */
	switch( q->in.op ){
		case ICON:
		case REG:
		case OREG:
			t = q;
			break;

		case MINUS:
			q->in.right->tn.lval = -q->in.right->tn.lval;
		case PLUS:
			t = q->in.right;
			break;

		default:
			cerror( "illegal makeor2" );
	}

	p->tn.lval = t->tn.lval;
#ifdef FLEXNAMES
	p->in.name = t->in.name;
#else
	for(i=0; i<NCHNAM; ++i)
		p->in.name[i] = t->in.name[i];
#endif

	/* init offset */
	p->tn.rval = R2PK( (b & 0177), o, (b>>7) );

	tfree(f);
	return;
	}

myoreg2( p ) register NODE *p; {
	/* look for doubly indexed expressions
	 */
# ifdef INDEX
	NODE *q;
	register i;
	register r;
	register NODE *ql, *qr;

	if( p->in.op == UNARY MUL ){
		q = p->in.left;

		if( q->in.op == PLUS ){
			ql = q->in.left;
			qr = q->in.right;
			if( (r=base(ql))>=0 && (i=offset(qr, tlen(p)))>=0){
				makeor2(p, ql, r, i);
				return;
			} else if( (r=base(qr))>=0 && (i=offset(ql, tlen(p)))>=0 ){
				makeor2(p, qr, r, i);
				return;
				}
			}
		}
# endif
	}
# endif

shltype( o, p ) register NODE *p; {
	/* is p addressable
	 */
	return( o== REG || o == NAME || o == ICON || o == OREG || ( o==UNARY MUL && shumul(p->in.left)) );
	}

flshape( p ) register NODE *p; {
	/* p is a FLD subtree, is p addressable
	 */
	return( shltype(p->in.op,p) );
	}

shtemp( p ) register NODE *p; {
	/* is p an INTEMP shape
	 */
	if( p->in.op == STARG ) p = p->in.left;
	return( p->in.op==NAME || p->in.op ==ICON || p->in.op == OREG || (p->in.op==UNARY MUL && shumul(p->in.left)) );
	}

shumul( p ) register NODE *p; {
	/* p is subtree of UNARY MUL, what shape do we have
	 */
	register o;

	o = p->in.op;
	if( o == NAME
# ifdef INDEX
		|| (o == OREG && !R2TEST(p->tn.rval))
# else
		|| o == OREG
# endif
		|| o == ICON ) return( STARNM );
	return(0);
	}

s3adr( p, type ) NODE *p; {
	/* determine whether node is acceptable as triadic operand
	 */
	if( !tshape(p,SAREG|SBREG|SNAME|SOREG|STARNM) || !ttype(p->in.type,type) )

		switch( type ){

		case TLONG|TULONG|TINT|TUNSIGNED|TPOINT:
			return(0);

# ifdef FLT
		case TDOUBLE:
			return(0);
# endif /*FLT*/

		case TSHORT|TUSHORT:
			return( tshape(p,SSCON) );

		case TCHAR|TUCHAR:
			return( tshape(p,SCCON) );
			}
	return(1);
	}

s3shape( p, type ) NODE *p; {
	/* determine whether node is a triadic shape
	 */
	register o;
	NODE *r;

	o = p->in.op;
	if( o==PLUS || o==MINUS || o==AND || o==OR || o==ER )
		return( s3adr( getlr(p,'L'), type ) &&
			s3adr( getlr(p,'R'), type ) );
	else if( (p->in.op==MUL || p->in.op==DIV || p->in.op==MOD)
		&& type==(TLONG|TULONG|TINT|TUNSIGNED|TPOINT) )
		return( s3adr( getlr(p,'L'), type ) &&
			s3adr( getlr(p,'R'), type ) );
# ifdef FLT
	else if( (p->in.op==MUL || p->in.op==DIV)
		&& type==(TDOUBLE) )
		return( s3adr( getlr(p,'L'), type ) &&
			s3adr(getlr(p,'R'), type ) );
# endif /*FLT*/

	else if( (p->in.op==LS || p->in.op==RS)
		&& type==(TLONG|TULONG|TINT|TUNSIGNED|TPOINT) )
		return( s3adr( getlr(p,'L'), type ) &&
			( s3adr( r=getlr(p,'R'), TCHAR|TUCHAR ) || r->in.op == REG ) );
	return(0);
	}

special( p, shape ) register NODE *p; {
# ifdef TV
	extern int tvflag;
# endif /*TV*/

	/* return true if the given special shape is appropriate for the node
	 */
	switch( shape ){

	case SSCON:
	case SCCON:
		if( p->in.op != ICON || p->in.name[0] != '\0' ) return(0);
		if( p->tn.lval >= 0 && p->tn.lval <= 255 && shape == SCCON ) return(1);
		else if( p->tn.lval >= -32768 && p->tn.lval <= 32767 && shape == SSCON ) return(1);
		else return(0);

# ifdef FLT
	case SSCONV:	/* eliminate some floating point SCONV's */
		if( p->in.op != SCONV )
			return(0);
		if( optype( p->in.left->in.op ) != LTYPE )
			return(0);
		return( ( p->in.left->in.type == INT ) ||
				( p->in.left->in.type == FLOAT ) ||
				( p->in.left->in.type == DOUBLE ) );
# endif /*FLT*/

	case SW3ADR:	/* triadic - word */
		return( s3shape(p,TINT|TUNSIGNED|TLONG|TULONG|TPOINT) );

	case SH3ADR:	/* triadic - halfword */
		return( s3shape(p,TSHORT|TUSHORT) );

	case SB3ADR:	/* triadic - byte */
		return( s3shape(p,TCHAR|TUCHAR) );

# ifdef FLT
	case SD3ADR:	/* triadic - double */
		return( s3shape( p,TDOUBLE ) );

# endif /*FLT*/
	case SW2ADR:	/* dyadic - word */
	case SH2ADR:	/* dyadic - halfword */
	case SB2ADR:	/* dyadic - byte */
		if( (p->in.op == UNARY MINUS || p->in.op == COMPL)
			&& tshape(p->in.left,SAREG|SBREG|SNAME|SOREG|SCON|STARNM)
			&& ttype(p->in.left->in.type,
				(shape==SW2ADR) ? TINT|TUNSIGNED|TLONG|TULONG|TPOINT
				: ((shape==SH2ADR) ? TSHORT|TUSHORT
				: ((p->in.op==COMPL) ? TCHAR|TUCHAR
				: 0))) )	/* don't have mnegb instr */
			return(1);
		else
			return(0);

	case SSTCALL:	/* right operand of STASG is completed STCALL */
		return( p->in.op==UNARY STCALL && optype(p->in.left->in.op)==LTYPE );
# ifdef INDEX

	case SSOREG:	/* non-indexed OREG */
		if( p->in.op == OREG && !R2TEST(p->tn.rval) ) return(1);
		else return(0);
# endif
# ifdef TV
	case STV:
		return(tvflag);
	case SNOTTV:
		return(!tvflag);
# else

	case SNOTTV:
		return(1);
# endif /*TV*/


	default:
		return(0);

		}
	}

offstar( p ) register NODE *p; {
	/* handle indirections
	 */
# ifdef INDEX
	if( p->in.op == PLUS ){
		if( p->in.left->in.su == fregs ){
			order( p->in.left, INTAREG|INAREG );
			return;
			}
		else if( p->in.right->in.su == fregs ){
			order( p->in.right, INTAREG|INAREG );
			return;
			}
		if( p->in.left->in.op==LS && 
		  (p->in.left->in.left->in.op!=REG || tlen(p->in.left->in.left)!=4 ) ){
			order( p->in.left->in.left, INTAREG|INAREG );
			return;
			}
		if( p->in.right->in.op==LS &&
		  (p->in.right->in.left->in.op!=REG || tlen(p->in.right->in.left)!=4 ) ){
			order( p->in.right->in.left, INTAREG|INAREG );
			return;
			}
		if( p->in.type == (PTR|CHAR) || p->in.type == (PTR|UCHAR) ){
			if( p->in.left->in.op!=REG || tlen(p->in.left)!=4 ){
				order( p->in.left, INTAREG|INAREG );
				return;
				}
			else if( p->in.right->in.op!=REG || tlen(p->in.right)!=4 ){
				order(p->in.right, INTAREG|INAREG);
				return;
				}
			}
		}
	if( p->in.op == PLUS || p->in.op == MINUS ){
		if( p->in.right->in.op == ICON ){
			order( p->in.left , INTAREG|INAREG);
			return;
			}
		}

# else
	if( p->in.op == PLUS && p->in.right->in.op == ICON && p->in.left->in.op != REG )
	{
		order( p->in.left, INTAREG|INAREG );
		return;
	}
# endif
	if( p->in.op == UNARY MUL && !shltype(p->in.op,p) ){
		offstar( p->in.left );
		return;
		}

	order( p, INTAREG|INAREG|SNAME );
	}

setincr(p, cookie)
register NODE *p;
int cookie;
{
	/* set up for INCR and DECR operators
	 */
	if ((p = p->in.left)->in.op == UNARY MUL)
	{
		offstar(p, cookie);
		return (1);
	}
	else
		return( 0 );
}

setbin( p ) register NODE *p; {
	/* set up for binary operator
	 */
	register NODE *l, *r;

	l = p->in.left;
	r = p->in.right;

	if( shltype(l->in.op,l) ){
		if( shltype(r->in.op,r) ){ /* deal with irregular nature of shift operand types */
			if( (p->in.op==LS || p->in.op==RS) && r->in.type!=CHAR && r->in.type!=UCHAR && r->in.op!=REG && r->in.op!=ICON ){
				order( r, INTAREG );
				return(1);
				}
			}
		else { /* address rhs */
			if( r->in.op == UNARY MUL ){
				offstar( r->in.left );
				return(1);
				}
			else {
# ifdef FLT
				order( r, INAREG|INTAREG|INBREG|INTBREG|SOREG|SNAME|STARNM );
# else
				order( r, INAREG|INTAREG|SOREG|SNAME|STARNM );
# endif /*FLT*/
				return(1);
				}
			}
		}
	else {
		order( l, INAREG|INTAREG|INBREG|INTBREG|SOREG|SNAME|STARNM );
		return(1);
		}
	if( l->in.type == CHAR || l->in.type == UCHAR || l->in.type == SHORT || l->in.type == USHORT ){
		order( l, INAREG|INBREG );
		return(1);
		}
	if( r->in.type == CHAR || r->in.type == UCHAR || r->in.type == SHORT || r->in.type == USHORT ){
		order( r, INAREG|INBREG );
		return(1);
		}
	if( !istnode( l) ){
#ifdef SPFORT
		order( l, INTAREG|INTBREG );
#else
		order( l, INTAREG );
#endif /* SPFORT */
		return(1);
		}
	return(0);
	}

setstr( p ) register NODE *p; {
	/* set up for structure assignment operator
	 */
	if( p->in.right->in.op != REG && !tshape(p->in.right,SSTCALL) ){
		if( p->in.right->in.op == UNARY STCALL || p->in.right->in.op == STCALL )
			return( !genscall(p->in.right,INTAREG) );
		order( p->in.right, INTAREG );
		return(1);
		}
	p = p->in.left;
	if( p->in.op != NAME && p->in.op != OREG ){
		if( p->in.op != UNARY MUL ) cerror( "bad setstr" );
		order( p->in.left, INTAREG );
		return( 1 );
		}
	return( 0 );
	}

setasg( p ) register NODE *p; {
	/* set up for assignment operator
	 */
	register NODE *r, *rl, *rr;

	r = p->in.right;
	if( !shltype(r->in.op,r) ){
		rl = r->in.left;
		if( r->in.op == UNARY MUL )
			offstar( rl );
		else {	/* get into form for special dyadic or triadic assign */

			switch( r->in.op ){

			case UNARY MINUS:
				if( rl->in.type == CHAR || rl->in.type == UCHAR )
					break;	/* don't have a mnegb instr */
			case COMPL:
				if( !shltype(rl->in.op,rl) ){
					order( rl, INAREG|INBREG|SOREG|SNAME|STARNM|SCON );
					return(1);
					}
				break;

			case PLUS:
			case MINUS:
			case MUL:
			case DIV:
			case MOD:
			case AND:
			case OR:
			case ER:
				rr = r->in.right;
				if( tlen(rl) != tlen(rr)
					|| !shltype(rl->in.op,rl)
					|| !shltype(rr->in.op,rr) )
					if( !(p->in.rall & MUSTDO) && setbin(r) )
						return(1);
				break;

			case LS:
			case RS:
				rr = r->in.right;
				if( tlen(rl) != 4 || tlen(rr) != 1
					|| !shltype(rl->in.op,rl)
					|| !shltype(rr->in.op,rr) )
					if( !(p->in.rall & MUSTDO) && setbin(r) )
						return(1);
				break;
				}

			order( r, INAREG|INBREG|SOREG|SNAME|STARNM|SCON );
			}
		return(1);
		}
	p = p->in.left;
	if( p->in.op == UNARY MUL ){
		offstar( p->in.left );
		return(1);
		}
	if( p->in.op == FLD && p->in.left->in.op == UNARY MUL ){
		offstar( p->in.left->in.left );
		return(1);
		}
	if( p->in.op == FLD && !(r->in.type==INT || r->in.type==UNSIGNED) ){
		order( r, INAREG);
		return(1);
		}
	return(0);
	}

setasop( p ) register NODE *p; {
	/* set up for =ops
	 */
	register rt, ro;

	rt = p->in.right->in.type;
	ro = p->in.right->in.op;

	if( ro == UNARY MUL && rt != CHAR ){
		offstar( p->in.right->in.left );
		return(1);
		}
	if( ( rt == CHAR || rt == SHORT || rt == UCHAR || rt == USHORT
		|| ( ro != REG && ro != ICON && ro != NAME && ro != OREG ) ) ){
		if( ro == REG )
			p->in.right->in.type = (ISUNSIGNED(rt) || rt==CHAR) ? UNSIGNED : INT;
		else {
			order( p->in.right, INAREG|INBREG );
			return(1);
			}
		}
	if( (p->in.op == ASG LS || p->in.op == ASG RS) && ro != ICON && ro != REG ){
		order( p->in.right, INAREG );
		return(1);
		}

	p = p->in.left;
	if( p->in.op == FLD ) p = p->in.left;

	switch( p->in.op ){

	case REG:
	case ICON:
	case NAME:
	case OREG:
		return(0);

	case UNARY MUL:
		if( p->in.left->in.op==OREG )
			return(0);
		else
			offstar( p->in.left );
		return(1);

		}
	cerror( "illegal setasop" );
	return(0);
	}

nextcook( p, cookie ) NODE *p; {
	/* we have failed to match p with cookie; try another
	 */
	if( cookie == FORREW ) return(0);  /* hopeless! */
	if( !(cookie&(INTAREG|INTBREG)) ) return( INTAREG|INTBREG );
	if( !(cookie&INTEMP) && asgop(p->in.op) ) return( INTEMP|INAREG|INTAREG|INTBREG|INBREG );
	return( FORREW );
	}

lastchance( p, cook ) NODE *p; {
	/* forget it! */
	return(0);
	}

optim2( p ) register NODE *p; {
	/* do local tree transformations and optimizations
	 */
	register NODE *r;

	switch( p->in.op ){

	case AND:
		/* commute L and R to eliminate compliments and constants */
		if( (p->in.left->in.op==ICON&&p->in.left->in.name[0]=='\0') || p->in.left->in.op==COMPL ){
			r = p->in.left;
			p->in.left = p->in.right;
			p->in.right = r;
			}
		}
	}

int crslab = 9999;  /* Honeywell */

getlab(){
	return( crslab-- );
	}

deflab( l ){
	printf( ".L%d:\n", l );
	}
