/*	@(#)code.c	1.9	*/
#include <stdio.h>
#include "mfile1"
#include "storclass.h"
#include <signal.h>
#include "messages.h"

/* static char ID[] = "@(#)code.c	1.9"; */
static char uxrel[] = "UNIX 6.0";

int proflg = 0;	/* are we generating profiling code? */
int gdebug = 0;	/* sdb flag	*/
int strftn = 0;	/* is the current function one which returns a value */
int fdefflag;	/* are we within a function definition ? */
char NULLNAME[8];
int labelno;

# define LINENO  lineno-startln+1	/* local line number, opening { = 1 */

int startln;		/* the editor line number of the opening '{' } */
int oldln;		/* remember the last line number printed */
char startfn[100] = "";	/* the file name of the opening '{' */

int bb_flags[BCSZ];	/* remember whether or not bb is needed */

int reg_use = MAXRVAR;

/* tempfile is used to hold initializing strings that cannot be output
 * immediately -- they are read back in at the end and appended to
 * the intermediate text
 */
FILE *outfile = stdout;
FILE *tempfile;
extern char *mktemp();

branch( n ){
	/* output a branch to label n
	 */
	/* exception is an ordinary function branching to retlab: then, return */
	if( n == retlab && !strftn ){
		register TWORD t;
		register r;

			/* set number of regs in assem comment field */
			/* so optimizers can do a better job */
		r = 0;
		if( retstat & RETVAL ){ /* the function rets a val somewhere */
			t = (&stab[curftn])->stype;
			t = DECREF(t);
			r++;	/* it is at least one */
			if( t == DOUBLE || t == FLOAT )
				r++;	/* it takes two */
			}
		printf( "	ret#%d\n", r );
		}
	else printf( "	jbr 	.L%d\n", n );
	}

int lastloc = { -1 };

defalign(n) {
	/* cause the alignment to become a multiple of n
	 */
	n /= SZCHAR;
	if( lastloc != PROG && n > 1 )
		fprintf( outfile, "	.align	%d\n", (n > 2) ? 4 : 2 );
	}

locctr( l ){
	/* output the location counter
	 */
	register temp;
	/* l is PROG, ADATA, DATA, STRNG, ISTRNG, or STAB */

	if( l == lastloc ) return(l);
	temp = lastloc;
	lastloc = l;
	switch( l ){

	case PROG:
		outfile = stdout;
		fputs( "\t.text\n", stdout );
		break;

	case DATA:
	case ADATA:
		outfile = stdout;
		if( temp != DATA && temp != ADATA )
			fputs( "\t.data\n", stdout );
		break;

	case STRNG:
	case ISTRNG:
		/* output string initializers to a temporary file for now
		 * don't update lastloc
		 */
		outfile = tempfile;
		break;

	case STAB:
		cerror( "locctr: STAB unused" );
		break;

	default:
		cerror( "illegal location counter" );
		}

	return( temp );
	}

deflab( n ){
	/* output something to define the current position as label n
	 */
	fprintf( outfile, ".L%d:\n", n );
	}

getlab(){
	/* return a number usable for a label
	 */
	static int crslab = 10;

	return( ++crslab );
	}

defnam( p ) register struct symtab *p; {
	/* define the current location as the name p->sname
	 * first give the debugging info for external definitions
	 */
	if( p->slevel == 0 )	/* make sure its external */
		prdef(p,0);
	if( p->sclass == EXTDEF )
#ifdef FLEXNAMES
		printf( "	.globl	%s\n", exname( p->sname ) );
#else
		printf( "	.globl	%.8s\n", exname( p->sname ) );
#endif
	if( p->sclass == STATIC && p->slevel > 1 )
		deflab( p->offset );
	else
#ifdef FLEXNAMES
		printf( "%s:\n", exname( p->sname ) );
#else
		printf( "%.8s:\n", exname( p->sname ) );
#endif

		}

commdec( id ){
	/* make a common declaration for id, if reasonable
	 */
	register struct symtab *q;
	int align;
	OFFSZ tsize();
	OFFSZ off;

	q = &stab[id];
	off = tsize( q->stype, q->dimoff, q->sizoff );
	off /= SZCHAR;
	if (q->sclass == EXTDEF) {
#ifdef FLEXNAMES
		werror("redeclaration of %s", q->sname);
#else
		werror("redeclaration of %.8s", q->sname);
#endif
		return;
	}
	if(q->slevel == 0)	/* make sure its external */
		prdef(q,0);
#ifdef FLEXNAMES
	printf( "	.globl	%s\n", exname(q->sname));
#else
	printf( "	.globl	%.8s\n", exname(q->sname));
#endif
	/** Allow multiple external definitions...
	*
	* printf( "	.bss	%.8s,", exname(q->sname));
	*/
#ifdef FLEXNAMES
	printf( "	.comm	%s,", exname(q->sname));
#else
	printf( "	.comm	%.8s,", exname(q->sname));
#endif
	printf( CONFMT, off);
	/** With multiple external definitions, this message is incorrect.
	*
	* if(off == 0L)
	*	uerror("declaration for %.8s allocates no space", exname(q->sname));
	*/
	/** No align for .comm
	*
	* align = talign(q->stype, q->sizoff)/SZCHAR;
	* printf( ",%d\n", align>2 ? 4 : align);
	*/
	putchar( '\n' );
	}

int ftlab1, ftlab2;

bfcode( a, n ) int a[]; {
	/* code for the beginning of a function
	 * a is an array of indices in stab for the arguments
	 * n is the number of arguments
	 */
	register i;
	register temp;
	register struct symtab *p;
	int off;
	char *toreg();

	/* function prolog */

	locctr( PROG );
	p = &stab[curftn];
	fputs( "\t.align\t4\n", stdout );
	defnam( p );
	temp = p->stype;
	temp = DECREF(temp);
	strftn = (temp==STRTY) || (temp==UNIONTY);

	retlab = getlab();

	/* routine prolog */

	printf( "	.word	.R%d\n", ftnno);
	if ( gdebug )
		fputs( "	.ln	1\n", stdout );
	ftlab1 = getlab();
	ftlab2 = getlab();
	printf( "	jbr 	.L%d\n", ftlab1);
	printf( ".L%d:\n", ftlab2);
	if( proflg ) {	/* profile code */
		i = getlab();
		printf("	movab	.L%d,r0\n", i);
		fputs("\tjsb\tmcount\n", stdout);
		fputs("\t.data\n", stdout);
		fputs("\t.align\t2\n", stdout);
		printf(".L%d:	.long	0\n", i);
		fputs("\t.text\n", stdout);
		}

	off = ARGINIT;

	for( i=0; i<n; ++i ){
		p = &stab[a[i]];
		if( p->sclass == REGISTER ){
			temp = p->offset;  /* save register number */
			p->sclass = PARAM;  /* forget that it is a register */
			p->offset = NOOFFSET;
			oalloc( p, &off );
			printf( "\t%s\t%d(ap),r%d\n", toreg(p->stype), p->offset/SZCHAR, temp );
			p->offset = temp;  /* remember register number */
			p->sclass = REGISTER;   /* remember that it is a register */
			}
		else if( p->stype == STRTY || p->stype == UNIONTY ) {
			p->offset = NOOFFSET;
			if( oalloc( p, &off ) ) cerror( "bad argument" );
			SETOFF( off, ALSTACK );
			}
		else {
			if( oalloc( p, &off ) ) cerror( "bad argument" );
			}

		}
	fdefflag = 1;

	/* initialize line number counters */

	oldln = startln = lineno;
	strcpy( startfn, ftitle );

	/* do .bf symbol and .defs for parameters
	 * paramters are delayed to here to two reasons:
	 *    1: they appear inside the .bf - .ef
	 *    2: the true declarations have been seen
	 */
	if ( gdebug ) {
		int line;

		line = lineno;	/* Line number information for sdb */
		printf( "	.def	.bf;	.val	.;	.scl	%d;	.line	%d;	.endef\n", C_FCN ,line );
		for( i=0; i<n; ++i ) {
			p = &stab[a[i]];
			prdef(p,0);
	}
		}

	}

beg_file() {
	/* called as the very first thing by the parser to do machine
	 * dependent stuff
	 */
	extern fpe_catch();
	register char * p;
	register char * s;

			/* note: double quotes already in ftitle... */
	p = ftitle + strlen( ftitle ) - 2;
	s = p - 14;	/* max name length */
	while ( p > s && *p != '"' && *p != '/' )
		--p;
	printf( "\t.file\t\"%.15s\n", p + 1 );
	/*
	 * Catch floating exceptions generated by the constant
	 * folding code.
	 */
	(void) signal( SIGFPE, fpe_catch );
	}

fpe_catch() {
	/*
	 * Floating exception generated by constant folding.
	 */
	/* "floating point constant folding causes exception" */
	UERROR( MESSAGE( 125 ) );
	dexit();
	}

bccode() {
	/* called just before the first executable statement
	 * by now, the automatics and register variables are allocated
	 */

	SETOFF( autooff, SZINT );

	/* this is the block header:
	 * autooff is the max offset for auto and temps
	 * regvar is the least numbered register variable
	 */

# ifdef ONEPASS
	p2bbeg( autooff, regvar );
# else
	printf( "%c%d\t%d\t%d\t\n", BBEG, ftnno, autooff, regvar );
# endif
	reg_use = (reg_use > regvar ? regvar : reg_use);

	}

ejobcode( flag ){
	/* called just before final exit
	 * flag is 1 if errors, 0 if none
	 */
	}

aobeg(){
	/* called before removing automatics from stab
	 */
	}

aocode(p) struct symtab *p; {
	/* called when automatic p removed from stab
	 */
	}

aoend(){
	/* called after removing all automatics from stab
	 * print eb here if bb printed for this block
	 */
	if( gdebug && bb_flags[blevel+1] ) {
		printf( "	.def	.eb;	.val	.;	.scl	%d;	.line	%d;	.endef\n", C_BLOCK, LINENO );
		bb_flags[blevel+1] = 0;
	}
	}

int ent_mask[] = {
	0,0,0,0,0, 0xfc0, 0xf80, 0xf00, 0xe00, 0xc00, 0x800, 0};

efcode(){
	/* code for the end of a function
	 */

	if( strftn ){  /* copy output (in R2) to caller */
		register NODE *l, *r;
		register struct symtab *p;
		register TWORD t;
		register int j;
		int i;

		p = &stab[curftn];
		t = p->stype;
		t = DECREF(t);

		deflab( retlab );

		i = getlab();	/* label for return area */
		fputs("\t.data\n", stdout);
		fputs("\t.align\t4\n", stdout);
		printf(".L%d:	.space	%d\n", i, tsize(t, p->dimoff, p->sizoff)/SZCHAR );
		fputs("\t.text\n", stdout);
		printf("	movab	.L%d,r1\n", i);

		reached = 1;
		l = block( REG, NIL, NIL, PTR|t, p->dimoff, p->sizoff );
		l->tn.rval = 1;  /* R1 */
		l->tn.lval = 0;  /* no offset */
		r = block( REG, NIL, NIL, PTR|t, p->dimoff, p->sizoff );
		r->tn.rval = 0;  /* return reg is R0 */
		r->tn.lval = 0;
		l = buildtree( UNARY MUL, l, NIL );
		r = buildtree( UNARY MUL, r, NIL );
		l = buildtree( ASSIGN, l, r );
		l->in.op = FREE;
		ecomp( l->in.left );
		printf( "	movab	.L%d,r0\n", i );
		/* turn off strftn flag, so return sequence will be generated */
		strftn = 0;
		}

	/* print end-of-function pseudo and its line number */

	if ( gdebug ){
		printf( "	.def	.ef;	.val	.;	.scl	%d;	.line	%d;	.endef\n", C_FCN, LINENO );
		if( LINENO > 1 )
			printf( "	.ln	%d\n", LINENO );
		}
	branch( retlab );
	printf( "	.set	.R%d,0x%x\n", ftnno, ent_mask[reg_use] );
	reg_use = MAXRVAR;
	p2bend();
	fdefflag = 0;
	}

	/* print end-of-function pseudo and its line number */

bycode( t, i ){
	/* put byte i+1 in a string */

	i &= 07;
	if( t < 0 ){ /* end of the string */
		if( i != 0 ) putc( '\n', outfile );
		}

	else { /* stash byte t into string */
		if( i == 0 ) fputs( "\t.byte\t", outfile );
		else putc( ',', outfile );
		fprintf( outfile, "0x%x", t );
		if( i == 07 ) putc( '\n', outfile );
		}
	}

zecode( n ){
	/* n integer words of zeros */
	OFFSZ temp;
	if( n <= 0 ) return;
	printf( "	.space	%d\n", (SZINT/SZCHAR)*n );
	temp = n;
	inoff += temp*SZINT;
	}

fldal( t ) unsigned t; { /* return the alignment of field of type t */
	uerror( "illegal field type" );
	return( ALINT );
	}

fldty( p ) struct symtab *p; { /* fix up type of field p */
	;
	}

where(c){ /* print location of error  */
	/* c is either 'u', 'c', or 'w' */
	/* GCOS version */
	fprintf( stderr, "%s, line %d: ", ftitle, lineno );
	}


/* toreg() returns a pointer to a char string
		  which is the correct  "register move" for the passed type 
 */
struct type_move {TWORD fromtype; char tostrng[8];} toreg_strs[] =
	{
	CHAR, "cvtbl",
	SHORT, "cvtwl",
	INT, "movl",
	LONG, "movl",
	FLOAT, "movf",
	DOUBLE, "movd",
	UCHAR,	"movzbl",
	USHORT,	"movzwl",
	UNSIGNED,	"movl",
	ULONG,	"movl",
	-1, ""
	};

char *
toreg(type) TWORD type; {
	struct type_move *p;

	for ( p=toreg_strs; p->fromtype > 0; p++)
		if (p->fromtype == type) return(p->tostrng);

	/* type not found, must be a pointer type */
	return("movl");
}

struct sw heapsw[SWITSZ];	/* heap for switches */

genswitch( p, n ) register struct sw *p; {
	/*	p points to an array of structures, each consisting
	 * of a constant value and a label.
	 * The first is >=0 if there is a default label;
	 * its value is the label number
	 * The entries p[1] to p[n] are the nontrivial cases
		*/
	register i;
	register CONSZ j, range;
	register dlab, swlab;

	range = p[n].sval-p[1].sval;

	if( range>0 && range <= 3*n && n>=4 ){ /* implement a direct switch */

		swlab = getlab();
		dlab = p->slab >= 0 ? p->slab : getlab();

		/* already in r0 */
		printf("	casel	r0,$%ld,$%ld\n", p[1].sval, range);
		printf(".L%d:\n", swlab);
		for( i=1,j=p[1].sval; i<=n; j++) {
			printf("	.word	.L%d-.L%d\n", (j == p[i].sval ? ((j=p[i++].sval), p[i-1].slab) : dlab),
				swlab);
			}

		if( p->slab >= 0 ) branch( dlab );
		else printf(".L%d:\n", dlab);
		return;

		}

	if( n>8 ) {	/* heap switch */

		heapsw[0].slab = dlab = p->slab >= 0 ? p->slab : getlab();
		makeheap(p, n, 1);	/* build heap */

		walkheap(1, n);	/* produce code */

		if( p->slab >= 0 )
			branch( dlab );
		else
			printf(".L%d:\n", dlab);
		return;
	}

	/* debugging code */

	/* out for the moment
	if( n >= 4 ) werror( "inefficient switch: %d, %d", n, (int) (range/n) );
	*/

	/* simple switch code */

	for( i=1; i<=n; ++i ){
		/* already in r0 */

		fputs( "\tcmpl\tr0,$", stdout );
		printf( CONFMT, p[i].sval );
		printf( "\n	jeql	.L%d\n", p[i].slab );
		}

	if( p->slab>=0 ) branch( p->slab );
	}

makeheap(p, m, n)
register struct sw *p;
{
	register int q;

	q = select(m);
	heapsw[n] = p[q];
	if( q>1 ) makeheap(p, q-1, 2*n);
	if( q<m ) makeheap(p+q, m-q, 2*n+1);
}

select(m) {
	register int l,i,k;

	for(i=1; ; i*=2)
		if( (i-1) > m ) break;
	l = ((k = i/2 - 1) + 1)/2;
	return( l + (m-k < l ? m-k : l));
}

walkheap(start, limit)
{
	int label;


	if( start > limit ) return;
	printf("	cmpl	r0,$%d\n",  heapsw[start].sval);
	printf("	jeql	.L%d\n", heapsw[start].slab);
	if( (2*start) > limit ) {
		printf("	jbr 	.L%d\n", heapsw[0].slab);
		return;
	}
	if( (2*start+1) <= limit ) {
		label = getlab();
		printf("	jgtr	.L%d\n", label);
	} else
		printf("	jgtr	.L%d\n", heapsw[0].slab);
	walkheap( 2*start, limit);
	if( (2*start+1) <= limit ) {
		printf(".L%d:\n", label);
		walkheap( 2*start+1, limit);
	}
}

int wloop_level = LL_BOT;	/* "while" loop test at loop bottom */
int floop_level = LL_DUP;	/* duplicate "for" loop test at top & bottom */

char *tmpname = "/tmp/pcXXXXXX";

main( argc, argv ) int argc; register char *argv[]; {

	register int i;
	register int c;
	register int fdef = 0;
	register char *cp;
	register int r;
	int dexit();

	for( i=1; i<argc; ++i ){
		if( *(cp=argv[i]) == '-' && *++cp == 'X' ){
			while( *++cp ){ /* set flags and other local options */
				switch( *cp ){

				case 'r':	/* print UNIX release no. */
					fprintf( stderr, "Release: %s\n",
						uxrel );
					break;

				case 'P':	/* profiling */
					++proflg;
					break;

				case 'g':	/* SDB option */
					++gdebug;
					/*
					* Cause simple-minded loop code
					* generation.  No surprises.
					*/
					floop_level = wloop_level = LL_TOP;
					break;
					}
				}
			}
		if( *(argv[i]) != '-' ) { /* open input and output files */
			if( fdef < 2 ){
#ifdef FORT
				if( freopen(argv[i], !fdef ?"r":"a", !fdef ?stdin:stdout) == NULL)
#else
				if( freopen(argv[i], !fdef ?"r":"w", !fdef ?stdin:stdout) == NULL)
#endif
				{
					fprintf(stderr, "ccom:can't open %s\n", argv[i]);
					exit(1);
				}
				++fdef;
				}
			}
		} /* end of for loop */

	mktemp(tmpname);
	if(signal( SIGHUP, SIG_IGN) != SIG_IGN) signal(SIGHUP, dexit);
	if(signal( SIGINT, SIG_IGN) != SIG_IGN) signal(SIGINT, dexit);
	if(signal( SIGTERM, SIG_IGN) != SIG_IGN) signal(SIGTERM, dexit);
	tempfile = fopen( tmpname, "w" );

	r = mainp1( argc, argv );

	if ( tempfile && ferror( tempfile ) )
		cerror( "Problems writing; probably out of temp-file space" );
	tempfile = freopen( tmpname, "r", tempfile );
	if( tempfile != NULL )
		while((c=getc(tempfile)) != EOF )
			putchar(c);
	else cerror( "Lost temp file" );
	unlink(tmpname);
	if ( ferror( stdout ) )
		cerror( "Problems with output file; probably out of temp space" );
	exit( r );

	}

dexit( v ) {
	unlink(tmpname);
	exit(1);
	}
