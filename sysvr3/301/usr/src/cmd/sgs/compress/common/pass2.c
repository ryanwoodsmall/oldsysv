/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)compress:common/pass2.c	1.6"

#include <stdio.h>
#include "filehdr.h"
#include "syms.h"
#include "scnhdr.h"
#include "linenum.h"
#include "reloc.h"
#include "ldfcn.h"
#include "tagitem.h"

/*  pass 2 of the compress utility compresses the symbol table and updates
    all pointers into the symbol table  */

extern LDFILE	*input;
extern FILE	*compress;
extern void	copy();
extern long	delsum;
extern TAGITEM	*tagtable;
extern TAGITEM	*nexttag;
extern SYMENT	*symtable;
extern long	strsize;

#define NEWLOC(x)	( (x)->realtag ? (x)->realtag->newloc : (x)->newloc )

void
secpass()
{
	long		newindx();
	long 		proln();
	long 		prorel();
	void		prosym();
	SCNHDR 		scptr;
	FILHDR 		fhead;

	register int 	i;
	register long	indx;
	long		datasze;
	int		sumrel;
	int		sumln;
	int		totsec;
	long		frstrel;
	long		frstln;

	sumrel = sumln = 0;
	indx = 0;
	frstrel = frstln = 0;


	fhead = HEADER(input);
	fhead.f_nsyms = HEADER(input).f_nsyms - delsum;
	totsec = HEADER(input).f_nscns;
	indx += FILHSZ;

	if ( fwrite( (char *) &fhead, FILHSZ, 1, compress ) != 1 )
	{
		fprintf( stderr, "cannot write output file\n" );
		exit( 2 );
	}

	/*  if there is an optional header   copy to compress  */
	if ( ldohseek(input) == SUCCESS)
		copy( (long)HEADER(input).f_opthdr );
	indx += HEADER(input).f_opthdr;

	/*  read section header information    */

	for( i=0; i<totsec; i++)
	{
		if ( FREAD( (char *) &scptr, SCNHSZ, 1, input) != 1)
		{
			fprintf(stderr, "read on scnheader failed\n");
			exit( 2 );
		}

		/*  save value for index of first reloc and line sections   */

		if (i==0 || frstrel == 0)
			frstrel = scptr.s_relptr;
		if (i == 0 || frstln == 0)
			frstln = scptr.s_lnnoptr;
		sumrel += scptr.s_nreloc;
		sumln += scptr.s_nlnno;

	/*  copy section information to compress file   */

		if ( fwrite( (char *) &scptr, SCNHSZ, 1, compress) != 1)
		{
			fprintf(stderr, "write on scnheader failed\n");
			exit( 2 );
		}
		indx += SCNHSZ;
	}


	/*  read raw data for all sections and copy to compress   */

	if ( sumrel !=0)
		datasze = frstrel - indx;
	else if (sumln !=0)
	  	datasze = frstln - indx;
	else
		datasze = HEADER( input ).f_symptr - indx;

	if ( datasze != 0)
		copy( datasze );
	indx += datasze;

	/*  if header flag not set, update relocation section  */

	if ( !(HEADER(input).f_flags & F_RELFLG) && sumrel != 0)
		indx = prorel(indx, sumrel);

	/* update linenumber section if flag not set    */

	if ( !(HEADER(input).f_flags & F_LNNO) && sumln != 0L)
		indx = proln(indx, sumln);

	/*  call function to modify symbol table  */
	prosym( HEADER( input ).f_nsyms );
}


/* function "proln" adjust linenumber information for compressed file  */


long
proln( indx, lnsum )
	register long	indx;
	register int	lnsum;
{
	extern long	newindx();

	struct  lineno lnptr;
	register int 	k;

	/* verify position of file ptr.  */

	for ( k=0; k < lnsum; k++)
	{

		if ( FREAD(&lnptr, LINESZ, 1, input) != 1)
		{
			fprintf(stderr, "cannot read lineno info\n");
			exit( 2 );
		}
		if (lnptr.l_lnno == 0)
			lnptr.l_addr.l_symndx=newindx(lnptr.l_addr.l_symndx);
		if ( fwrite(&lnptr, LINESZ, 1, compress) != 1)
		{
			fprintf(stderr, "write to lineno failed\n");
			exit( 2 );
		}
		indx += LINESZ;
	}
	return(indx);
}


/*  function prorel, updates relocation entries for compression  *?
*/

long
prorel( indx, sumrel )
	register long indx; 
	register int sumrel;
{
	struct reloc 	relptr;
	register long 		k;

	for( k=0; k<sumrel; k++)
	{
		if ( FREAD(&relptr, RELSZ, 1, input) < 1)
		{
			fprintf(stderr, "read rel failed\n");
			exit( 2 );
		}

		relptr.r_symndx = newindx(relptr.r_symndx);
		if ( fwrite(&relptr, RELSZ, 1, compress) != 1)
		{
			fprintf(stderr, "write to rel failed\n");
			exit( 2 );
		}

		indx = indx + RELSZ;
	}
	return(indx);
}



/*  function "newindx" computes newloc for pointers into the symbol table  */

long
newindx(x)
	register long x;
{
	register TAGITEM *tag_ptr;

	/*  compare number against old/new/real table,  if less than 1st entry
 	 *  return the same value         */

	for ( tag_ptr = tagtable; tag_ptr < nexttag; tag_ptr++ )
	{
		if ( x == tag_ptr->oldloc )
			return( NEWLOC( tag_ptr ));
		else if ( x < tag_ptr->oldloc )
		{
			if ( tag_ptr == tagtable )
				return(x);
			x -= (tag_ptr->oldloc - tag_ptr->newloc);
			return(x);
		}
		else if ( (tag_ptr + 1) == nexttag && x > tag_ptr->oldloc )
		{
			x -= delsum;
			return(x);
		}
	}

	fprintf( stderr, "failed to compute indx\n" );
	exit(1);
	/*NOTREACHED*/
}


/*  function "realindx" gives actual index of any "real" tag  */

/* given that newloc != oldloc in our table, tag is C_ALIAS  */

long
realindx(x)
	register long x;
{
	register TAGITEM *tag_ptr;

	for ( tag_ptr = tagtable; tag_ptr < nexttag; tag_ptr++ )
	{
		if (x == tag_ptr->oldloc )
			return( NEWLOC( tag_ptr ));
	}

	fprintf(stderr, "failed to compute realindx at %ld\n", x);
	exit(2);
	/*NOTREACHED*/
}



/*  "prosym" processes entries in symbol table by actually deleting 
 *     duplicate structures and modifying pointer information   */

void
prosym( nsyms )
	register long nsyms;
{
	extern long	realindx(),
			newindx();
	char	*calloc();

	register long 	i, j;
	char 	*s;
	long	slength;
	register SYMENT	*symptr;
	register AUXENT	*auxptr;
	char		*strbase;
	char		*strnext;

	if ( strsize && (strbase = calloc( strsize, 1 )) == NULL )
	{
		fprintf( stderr, "insufficient memory or string table\n" );
		exit( 2 );
	}
	strnext = strbase;

	for ( i = 0, symptr = symtable; i < nsyms; i++, symptr++ )
	{
nextsym:
		if ( symptr->n_numaux == 0)
		{
			if ( symptr->n_sclass == C_ALIAS)
				symptr->n_value = realindx( symptr->n_value );
			goto writesym;
		}

		auxptr = (AUXENT *) symptr + 1;
		switch (symptr->n_sclass)
		{
		case C_BLOCK:
			s = symptr->n_name;
			if (*s=='.' && *(++s)=='b' && *(++s)=='b')
				auxptr->x_sym.x_fcnary.x_fcn.x_endndx
					= newindx( auxptr->x_sym.x_fcnary.x_fcn.x_endndx );
			break;

		case C_ENTAG:
			auxptr->x_sym.x_fcnary.x_fcn.x_endndx = newindx(auxptr->x_sym.x_fcnary.x_fcn.x_endndx);
			auxptr->x_sym.x_tagndx=newindx(auxptr->x_sym.x_tagndx);
			break;

		case C_EOS:
			auxptr->x_sym.x_tagndx=newindx(auxptr->x_sym.x_tagndx);
			break;

		case C_FILE:
			symptr->n_value=newindx(symptr->n_value);
			break;

		case C_STRTAG:
		case C_UNTAG:
  			if ( tagindx( i ) != -1 )
			{
				i = auxptr->x_sym.x_fcnary.x_fcn.x_endndx;
				symptr = symtable + i;
				goto nextsym;
			}
			else
				auxptr->x_sym.x_fcnary.x_fcn.x_endndx=newindx(auxptr->x_sym.x_fcnary.x_fcn.x_endndx);
			break;

		default:
			if (ISFCN(symptr->n_type))
				auxptr->x_sym.x_fcnary.x_fcn.x_endndx=newindx(auxptr->x_sym.x_fcnary.x_fcn.x_endndx);

			switch BTYPE(symptr->n_type)
			{
			case T_ENUM:
				if (auxptr->x_sym.x_tagndx != 0L)
					auxptr->x_sym.x_tagndx = newindx(auxptr->x_sym.x_tagndx);
				break;

			case T_STRUCT:
			case T_UNION:
				if (auxptr->x_sym.x_tagndx != 0L)
					auxptr->x_sym.x_tagndx = realindx(auxptr->x_sym.x_tagndx);
				break;
			}  /* end of second switch  */

			break;
		}  /*  end of main switch  */

writesym:
		if (symptr->n_zeroes == 0)	/* put name in string table */
		{
			strcpy( strnext, symptr->n_nptr );
			symptr->n_offset = (strnext - strbase) + 4;	/* new offset */
			strnext += strlen( strnext ) + 1;
		}

		if ( fwrite( (char *) symptr, SYMESZ, 1, compress) != 1)
		{
			fprintf(stderr, "failed to write symbol\n");
			exit( 2 );
		}
		for ( i += symptr->n_numaux, j = symptr->n_numaux; j; j-- )
		{
			symptr++;
			if ( fwrite( (char *) symptr, AUXESZ, 1, compress) != 1)
			{
				fprintf(stderr, "failed to write aux\n");
				exit( 2 );
			}
		}
	}

	/*
	* Write out modified string table.  Place string table length
	* at beginning.  Don't bother to check stuff at end of old file.
	*/
	slength = strnext - strbase;
	if ( slength )
	{
		slength += 4;
		if (fwrite((char *)&slength, sizeof(long), 1, compress) != 1
			|| fwrite(strbase, slength - 4, 1, compress) != 1 )
		{
			fprintf(stderr, "failed to write string table\n");
			exit( 2 );
		}
	}
}



/*  "tagindx" computes value for C_ALIAS entries   */


int
tagindx( symindx )
	register long symindx;
{
	register TAGITEM	*tag_ptr;

	/* search tag table for oldloc of given tag indx   */

	for ( tag_ptr = tagtable; tag_ptr < nexttag; tag_ptr++ )
	{
		if ( symindx == tag_ptr->oldloc )
		{
			if ( tag_ptr->realtag )
				return( tag_ptr->realtag->newloc );
			else
				return( -1 );
		}
	}
	return( -1 );
}


void
copy( size )
	register long	size;
{
	extern int	fread( ),
			fwrite( );

	char		buffer[BUFSIZ];
	register long		index;
	register short		remaindr;


	for ( index = size/BUFSIZ; index > 0; --index )
		if (FREAD( buffer, BUFSIZ, 1, input ) < 1
			|| fwrite (buffer, BUFSIZ, 1, compress ) != 1)
		{
			fprintf( stderr, "cannot copy input\n" );
			exit( 2 );
		}

	if ((remaindr = size % BUFSIZ) != 0)
		if (FREAD( buffer, remaindr, 1, input ) != 1
			|| fwrite( buffer, remaindr, 1, compress ) != 1)
		{
			fprintf( stderr, "cannot copy input\n" );
			exit( 2 );
		}
}
