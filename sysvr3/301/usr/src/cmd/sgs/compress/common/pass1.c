/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)compress:common/pass1.c	1.9"

/*  program to remove duplicate structures and unions */
/*  2 pass process;  1st pass makes comparisons     2nd pass compresses,
 *  reading complete object file and redirecting pointers    
 */

#include <stdio.h>
#include "filehdr.h"
#include "syms.h"
#include "storclass.h"
#include "ldfcn.h"
#include "tagitem.h"
#include <sys/types.h>
#include <sys/stat.h>

#define SYMNAME(x)	((x)->n_zeroes ? (x)->n_name : (x)->n_nptr)


/*	TAGITEM is a chain containing information needed in 
 *	comparing various tags.  there are as many tagitems as there
 *	are union and structure declaractions.
 */
LDFILE	*input;
SYMENT	*symtable;
TAGITEM	*tagtable;
TAGITEM	*nexttag;
FILE	*compress;

/*  variable representing total # of entries removed    */
long 	delsum = 0;

long	numdup=0; /* number of duplicate tags */
long	strsize;
long	pflag;	/* print statistical information */


/*  main opens object file and reads header info   
 */

main(argc, argv)
	int	argc;
	char	*argv[];
{
	LDFILE	*ldopen();
	void	firstpass();
	void	secpass();
	void	copy();
	void	exit();

	extern long	delsum;
	extern int	optind;	/* needed for getopt() */
	int		c;
	int		errflag=0;
	struct stat	statrec;

	while ((c = getopt(argc, argv,"pv")) != EOF) {
		switch (c) {
			case 'p':
				pflag++;
				break;
			case 'v':
				fprintf(stderr, "Obsolete option '-v' ignored\n");
				break;
			case '?':
				errflag++;
				break;
		}
	}
	if ( (argc != optind + 2) || errflag )
	{
		(void) fprintf(stderr,"usage: cprs [-pv] infile outfile\n");
		exit (1);
	}

	if ((input = ldopen( argv[optind], (LDFILE *) NULL)) == NULL)
	{
		fprintf( stderr, "cannot open %s\n", argv[optind] );
		exit( 2 );
	}

	firstpass( argv[optind] );
	if ((compress = fopen( argv[optind+1], "w" )) == NULL )
	{
		fprintf( stderr, "unable to create %s\n", argv[optind+1] );
		exit( 2 );
	}
	if ( stat( argv[optind], &statrec ) != 0 )
	{
		(void) fprintf( stderr, "cannot stat %s\n", argv[optind] );
		exit( 2 );
	}

	if ( delsum == 0)
	{
		FSEEK( input, 0, 0 );
		copy( statrec.st_size );
	}
	else
		secpass();

	(void) chmod( argv[optind+1], (int) statrec.st_mode );
	fclose( compress );
	ldaclose( input );
	exit(0);
	/*NOTREACHED*/
}


void
firstpass( argv )
	char	*argv;
{
	char	*malloc();
	char	*init_strings();
	void	tagstor();

	register SYMENT		*symbuf;
	register long 		nnext;
	register long		nsyms;
	register char		*strbase;
	register long		tagtotal=0;	/* total number of tags read */

	if (!(ISCOFF(HEADER(input).f_magic)))
	{
		(void) fprintf(stderr, "%s has incorrect magic number\n", argv);
		exit(2);
	}

	if ((nsyms = HEADER( input ).f_nsyms) == 0)
	{
		(void)fprintf(stderr, "%s has no symbol table\n", argv);
		exit(2);
	}
	if ((symtable = (SYMENT *) malloc( nsyms * sizeof( SYMENT ))) == NULL
		|| (tagtable = (TAGITEM *) calloc( nsyms / 4, sizeof( TAGITEM ))) == NULL)
	{
		(void) fprintf( stderr, "cannot malloc enough space\n" );
		exit( 2 );
	}
	nexttag = tagtable;

	symbuf = symtable;
	FSEEK( input, HEADER( input ).f_symptr, 0 );
	for ( nnext = 0; nnext < nsyms; nnext++, symbuf++ )
		if ( FREAD( symbuf, SYMESZ, 1, input ) != 1 )
		{
			(void) fprintf( stderr,"cannot read symbol table\n" );
			exit( 2 );
		}

	/*
	* get initial string table set up
	*/
	strbase = init_strings( input );

	for ( nnext = 0, symbuf = symtable; nnext < nsyms; )
	{
		if ( symbuf->n_zeroes == 0 )
			symbuf->n_nptr = strbase + symbuf->n_offset;
		nnext += 1 + symbuf->n_numaux;
		symbuf += 1 + symbuf->n_numaux;
	}

	for ( nnext = 0, symbuf = symtable; nnext < nsyms; nnext++, symbuf++ )
	{
		/*  call subroutine to store all entries of tag       */
		if (symbuf->n_sclass== C_STRTAG || symbuf->n_sclass == C_UNTAG)
		{
			tagstor( symbuf, nnext );
			tagtotal++;
		}

		if ( symbuf->n_numaux != 0 )
		{
			nnext += symbuf->n_numaux;
			symbuf += symbuf->n_numaux;
		}
	}
	if (pflag) {
		printf("number of tags read %ld\n", tagtotal);
		if (delsum == 0)
			printf("no duplicate tags\n");
		else {
			printf("total number of duplicate tags = %ld\n", numdup);
			printf("compression reduces symbol table %ld%%\n",
							(delsum * 100L)/nsyms);
		}
	}
}

/*  function tagstor saves all tag entry info so comparison can be done */

void
tagstor(symbuf, nthis)
	register SYMENT	*symbuf;
	long 	nthis;
{
	extern TAGITEM	*compar();

	register TAGITEM *realval;
	AUXENT		*auxptr;

	auxptr = (AUXENT *) symbuf + 1;
	nexttag->nentrys = auxptr->x_sym.x_fcnary.x_fcn.x_endndx - nthis;
	nexttag->symptr = symbuf;
	nexttag->oldloc = nthis;

	/*  compare for duplicate tags    */

	nexttag->newloc = nexttag->oldloc - delsum;
	if ((realval = compar( nexttag )) != NULL)
	{
		delsum = delsum + nexttag->nentrys;
		nexttag->realtag = realval;
		numdup++;
	}

	nexttag++;
}


/*  function to compare requires last tag read in and compares it against
 *  all others       
 */

TAGITEM *
compar( p_test )
	register TAGITEM *p_test;
{
	register TAGITEM	*p_real;

	for ( p_real = tagtable; p_real < nexttag; p_real++ )
		if (pair( p_real, p_test ))
			return( p_real );
	return( NULL );
}


/* the function pair, does actual comparison of 2 given data structures  */


int
pair( preal, ptest )
	register TAGITEM *preal, *ptest;
{
	register SYMENT	*ptr1;
	register SYMENT	*ptr2;
	AUXENT	*aux1, *aux2;
	int 	i,
		aux;

/* pair takes the elements of 2 different data structures and compares each
 * element.  if any element varies, the 2 are different and function returns
 * zero to calling routine.          */
/* if all elements are equal, the function returns one     */

	if (preal->nentrys != ptest->nentrys || preal->realtag != 0L )
		return( FALSE );

	ptr1 = preal->symptr;
	ptr2 = ptest->symptr;

	/* ptr1 & ptr2 point to symbol table infor. stored.  Compare tagname  
 	 * and auxentry of each tag.                
	 */

	if (strcmp( SYMNAME( ptr1 ), SYMNAME( ptr2 )) != 0 )
                if ( !(gen( SYMNAME( ptr1 )) && gen( SYMNAME( ptr2 ))))
                        return( FALSE );


	if (ptr1->n_scnum != ptr2->n_scnum
		|| ptr1->n_value != ptr2->n_value
		|| ptr1->n_sclass != ptr2->n_sclass 
		|| ptr1->n_type != ptr2->n_type 
		|| ptr1->n_numaux != ptr2->n_numaux) 
		return( FALSE );

	/*  tag names matched, now test the aux. entry for respective tags     
	 */
	if (ptr1->n_numaux != 0)
	{
		aux1 = (AUXENT *) ptr1 + 1;
		aux2 = (AUXENT *) ptr2 + 1;
		if (aux1->x_sym.x_misc.x_lnsz.x_size != aux2->x_sym.x_misc.x_lnsz.x_size)   
			return( FALSE );
	}

	aux = ptr1->n_numaux;
	ptr1 += aux + 1;
	ptr2 += aux + 1;

	for ( i=0; i < preal->nentrys - (aux + 3); i++)
	{
		if ( !memcom(ptr1, ptr2, ptest, preal))
			return( FALSE );
		ptr1 += ptr1->n_numaux + 1;
		ptr2 += ptr2->n_numaux + 1;
		i += ptr1->n_numaux;
	}

	/*  all members are equal; compare .eos and auxentry  */

	if (ptr1->n_sclass != C_EOS && ptr2->n_sclass != C_EOS)   
		return( FALSE );
	if (ptr1->n_value != ptr2->n_value  
		|| ptr1->n_numaux != ptr2->n_numaux)  
		return( FALSE );

	aux1 = (AUXENT *) ptr1 + 1;
	aux2 = (AUXENT *) ptr2 + 1;
	for ( i = 0; i < ptr1->n_numaux; i++, aux1++, aux2++ )
		if (aux1->x_sym.x_misc.x_lnsz.x_size != aux2->x_sym.x_misc.x_lnsz.x_size) 
			return( FALSE );

	return( TRUE );
}


/*  memcom compares corresponding members of structures
 */

int
memcom( ptr1, ptr2, ptest, preal )
	register SYMENT	*ptr1;
	register SYMENT	*ptr2;
	register TAGITEM *ptest, *preal;
{
	AUXENT *aux1, *aux2;

	if (strcmp( SYMNAME( ptr1 ), SYMNAME( ptr2 )) != 0
		|| ptr1->n_value != ptr2->n_value
		|| ptr1->n_scnum != ptr2->n_scnum
		|| ptr1->n_type != ptr2->n_type
		|| ptr1->n_sclass != ptr2->n_sclass
		|| ptr1->n_numaux != ptr2->n_numaux)
		return( FALSE );

	if (ptr1->n_numaux != 0)
	{
		if (BTYPE( ptr1->n_type ) == T_STRUCT || BTYPE( ptr1->n_type ) == T_UNION || ISARY( ptr1->n_type ))
		{
			aux1 = (AUXENT *) ptr1 + 1;
			aux2 = (AUXENT *) ptr2 + 1;
			if (aux1->x_sym.x_misc.x_lnsz.x_size != aux2->x_sym.x_misc.x_lnsz.x_size )

				return( FALSE );
			if ( auxcom( ptr1, aux1, aux2, ptest, preal ) == FALSE )
				return( FALSE );
		}
	}
	return( TRUE );
}


/* gen checks to see if symbol name is compiler generated
*/


int
gen( px )
	register char *px;
{
	int pos = 1;

	if (*px != '.')
		return(0);
	px++;
	pos++;
	while(*px >= '0' && *px <= '9')
	{
		px++;
		pos++;
	}
	if(pos== 1 || pos > 5)
		return(0);
	if(*px != 'f' || *(px+1) != 'a' || *(px+2) != 'k' || *(px+3) != 'e')
		return(0);
	return(1);
}


/* auxcom compares corresponding aux.entries for members being tested    */

auxcom( ptr1, aux1, aux2, ptest, preal )
	register SYMENT	*ptr1;
	register AUXENT	*aux1, *aux2;
	register TAGITEM *ptest, *preal;
{
	SYMENT	*check1, *check2;

	if (BTYPE( ptr1->n_type ) == T_STRUCT
		|| BTYPE(ptr1->n_type) == T_UNION)
	{  
		if (aux1->x_sym.x_tagndx == preal->oldloc
			&& aux2->x_sym.x_tagndx == ptest->oldloc) 
			return( TRUE );

		if (aux1->x_sym.x_tagndx == preal->oldloc)
			check1 = preal->symptr;
		else
                	for (preal = tagtable; preal != nexttag; preal++ )
				if (preal->oldloc == aux1->x_sym.x_tagndx)
				{
					check1 = preal->symptr;
					break;
				}

		/* At this point check1 points to something reasonable	 */

		if ( aux2->x_sym.x_tagndx == ptest->oldloc )
			check2 = ptest->symptr;
		else
			for (ptest = tagtable; ptest != nexttag; ptest++ )
			{
				if (ptest->oldloc == aux2->x_sym.x_tagndx)
				{
					check2 = ptest->symptr;
					break;
				}
			}

		if (check1 == check2)
			return( TRUE );
             	if ( strcmp( SYMNAME( check1 ), SYMNAME( check2 )) != 0 )
                        if ( !(gen( SYMNAME( check1 )) && gen( SYMNAME( check2 ))))
                                return( FALSE );

		if ( check1->n_value != check2->n_value
			|| check1->n_type != check2->n_type
			|| check1->n_sclass != check2->n_sclass )
			return( FALSE );

		if ( check1->n_numaux != check2->n_numaux )
			return( FALSE );
		return( TRUE );

	}	/* if BTYPE */
	return( TRUE );
}



char *
init_strings( infile )
	LDFILE	*infile;
{
	extern char *malloc();
	register char	*str_table;

	FSEEK( infile, STROFFSET( infile ), 0 );
	if (FREAD( (char *) &strsize, sizeof( long ), 1, infile ) != 1 )
	{
		strsize = 0;
		return( NULL );
	}

	if ((str_table = malloc( strsize )) == NULL)
	{
		fprintf(stderr, "Insufficient memory for string table\n");
		exit( 2 );
	}
	if ( FREAD( str_table + 4, strsize - 4, 1, infile ) != 1 )
	{
		fprintf( stderr, "cannot read string table\n" );
		exit( 2 );
	}
	return( str_table );
}
