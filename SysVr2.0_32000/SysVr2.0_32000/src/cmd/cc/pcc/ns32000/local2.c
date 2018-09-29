/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
/*	@(#)local2.c	1.5	*/
# include <stdio.h>
# include "mfile2"
# include "ctype.h"
/* a lot of the machine dependent parts of the second pass */

# define BITMASK(n) ((1L<<n)-1)

extern eprint();
static incflag = 0;
static short cc_rev = 0;

where(c){
	fprintf( stderr, "%s, line %d: ", filename, lineno );
	}

lineid( l, fn ) char *fn; {
	/* identify line l and file fn */
	printf( "#	line %d, file %s\n", l, fn );
	}


eobl2(){
	OFFSZ spoff;	/* offset from stack pointer */
#ifdef FORT
	spoff = maxoff;
	if( spoff >= AUTOINIT ) spoff -= AUTOINIT;
	spoff /= SZCHAR;
	SETOFF(spoff,4);
	printf("\t.set\t.F%d,%ld+.FSP%d\n", ftnno, spoff, ftnno);
#else
	extern int reg_use, ftlab1, ftlab2;
   extern char *str_regs[];

	spoff = maxoff;
	if( spoff >= AUTOINIT ) spoff -= AUTOINIT;
	spoff /= SZCHAR;
	SETOFF(spoff,4);
   deflab( ftlab1 );
   printf( "\tenter\t[%s], %d\n", str_regs[reg_use], spoff);
   branch( ftlab2 );
#endif
	maxargs = -1;
	}

struct hoptab { int opmask; char * opstring; } ioptab[] = {

	ASG PLUS, "add",
	ASG MINUS, "sub",
	ASG MUL, "mul",
	ASG DIV, "quo",
   ASG MOD, "rem",
	ASG OR, "or",
	ASG ER,	"xor",
	ASG AND, "and",
	PLUS,	"add",
	MINUS,	"sub",
	MUL,	"mul",
	DIV,	"quo",
   MOD, "rem",
	OR,	"or",
	ER,	"xor",
	AND,	"and",
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
/* temporary fix for d & l */
         if (f == 'D')
            f = 'L';
         else if (f == 'L')
            f = 'D';
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

	"r0", "r1", "r2", "r3",
	"r4", "r5", "r6", "r7",
	"fp", "fp", "sp", "pc",
	"f0", "f2", "f4", "f6",
	};

int rstatus[] = {
	SAREG|STAREG, SAREG|STAREG, SAREG|STAREG, SAREG|STAREG,
	SAREG, SAREG, SAREG, SAREG,
	SBREG, SBREG, SBREG, SBREG,

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

  	return( (tp==FLOAT || tp==DOUBLE) !=
		(tq==FLOAT || tq==DOUBLE) );
}

prtype(n) NODE *n;
{
			/* calls to printf changed to putchar */
	switch (n->in.type)
		{
		case DOUBLE:
			putchar('l');
			return;

		case FLOAT:
			putchar('f');
			return;

		case LONG:
		case ULONG:
		case INT:
		case UNSIGNED:
                case UNDEF:     /* fix for (void) cast */
			putchar('d');
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
				putchar('d');
				return;
				}
		}
}

zzzcode( p, c ) register NODE *p; {
	register m;
	CONSZ val;
	register NODE *l, *r;
	int incval = 1;
	int noname;
	switch( c ){

	case 'N':  /* logical ops, turned into 0-1 */
		/* use register given by register 1 */
		cbgen( 0, m=getlab(), 'I' );
		deflab( p->bn.label );
		printf( "\tmovqd\t0,%s\n", rnames[getlr( p, '1' )->tn.rval]);
		deflab( m );
		return;

	case 'Q':
	case 'I':
	case 'P':
		cbgen( p->in.op, p->bn.label, c );
		return;

	case 'J':	/* cmp? AL, constant */
		l = getlr(p,'L');
		r = getlr(p,'R');
		if ( r->in.name[0] || r->tn.lval < -8 || r->tn.lval >= 8) {
			fputs("\tcmp", stdout);
			prtype(l);
			putchar('\t');
			adrput(l);
			putchar(',');
			adrput(r);
			putchar('\n');
			return;
			}
		/* can optimize to cmpq? constant, AL */
		fputs("\tcmpq", stdout);
		prtype(l);
		printf("\t%d, ", r->tn.lval);
		adrput(l);
		putchar('\n');

	case 'r':
		cc_rev = 1;
		return;

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

	case 'K':	/* if necessary, put left in resc[1] first */
		incflag = 1;
		return;

	case 'D':	/* DECR */
		incval = -1;
	case 'E':	/* INCR */
		r = getlr(p, 'R');
		l = getlr(p, 'L');
		m = incval * r->tn.lval;
		noname = r->in.name[0] == '\0';
		if ( incflag && (tlen(l) != 4 || (! shumul(l)) || (-8192>m) || (m>=8192) ||
			(! noname) || (noname && -8 <= m && m < 8))) {
			zzzcode( l, 'M');
		 	putchar(',');
			adrput( l = getlr(p, '1') );
			putchar('\n');
			}
		if (noname && -8 <= m && m < 8 ) {
			printf("\taddq");
			prtype(l);
			printf("\t%d", m);
			}
		else if(noname&&tlen(l)==4&&shumul(l)&&-8192<=m&&m<8192) {
			printf("\taddr\t%d(", m);
			adrput(l);
			putchar(')');
			}
		else if (noname && m < 0) {
			printf("\tsub");
			prtype(l);
			printf("\t$%d", -m);
			}
		else if ( incval < 0 ) {
			fputs("\tsub", stdout);
			prtype(l);
			putchar('\t');
			adrput(r);
			}
		else if ( noname ) {
			printf("\tadd");
			prtype(l);
			printf("\t$%d", m);
			}
		else {
			printf("\tadd");
			prtype(l);
			putchar('\t');
			adrput(r);
			}
		incflag = 0;
		return;

	case 'G':	/* bit field extract */
		l = getlr(p,'L');
		r = getlr(p,'1');
		printf("\tmovd\t$%d, ", UPKFOFF(p->tn.rval)); adrput(r);
		printf("\n\textd\t"); adrput(r);
		printf(", "); adrput(l); printf(", "); adrput(r);
		printf(", %d\n", UPKFSZ(p->tn.rval));
                if ( ISUNSIGNED(p->in.type) ) {
                        m = UPKFSZ(p->tn.rval);
                        printf("\tandd\t$%d, ", ((unsigned long)1<<m)-1);
                        adrput(r);
                        printf("\n");
                        }
		return;

	case 'H':	/* bit field insert */
		l = getlr(p,'L');
		r = getlr(p,'1');
		printf("\tmovd\t$%d, ", UPKFOFF(l->tn.rval)); adrput(r);
		printf("\n\tins"); prtype( getlr(p,'R') ); printf("\t");
		adrput(r); printf(", "); adrput( getlr(p,'R') ); printf(", ");
		adrput( getlr(l,'L') ); printf(", %d\n", UPKFSZ(l->tn.rval));
		return;

#ifndef FORT
        case 'f':       /* print floating constant as int */
                printf("$%d", (int) p->fpn.dval);
		return;
#endif

	case 'u':		/* convert unsigned => double in A1 */
		{
			int labl;
			
			/*
			movdl	AL,A1
                        tbitd   $31, AL
			bfc 	L
			addl  	2**32,A1
			*/

			fputs("\tmovdl\t", stdout);
			adrput(getlr(p, 'L'));
			putchar(',');
			adrput(getlr(p, '1'));
                        fputs("\n\ttbitd\t$31,", stdout);
                        adrput(getlr(p, 'L'));
			printf("\n\tbfc\t.L%d\n\taddl\t$0l4294967296e0,",
			    labl = getlab());
			adrput(getlr(p, '1'));
			putchar('\n');
			deflab(labl);
		}
		break;

	case 'F':	/* make sure node is type DOUBLE */
		p->in.type = DOUBLE;
		return;

	case 'T':	/* type of operand */
		if (xdebug) printf("->%d<-", p->in.type);
		prtype(p);
		return;


	case 'M':	/* mov?d A? */
		if ( p->in.op == REG ) {
			fputs("\tmovd\t", stdout);
			adrput(p);
			return;
			}
		putmov(p, p->in.type, INT);
		return;

	case 'O':	/* mov?ZRTZLT AR */
		l = getlr(p, 'L');
		r = getlr(p, 'R');
                if (l->in.op != REG)
			putmov(r, r->in.type, l->in.type);
		else if (l->in.type == USHORT && r->in.type == CHAR) {
			putmov(r, CHAR, INT);
			fputs(", ", stdout);
			adrput(l);
			putchar('\n');
			putmov(l, USHORT, INT);
			}
		else if ( tlen(l) <= tlen(r) )
			putmov(r, l->in.type, INT);
		else
			putmov(r, r->in.type, INT);
		fputs(", ", stdout);
		adrput(l);
		putchar('\n');
		return;

	case 'Z':	/* complement mask for bit instr */
		printf("$%ld", ~p->in.right->tn.lval);
		return;

        case 'U':       /* update register variables */
                if ( p->in.op == REG && tlen(p) < sizeof(int) ) {
                        putmov(p, p->in.type, INT);
                        fputs(", ", stdout);
                        adrput(p);
                        putchar('\n');
                        }
                break;

	case 'V':	/* rounded structure length for arguments */
		{
		int size;

		size = p->stn.stsize;
		SETOFF( size, 4);
		if (size <= 63) putchar('b');
		else if (size <= 8191) putchar('w');
		else putchar('d');
		printf("\t$%d", size);
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
/*
			if( r->in.op == ICON ) r->in.op = NAME;
			else if( r->in.op == REG ) r->in.op = OREG;
			else if( r->in.op != OREG ) cerror( "STASG-r" );
*/
			size = p->stn.stsize;

			if( size <= 0 || size > 65535 )
				cerror("structure size <0=0 or >65535");

			switch(size) {
				case 1:
					fputs("\tmovb\t",stdout);
argput:					adrput(r);
					fputs(", ", stdout);
					adrput(l);
					fputs("\n", stdout);
					break;
				case 2:
					fputs("\tmovw\t",stdout);
					goto argput;
				case 4:
					fputs("\tmovd\t",stdout);
					goto argput;
				case 8: case 12: case 16:
					m = 4;
					fputs("\tmovmd\t",stdout);
margput:
					adrput(r);
					putchar(',');
					adrput(l);
					printf(",%d\n", size/m);
					break;
				case 6: case 10: case 14:
					m = 2;
					fputs("\tmovmw\t",stdout);
					goto margput;
				case 3: case 5: case 7: case 9:
				case 11: case 13: case 15:
					m = 1;
					fputs("\tmovmb\t",stdout);
					goto margput;
				default:
					strmov(p,size);
					break;
			}
/*
			if( r->in.op == NAME ) r->in.op = ICON;
			else if( r->in.op == OREG ) r->in.op = REG;
*/
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

strmov(p,size)
   NODE *p;
   int size;
{
   NODE *q;
   int len;

   q = p->in.left;
   if ( q->in.op != OREG || q->tn.rval != R2 )
      if ( canaddr( q ) ) {
         printf("\taddr\t");
         adrput(q);
         printf(", r2\n");
         }
      else
      	order( q, INTAREG);
   q = p->in.right;
   if ( q->in.op != OREG || q->tn.rval != R1 )
      if ( canaddr( q ) ) {
         printf("\taddr\t");
         adrput(q);
         printf(", r1\n");
         }
      else
      	order( q, INTAREG);
   switch ( size % 4 ) {
   case 0:
      len = 4;
      break;
   case 2:
      len = 2;
      break;
   default:
      len = 1;
      }
   if ( size / len < 8 )
      printf("\tmovqd\t%d, r0\n", size/len);
   else
      printf("\taddr\t%d, r0\n", size/len);
   printf("\tmovs%c\n", len == 1 ? 'b' : ( len == 2 ? 'w' : 'd'));
}

rmove( rt, rs, t ){
	printf( "	%s	%s,%s\n",
		(t==FLOAT ? "movf" : (t==DOUBLE ? "movl" : "movd")),
		(t==FLOAT||t==DOUBLE ? rnames[12+rs] : rnames[rs]),
		(t==FLOAT||t==DOUBLE ? rnames[12+rt] : rnames[rt]));
	}

struct respref
respref[] = {
	INTAREG|INTBREG,	INTAREG|INTBREG,
	INAREG|INBREG,	INAREG|INBREG|SOREG|STARREG|STARNM|SNAME|SFLD|SCON,
	INTEMP,	INTEMP,
	FORARG,	FORARG,
	INTEMP,	INTAREG|INAREG|INTBREG|INBREG|SOREG|STARREG|STARNM,
	0,	0 };

setregs(){ /* set up temporary registers */
	fregs = 4;	/* 4 free regs on NS32 (0-3) */
	}

szty(t){ /* size, in registers, needed to hold thing of type t */
	return 1;
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

#ifdef vax

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

#else
	/* code for ibm and interdata */
	register char *cp;
	CONSZ temp;

	ql = q->in.left;
	qr = q->in.right;

	if( q->in.op==PLUS && qr->in.op==REG && ql->in.op==REG &&
			(szty(ql->in.type)==1||szty(qr->in.type)==1) ) {
		temp = 0;
		cp = ql->in.name;
		if( *cp ){
			if( *qr->in.name ) return(1);
			}
		else {
			cp = qr->in.name;
			}
		if( szty(qr->in.type)>1) r = R2PACK(qr->tn.rval,ql->tn.rval, 0);
		else r = R2PACK(ql->tn.rval,qr->tn.rval, 0);
		goto ormake;
		}

	if( (q->in.op==PLUS||q->in.op==MINUS) && qr->in.op==ICON && ql->in.op==PLUS &&
			ql->in.left->in.op==REG &&
			ql->in.right->in.op==REG ){
		temp = qr->tn.lval;
		cp = qr->in.name;
		if( q->in.op == MINUS ){
			if( *cp ) return(1);
			temp = -temp;
			}
		if( *cp ){
			if( *ql->in.name ) return(1);
			}
		else {
			cp = ql->in.name;
			}
		r = R2PACK(ql->in.left->tn.rval,ql->in.right->tn.rval, 0);
		goto ormake;
		}

	ormake:
		if( notoff( p->in.type, r, temp, cp ) ) return(0);
		p->in.op = OREG;
		p->tn.rval = r;
		p->tn.lval = temp;
#ifdef FLEXNAMES
		p->in.name = cp;
#else
		for( i=0; i<NCHNAM; ++i )
			p->in.name[i] = *cp++;
#endif
		tfree(q);
		return(1);
#endif
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

	if( o==NAME || o==REG || o==ICON || o==FCON || o==OREG ) return(1);
	if ( o == UNARY MUL && shumul(p->in.left) )
		return 1;
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
        if ( o == OREG && tlen(p) == 4 && (p->tn.rval == FP || p->tn.rval == AP) )
           return STARNM;
	else if ( o == REG )
           return STARNM;
        if ( o == PLUS && indexable(p) )
                return STARNM;
	if ( (o==MINUS || o==PLUS) && p->in.right->in.op == ICON ) {
		p = p->in.left;
		o = p->in.op;
        	if (o==OREG && tlen(p)==4 && (p->tn.rval==FP || p->tn.rval==AP))
           		return STARNM;
		if ( o == REG )
           		return STARNM;
		}	
	return 0;
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
		if (p->in.type == DOUBLE || p->in.type == FLOAT)
			fputs( rnames[12+p->tn.rval], stdout );
		else
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

		printf("$0%c%.20e", p->fpn.type == FLOAT?'f':'l', p->fpn.dval);
		return;
#endif

	case REG:
		if (p->in.type == DOUBLE || p->in.type == FLOAT)
			fputs( rnames[12+p->tn.rval], stdout );
		else
			fputs( rnames[p->tn.rval], stdout );
		return;

	case OREG:
		r = p->tn.rval;
		if( R2TEST(r) ){ /* double indexing */
			register int flags;

			flags = R2UPK3(r);
			if( flags & 1 ) printf("0(");
			if( flags & 4 ) printf("-");
			acon(p);
			if( R2UPK1(r) != 100) printf( "(%s)", rnames[R2UPK1(r)] );
			if( flags & 1 ) printf(")");
			printf( "[%s:", rnames[R2UPK2(r)]);
         		prtype(p);
         		printf( "]" );
			return;
			}
		acon( p );
#ifdef FORT
		if (r == TMPREG && p->in.name[2] == '\1')
			printf("-.FSP%d", ftnno);
#endif /* FORT */
		printf( "(%s)", rnames[p->tn.rval] );
		return;

	case UNARY MUL:
		/* STARNM or STARREG found */
                if (! indexput(p->in.left) ) {
        		switch ( p->in.left->in.op ) {
        		case MINUS:
        			putchar('-');
        		case PLUS:
        			acon( p->in.left->in.right );
        			putchar('(');
        			adrput( p->in.left->in.left );
        			break;
        		default:
                 		printf( "0(" );
        			adrput( p->in.left);
        			}
        		putchar(')');
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
   if ( temp == 0 )
           ;
   else if ( temp < 64 )
           printf("\tadjspb\t$-%d\n", temp);
   else if ( temp < 8192 )
           printf("\tadjspw\t$-%d\n", temp);
   else
           printf("\tadjspd\t$-%d\n", temp);
	return(m != MDONE);
	}

char *
ccodes[] =
	{ "eq", "ne", "le", "lt", "ge", "gt", "ls", "lo", "hs", "hi" };
char *
rv_ccodes[] =
	{ "eq", "ne", "ge", "gt", "le", "lt", "hs", "hi", "ls", "lo" };

cbgen( o, lab, mode ) { /*   printf conditional and unconditional branches */
   char **cc;

   cc = cc_rev ? rv_ccodes : ccodes;
   cc_rev = 0;

   if ( mode == 'Q' )
      printf("\ts%s", cc[o-EQ]);
   else if ( o == 0 )
      printf( "\tbr\t.L%d\n", lab );
	else {
		if( o > UGT ) cerror( "bad conditional branch: %s", opst[o] );
		printf( "\tb%s\t.L%d\n", cc[o-EQ], lab );
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

	if ( AND == p->in.op ) {
		/* commute L and R to eliminate compliments and constants */
		if( (p->in.left->in.op==ICON&&p->in.left->in.name[0]==0) || p->in.left->in.op==COMPL ) {
			r = p->in.left;
			p->in.left = p->in.right;
			p->in.right = r;
			}
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



# if !defined( ONEPASS ) && !defined( FONEPASS )

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

char c_type(t)
   int t;
{
   if ( t == SHORT )
      return 'w';
   if ( t == CHAR )
      return 'b';
   return 'd';
}

putmov(l,lt,rt)
   NODE *l;
   int lt,rt;
{
   int xp;

   rt = ISUNSIGNED(rt) ? DEUNSIGN(rt) : rt;
   if ( xp = ISUNSIGNED(lt) )
      lt = DEUNSIGN(lt);
   if ( l->in.op == ICON && l->in.name[0] == '\0' ) {
      if ( l->tn.lval >= -8 && l->tn.lval <= 7 ) {
         printf("\tmovq%c\t%d", c_type(rt), l->tn.lval);
         return;
         }
      else if ( l->tn.lval > 0 && l->tn.lval < 8192 && (rt == INT || rt == LONG || ISPTR(rt))) {
         printf("\taddr\t%d", l->tn.lval);
         return;
         }
      else if ( l->tn.lval >= -128 && l->tn.lval <= 127 )
         lt = CHAR;
      else if ( l->tn.lval >= -32768 && l->tn.lval <= 32767 )
         lt = SHORT;
      }
   printf("\tmov");
   if (lt < rt && lt < INT) {
      if ( xp )
         putchar('z');
      else
         putchar('x');
      printf("%c", c_type(lt));
      }
   printf("%c\t", c_type(rt));
   adrput(l);
}

indexable(p) register NODE *p; {
   register NODE *l = p->in.left;
   register NODE *r = p->in.right;
   register NODE *q;
   int t = BTYPE(p->in.type);

/* what about PTR PTR INT? etc?
 * if ( p->in.type != (PTR | t) )
 *    return 0;
 */
   t = ISUNSIGNED(t) ? DEUNSIGN(t) : t;
   if ( t < CHAR || t > LONG )
      return 0;
   if ( l->in.op == LS && r->in.op == ICON ) {
      q = r;
      r = p->in.right = l;
      l = p->in.left = q;
      }
   q = ( r->in.op == LS ) ? r->in.left : r;
   if ( r->in.op == LS )
      if ( r->in.right->in.op != ICON )
         return 0;
      else switch ( r->in.right->tn.lval ) {
      default:
         return 0;
      case 1: case 2:
         ;
         }
   if ( (r->in.op != LS && t != CHAR) || q->in.op != REG )
      return 0;
   if ( l->in.op == ICON || l->in.op == REG )
      return 1;
   if ( l->in.op == OREG && (l->tn.rval==FP || p->tn.rval==AP))
      return 1;
   if ( (l->in.op == MINUS || l->in.op == PLUS) && l->in.right->in.op == ICON ){
      q = l->in.left;
      if ( q->in.op == REG || q->in.op == OREG && tlen(q) == 4 && (q->tn.rval == FP || q->tn.rval == AP) )
            return 1;
      }
   return 0;
}

indexput(p) register NODE *p; {
        register NODE *l;
        register NODE *r;
        char buff[8];

        if ( p->in.op != PLUS )
                return 0;
        l = p->in.left;
        r = p->in.right;
        switch ( r->in.op ) {
        case REG:
                sprintf(buff, "[%s:b]", rnames[r->tn.rval]);
                break;
        case LS:
                switch ( r->in.right->tn.lval ) {
                case 1:
                        sprintf( buff, "[%s:w]", rnames[r->in.left->tn.rval]);
                        break;
                case 2:
                        sprintf( buff, "[%s:d]", rnames[r->in.left->tn.rval]);
                        break;
                default:
                        return 0;
                        }
                break;
        default:
                return 0;
                }
        switch ( l->in.op ) {
        case ICON:
                acon( l );
                break;
        case REG:
		fputs( rnames[l->tn.rval], stdout );
		break;
        case OREG:
                fputs( "0(" , stdout );
                adrput( l );
                putchar(')');
                break;
        case MINUS:
                putchar('-');
        case PLUS:
                acon ( l->in.right );
                putchar('(');
                adrput( l->in.left );
                putchar(')');
                break;
        default:
                return 0;
                }
        fputs( buff, stdout);
        return 1;
}
