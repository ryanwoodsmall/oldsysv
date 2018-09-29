/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)comp:u3b/local2.c	1.20.3.1"
# include "mfile2"
# include "ctype.h"

/*	@(#) local2.c: 1.20.3.1 11/26/85	*/

#ifdef ONEPASS
FILE *outfile;
#else
FILE *outfile = stdout;
#endif

where(c){
	fprintf( stderr, "%s, line %d: ", filename, lineno );
	}

lineid( l, fn ) char *fn; {
	/* identify line l and file fn
	 */
	printf( "#	line %d, file %s\n", l, fn );
	}

eobl2(){
	/* end of function - output max offset for autos and temps
	 */
	OFFSZ spoff;	/* offset from stack pointer */
#ifndef FORT
# ifdef ONEPASS
	extern char fncname[];
# else
#ifdef FLEXNAMES
	char fncname[BUFSIZ + 1];
#else
	char fncname[NCHNAM + 1];
#endif /* FLEXNAMES */
	int i;
	char c;

	i = 0;
	getchar();	/* pass the tab character */
	while( ( c = getchar() ) != '\t' )
		fncname[i++] = c;
	fncname[i] = '\0';
# endif /* ONEPASS */
#endif  /* FORT */

	spoff = maxoff;
	if( spoff >= AUTOINIT ) spoff -= AUTOINIT;
	spoff /= SZCHAR;
	SETOFF(spoff,4);
#ifdef FONEPASS
	printf( "	.set	.F%d,%ld%s%d\n", ftnno, spoff, FSPOFF, ftnno);
#else
	printf( "	.set	.F%d,%ld\n", ftnno, spoff );
#endif
#ifndef FORT
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
	ASG MOD, "mod",
	ASG OR, "or",
	ASG ER,	"xor",
	ASG AND, "and",
	PLUS,	"add",
	MINUS,	"sub",
	MUL,	"mul",
	DIV,	"div",
	MOD,	"mod",
	OR,	"or",
	ER,	"xor",
	AND,	"and",
	-1, ""    };

hopcode( f, o ){
	/* output the appropriate string from the above table
	 */
	register struct hoptab *q;

# ifdef FLT
	if( f == 'D' )	/* Floating point operation--Double precision */
		printf( "f" );
#ifdef SPFORT
	else if ( f == 'S') /* FP operation--Single precision (f77 only) */
		printf( "f" );
#endif /* SPFORT */
# endif /*FLT*/
	for( q = ioptab;  q->opmask>=0; ++q ){
		if( q->opmask == o ){
			printf( "%s", q->opstring );
			switch( f ){

# ifdef FLT
			case 'D':
#ifdef SPFORT
			case 'S':
#endif /* SPFORT */
# endif /*FLT*/
			case 'W':
			case 'H':
			case 'B':
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

	"%r0", "%r1",
	"%r2", "%r3", "%r4",
	"%r5", "%r6", "%r7", "%r8",
	"%ap", "%fp", "%sp",
# ifdef INDEX
	"",	/* for RDUMMY */
# endif
	};

int rstatus[] = {
	SAREG|STAREG, SAREG|STAREG, SAREG|STAREG,
	SAREG|STAREG, SAREG, SAREG, SAREG, SAREG, SAREG,
	SAREG, SAREG, SAREG,
# ifdef INDEX
	0,	/* for RDUMMY */
# endif

# ifdef FLT
	SBREG|STBREG, SBREG|STBREG, SBREG|STBREG,
	SBREG|STBREG, SBREG|STBREG
# endif /*FLT*/
	};

struct respref
respref[] = {
	INTAREG|INTBREG,	INTAREG|INTBREG,
	INAREG|INBREG,	INAREG|INBREG|SOREG|STARNM|SNAME|SCON,
	INTEMP,	INTEMP,
	FORARG,	FORARG,
	INTEMP,	INTAREG|INAREG|INTBREG|INBREG|SOREG|STARNM,
	0,	0 };

tlen( p ) NODE *p; {

	switch( p->in.type ){

	case CHAR:
	case UCHAR:
		return(1);

	case SHORT:
	case USHORT:
		return(2);

# ifdef FLT
	case FLOAT:
		return(8);

	case DOUBLE:
		return(16);
# endif /*FLT*/

	default:
		return(4);
		}
	}

# ifdef FLT
# ifndef MOVD
/* This code moves a double from 'from' to 'to'.  It can handle
** sharing of the destination.  The code for handling UMUL over
** something appears dubious, but never seems to be called.
*/

movd( from, to)	/* move a double from 'from' to 'to' */
register NODE *from, *to;
{
	/* peel out special case */

	if( from->in.op == UNARY MUL || to->in.op == UNARY MUL ) {
		mvdoub(to, from);
		return;
	}

	/* Each operand may be a register or OREG.  That gives four
	** cases.  The first word to be moved may always be moved
	** by "movw AR,AL".  Do the moves backwards -- it makes the
	** most common case of sharing work better:  movd n(%r0),%r0.
	*/

	printf("movw");			/* start things off */
	if (from->in.op == REG && to->in.op == REG) {
		from->tn.rval++; to->tn.rval++; /* second reg. first */
		props( to, from );	/* print operands */
		from->tn.rval--; to->tn.rval--;
	}
	/* two OREG case */
	else if ( from->in.op != REG && to->in.op != REG ) {
		from->tn.lval += 4; to->tn.lval += 4; /* second word first */
		props( to, from );
		from->tn.lval -= 4; to->tn.lval -= 4;
	}
	/* the messy case if sharing:  one REG, one OREG.  Check whether
	** we have a case like:  movd n(%r1), %r0.  To make life easier
	** we will move %r1 to %r0.  However, making the move is messy.
	*/

	else {
		register NODE * reg;	/* pointer to register side */
		register NODE * oreg;	/* pointer to OREG side */

		if (from->in.op == REG) {
			reg = from; oreg = to;
		}
		else {
			oreg = from; reg = to;

			/* check for messy case */

			if ( to->tn.rval == from->tn.rval + 1 ) {
				from->tn.rval--; /* OREG pointer will have moved */
				/* movw already out */
				printf("\t%s,%s\n\tmovw",
					rnames[from->tn.rval+1], rnames[from->tn.rval]);
			}
		}
		/* do second word first */

		oreg->tn.lval += 4; reg->tn.rval++;
		props( to, from );
		oreg->tn.lval -= 4; reg->tn.rval--;
	}

	/* now do first word */

	printf("\n\tmovw");
	props( to, from );
}

pshdoub(p)	/* push contents of pointer to double on the stack */
NODE *p;	/* Called only for 3B5--3B20 uses pushd */
{
	printf("pushw	");
	adrput( p );
	printf( "\n" );
	expand( p,FOREFF,"	movw	AL,%r0\n" );
	expand( p,FOREFF,"	addw2	&4,%r0\n" );
	expand( p,FOREFF,"	pushw	0(%r0)" );
}

mvdoub(l,r)	/* access the content of pointer to double */
NODE *l, *r;
	{
	printf("movw");
	props( l, r );
	if( l->in.op == UNARY MUL )
		{
		expand( l,FOREFF,"\n	movw	AL,%r0\n" );
		expand( l,FOREFF,"	addw2	&4,%r0\n" );
		if( r->in.op == UNARY MUL )
			{
			expand( r,FOREFF,"	movw	AL,%r1\n" );
			expand( r,FOREFF,"	addw2	&4,%r1\n" );
			fputs( "\tmovw\t0(%r1),0(%r0)", outfile);
			/*expand( p,FOREFF,"	movw	0(%r1),0(%r0)\n" );*/
			}
		else
			{
			r->tn.lval += 4;
			/*expand( p,FOREFF,"	movw	AR,0(%r0)\n" );*/
			fputs( "	movw	", outfile);
			adrput(r);
			fputs ( ",0(%r0)", outfile );
			r->tn.lval -= 4;
			}
		}
	else
		{
		expand( r,FOREFF,"\n	movw	AL,%r0\n" );
		expand( r,FOREFF,"	addw2	&4,%r0\n" );
		l->tn.lval += 4;
		printf( "\tmovw\t0(%%r0),");
		adrput( l );
		l->tn.lval -= 4;
		}
	}

# endif /*MOVD*/
# endif /*FLT*/
props( l, r ) NODE *l, *r;{
	printf( "	" );
	adrput( r );
	printf( "," );
	adrput( l );
	return;
}

prtype( n ) NODE *n; {

	switch( n->in.type ){

	case LONG:
	case ULONG:
	case INT:
	case UNSIGNED:
	printf("w");
	return;

	case SHORT:
	case USHORT:
		printf("h");
		return;

	case CHAR:
	case UCHAR:
		printf("b");
		return;

# ifdef FLT
	case FLOAT:
		printf( "s" );
		return;

	case DOUBLE:
		printf( "d" );
		return;
# endif /*FLT*/

	default:
		if ( !ISPTR( n->in.type ) ) {
			char bb[50];
			sprintf(bb,"zzzcode(prtype): bad type 0%o",n->in.type);
			cerror(bb);
		}
		else {
			printf("w");
			return;
			}
		}
	}

int cmpflag = 0;	/* set when cmp generated to control jump generation */
#if TV && !ONEPASS
	int	tvflag;
#endif

zzzcode( p, c ) register NODE *p; {
/* expand Z macros in code strings;
* c is character following Z
*/
	register m;
# ifdef TV
	extern int tvflag;
# endif /*TV*/

	switch( c ){

	case 'B':	/* Normal pushes-Need this special case to do
			   pushaw's for labels for PASCAL */
		if (p->in.op == ICON && p->in.name[0] == '.' &&
			p->in.name[1] == 'L')  {
			printf("pushaw	%s\n", p->in.name);
		} else {
			expand( p, FOREFF, "pushw	AR\n" );
		}
		break;
	case 'N':	/* logical ops, setting destination to 0 or 1 */
		{
		register char *n;

		printf( "	movw	&1,%s\n", n=rnames[getlr( p, '1' )->tn.rval] );
		cbgen( 0, m=getlab(), 'I' );
		deflab( p->bn.label );
		printf( "	movw	&0,%s\n", n );
		deflab( m );
		return;
		}

	case 'P':	/* conditional branch following compare */
		cbgen( p->in.op, p->bn.label, c );
		return;

# ifdef FLT
# ifndef MOVD
	case 'p':	/* push floating point argument on stack */
		if( p->in.type == FLOAT )
		{
			int inreg = 0;

			expand( p, FORARG, "movsd	AR,A1\n" );
			expand( p,FORARG, "	pushw	A1\n" );
			if (resc[0].in.op == REG)  {
			/* either a REG */
				inreg = 1;
				resc[0].tn.rval++;
			} else			/* or an OREG */
				resc[0].tn.lval += 4;
			/* push 2nd word */
			expand( p,FORARG, "\tpushw\tA1");
			if (inreg)	/* get back to where we were */
				resc[0].tn.rval--;
			else
				resc[0].tn.lval -= 4;
			}
		else {
			if( p->in.op == UNARY MUL ) 
				pshdoub(p);
			else 	{
				register int inreg = 0;

				printf( "pushw	");
				adrput( p );
				printf( "\n" );
				if (p->in.op == REG)
				{
					p->tn.rval++;
					inreg = 1;
				} else
					p->tn.lval += 4;
				printf( "	pushw	");
				adrput( p );
				if (inreg)
					p->tn.rval--;
				else
					p->tn.lval -= 4;
				}
			}
		return;
	case 'm':	/* move a double from R to 1 */
		movd( getlr( p, 'R'), getlr(p, '1'));
		return;
# endif /*MOVD*/
# endif /*FLT*/

	case 'Q':	/* compare preceding conditional branch */
		{
# ifdef FLT
		register NODE *l, *r;
		r = getlr( p, 'R' );
		l = getlr( p, 'L' );
		if( ( r->in.type == FLOAT ) || (l->in.type == DOUBLE ) )
			printf( "fcmp" );
		else
# endif /*FLT*/
			printf( "cmp" );
			cmpflag++;
			return;
			}

	case 'A':	/* assignment */
		{
		register NODE *l, *r;

		r = getlr( p, 'R' );
		if( optype(p->in.op) == LTYPE ||
		    p->in.op == UNARY MUL ){
			l = getlr( p, '1' );
# ifdef FLT
			if ( p->in.type == FLOAT ||
			   p->in.type == DOUBLE )
				l->in.type = p->in.type;
			else
# endif /*FLT*/
				l->in.type = (ISUNSIGNED(p->in.type) ||
				  p->in.type == CHAR) ? UNSIGNED : INT;
		} else
			l = getlr( p, 'L' );
		if( r->in.op == ICON ) {
			if ( r->in.name[0] == '\0' ){
				r->in.type = ( r->tn.lval < 0 ?
					(r->tn.lval >= -128 ? CHAR
					: (r->tn.lval >= -32768 ? SHORT
					: INT ))
					: (r->tn.lval <= 127 ? CHAR
					: (r->tn.lval <= 255 ? UCHAR
					: (r->tn.lval <= 32767 ? SHORT
					: (r->tn.lval <= 65535L ? USHORT
					: INT )))) );
			if( tlen(l) > tlen(r) )
				r->in.type = l->in.type;
			}
		else
			r->in.type = INT;
		}
		if( tlen(l) == tlen(r) ){
# ifdef TV
			if( tvflag && r->in.op == ICON &&
					r->in.name[0] != '\0') {
				TWORD t;
				t = r->in.type & ~BTMASK;
				if( t == FTN ||
					(t == ((FTN << TSHIFT) | PTR)))
#ifdef FLEXNAMES
					printf(".tv	%s\n	",r->in.name);
#else
					printf(".tv	%.8s\n	",r->in.name);
#endif
			}
# endif /*TV*/
			/* optimizer wants movaw's for labels */
			/* this should only occur for f77 */
			if ((r->in.op == ICON) && (r->in.name[0] == '.')
			    && (r->in.name[1] == 'L'))  {
				printf("movaw	%s,",
					r->in.name);
				adrput(l);
				return;
			}
			if (l->in.op == REG && l->in.type == USHORT) {
				expand( p, FOREFF, "movzhw	AR,AL");
				return;
			}
			if (l->in.type != DOUBLE) {
				printf( "mov" );
				if( l->in.type == FLOAT)
					printf( "w" );
				else
					prtype( l );
				props( l, r );
				}
# ifdef FLT
			else
# ifdef MOVD
				expand(p, FOREFF, "movd	AR,AL");
# else  /*MOVD*/
				movd(r,l);
# endif /*MOVD*/
# endif /*FLT*/
			return;
			}
# ifdef FLT
		/* Floating point: single || double = single || double 
		   || int */
		if (floating( l )) printf( "mov" );
		else if( floating( r )) printf( "movt" );
# endif /*FLT*/
		else if( tlen(l) > tlen(r) ){
			if( ISUNSIGNED(r->in.type) || r->in.type == CHAR )
				printf( "movz" );
			else
				printf( "movb" );
			}
		else {
			printf( "movt" );
			if( l->in.op==REG && l->in.type==USHORT &&
				tlen(r)==4 ){

				/* zero out sign extended bits */

				expand(p, FOREFF, "wh	AR,AL\n	movzhw	AL,AL");
				return;
			}
		}
		prtype( r );
		prtype( l );
		props( l, r );
		return;
		}

	case '1':	/* triadic into temp reg, try to make dyadic */
		{
		register NODE *a1;

		a1 = getlr( p, '1' );
		if( same( p->in.left, a1 ) )
			expand( p, FOREFF, "2	AR,A1" );
		else if( same( p->in.right, a1 )
			&& p->in.op!=MINUS && p->in.op!=DIV && p->in.op!=MOD )
			expand( p, FOREFF, "2	AL,A1" );
		else
			expand( p, FOREFF, "3	AR,AL,A1" );
		return;
		}

	case '2':	/* dyadic assignop */
		if( p->in.right->in.op == UNARY MINUS )
			expand( p->in.right, FOREFF, "mnegZL	AL" );
		else
			expand( p->in.right, FOREFF, "mcomZL	AL" );
		return;

	case '3':	/* triadic assignop, try to make dyadic */
	{
		register NODE *r;

		r = p->in.right;
		if( r->in.op==LS )
		expand( r, FOREFF, "llsw3	AR,AL" );
	else if ( r->in.op==RS )
		expand( r, FOREFF, "lrsw3	AR,AL" );
	else if( (r->in.op==PLUS || r->in.op==MINUS)
		&& (r->in.left->in.op==REG && r->in.right->in.op==ICON
		&& tlen(p->in.left)==4) ){
		if( r->in.op == MINUS )
			r->in.right->tn.lval = -r->in.right->tn.lval;
		expand( r, FOREFF, "movaw	CR(AL)" );
		}
	else if( same( p->in.left, r->in.left ) )
		expand( r, FOREFF, "ZUO ZL2	AR" );
	else if( same( p->in.left, r->in.right )
		&& r->in.op!=MINUS && r->in.op!=DIV && r->in.op!=MOD )
		expand( r, FOREFF, "ZUO ZR2	AL" );
	else {
			/* pass type of triadic op down */
		r->in.left->in.type = p->in.type;
		expand( r, FOREFF, "ZUO ZL3	AR,AL" );
		}
	return;
	}

	case 'T':
		/* number of words pushed on arg stack, checks for tv */
		{
		extern int callsz;

		printf( "	call	&%d,", callsz/(SZLONG/SZCHAR) );
		callsz = 0;
# ifdef TV
		if (tvflag)
			printf("*");
# endif /*TV*/
		return;
		}

	case 'C':	/* number of words pushed on arg stack */
		{
		extern int callsz;

		printf( "&%d", callsz/(SZLONG/SZCHAR) );
		callsz = 0;
		return;
		}

	case 'c':	/* number of bytes pushed on arg stack */
	{
		extern int callsz;

		printf( "%d", callsz );
		return;
	}

	case 'D':	/* INCR and DECR */
		zzzcode( p->in.left, 'A' );
		return;

# ifdef FLT
	case 'F':	/* floating type SCONV */
		p->in.right->in.op = FREE;
		p->in.right = p->in.right->in.left;
		return;
# endif /*FLT*/

case 'L':	/* type of left operand */
case 'R':	/* type of right operand */
	{
		register NODE *n;
		extern int xdebug;

		n = getlr( p, c );
		if( xdebug ) printf( "->%d<-", n->in.type );

# ifdef FLT
	if( floating( n ))
		prtype( n );
	else
# endif /*FLT*/
	prtype( n );
	return;
	}

	case 'l':
		/* zero or bit extend depending on type of left operand
		 * truncate if operand is word length
		 */
	{
		register NODE *n;

		n = getlr( p, 'L' );
# ifdef FLT
		if( floating( n )) return;
# endif /*FLT*/
		if( tlen(n) > tlen( getlr(p,'R') ) )
			printf( "t" );
		else if( ISUNSIGNED(n->in.type) || n->in.type == CHAR )
			printf( "z" );
		else
			printf( "b" );
		return;
	}

case 'r':	/* zero or bit extend depending on type of right operand */
	{
		register NODE *n;

		n = getlr( p, 'R' );
		if( tlen(n) >= 4 ) return;
		if( ISUNSIGNED(n->in.type) || n->in.type == CHAR )
			printf( "z" );
		else
			printf( "b" );
		return;
	}

	case 'U':	/* signed/unsigned arithmetic operators */
	{
		switch( p->in.op )
			{
			case ASG MUL:
			case MUL:
			case ASG DIV:
			case DIV:
			case ASG MOD:
			case MOD:
				if( ISUNSIGNED(p->in.left->in.type) || ISUNSIGNED(p->in.right->in.type) )
					printf( "u" );
			}
		return;
	}

	case 'W':		/* convert unsigned => double in A1 */
	{
		int dlabl = getlab();
		int labl = getlab();
		
		/*				.data
						.align	4
					L1:
		movwd	AL,A1			.double	4294967296e0
		jnneg	L	-->		.text
		faddd2	2**32,A1		movwd	AL,A1
						jnneg	L2
						faddd2	L1,A1
					L2:
		*/
		fputs( "	.data\n	.align	4\n", outfile);
		fprintf(outfile, 
		    ".L%d:\n	.double	4294967296e0\n	.text\n",dlabl);
		fputs( "	movwd	", outfile);
		adrput(getlr(p, 'L'));
		putc(',', outfile);
		adrput(getlr(p, '1'));
		fprintf(outfile, "\n\tjnneg\t.L%d\n\tfaddd2\t.L%d,",
			labl, dlabl);
		adrput(getlr(p, '1'));
		putc('\n', outfile);
		deflab(labl);
	}
	break;

	case 'S':  /* structure assignment */
	{
		register NODE *l, *r;
		register size, align, count;
		char *suffix;
		OFFSZ loff, roff;
		int msrc, mdst;

		if( p->in.op == STASG ){
			l = p->in.left;
			r = p->in.right;
			}
		else if( p->in.op == STARG ){
			/* store an arg into a temporary */
			l = getlr( p, '1' );
			r = p->in.left;
			}
		else cerror( "STASG bad" );

		if( r->in.op == ICON ) r->in.op = NAME;
		else if( r->in.op == REG ) r->in.op = OREG;
		else if( r->in.op != OREG ) cerror( "STASG-r" );

		size = p->stn.stsize;
		if( size <= 0 || size > 65535 )
			cerror("structure size <0=0 or >65535");

		align = p->stn.stalign;
		if( align > 4 ) align = 4;
		count = size / align;

		if( align == 1 ) suffix = "b";
		else if( align == 2 ) suffix = "h";
		else suffix = "w";

		if( size == 2 && l->in.op == OREG && l->tn.rval == SP )
			suffix = "zhw";	/* a small struct arg */

		msrc = (r->in.op!=OREG || r->tn.rval!=R0 || r->tn.lval!=0 || r->in.name[0]!='\0') ? 1 : 0;
		mdst = (l->in.op!=OREG || l->tn.rval!=R1 || l->tn.lval!=0 || l->in.name[0]!='\0') ? 1 : 0;
		if( count <= msrc+mdst+2 ){
			loff = l->tn.lval;
			roff = r->tn.lval;
			while( count-- ){
				printf( "	mov%s	", suffix );
				adrput( r );
				printf( "," );
				adrput( l );
				printf( "\n" );
				l->tn.lval += align;
				r->tn.lval += align;
				}
			l->tn.lval = loff;
			r->tn.lval = roff;
			}
		else {
			if( mdst ){
				if( r->in.op==OREG && r->tn.rval==R1 )
					cerror( "overwriting %r1 in structure assignment" );
				printf( "	movaw	");
				adrput( l );
				printf( ",%s\n", rnames[R1] );
				}
			if( msrc ){
				printf( "	movaw	" );
				adrput( r );
				printf( ",%s\n", rnames[R0] );
				}
			printf( "	movw	&%d,%s\n	movbl%s\n", count, rnames[R2], suffix );
			}

		if( r->in.op == NAME ) r->in.op = ICON;
		else if( r->in.op == OREG ) r->in.op = REG;

		}
	break;

	case 's':	/* long to unsigned short conversion */
	{
		NODE *l;

		l = getlr(p,'L');
		(l->in.op == REG) ? expand(p,FOREFF,"movzhw	AL,A1"):
			expand(p,FOREFF,"movtwh	AL,A1\n	movzhw	A1,A1");
		return;
	}

	case 'E':	/* lhs of assignment is receiving value of struct call */
		if( p->in.right->in.left->in.op == ICON )
# ifdef TV
		tvflag ? expand( p->in.right, FOREFF, "	.tv	CL\n	call	ZC,*CL\n" ) :
# endif /*TV*/
			expand( p->in.right, FOREFF, "	call	ZC,CL\n" );
		else if( p->in.right->in.left->in.op == REG )
# ifdef TV
			tvflag ? expand( p->in.right, FOREFF, "	call	ZC,*0(AL)\n" ):
# endif /*TV*/
			expand( p->in.right, FOREFF, "	call	ZC,0(AL)\n" );
		else
# ifdef TV
		tvflag ? expand( p->in.right, FOREFF, "	movw	AL,%r0\n	call	ZC,*0(%r0)\n" ):
# endif /*TV*/
		expand( p->in.right, FOREFF, "	call	ZC,*AL\n" );
		recl2(p->in.right->in.left);
		p->in.right->in.left->in.op = FREE;
		p->in.right->in.op = REG;
		p->in.right->tn.rval = R0;
		p->in.right->tn.lval = 0;	/* call has been generated, fake reclaim */
#ifdef FLEXNAMES
		p->in.right->in.name = "";
#else
		p->in.right->in.name[0] = '\0';
#endif
		rbusy( R0, INT );
		break;

# ifdef FLT
	case 'Y': 

#ifdef SPFORT
		if (p->in.type == FLOAT)
			expand (p, FOREFF, "Tmovw	%r0,A3\n");
		else
#endif /* SPFORT */
			expand(p, FOREFF, "movw\t%r0,A3\n\tmovw\t%r1,A3+4");
break;

# endif /*FLT*/

	default:
		cerror( "illegal zzzcode" );
		}
	}

# ifdef FLT
floating( n ) NODE *n; {

	return( n->in.type == FLOAT || n->in.type == DOUBLE);
}
# endif /*FLT*/

same( p, q ) register NODE *p, *q; {
	/* are given leaf nodes identical
	*/
	int i;

	if( p->in.op == q->in.op && p->tn.rval == q->tn.rval && p->tn.lval == q->tn.lval ){
#ifdef FLEXNAMES
	if ( p->in.name == q->in.name )
#else
#ifdef FONEPASS
	for( i=0; i<NCHNAM &&
		(q->in.name[i] != '\0' || p->in.name[i] != '\0'); i++)
#else
	for( i=0; i<NCHNAM && p->in.name[i] != '\0'; i++)
#endif /*FONEPASS */
			
	if( p->in.name[i] != q->in.name[i] ) return(0);
#endif
		return(1);
		}
	return(0);
	}

rmove( rt, rs, t ) TWORD t; {
# ifdef FLT
	if( t == DOUBLE )  {
#  ifdef MOVD
		printf( "\tmovd	%s,%s\n", rnames[rs], rnames[rt] );
#  else
		printf( "\tmovw	%s,%s\n", rnames[rs], rnames[rt] );
		printf( "\tmovw	%s,%s\n", rnames[rs+1], rnames[rt+1] );
#  endif /*MOVD*/
	} else   /* else not double */
# endif /*FLT*/
		printf( "	movw	%s,%s\n", rnames[rs], rnames[rt] );
}

adrcon( val ) CONSZ val; {
	printf( "&" );
	printf( CONFMT, val );
	}

acon( p ) register NODE *p; {
/* print out a constant
*/
	if( p->in.name[0] == '\0' )
	/* negative character constants are not really negative on 3Bs
	/* support unsigned characters. cf. "cmpb foo,&-1" problem.
	*/
	{
		if (p->in.op == ICON && (p->tn.type==CHAR || p->tn.type == UCHAR) && p->tn.lval < 0)
			p->tn.lval &= 0xff;
		printf( CONFMT, p->tn.lval );
		}
	else {
#ifdef FLEXNAMES
		printf( "%s", p->in.name );
#else
		printf( "%.8s", p->in.name );
#endif
		if( p->tn.lval > 0 )
			printf( "+" );
		if( p->tn.lval )
			printf( CONFMT, p->tn.lval );
		}
	}

conput( p ) register NODE *p; {

	switch( p->in.op ){

	case ICON:
		acon( p );
		return;

	case REG:
# ifdef FLT
		if( isbreg(p->tn.rval) )
			printf( "%s+%d", Blbl,
				(p->tn.rval - REGSZ - 5) * 8);
		else
# endif /*FLT*/
		printf( "%s", rnames[p->tn.rval] );
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
	/* output an address, with offsets, from p
	*/
	register int r;

	if( p->in.op == FLD ){
		p = p->in.left;
		}
	switch( p->in.op ){

	case NAME:
		acon( p );
		return;

	case ICON:
		/* addressable value of the constant */
		printf( "&" );
		acon( p );
		return;

	case REG:
# ifdef FLT
		if( rstatus[p->tn.rval] & STBREG ||
		    rstatus[p->tn.rval] & SBREG )
			printf( "%s+%ld",Blbl,(p->tn.rval - REGSZ + 5 )* 8
				+ p->tn.lval );
		else
# endif /*FLT*/
			printf( "%s", rnames[p->tn.rval] );
		return;

		case OREG:
			r = p->tn.rval;
# ifdef INDEX
		if( R2TEST(r) ){ /* double indexing */
			if( R2UPK3(r) ) printf("*");
			acon( p );
			if( R2UPK1(r) != RDUMMY) printf( "(%s)", rnames[R2UPK1(r)] );
			printf( "[%s]", rnames[R2UPK2(r)] );
		}
		else
# endif
		if( r == AP ){  /* in the argument region */
			if( p->tn.lval < 0 || p->in.name[0] != '\0' ) werror( "bad arg temp" );
			acon( p );
		printf( "(%s)", rnames[AP] );
		}
		else
			{
#ifdef FONEPASS
		if (r == TMPREG && p->in.name[1] == '\1')
		/* know temps generated by "pass2" (as opposed to those from
		 * "pass1") by the stigma
		 */
			{
			p->in.name[0] = '\0';
			acon(p);
			printf("%s%d", FSPOFF, ftnno);
			p->in.name[0] = '\1';
			}
		else
#endif /* FONEPASS */
			acon( p );
			printf( "(%s)", rnames[p->tn.rval] );
			}
		return;

	case UNARY MUL:
		printf( "*" );
		adrput( p->in.left );
		return;

		default:
		cerror( "illegal address" );
		return;

		}
	}

char *
cmpjump[] = {
	"	je	.L%d\n",
	"	jne	.L%d\n",
	"	jle	.L%d\n",
	"	jl	.L%d\n",
	"	jge	.L%d\n",
	"	jg	.L%d\n",
	"	jleu	.L%d\n",
	"	jlu	.L%d\n",
	"	jgeu	.L%d\n",
	"	jgu	.L%d\n",
};
char *
arithjump[] = {
	"	jz	.L%d\n",
	"	jnz	.L%d\n",
	"	jnpos	.L%d\n",
	"	jneg	.L%d\n",
	"	jnneg	.L%d\n",
	"	jpos	.L%d\n",
	"	jnpos	.L%d\n",
	"	jneg	.L%d\n",
	"	jnneg	.L%d\n",
	"	jpos	.L%d\n",
	};

cbgen( o, lab, mode ){
	/* printf conditional and unconditional branches
	*/
	if( o == 0 ) printf( "	jmp	.L%d\n", lab );
	else {
		if( o > UGT ) cerror( "bad conditional branch: %s", opst[o] );
		printf( cmpflag ? cmpjump[o-EQ] : arithjump[o-EQ], lab );
	}
	cmpflag = 0;
}

genscall( p, cookie ) register NODE *p; {
/* structure valued call
*/
	p->in.op = UNARY STCALL;
	return( gencall( p, cookie ) );
}

int callsz;	/* size of arg stack in bytes */

gencall( p, cookie ) register NODE *p; {
	/* generate the call given by p
	*/
	register NODE *ptemp;
	int o, temp;
# ifdef TV
	extern int tvflag;
# endif /*TV*/

	temp = argsize( p->in.right );

	if( temp > maxargs ) maxargs = temp;

	if( p->in.right ){
		/* make temp node for STARGs, put offset in, and generate args */
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

	o = p->in.left->in.op;
	if( o != ICON && o != REG && (
# ifdef TV
	tvflag ||
# endif /*TV*/
	o != OREG && o != NAME))
	order( p->in.left, INAREG );

	callsz = temp;	/* for use by ZC and Zc */

	if( p->in.op == UNARY STCALL && cookie == INTAREG )
	return(0);	/* only here to setup call for SSTCALL shape */
	else
	cookie = INTAREG;	/* to satisfy UNARY CALL templates */
	return( match(p,cookie) != MDONE );
	}

genargs( p, ptemp ) register NODE *p, *ptemp; {
	/* generate code for the arguments
	*/
	register size;

	/*  first, do the arguments on the left */

	while( p->in.op == CM ){
		genargs( p->in.left, ptemp );
		p->in.op = FREE;
		p = p->in.right;
	}

	if( p->in.op == STARG ){ /* structure valued argument */

		size = p->stn.stsize;
		SETOFF( size, 4 );
		ptemp->tn.lval = -size;

		printf( "	addw2	&%d,%s\n", size, rnames[SP] );

		p->in.op = STASG;
		p->in.right = p->in.left;
		p->in.left = tcopy( ptemp );

		/* the following line is done only with the knowledge
		that it will be undone by the STASG node, with the
		offset (lval) field retained */
		/*
		* This looks extremely suspect, and in fact fails when
		* the rhs is a pointer to unionty--just patch and pray
		*/
		if (p->in.right->in.op == OREG &&
			p->in.right->in.type != (PTR|STRTY) &&
			p->in.right->in.type != (PTR|UNIONTY))
		    {

			p->in.right->in.op = REG;  /* only for temporaries */
		    }
		}

	order( p, FORARG );
	}

argsize( p ) register NODE *p; {
	/* determine total bytes for arguments
	*/
	register t;
	if( p == NULL ) return( 0 );
	t = 0;
	if( p->in.op == CM ){
		t = argsize( p->in.left );
		p = p->in.right;
	}
	if( p->in.op == STARG ){
		t = t + p->stn.stsize;
		SETOFF( t, 4 );
		return( t );
	} else
# ifdef FLT
		if( p->in.type == FLOAT || p->in.type == DOUBLE )
			return( t+8 );
		else
# endif /*FLT*/
			return( t+4 );
	}

# ifndef ONEPASS
# ifdef FLT
int fltused = 0; /* when true create storage space for floating
		    point intermediate values */
# endif /*FLT*/
main( argc, argv ) char *argv[]; {
	int i, j, n;
	int ret;
	for( i =1; (i < argc) && (argv[i][0] == '-'); i++) {
		switch( argv[i][1] ) {
	case 'i':	/* input file name */
		if( freopen(argv[i+1], "r", stdin) == NULL ){
			fprintf( stderr, "Can't open %s\n", argv[i+1] );
			exit(2);
			}
		n = 2;
		break;

	case 'o':	/* output file name */
#ifdef FORT
		if( freopen(argv[i+1], "a", stdout) == NULL ){
#else
		if( freopen(argv[i+1], "w", stdout) == NULL ){
#endif
			fprintf( stderr, "Can't open %s\n", argv[i+1] );
			exit(2);
			}
		n = 2;
		break;

#ifdef TV
	case 't':	/* transfer vector flag */
		tvflag++;
		n = 1;
		break;
#endif /* TV */

	default:
		continue;
	}

	argc -= n;
	for( j=i; j<argc; j++ )
		argv[j] = argv[j+n];
	i--;
	}

	ret = mainp2( argc, argv );
	return( ret );
	}
# endif
