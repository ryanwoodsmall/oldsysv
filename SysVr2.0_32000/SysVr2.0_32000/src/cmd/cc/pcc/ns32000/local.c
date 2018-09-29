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
/*	@(#)local.c	2.5	*/
# include <stdio.h>
# include "mfile1"
# include "storclass.h"

/* static char ID[] = "@(#)local.c	1.6"; */

extern int eprint();

NODE *tcopy();
extern FILE *outfile;
extern int lastloc;

extern int startln, oldln;
extern char startfn[];
extern int bb_flags[];
extern int gdebug;
# define LINENO  lineno-startln+1

extern char *rnames[];

/*	this file contains code which is dependent on the target machine */

NODE *
cast( p, t ) register NODE *p; TWORD t; {
	/* cast node p to type t */

	p = buildtree( CAST, block( NAME, NIL, NIL, t, 0, (int)t ), p );
	p->in.left->in.op = FREE;
	p->in.op = FREE;
	return( p->in.right );
	}


static NODE*
assign( p )			/* used in integral op= floating below */
NODE *p;
	{
	NODE	*l,
		*r;
	r = p->in.right;
	if( r->in.op == SCONV )
		{
		r->in.op = FREE;
		r = r->in.left;
		}
	r = buildtree( p->in.op - 1, p->in.left, r );
	l = tcopy( p->in.left );
	p->in.op = FREE;
	p = buildtree( ASSIGN,l,r );
	return( p );
	}

NODE *
clocal(p) NODE *p; {

	/* this is called to do local transformations on
	   an expression tree preparitory to its being
	   written out in intermediate code.
	*/

	/* the major essential job is rewriting the
	   automatic variables and arguments in terms of
	   REG and OREG nodes */
	/* conversion ops which are not necessary are also clobbered here */
	/* in addition, any special features (such as rewriting
	   exclusive or) are easily handled here as well */

	register struct symtab *q;
	register NODE *r;
	register o;
	register m, ml;

	switch( o = p->in.op ){

        case FORCE:     /* fix introduction of U& in buildtree */
                if ( p->in.left->in.type == STRTY || p->in.left->in.type == UNIONTY ) {
                        p->in.left = buildtree(UNARY AND, p->in.left, NIL);
                        p->in.left = clocal(p->in.left);
                        }
                break;

        case STASG:     /* fix introduction of U& in buildtree */
		if ( ISPTR(p->in.right->in.type) ) {
                	p->in.right = buildtree(UNARY MUL, p->in.right, NIL);
                	p->in.right = clocal(p->in.right);
			}
                break;

        case STARG:     /* fix introduction of U& in buildtree */
		if ( ISPTR(p->in.left->in.type) ) {
                	p->in.left = buildtree(UNARY MUL, p->in.left, NIL);
                	p->in.left = clocal(p->in.left);
			}
                break;

	case NAME:
		if( p->tn.rval < 0 ) { /* already processed; ignore... */
			return(p);
			}
		q = &stab[p->tn.rval];
		switch( q->sclass ){

		case AUTO:
		case PARAM:
			/* fake up a structure reference */
			r = block( REG, NIL, NIL, PTR+STRTY, 0, 0 );
			r->tn.lval = 0;
			r->tn.rval = (q->sclass==AUTO?STKREG:ARGREG);
			p = stref( block( STREF, r, p, 0, 0, 0 ) );
			break;

		case ULABEL:
		case LABEL:
		case STATIC:
			if( q->slevel == 0 ) break;
			p->tn.lval = 0;
			p->tn.rval = -q->offset;
			break;

		case REGISTER:
			p->in.op = REG;
			p->tn.lval = 0;
			p->tn.rval = q->offset;
			break;

			}
		break;

	case PCONV:
		/* do pointer conversions for char and longs */
		ml = p->in.left->in.type;
		if( ( ml==CHAR || ml==UCHAR || ml==SHORT || ml==USHORT ) && p->in.left->in.op != ICON ) break;

		/* pointers all have the same representation; the type is inherited */

	inherit:
		p->in.left->in.type = p->in.type;
		p->in.left->fn.cdim = p->fn.cdim;
		p->in.left->fn.csiz = p->fn.csiz;
		p->in.op = FREE;
		return( p->in.left );

	case SCONV:
		m = (p->in.type == FLOAT || p->in.type == DOUBLE );
		ml = (p->in.left->in.type == FLOAT || p->in.left->in.type == DOUBLE );
		if( m != ml ) break;

		/* now, look for conversions downwards */

		m = p->in.type;
		ml = p->in.left->in.type;
		if( p->in.left->in.op == ICON && !ISPTR(ml) ){
					/* simulate the conversion here */
					/* ICON PTR has a name keep the SCONV */
			CONSZ val;
			val = p->in.left->tn.lval;
			switch( m ){
			case CHAR:
				p->in.left->tn.lval = (char) val;
				break;
			case UCHAR:
				p->in.left->tn.lval = val & 0XFF;
				break;
			case USHORT:
				p->in.left->tn.lval = val & 0XFFFFL;
				break;
			case SHORT:
				p->in.left->tn.lval = (short)val;
				break;
			case UNSIGNED:
				p->in.left->tn.lval = val & 0xFFFFFFFFL;
				break;
			case INT:
				p->in.left->tn.lval = (int)val;
				break;
				}
			p->in.left->in.type = m;
			}
		else {
			/* meaningful ones are conversion of int to char,
			   int to short, and short to char, and unsigned
			   version of them.  unsigned fields cannot have
			   the type colored down on a signed extraction */
			if ( p->in.left->in.op == FLD && ( ml == UCHAR ||
				ml == USHORT || ml == UNSIGNED ) &&
				( m == CHAR || m == SHORT || m == INT ) )
			{
				break;
			}
			if( m==CHAR || m==UCHAR ){
				if( ml!=CHAR && ml!= UCHAR ) break;
				}
			else if( m==SHORT ){
				if( ml!=CHAR && ml!=UCHAR && ml!=SHORT && ml!=USHORT ) break;
				}
			else if( m==USHORT ){
					/* (unsigned short)char is meaningful */
				if( ml!=UCHAR && ml!=SHORT && ml!=USHORT ) break;
				}
			}

		/* clobber conversion */
		if( tlen(p) == tlen(p->in.left) ) goto inherit;
		/*if ( p->in.left->in.op == ASSIGN )	/* should leave it */
			/*break;*/
		p->in.op = FREE;
		return( p->in.left );  /* conversion gets clobbered */

	case PVCONV:
 	case PMCONV:
		if( p->in.right->in.op != ICON ) cerror( "bad conversion", 0);
		p->in.op = FREE;
		return( buildtree( o==PMCONV?MUL:DIV, p->in.left, p->in.right ) );

	case RS:
	case ASG RS:
		/* convert >> to << with negative shift count */
		/* only if type of left operand is not unsigned */

		if( ISUNSIGNED(p->in.left->in.type) ) break;
		p->in.right = buildtree( UNARY MINUS, p->in.right, NIL );
		if( p->in.op == RS ) p->in.op = LS;
		else p->in.op = ASG LS;
		break;

	case ASG PLUS :
	case ASG MINUS:
	case ASG MUL:
	case ASG DIV:
	/* pcc treats these as assign operators rather than arithmetic ops
	 * so the conversions for floating point operands are not done
	 * properly. The tree is rebuilt with proper conversions here
	 */
	{
	NODE *l,*r;
			l = p->in.left;
			r = p->in.right;
			if( l->in.type == FLOAT )
				p = assign(p);
			else if( ( l->in.type != DOUBLE ) &&
				 ( r->in.op == SCONV ) )
   				if( (r->in.left->in.type == FLOAT )
				      || (r->in.left->in.type == DOUBLE ) )
					p = assign(p);
			goto assigns;
	}

	case FLD:
		/* make sure that the second pass does not make the
		   descendant of a FLD operator into a doubly indexed OREG */

		if( p->in.left->in.op == UNARY MUL
				&& (r=p->in.left->in.left)->in.op == PCONV)
			if( r->in.left->in.op == PLUS || r->in.left->in.op == MINUS )
				if( ISPTR(r->in.type) ) {
					if( ISUNSIGNED(p->in.left->in.type) )
						p->in.left->in.type = UCHAR;
					else
						p->in.left->in.type = CHAR;
				}
		break;

	case ASSIGN:
	assigns:
		if (p->in.right->in.op == QUEST ||
		    (p->in.right->in.op == SCONV && p->in.right->in.left->in.op == QUEST) ||
		    (p->in.right->in.op == SCONV && p->in.right->in.left->in.op == SCONV &&
		    p->in.right->in.left->in.left->in.op == QUEST))
					/* distribute assigns over colons */
		{
			register NODE *pwork;
			NODE *pcpy = tcopy(p), *pnew;
			extern int xdebug, eprint();

			if (xdebug)
			{
				puts("Entering ?: distribution");
				fwalk(p, eprint, 0);
			}
			pnew = pcpy->in.right;
			while (pnew->in.op != QUEST)
				pnew = pnew->in.left;
						/* pnew is top of new tree */
			if ((pwork = p)->in.right->in.op == QUEST)
			{
				tfree(pwork->in.right);
				pwork->in.right = pnew->in.right->in.left;
				pnew->in.right->in.left = pwork;
			/* at this point, 1/2 distributed. Tree looks like:
					ASSIGN|ASGOP
				LVAL			QUEST
					EXPR1		COLON
						ASSIGN|ASGOP	EXPR3
					LVAL		EXPR2
			pnew "holds" new tree from QUEST node */
			}
			else
			{
				NODE *pholdtop = pwork;
				pwork = pwork->in.right;
				while (pwork->in.left->in.op != QUEST)
					pwork = pwork->in.left;
				tfree(pwork->in.left);
				pwork->in.left = pnew->in.right->in.left;
				pnew->in.right->in.left = pholdtop;
			/* at this point, 1/2 distributed. Tree looks like:
					ASSIGN|ASGOP
				LVAL			ANY # OF SCONVs
						QUEST
					EXPR1		COLON
						ASSIGN|ASGOP	EXPR3
					LVAL		ANY # OF SCONVs
						EXPR2
			pnew "holds" new tree from QUEST node */
			}
			if ((pwork = pcpy)->in.right->in.op == QUEST)
			{
				pwork->in.right = pnew->in.right->in.right;
				pnew->in.right->in.right = pwork;
				/* done with the easy case */
			}
			else
			{
				NODE *pholdtop = pwork;
				pwork = pwork->in.right;
				while (pwork->in.left->in.op != QUEST)
					pwork = pwork->in.left;
				pwork->in.left = pnew->in.right->in.right;
				pnew->in.right->in.right = pholdtop;
				/* done with the SCONVs case */
			}
			p = pnew;
			if (xdebug)
			{
				puts("Leaving ?: distribution");
				fwalk(p, eprint, 0);
			}
		}
	}

	return(p);
}

andable( p ) NODE *p; {
	return(1);  /* all names can have & taken on them */
	}

cendarg(){ /* at the end of the arguments of a ftn, set the automatic offset */
	autooff = AUTOINIT;
	}

cisreg( t ) TWORD t; { /* is an automatic variable of type t OK for a register variable */
			/* if so, return number of registers needed */

	if ( ISPTR(t) )
		return (1);
	switch ( t )
	{
	case INT: case UNSIGNED: case LONG: case ULONG:
	case CHAR: case UCHAR: case SHORT: case USHORT:
		return (1);
	
	}
	return(0);
	}

NODE *
offcon( off, t, d, s ) OFFSZ off; TWORD t; {

	/* return a node, for structure references, which is suitable for
	   being added to a pointer of type t, in order to be off bits offset
	   into a structure */

	register NODE *p;

	/* t, d, and s are the type, dimension offset, and sizeoffset */
	/* in general they  are necessary for offcon, but not on H'well */

	p = bcon(0);
	p->tn.lval = off/SZCHAR;
	return(p);

	}


static inwd	/* current bit offsed in word */;
static word	/* word being built from fields */;

incode( p, sz ) register NODE *p; {

	/* generate initialization code for assigning a constant c
		to a field of width sz */
	/* we assume that the proper alignment has been obtained */
	/* inoff is updated to have the proper final value */
	/* we also assume sz  < SZINT */

	if((sz+inwd) > SZINT) cerror("incode: field > int");
	word |= ((unsigned)(p->tn.lval<<(32-sz))) >> (32-sz-inwd);
	inwd += sz;
	inoff += sz;
	if(inoff%SZINT == 0) {
		printf( "	.double	0x%x\n", word);
		word = inwd = 0;
		}
	}

fincode( d, sz ) double d; {
	/* output code to initialize space of size sz to the value d */
	/* the proper alignment has been obtained */
	/* inoff is updated to have the proper final value */
	/* on the target machine, write it out in octal! */


	printf("	%s	0%c%.20e\n", sz == SZDOUBLE ? ".long" : ".float",
		sz == SZDOUBLE ? 'l' : 'f', d);
	inoff += sz;
	}

cinit( p, sz ) NODE *p; {
	/* arrange for the initialization of p into a space of
	size sz */
	/* the proper alignment has been opbtained */
	/* inoff is updated to have the proper final value */
	ecode( p );
	inoff += sz;
	}

vfdzero( n ){ /* define n bits of zeros in a vfd */

	if( n <= 0 ) return;

	inwd += n;
	inoff += n;
	if( inoff%ALINT ==0 ) {
		printf( "	.double	0x%x\n", word );
		word = inwd = 0;
		}
	}

char *
exname( p ) char *p; {
	/* make a name look like an external name in the local machine */

#ifdef FLEXNAMES
	static char text[BUFSIZ+1];
	extern int truncate_flag;

	text[0] = '_';
	(void) strncpy( text + 1, p, BUFSIZ );
	text[ BUFSIZ ] = '\0';
	if ( truncate_flag )		/* backward compatibility option */
		text[8] = '\0';
#else
	static char text[NCHNAM+1];

	text[0] = '_';
	(void) strncpy( text + 1, p, NCHNAM );
	text[ NCHNAM ] = '\0';		/* make sure is a string */
#endif
	return( text );
	}

ctype( type ){ /* map types which are not defined on the local machine */
	switch( BTYPE(type) ){

	case LONG:
		MODTYPE(type,INT);
		break;

	case ULONG:
		MODTYPE(type,UNSIGNED);
		}
	return( type );
	}

noinit( t ) { /* curid is a variable which is defined but
	is not initialized (and not a function );
	This routine returns the stroage class for an uninitialized declaration */

	return(EXTERN);

	}

isitlong( cb, ce ){ /* is lastcon to be long or short */
	/* cb is the first character of the representation, ce the last */

	if( ce == 'l' || ce == 'L' ||
		lastcon >= (1L << (SZINT-1) ) ) return (1);
	return(0);
	}


isitfloat( s ) char *s; {
	double atof();
	dcon = atof(s);
	return( FCON );
	}

ecode( p ) NODE *p; {

	/* walk the tree and write out the nodes.. */

	if( nerrors ) return;

	/* generate a new line number breakpoint if
	 * the line number has not changed.
	 */
	if( gdebug && lineno != oldln ) {
		oldln = lineno;
		if( lastloc == PROG && strcmp( startfn, ftitle ) == 0 )
			fprintf( outfile, "	.ln	%d\n", LINENO );
		}

	p2tree( p );
	p2compile( p );
	}

fixdef(p) struct symtab *p; {

	/* print debugging info
	 *
	 * don't do params, externs, structure names or members
	 * at this time; they are delayed until more information
	 * is known about them
	 */

	switch( p->sclass ) {

	case USTATIC:
	case EXTERN:
	case EXTDEF:
	case STNAME:
	case UNAME:
	case ENAME:
	case MOS:
	case MOE:
	case MOU:
		return;
	default:
		if( p->sclass & FIELD ) return;
		}

	/* parameters */
	if( p->slevel == 1 ) return;

	/* static externs */
	if( p->sclass == STATIC && p->slevel == 0 ) return;

	prdef(p,0);
	}

prdims( p ) struct symtab *p; {
	/* print debugging info for dimensions
	 */
	int temp, dtemp, dflag;
	OFFSZ tsize();

	dflag = 0; /* need to print .dim */
	dtemp = p->dimoff;

	for( temp=p->stype; temp&TMASK; temp = DECREF(temp) ) {
		/* put out a dimension for each instance of ARY in type word */

		if( ISARY(temp) ) {
			if( !dflag ) {
				fprintf( outfile, "	.dim	%d", dimtab[dtemp++] );
				dflag = 1;
				}
			else
				fprintf( outfile, ",%d", dimtab[dtemp++] );
			}
		}

	if( dflag ) {
		fputs( ";\t.size\t", outfile );
		fprintf( outfile, CONFMT, tsize(p->stype, p->dimoff, p->sizoff ) / SZCHAR  );
		putc( ';', outfile );
		}
	}

/* local table of fakes for un-names structures
 * sizoff for .ifake is stored in mystrtab[i]
 */
#define FAKENM 99	/* maximum number of fakenames */
int mystrtab[FAKENM], ifake = 0;
struct symtab mytable;
#ifdef FLEXNAMES
	char tagname[BUFSIZ] = "";
#else
	char tagname[10] = "";
#endif

prdef(p,dsflg) struct symtab *p; int dsflg; {
	/* print symbol definition pseudos
	 */
	TWORD pty, bpty;
	char *strname(), savech, *tagnm;
	int sclass;
	int saveloc;

	if ( dsflg || !gdebug )
		return;

	/* translate storage class */

	switch( p->sclass ){
	case AUTO:	sclass = C_AUTO; break;
	case EXTDEF:
	case EXTERN:	sclass = C_EXT; break;
	case STATIC:	sclass = C_STAT; break;
	case REGISTER:	sclass = blevel==1 ? C_REGPARM : C_REG; break;
	case ULABEL:
	case LABEL:	sclass = C_LABEL; break;
	case MOS:	sclass = C_MOS; break;
	case PARAM:	sclass = C_ARG; break;
	case STNAME:	sclass = C_STRTAG; break;
	case MOU:	sclass = C_MOU; break;
	case UNAME:	sclass = C_UNTAG; break;
	case TYPEDEF:	sclass = C_TPDEF; break;
	case ENAME:	sclass = C_ENTAG; break;
	case MOE:	sclass = C_MOE; break;
	default:	if( p->sclass & FIELD )
				sclass = C_FIELD;
			else
				cerror( "bad storage class %d", p->sclass );
			break;
		}

	/* print a bb symbol if this is the first symbol in the block */

	if( blevel > 2 && !bb_flags[blevel] && sclass != C_LABEL ) {
		fprintf(outfile,"     .def    .bb;    .val    .;      .scl    %d;     .line   %d;     .endef\n", C_BLOCK, LINENO );
		bb_flags[blevel] = 1;   /* don't let another bb print */
		}

	/* make sure that .defs in functions are in text section */
	if( blevel > 1 )
		saveloc = locctr( PROG );

#ifdef FLEXNAMES
	fprintf( outfile, "	.def	%s;", exname( p->sname ) );
#else
	fprintf( outfile, "	.def	%.8s;", exname( p->sname ) );
#endif

	switch( sclass ) {	/* print .val based on storage class */

	case C_AUTO:
	case C_MOS:
	case C_MOU:
	case C_ARG:
		/* offset in bytes */
		fprintf( outfile, "	.val	%d;", p->offset/SZCHAR );
		break;

	case C_FIELD:
	case C_MOE:
		/* offset in bits, or internal value of enum symbol */
		fprintf( outfile, "	.val	%d;", p->offset );
		break;

	case C_REGPARM:
	case C_REG:
		/* offset in bytes in savearea for reg vars */
		/* actual offset determination is deferred to the asembler */
		fprintf( outfile, "	.val	%d;", p->offset );
		break;

	case C_STAT:
	case C_LABEL:
	case C_EXT:
		/* actual or hidden name, depending on scope */
		if( p->slevel >0 )
			fprintf( outfile, "	.val	.L%d;", p->offset );
		else
#ifdef FLEXNAMES
			fprintf( outfile, "	.val	%s;",
				exname( p->sname ) );
#else
			fprintf( outfile, "	.val	%.8s;",
				exname( p->sname ) );
#endif
		break;

	default:
		break;
		}

	pty = p->stype;
	bpty = BTYPE(pty);
	fprintf( outfile, "	.scl	%d;	.type	0%o;", sclass, pty );

	/* print tag and size info, size only for tags themselves;
	 * don't print if size not known
	 */

	if (dimtab[p->sizoff] > 0)
		switch( sclass ) {

		case C_STRTAG:
		case C_UNTAG:
		case C_ENTAG:
			fprintf( outfile, "	.size	%d;", (unsigned)dimtab[p->sizoff] / SZCHAR );
			break;

		default:	/* bpty is base type */
			if( (bpty == STRTY || bpty == UNIONTY || bpty == ENUMTY) && gdebug ) {
#ifdef FLEXNAMES
				char buf[BUFSIZ];

				(void) strcpy( buf, strname( p->sizoff ) );
				if ( buf[0] == '$' )
					buf[0] = '_';
				fprintf( outfile, "	.tag	%s;",
					exname( buf ) );
				fprintf( outfile, "	.size	%d;",
					(unsigned) dimtab[p->sizoff] / SZCHAR );
#else
				tagnm = strname(p->sizoff);
				savech = *tagnm;
				if( savech == '$' )
					*tagnm = '_';
				fprintf( outfile, "	.tag	%.8s;",
					exname( tagnm ) );
				*tagnm = savech;
				fprintf( outfile, "	.size	%d;",
					(unsigned)dimtab[p->sizoff] / SZCHAR );
#endif
				}
			break;
			}

	/* print line for block symbols */

	if( p->slevel > 2 )
		fprintf( outfile, "	.line	%d;", LINENO );

	/* look for dimensions to print */

	if( sclass != C_LABEL && gdebug )
		prdims( p );

	/* size of field is its length in bits */

	if( sclass == C_FIELD )
		fprintf( outfile, "	.size	%d;", p->sclass & FLDSIZ );

	fputs( "\t.endef\n", outfile );
	if( blevel > 1 )
		locctr( saveloc );
	}

strend( dimst ) int dimst; {
	/* called at the end of a structure declaration
	 * this routine puts out the structure tag, its members
	 * and an eos.  dimst is the index in dimtab of the
	 * structure description
	 */
	int member, size, saveloc;
	char savech;
	struct symtab *memptr, *tagnm, *strfind();

	if( !gdebug ) return;

	/* set locctr to text */
	saveloc = locctr( PROG );

	/* set up tagname */
	member = dimtab[dimst + 1];
	tagnm = strfind(dimst);

	if( tagnm == NULL ) {
		/* create a fake if there is no tagname */
		/* use the local symbol table */
		tagnm = &mytable;
		if( ifake == FAKENM )
			cerror( "fakename table overflow" );

		/* generate the fake name and enter into the fake table */
#ifdef FLEXNAMES
		{
			char buf[BUFSIZ];

			sprintf( buf, ".%dfake", ifake );
			mytable.sname = savestr( buf );	/* lives forever! */
		}
#else
		sprintf( mytable.sname, ".%dfake", ifake );
#endif
		mystrtab[ifake++] = dimst;
		memptr = &stab[dimtab[member]];

		/* fix up the fake's class, type, and sizoff based on class of its members */
		switch( memptr->sclass ) {
		case MOS:
			tagnm->sclass = STNAME;
			tagnm->stype = STRTY;
			break;
		case MOE:
			tagnm->sclass = ENAME;
			tagnm->stype = ENUMTY;
			break;
		case MOU:
			tagnm->sclass = UNAME;
			tagnm->stype = UNIONTY;
			break;
		default:
			if( memptr->sclass & FIELD ){
				tagnm->sclass = STNAME;
				tagnm->stype = STRTY;
				}
			else
				cerror( "can't identify type of fake tagname" );
			}
		tagnm->slevel = 0;;
		tagnm->sizoff = dimst;
		}

	/* print out the structure header */
#ifdef FLEXNAMES
	{
		char buf[BUFSIZ], *saveit;

		saveit = tagnm->sname;
		(void) strncpy( buf, saveit, BUFSIZ - 1 );
		buf[BUFSIZ - 1] = '\0';
		tagnm->sname = buf;
		if ( buf[0] == '$' )
			buf[0] = '_';
		prdef( tagnm, 0 );
		tagnm->sname = saveit;
	}
#else
	savech = tagnm->sname[0];
	if( savech == '$' )
		tagnm->sname[0] = '_';
	prdef(tagnm,0);
	tagnm->sname[0] = savech;
#endif

	/* print out members */
	while( dimtab[member] >= 0 ) {
		memptr = &stab[dimtab[member++]];
#ifdef FLEXNAMES
		{
			char buf[BUFSIZ], *saveit;

			saveit = memptr->sname;
			(void) strncpy( buf, saveit, BUFSIZ - 1 );
			buf[BUFSIZ - 1] = '\0';
			memptr->sname = buf;
			if ( buf[0] == '$' )
				buf[0] = '_';
			prdef( memptr, 0 );
			memptr->sname = saveit;
		}
#else
		savech = memptr->sname[0];
		if( savech == '$' )
			memptr->sname[0] = '_';
		prdef(memptr,0);
		memptr->sname[0] = savech;
#endif
		}

	/* print eos */
	size = (unsigned)dimtab[dimst] / SZCHAR;
	fprintf( outfile, "	.def	.eos;	.val	%d;	.scl	%d;", size, C_EOS );
#ifdef FLEXNAMES
	fprintf( outfile, "	.tag	%s;	.size	%d;	.endef\n",
		exname( tagnm->sname ), size );
#else
	fprintf( outfile, "	.tag	%.8s;	.size	%d;	.endef\n",
		exname( tagnm->sname ), size );
#endif

	/* return to old locctr */
	locctr( saveloc );
	}

struct symtab *
strfind( key ) int key; {
	/* find the structure tag in the symbol table, 0 == not found
	 */
	struct symtab *sptr;
	char spc;
	for( sptr = stab; sptr < stab + SYMTSZ; ++sptr ) {
		spc = sptr->sclass;
		if( (spc == STNAME || spc == ENAME || spc == UNAME ) && sptr->sizoff == key && sptr->stype != TNULL )
			return( sptr );
		}
	/* not found */
	return( NULL );
	}

char *
strname( key ) int key; {
	/* return a pointer to the tagname,
	 * the fake table is used if not found by strfind
	 */
	int i;
	struct symtab *tagnm, *strfind();
	tagnm = strfind( key );
	if( tagnm != NULL )
		return( tagnm->sname );

	for( i = 0; i < FAKENM; ++i )
		if( mystrtab[i] == key ) {
			sprintf( tagname, ".%dfake", i );
			return( tagname );
			}

	cerror( "structure tagname not found" );
	return(NULL);
	}

/*
* since there exist contexts where the length of the FCON zero needs
* to stay as it is now, cannot change FCON zero to ICON zero.  An
* example is when an FCON is an argument to a function call.
*
* setdcon(p)
* register NODE *p;
* {
*	if( !p->fpn.dval ){	/* make FCON zero into ICON zero *//*
*		p->tn.op = ICON;
*		p->tn.type = INT;
* #ifdef FLEXNAMES
*		p->tn.name = "";
* #else
*		p->tn.name[0] = '\0';
* #endif
*		p->tn.lval = 0;
*		p->tn.rval = NONAME;
*		}
*	return(1);
* }
*/

#ifndef ONEPASS
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
#endif
