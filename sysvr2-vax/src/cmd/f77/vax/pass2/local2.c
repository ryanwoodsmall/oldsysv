/*	@(#) local2.c: 1.11 7/3/84	*/
# include <stdio.h>
# include "mfile2"
# include "ctype.h"
/* a lot of the machine dependent parts of the second pass */

# define BITMASK(n) ((1L<<n)-1)

where(c){
	fprintf( stderr, "%s, line %d: ", filename, lineno );
	}

lineid( l, fn ) char *fn; {
	/* identify line l and file fn */
	printf( "#	line %d, file %s\n", l, fn );
	}


eobl2(){
	OFFSZ spoff;	/* offset from stack pointer */
# ifdef ONEPASS
	extern char fncname[];
# else
#ifndef FORT
#ifdef FLEXNAMES
	char fncname[BUFSIZ + 1];
#else
	char fncname[NCHNAM + 1];
#endif
	int i;
	char c;

	fncname[0] = '_';
	i = 1;
	getchar();	/* pass the tab character */
	while( ( c = getchar() ) != '\t' )
		fncname[i++] = c;
	fncname[i] = '\0';
# endif
# endif
#ifdef FORT
	spoff = maxoff;
	if( spoff >= AUTOINIT ) spoff -= AUTOINIT;
	spoff /= SZCHAR;
	SETOFF(spoff,4);
	printf("\t.set\t.F%d,%ld+.FSP%d\n", ftnno, spoff, ftnno);
#else
	extern int ftlab1, ftlab2;

	spoff = maxoff;
	if( spoff >= AUTOINIT ) spoff -= AUTOINIT;
	spoff /= SZCHAR;
	SETOFF(spoff,4);
	printf( ".L%d:\n", ftlab1);
	if( spoff!=0 )
		if( spoff < 64 )
			printf( "	subl2	$%ld,sp\n", spoff);
		else
			printf( "	movab	-%ld(sp),sp\n", spoff);
	printf( "	jbr 	.L%d\n", ftlab2);
#endif
#ifndef	FORT
#ifdef FLEXNAMES
	printf( "\t.def	%s;	.val	.;	.scl	-1;	.endef\n",
		fncname);
#else
	printf( "\t.def	%.8s;	.val	.;	.scl	-1;	.endef\n",
		fncname);
#endif
#endif
	maxargs = -1;
	}

struct hoptab { int opmask; char * opstring; } ioptab[] = {

	ASG PLUS, "add",
	ASG MINUS, "sub",
	ASG MUL, "mul",
	ASG DIV, "div",
	ASG OR, "bis",
	ASG ER,	"xor",
	ASG AND, "bic",
	PLUS,	"add",
	MINUS,	"sub",
	MUL,	"mul",
	DIV,	"div",
	OR,	"bis",
	ER,	"xor",
	AND,	"bic",
	-1, ""    };

hopcode( f, o ){
	/* output the appropriate string from the above table */

	register struct hoptab *q;

	for( q = ioptab;  q->opmask>=0; ++q ){
		if( q->opmask == o ){
			fputs( q->opstring, stdout );
/*******
			if( f == 'F' ) putchar( 'e' );
			else if( f == 'D' ) putchar( 'd' );
********/
			switch( f ) {
				case 'L':
				case 'W':
				case 'B':
				case 'D':
				case 'F':
					printf("%c", tolower(f));
					break;

				}
			return;
			}
		}
	cerror( "no hoptab for %s", opst[o] );
	}

char *
rnames[] = {  /* keyed to register number tokens */

	"r0", "r1",
	"r2", "r3", "r4", "r5",
	"r6", "r7", "r8", "r9", "r10", "r11",
	"ap", "fp", "sp", "pc",

	};

int rstatus[] = {
	SAREG|STAREG, SAREG|STAREG,
	SAREG|STAREG, SAREG|STAREG, SAREG|STAREG, SAREG|STAREG,
	SAREG, SAREG, SAREG, SAREG, SAREG, SAREG,
	SAREG, SAREG, SAREG, SAREG,

	};

tlen(p) NODE *p;
{
	switch(p->in.type) {
		case CHAR:
		case UCHAR:
			return(1);

		case SHORT:
		case USHORT:
			return(2);

		case DOUBLE:
			return(8);

		default:
			return(4);
		}
}

mixtypes(p, q) NODE *p, *q;
{
	register tp, tq;

	tp = p->in.type;
	tq = q->in.type;

/*	return( (tp==FLOAT || tp==DOUBLE) !=
		(tq==FLOAT || tq==DOUBLE) );
*/
	return ((tp == DOUBLE) != (tq == DOUBLE));
}

prtype(n) NODE *n;
{
			/* calls to printf changed to putchar */
	switch (n->in.type)
		{
		case DOUBLE:
			putchar('d');
			return;

		case FLOAT:
			putchar('f');
			return;

		case LONG:
		case ULONG:
		case INT:
		case UNSIGNED:
			putchar('l');
			return;

		case SHORT:
		case USHORT:
			putchar('w');
			return;

		case CHAR:
		case UCHAR:
			putchar('b');
			return;

		default:
			if ( !ISPTR( n->in.type ) ) cerror("zzzcode- bad type");
			else {
				putchar('l');
				return;
				}
		}
}

zzzcode( p, c ) register NODE *p; {
	register m;
	CONSZ val;
	switch( c ){

	case 'N':  /* logical ops, turned into 0-1 */
		/* use register given by register 1 */
		cbgen( 0, m=getlab(), 'I' );
		deflab( p->bn.label );
		printf( "	clrl	%s\n", rnames[getlr( p, '1' )->tn.rval] );
		deflab( m );
		return;

	case 'I':
	case 'P':
		cbgen( p->in.op, p->bn.label, c );
		return;

	case 'A':
		{
		register NODE *l, *r;
		TWORD regtype;

		if (xdebug) eprint(p, 0, &val, &val);
		r = getlr(p, 'R');
		if (optype(p->in.op) == LTYPE || p->in.op == UNARY MUL)
			{
			l = resc;
#ifdef FORT
			if(r->in.type==FLOAT || r->in.type==DOUBLE)
				l->in.type = r->in.type;
			else
				l->in.type = INT;
#else
			l->in.type = (r->in.type==FLOAT || r->in.type==DOUBLE ? DOUBLE : INT);
#endif
			}
		else
			l = getlr(p, 'L');

		regtype = l->in.type;

		if (l->in.op == REG)
			{
			switch(regtype){
				case FLOAT:
				case DOUBLE:
				case UNSIGNED:
				case ULONG:
					break;
				case UCHAR:
				case USHORT:
					l->in.type = UNSIGNED;
					break;
				default:
					l->in.type = INT;
				}
			}

		if (r->in.op == ICON  && r->in.name[0] == '\0')
			{
			if (r->tn.lval == 0)
				{
				fputs("clr",stdout);
				prtype(l);
				putchar('\t');
				adrput(l);
				return;
				}
			if (r->tn.lval < 0 && r->tn.lval >= -63)
				{
				fputs("mneg",stdout);
				prtype(l);
				r->tn.lval = -r->tn.lval;
				goto ops;
				}
			r->in.type = (r->tn.lval < 0 ?
					(r->tn.lval >= -128 ? CHAR
					: (r->tn.lval >= -32768 ? SHORT
					: INT )) : r->in.type);
			r->in.type = (r->tn.lval >= 0 ?
					(r->tn.lval <= 63 ? INT
					: ( r->tn.lval <= 127 ? CHAR
					: (r->tn.lval <= 255 ? UCHAR
					: (r->tn.lval <= 32767 ? SHORT
					: (r->tn.lval <= 65535 ? USHORT
					: INT ))))) : r->in.type );
			}

		if (!mixtypes(l,r))
			{
			if (tlen(l) == tlen(r))
				{
				fputs("mov",stdout);
				prtype(l);
				goto ops;
				}
			if ( l->in.op == REG && r->in.op == REG )
				{
				fputs("movl",stdout);
				goto ops;
				}
			if (tlen(l) > tlen(r) && ISUNSIGNED(r->in.type))
				{
				fputs("movz",stdout);
				}
			else
				{
				fputs("cvt",stdout);
				}
			}
		else
			{
			fputs("cvt",stdout);
			}
		prtype(r);
		prtype(l);
	ops:
		putchar('\t');
		adrput(r);
		putchar(',');
		adrput(l);

		l->in.type = regtype;
		if( l->in.op == REG && tlen(l) < tlen(r) && regtype != FLOAT)
			{
			fputs("\n\t",stdout);
			fputs( !ISUNSIGNED(l->in.type) ? "cvt":"movz", stdout );
			prtype(l);
			putchar('l');
			putchar('\t');
			adrput(l);
			putchar(',');
			adrput(l);
			}
		return;
		}

	case 'B':	/* get oreg value in temp register for left shift */
		{
		register NODE *r;
		if (xdebug) eprint(p, 0, &val, &val);
		r = p->in.right;
		if( tlen(r) == sizeof(int) && r->in.type != FLOAT )
			fputs("movl",stdout);
		else {
			fputs("cvt",stdout);
			prtype(r);
			putchar('l');
			}
		return;
		}

	case 'C':	/* num words pushed on arg stack */
		{
		extern int gc_numbytes;
		extern int xdebug;

		if (xdebug) printf("->%d<-",gc_numbytes);

		printf("$%d", gc_numbytes/(SZLONG/SZCHAR) );
		return;
		}

	case 'D':	/* INCR and DECR */
		zzzcode(p->in.left, 'A');
		fputs("\n\t",stdout);

	case 'E':	/* INCR and DECR, FOREFF */
		if (p->in.right->tn.op == ICON && p->in.right->tn.lval == 1)
			{
			fputs( p->in.op == INCR ? "inc" : "dec", stdout );
			prtype(p->in.left);
			putchar('\t');
			adrput(p->in.left);
			return;
			}
		fputs( p->in.op == INCR ? "add" : "sub", stdout );
		prtype(p->in.left);
		fputs("2\t",stdout);
		adrput(p->in.right);
		putchar(',');
		adrput(p->in.left);
		return;

	case 'F':	/* register type of right operand */
		{
		register NODE *n;
		extern int xdebug;
		register int ty;

		n = getlr( p, 'R' );
		ty = n->in.type;

		if (xdebug) printf("->%d<-", ty);

		if ( ty==DOUBLE) putchar('d');
		else if ( ty==FLOAT ) putchar('f');
		else putchar('l');
		return;
		}

	case 'L':	/* type of left operand */
	case 'R':	/* type of right operand */
		{
		register NODE *n;
		extern int xdebug;

		n = getlr ( p, c);
		if (xdebug) printf("->%d<-", n->in.type);

		prtype(n);
		return;
		}

	case 'Z':	/* complement mask for bit instr */
		printf("$%ld", ~p->in.right->tn.lval);
		return;

	case 'U':	/* 32 - n, for unsigned right shifts */
		printf("$%d", 32 - p->in.right->tn.lval );
		return;

	case 'T':	/* rounded structure length for arguments */
		{
		int size;

		size = p->stn.stsize;
		SETOFF( size, 4);
		printf("$%d", size);
		return;
		}

	case 'S':  /* structure assignment */
		{
			register NODE *l, *r;
			register size;

			if( p->in.op == STASG ){
				l = p->in.left;
				r = p->in.right;

				}
			else if( p->in.op == STARG ){  /* store an arg into a temporary */
				l = getlr( p, '3' );
				r = p->in.left;
				}
			else cerror( "STASG bad" );

			if( r->in.op == ICON ) r->in.op = NAME;
			else if( r->in.op == REG ) r->in.op = OREG;
			else if( r->in.op != OREG ) cerror( "STASG-r" );

			size = p->stn.stsize;

			if( size <= 0 || size > 65535 )
				cerror("structure size <0=0 or >65535");

			switch(size) {
				case 1:
					fputs("\tmovb\t",stdout);
					break;
				case 2:
					fputs("\tmovw\t",stdout);
					break;
				case 4:
					fputs("\tmovl\t",stdout);
					break;
				case 8:
					fputs("\tmovq\t",stdout);
					break;
				default:
					printf("\tmovc3\t$%d,", size);
					break;
			}
			adrput(r);
			putchar(',');
			adrput(l);
			putchar('\n');

			if( r->in.op == NAME ) r->in.op = ICON;
			else if( r->in.op == OREG ) r->in.op = REG;

			}
		break;
	
	case 'W':		/* convert unsigned => double in A1 */
		{
			int labl;
			
			/*
			cvtld	AL,A1
			jpos	L
			faddd2	2**32,A1
			*/
			fputs("	cvtld	", stdout);
			adrput(getlr(p, 'L'));
			putchar(',');
			adrput(getlr(p, '1'));
			printf("\n\tjgeq\t.L%d\n\taddd2\t$0d4294967296e0,",
			    labl = getlab());
			adrput(getlr(p, '1'));
			putchar('\n');
			deflab(labl);
		}
		break;

	default:
		cerror( "illegal zzzcode" );
		}
	}

rmove( rt, rs, t ){
	printf( "	%s	%s,%s\n",
		(t==FLOAT ? "movf" : (t==DOUBLE ? "movd" : "movl")),
		rnames[rs], rnames[rt] );
	}

struct respref
respref[] = {
	INTAREG|INTBREG,	INTAREG|INTBREG,
	INAREG|INBREG,	INAREG|INBREG|SOREG|STARREG|STARNM|SNAME|SCON,
	INTEMP,	INTEMP,
	FORARG,	FORARG,
	INTEMP,	INTAREG|INAREG|INTBREG|INBREG|SOREG|STARREG|STARNM,
	0,	0 };

setregs(){ /* set up temporary registers */
	fregs = 6;	/* 6 free regs on VAX (0-5) */
	}

szty(t){ /* size, in registers, needed to hold thing of type t */
#ifdef FORT
	return( (t==DOUBLE) ? 2 : 1 );
#else
	return( (t==DOUBLE||t==FLOAT) ? 2 : 1 );
#endif
	}

rewfld( p ) NODE *p; {
	return(1);
	}

callreg(p) NODE *p; {
	return( R0 );
	}

base( p, ty1 ) register NODE *p; int ty1; {
	register int o = p->in.op;

	if( (o==ICON && p->in.name[0] != '\0')) return( R2NOBASE ); /* ie no base reg */
	if( o==REG ) return( p->tn.rval );
    if( (o==PLUS || o==MINUS) && p->in.left->in.op == REG && p->in.right->in.op==ICON)
		return( p->in.left->tn.rval );
    if( o==OREG && !R2TEST(p->tn.rval) && (p->in.type==INT || p->in.type==UNSIGNED || ISPTR(p->in.type)) )
		return( p->tn.rval + 0200*1 );
	if( o==INCR && p->in.left->in.op==REG &&
	    p->in.right->in.op == ICON && p->in.right->tn.lval == ty1)
	   	 return( p->in.left->tn.rval + 0200*2 );
	if( o==ASG MINUS && p->in.left->in.op==REG &&
	    p->in.right->in.op == ICON && p->in.right->tn.lval == ty1)
		/* only if -= by constant size of this type */
		return( p->in.left->tn.rval + 0200*4 );
	if( o==UNARY MUL && p->in.left->in.op==INCR && p->in.left->in.left->in.op==REG
	  && (p->in.type==INT || p->in.type==UNSIGNED || ISPTR(p->in.type)) )
		return( p->in.left->in.left->tn.rval + 0200*(1+2) );
	return( -1 );
	}

offset( p, tyl ) register NODE *p; int tyl; {

	if( tyl==1 && p->in.op==REG && (p->in.type==INT || p->in.type==UNSIGNED) ) return( p->tn.rval );
	if( (p->in.op==LS && p->in.left->in.op==REG && (p->in.left->in.type==INT || p->in.left->in.type==UNSIGNED) &&
	      (p->in.right->in.op==ICON && p->in.right->in.name[0]=='\0')
	      && (1<<p->in.right->tn.lval)==tyl))
		return( p->in.left->tn.rval );
	return( -1 );
	}

r2regs( p, q ) register NODE *p, *q; { /* p is U* q is a +/- on its left */
	register NODE *ql, *qr;
	register i, r;

	ql = q->in.left;
	qr = q->in.right;

	if( q->in.op == PLUS) {
		if( (r=base(ql, tlen(p)))>=0 && (i=offset(qr, tlen(p)))>=0) {
			makeor2(p, ql, r, i);
			return(1);
		} else if( (r=base(qr, tlen(p)))>=0 &&
			   (i=offset(ql, tlen(p)))>=0) {
			makeor2(p, qr, r, i);
			return(1);
			}
		}
	return(0);

	}

makeor2( p, q, b, o) register NODE *p, *q; register int b, o; {
	register NODE *t;
	register int i;
	NODE *f;

	if (tdebug)
		printf( "makeor2(p=0%o, q=0%o, b=%d, o=%d)\n", p, q, b, o );
	p->in.op = OREG;
	f = p->in.left; 	/* have to free this subtree later */

	/* init base */
	switch (q->in.op) {
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

		case INCR:
		case ASG MINUS:
			t = q->in.left;
			break;

		case UNARY MUL:
			t = q->in.left->in.left;
			break;

		default:
			cerror("illegal makeor2");
	}

	p->tn.lval = t->tn.lval;
#ifdef FLEXNAMES
	p->in.name = t->in.name;
#else
	for(i=0; i<NCHNAM; ++i)
		p->in.name[i] = t->in.name[i];
#endif

	/* init offset */
	p->tn.rval = R2PACK( (b & 0177), o, (b>>7) );

	tfree(f);
	return;
	}

canaddr( p ) NODE *p; {
	register int o = p->in.op;

	if( o==NAME || o==REG || o==ICON || o==FCON || o==OREG || (o==UNARY MUL && shumul(p->in.left)) ) return(1);
	return(0);
	}

shltype( o, p ) register NODE *p; {
	return( o== REG || o == NAME || o == ICON || o == FCON || o == OREG || ( o==UNARY MUL && shumul(p->in.left)) );
	}

flshape( p ) register NODE *p; {
	return( p->in.op == REG || p->in.op == NAME || p->in.op == ICON ||
		(p->in.op == OREG && (!R2TEST(p->tn.rval) || tlen(p) == 1)) );
	}

shtemp( p ) register NODE *p; {
	if( p->in.op == STARG ) p = p->in.left;
	return( p->in.op==NAME || p->in.op ==ICON || p->in.op == OREG || (p->in.op==UNARY MUL && shumul(p->in.left)) );
	}

shumul( p ) register NODE *p; {
	register o;
	extern int xdebug;

	if (xdebug) {
		 printf("\nshumul:op=%d,lop=%d,rop=%d", p->in.op, p->in.left->in.op, p->in.right->in.op);
		printf(" prname=%s,plty=%d, prlval=%D\n", p->in.right->in.name, p->in.left->in.type, p->in.right->tn.lval);
		}


	o = p->in.op;
	if( o == NAME || (o == OREG && !R2TEST(p->tn.rval)) || o == ICON ) return( STARNM );

	if( ( o == INCR || o == ASG MINUS ) &&
	    ( p->in.left->in.op == REG && p->in.right->in.op == ICON ) &&
	    p->in.right->in.name[0] == '\0' )
		{
		switch (p->in.left->in.type)
			{
			case CHAR|PTR:
			case UCHAR|PTR:
				o = 1;
				break;

			case SHORT|PTR:
			case USHORT|PTR:
				o = 2;
				break;

			case INT|PTR:
			case UNSIGNED|PTR:
			case LONG|PTR:
			case ULONG|PTR:
			case FLOAT|PTR:
				o = 4;
				break;

			case DOUBLE|PTR:
				o = 8;
				break;

			case STRTY|PTR:
			case UNIONTY|PTR:
				return( 0 );	/* can't use auto inc, dec */

			default:
				if ( ISPTR(p->in.left->in.type) ) {
					o = 4;
					break;
					}
				else return(0);
			}
		return( p->in.right->tn.lval == o ? STARREG : 0);
		}

	return( 0 );
	}

adrcon( val ) CONSZ val; {
	putchar( '$' );
	printf( CONFMT, val );
	}

conput( p ) register NODE *p; {
	switch( p->in.op ){

	case ICON:
		acon( p );
		return;

	case REG:
		fputs( rnames[p->tn.rval], stdout );
		return;

	default:
		cerror( "illegal conput" );
		}
	}

insput( p ) register NODE *p; {
	cerror( "insput" );
	}

upput( p ) register NODE *p; {
	cerror( "upput" );
	}

adrput( p ) register NODE *p; {
	register int r;
	/* output an address, with offsets, from p */

	if( p->in.op == FLD ){
		p = p->in.left;
		}
	switch( p->in.op ){

	case NAME:
		acon( p );
		return;

	case ICON:
		/* addressable value of the constant */
		putchar( '$' );
		acon( p );
		return;
#ifndef FORT
	case FCON:

		printf("$0%c%.20e", p->fpn.type == FLOAT?'f':'d', p->fpn.dval);
		return;
#endif

	case REG:
		fputs( rnames[p->tn.rval], stdout );
		return;

	case OREG:
		r = p->tn.rval;
		if( R2TEST(r) ){ /* double indexing */
			register int flags;

			flags = R2UPK3(r);
			if( flags & 1 ) putchar('*');
			if( flags & 4 ){ /* issue -, turn off auto - */
				putchar('-');
				p->tn.rval &= ~( R2PACK(-1,0,4) );
				}
			if( p->tn.lval != 0 || p->in.name[0] != '\0' ) acon(p);
			if( R2UPK1(r) != R2NOBASE ) printf( "(%s)", rnames[R2UPK1(r)] );
			if( flags & 2 ){ /* issue +, turn off auto +, fix displacement */
				putchar('+');
				p->tn.rval &= ~( R2PACK(-1,0,2) );
				p->tn.lval =  ( flags & 1 ) ? -4 : -tlen(p) ;
				}
			printf( "[%s]", rnames[R2UPK2(r)] );
			return;
			}
		if( r == AP ){  /* in the argument region */
			if( p->tn.lval <= 0 || p->in.name[0] != '\0' ) werror( "bad arg temp" );
			printf( CONFMT, p->tn.lval );
			fputs( "(ap)", stdout );
			return;
			}
		if( p->tn.lval != 0 || p->in.name[0] != '\0') acon( p );
#ifdef FORT			 /* check for "2nd pass" temp */
		if (r == TMPREG && p->in.name[1] == '\1')
			printf("-.FSP%d", ftnno);     /* symbolic addressing */
#endif /*FORT*/
		printf( "(%s)", rnames[p->tn.rval] );
		return;

	case UNARY MUL:
		/* STARNM or STARREG found */
		if( tshape(p, STARNM) ) {
			putchar( '*' );
			adrput( p->in.left);
			}
		else {	/* STARREG - really auto inc or dec */
			register NODE *q;

/************
			p = p->in.left;
			p->in.left->in.op = OREG;
			if( p->in.op == INCR ) {
				adrput( p->in.left );
				putchar( '+' );
				}
			else {
				putchar( '-' );
				adrput( p->in.left );
				}
*************/
			printf("%s(%s)%s", (p->in.left->in.op==INCR ? "" : "-"),
				rnames[p->in.left->in.left->tn.rval], 
				(p->in.left->in.op==INCR ? "+" : "") );
			p->in.op = OREG;
			p->tn.rval = p->in.left->in.left->tn.rval;
			q = p->in.left;
			p->tn.lval = (p->in.left->in.op == INCR ? -p->in.left->in.right->tn.lval : 0);
#ifdef FLEXNAMES
			p->in.name = "";
#else
			p->in.name[0] = '\0';
#endif
			tfree(q);
		}
		return;

	default:
		cerror( "illegal address" );
		return;

		}

	}

acon( p ) register NODE *p; { /* print out a constant */

	if( p->in.name[0] == '\0' ){
		printf( CONFMT, p->tn.lval);
		}
	else if( p->tn.lval == 0 ) {
#ifdef FLEXNAMES
		printf( "%s", p->in.name );
#else
		printf( "%.8s", p->in.name );
#endif
		}
	else {
#ifdef FLEXNAMES
		printf( "%s+", p->in.name );
#else
		printf( "%.8s+", p->in.name );
#endif
		printf( CONFMT, p->tn.lval );
		}
	}

/*
aacon( p ) register NODE *p; { /* print out a constant */
/*

	if( p->in.name[0] == '\0' ){
		printf( CONFMT, p->tn.lval);
		return( 0 );
		}
	else if( p->tn.lval == 0 ) {
#ifdef FLEXNAMES
		printf( "$%s", p->in.name );
#else
		printf( "$%.8s", p->in.name );
#endif
		return( 1 );
		}
	else {
		fputs( "$(", stdout );
		printf( CONFMT, p->tn.lval );
		putchar( '+' );
#ifdef FLEXNAMES
		printf( "%s)", p->in.name );
#else
		printf( "%.8s)", p->in.name );
#endif
		return(1);
		}
	}
 */

genscall( p, cookie ) register NODE *p; {
	/* structure valued call */
	return( gencall( p, cookie ) );
	}

int gc_numbytes;

gencall( p, cookie ) register NODE *p; {
	/* generate the call given by p */
	register NODE *p1, *ptemp;
	register temp, temp1;
	register m;

	if( p->in.right ) temp = argsize( p->in.right );
	else temp = 0;

	if( p->in.op == STCALL || p->in.op == UNARY STCALL ){
		/* set aside room for structure return */

		if( p->stn.stsize > temp ) temp1 = p->stn.stsize;
		else temp1 = temp;
		}

	if( temp > maxargs ) maxargs = temp;
	SETOFF(temp1,4);

	if( p->in.right ){ /* make temp node, put offset in, and generate args */
		ptemp = talloc();
		ptemp->in.op = OREG;
		ptemp->tn.lval = -1;
		ptemp->tn.rval = SP;
#ifdef FLEXNAMES
		ptemp->in.name = "";
#else
		ptemp->in.name[0] = '\0';
#endif
		ptemp->in.rall = NOPREF;
		ptemp->in.su = 0;
		genargs( p->in.right, ptemp );
		ptemp->in.op = FREE;
		}

	p1 = p->in.left;
	if( p1->in.op != ICON ){
		if( p1->in.op != REG ){
			if( p1->in.op != OREG || R2TEST(p1->tn.rval) ){
				if( p1->in.op != NAME ){
					order( p1, INAREG );
					}
				}
			}
		}

/*
	if( p1->in.op == REG && p->tn.rval == R5 ){
		cerror( "call register overwrite" );
		}
 */
/*	setup gc_numbytes so reference to ZC works */

	gc_numbytes = temp;

	p->in.op = UNARY CALL;
	m = match( p, INTAREG|INTBREG );
/********************
	switch( temp ) {
	case 0:
		break;
	case 2:
		fputs( "\ttst\t(sp)+\n", stdout );
		break;
	case 4:
		fputs( "\tcmp\t(sp)+,(sp)+\n", stdout );
		break;
	default:
		printf( "	add	$%d,sp\n", temp);
		}
**********************/
	return(m != MDONE);
	}

char *
ccbranches[] = {
	"	jeql	.L%d\n",
	"	jneq	.L%d\n",
	"	jleq	.L%d\n",
	"	jlss	.L%d\n",
	"	jgeq	.L%d\n",
	"	jgtr	.L%d\n",
	"	jlequ	.L%d\n",
	"	jlssu	.L%d\n",
	"	jgequ	.L%d\n",
	"	jgtru	.L%d\n",
	};

cbgen( o, lab, mode ) { /*   printf conditional and unconditional branches */

	if( o == 0 ) printf( "	jbr     .L%d\n", lab );
	else {
		if( o > UGT ) cerror( "bad conditional branch: %s", opst[o] );
		printf( ccbranches[o-EQ], lab );
		}
	}

nextcook( p, cookie ) NODE *p; {
	/* we have failed to match p with cookie; try another */
	if( cookie == FORREW ) return( 0 );  /* hopeless! */
	if( !(cookie&(INTAREG|INTBREG)) ) return( INTAREG|INTBREG );
	if( !(cookie&INTEMP) && asgop(p->in.op) ) return( INTEMP|INAREG|INTAREG|INTBREG|INBREG );
	return( FORREW );
	}

lastchance( p, cook ) NODE *p; {
	/* forget it! */
	return(0);
	}

optim2( p ) register NODE *p; {
	/* do local tree transformations and optimizations */

	register NODE *r;

	switch( p->in.op ) {

	case AND:
		/* commute L and R to eliminate compliments and constants */
		if( (p->in.left->in.op==ICON&&p->in.left->in.name[0]==0) || p->in.left->in.op==COMPL ) {
			r = p->in.left;
			p->in.left = p->in.right;
			p->in.right = r;
			}
	case ASG AND:
		/* change meaning of AND to ~R&L - bic on pdp11 */
		r = p->in.right;
		if( r->in.op==ICON && r->in.name[0]==0 ) { /* compliment constant */
			r->tn.lval = ~r->tn.lval;
			}
		else if( r->in.op==COMPL ) { /* ~~A => A */
			r->in.op = FREE;
			p->in.right = r->in.left;
			}
		else { /* insert complement node */
			p->in.right = talloc();
			p->in.right->in.op = COMPL;
			p->in.right->in.rall = NOPREF;
			p->in.right->in.type = r->in.type;
			p->in.right->in.left = r;
			p->in.right->in.right = NULL;
			}
		break;

		}
	}

NODE * addroreg(l)
				/* OREG was built in clocal()
				 * for an auto or formal parameter
				 * now its address is being taken
				 * local code must unwind it
				 * back to PLUS/MINUS REG ICON
				 * according to local conventions
				 */
{
	cerror("address of OREG taken");
}



# if !defined( ONEPASS )

main( argc, argv ) char *argv[]; {
	return( mainp2( argc, argv ) );
	}

# endif


struct functbl {
	int fop;
	TWORD ftype;
	char *func;
	} opfunc[] = {
	DIV,		TANY,	"udiv",
	MOD,		TANY,	"urem",
	ASG DIV,	TANY,	"udiv",
	ASG MOD,	TANY,	"urem",
	0,	0,	0 };

hardops(p)  register NODE *p; {
	/* change hard to do operators into function calls.  */
	register NODE *q;
	register struct functbl *f;
	register o;
	register TWORD t;

	o = p->in.op;
	t = p->in.type;

	for( f=opfunc; f->fop; f++ ) {
		if( o==f->fop ) goto convert;
		}
	return;

	convert:

		/* if r&l signed, return, don't do unsigned(int op int)	*/
	if( !( ISUNSIGNED(p->in.left->in.type) ||
		ISUNSIGNED(p->in.right->in.type) ) ) return;

	if( t==UCHAR || t==USHORT ) return;

	/* need to rewrite tree for ASG OP */
	/* must change ASG OP to a simple OP */
	if( asgop( o ) ) {
		q = talloc();
		switch( p->in.op ) {
			case ASG DIV:
				q->in.op = DIV;
				break;
			case ASG MOD:
				q->in.op = MOD;
				break;
		}
		q->in.rall = NOPREF;
		q->in.type = p->in.type;
		q->in.left = tcopy(p->in.left);
		q->in.right = p->in.right;
		p->in.op = ASSIGN;
		p->in.right = q;
		zappost(q->in.left); /* remove post-INCR(DECR) from new node */
		fixpre(q->in.left);	/* change pre-INCR(DECR) to +/-	*/
		p = q;

	}

	/* build comma op for args to function */
	q = talloc();
	q->in.op = CM;
	q->in.rall = NOPREF;
	q->in.type = INT;
	q->in.left = p->in.left;
	q->in.right = p->in.right;
	p->in.op = CALL;
	p->in.right = q;

	/* put function name in left node of call */
	p->in.left = q = talloc();
	q->in.op = ICON;
	q->in.rall = NOPREF;
	q->in.type = INCREF( FTN + p->in.type );
#ifdef FLEXNAMES
	q->in.name = f->func;
#else
	strcpy( q->in.name, f->func );
#endif
	q->tn.lval = 0;
	q->tn.rval = 0;

	return;

	}

zappost(p) NODE *p; {
	/* look for ++ and -- operators and remove them */

	register o, ty;
	register NODE *q;
	o = p->in.op;
	ty = optype( o );

	switch( o ){

	case INCR:
	case DECR:
			q = p->in.left;
			p->in.right->in.op = FREE;  /* zap constant */
			*p = *q;
			q->in.op = FREE;
			return;

		}

	if( ty == BITYPE ) zappost( p->in.right );
	if( ty != LTYPE ) zappost( p->in.left );
}

fixpre(p) NODE *p; {

	register o, ty;
	o = p->in.op;
	ty = optype( o );

	switch( o ){

	case ASG PLUS:
			p->in.op = PLUS;
			break;
	case ASG MINUS:
			p->in.op = MINUS;
			break;
		}

	if( ty == BITYPE ) fixpre( p->in.right );
	if( ty != LTYPE ) fixpre( p->in.left );
}

myreader(p) register NODE *p; {
	walkf( p, hardops );	/* convert ops to function calls */
	canon( p );		/* expands r-vals for fileds */
	walkf( p, optim2 );
	}

special( p, shape ) register NODE *p; {

	switch( shape ){

	case SSCON:
	case SCCON:
		if( p->in.op != ICON || p->in.name[0] ) return(0);
		if( p->tn.lval > -129 && p->tn.lval < 128 && shape == SCCON ) return(1);
		else if( p->tn.lval > -32769 && p->tn.lval < 32768 && shape == SSCON ) return(1);
		else return(0);

	case SSOREG:	/* non-indexed OREG */
		if( p->in.op == OREG && !R2TEST(p->tn.rval) ) return(1);
		else return(0);

	default:

		cerror( "bad special shape" );

		}

	return(0);
	}
