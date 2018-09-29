/*	@(#)code.c	1.3	*/
/*	3.0 SID #	1.2	*/
# include <stdio.h>
# include <signal.h>

# include "mfile1"

int proflag = 0;
int strftn = 0;	/* is the current function one which returns a value */
FILE *tmp_file;
FILE *outfile = stdout;

branch( n ){
	/* output a branch to label n */
	/* exception is an ordinary function branching to retlab: then, return */
	printf( "	jbr	L%d\n", n );
	}

int lastloc = PROG;

defalign(n) register n; {
	/* cause the alignment to become a multiple of n */
	n /= SZCHAR;
	if( lastloc != PROG && n > 1 ) printf( "	.even\n" );
	}

locctr( l ) register l; {
	register temp;
	/* l is PROG, ADATA, DATA, STRNG, ISTRNG, or STAB */

	if( l == lastloc ) return(l);
	temp = lastloc;
	lastloc = l;
	switch( l ){

	case PROG:
		outfile = stdout;
		printf( "	.text\n" );
		break;

	case DATA:
	case ADATA:
		outfile = stdout;
		if( temp != DATA && temp != ADATA )
			printf( "	.data\n" );
		break;

	case STRNG:
	case ISTRNG:
		outfile = tmp_file;
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
	/* output something to define the current position as label n */
	fprintf( outfile, "L%d:", n );
	}

beg_file(){}	/* do nothing special at beginning of file */

int crslab = 10;

getlab(){
	/* return a number usable for a label */
	return( ++crslab );
	}

efcode(){
	/* code for the end of a function */

	if( strftn ){  /* copy output (in r0) to caller */
		register struct symtab *p;
		register int stlab;
		register int count;
		int size;

		p = &stab[curftn];

		deflab( retlab );

		stlab = getlab();
		printf( "	mov	$L%d,r1\n", stlab );
		size = tsize( DECREF(p->stype), p->dimoff, p->sizoff ) / SZCHAR;
		count = size/2;
		while( count-- ) {
			printf( "	mov	(r0)+,(r1)+\n" );
			}
		printf( "	mov	$L%d,r0\n", stlab );
		printf( "	.bss\nL%d:	.=.+%d.\n	.text\n", stlab, size );
		/* turn off strftn flag, so return sequence will be generated */
		strftn = 0;
		printf( "	jmp	cret\n" );
		}
	else printf( "L%d:	jmp	cret\n", retlab );
#ifdef ONEPASS
	p2bend();
#else
	printf( "%c\n", BEND );
#endif
	}

#ifdef ONEPASS
int ftlab1, ftlab2;
#endif

bfcode( a, n ) int a[]; {
	/* code for the beginning of a function; a is an array of
		indices in stab for the arguments; n is the number */
	register i;
	register temp;
	register struct symtab *p;
	int off;

	locctr( PROG );
	p = &stab[curftn];
	defnam( p );
	temp = p->stype;
	temp = DECREF(temp);
	strftn = (temp==STRTY) || (temp==UNIONTY);

	retlab = getlab();
	if( proflag ){
		int plab;
		plab = getlab();
		printf( "	mov	$L%d,r0\n", plab );
		printf( "	jsr	pc,mcount\n" );
		printf( "	.bss\nL%d:	.=.+2\n	.text\n", plab );
		}

	/* routine prolog */

	printf( "	jsr	r0,csav\n" );
#ifdef ONEPASS
	ftlab1 = getlab();
	ftlab2 = getlab();
	printf("	jbr	L%d\n", ftlab1);
	printf("L%d:", ftlab2 );
#else
	/* adjust stack for autos */
	printf( "	sub	$.F%d,sp\n", ftnno );
#endif

	off = ARGINIT;

	for( i=0; i<n; ++i ){
		p = &stab[a[i]];
		if( p->sclass == REGISTER ){
			temp = p->offset;  /* save register number */
			p->sclass = PARAM;  /* forget that it is a register */
			p->offset = NOOFFSET;
			oalloc( p, &off );
			printf( "	mov	%d.(r5),r%d\n", p->offset/SZCHAR, temp );
			p->offset = temp;  /* remember register number */
			p->sclass = REGISTER;   /* remember that it is a register */
			}
		else {
			if( oalloc( p, &off ) ) cerror( "bad argument" );
			}

		}
	}

bccode(){ /* called just before the first executable statment */
		/* by now, the automatics and register variables are allocated */
	SETOFF( autooff, SZINT );
	/* set aside store area offset */
#ifdef ONEPASS
	p2bbeg( autooff, regvar );
#else
	printf( "%c%d\t%d\t%d\t\n", BBEG, ftnno, autooff, regvar );
#endif
	}

#ifndef ejobcode
ejobcode( flag ){
	/* called just before final exit */
	/* flag is 1 if errors, 0 if none */
	}
#endif

#ifndef aobeg
aobeg(){
	/* called before removing automatics from stab */
	}
#endif

#ifndef aocode
aocode(p) struct symtab *p; {
	/* called when automatic p removed from stab */
	}
#endif

#ifndef aoend
aoend(){
	/* called after removing all automatics from stab */
	}
#endif

defnam( p ) register struct symtab *p; {
	/* define the current location as the name p->sname */

	if( p->sclass == EXTDEF ){
		printf( "	.globl	%s\n", exname( p->sname ) );
		}
	if( p->sclass == STATIC && p->slevel>1 ) deflab( p->offset );
	else printf( "%s:\n", exname( p->sname ) );

	}

bycode( t, i ) register t,i; {
	/* put byte i+1 in a string */

	i &= 07;
	if( t < 0 ){ /* end of the string */
		if( i != 0 ) fprintf( outfile, "\n" );
		}

	else { /* stash byte t into string */
		if( i == 0 ) fprintf( outfile, "	.byte	" );
		else fprintf( outfile, "," );
		fprintf( outfile, "%o", t );
		if( i == 07 ) fprintf( outfile, "\n" );
		}
	}

zecode( n ) register n; {
	/* n integer words of zeros */
	OFFSZ temp;
	register i;

	if( n <= 0 ) return;
	printf( "	" );
	for( i=1; i<n; i++ ) {
		if( i%8 == 0 )
			printf( "\n	" );
		printf( "0; " );
		}
	printf( "0\n" );
	temp = n;
	inoff += temp*SZINT;
	}

fldal( t ) unsigned t; { /* return the alignment of field of type t */
	uerror( "illegal field type" );
	return( ALINT );
	}

#ifndef fldty
fldty( p ) struct symtab *p; { /* fix up type of field p */
	;
	}
#endif

where(c){ /* print location of error  */
	/* c is either 'u', 'c', or 'w' */
	fprintf( stderr, "%s, line %d: ", ftitle, lineno );
	}

char *tmp_name = "/tmp/pcXXXXXX";

main( argc, argv ) char *argv[]; {
	int dexit();
	register int c;
	register int i;
	register int r;
	int fdef = 0;

	mktemp(tmp_name);
	if(signal( SIGHUP, SIG_IGN) != SIG_IGN) signal(SIGHUP, dexit);
	if(signal( SIGINT, SIG_IGN) != SIG_IGN) signal(SIGINT, dexit);
	if(signal( SIGTERM, SIG_IGN) != SIG_IGN) signal(SIGTERM, dexit);
	tmp_file = fopen( tmp_name, "w" );

	for( i=1; i<argc; ++i ){
		if( *(argv[i]) != '-' ) {
			if( fdef < 2 ){
				if( freopen(argv[i], !fdef ?"r":"w", !fdef ?stdin:stdout) == NULL) {
					fprintf(stderr, "ccom:can't open %s\n", argv[i]);
					exit(1);
					}
				++fdef;
				}
			}
		else if( argv[i][1] == 'X' && argv[i][2] == 'P' )
			proflag++;
		}

	r = mainp1( argc, argv );

	tmp_file = freopen( tmp_name, "r", tmp_file );
	if( tmp_file != NULL )
		while((c=getc(tmp_file)) != EOF )
			putchar(c);
	else cerror( "Lost temp file" );
	unlink(tmp_name);
	return( r );
	}

dexit(x) {
	unlink(tmp_name);
	exit(x);
	}

genswitch(p,n) register struct sw *p;{
	/*	p points to an array of structures, each consisting
		of a constant value and a label.
		The first is >=0 if there is a default label;
		its value is the label number
		The entries p[1] to p[n] are the nontrivial cases
		*/
	register i;
	register CONSZ j, range;
	register dlab, swlab;

	range = p[n].sval-p[1].sval;

	if( range>0 && range <= 3*n && n>=4 ){ /* implement a direct switch */

		dlab = p->slab >= 0 ? p->slab : getlab();

		if( p[1].sval ){
			printf( "	sub	$" );
			printf( CONFMT, p[1].sval );
			printf( ".,r0\n" );
			}

		/* note that this is a cl; it thus checks
		   for numbers below range as well as out of range.
		   */
		printf( "	cmp	r0,$%ld.\n", range );
		printf( "	jhi	L%d\n", dlab );

		printf( "	asl	r0\n" );
		printf( "	jmp	*L%d(r0)\n", swlab = getlab() );

		/* output table */

		locctr( ADATA );
		defalign( ALPOINT );
		deflab( swlab );

		for( i=1,j=p[1].sval; i<=n; ++j ){

			printf( "	L%d\n", ( j == p[i].sval ) ?
				p[i++].slab : dlab );
			}

		locctr( PROG );

		if( p->slab< 0 ) deflab( dlab );
		return;

		}

	/* debugging code */

	/* out for the moment
	if( n >= 4 ) werror( "inefficient switch: %d, %d", n, (int) (range/n) );
	*/

	/* simple switch code */

	for( i=1; i<=n; ++i ){
		/* already in r0 */

		printf( "	cmp	r0,$" );
		printf( CONFMT, p[i].sval );
		printf( ".\n	jeq	L%d\n", p[i].slab );
		}

	if( p->slab>=0 ) branch( p->slab );
	}
