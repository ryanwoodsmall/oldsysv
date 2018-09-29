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
/* UNIX HEADERS */
#include	<stdio.h>
#include	<ar.h>

/* COMMON SGS HEADERS */
#include	"filehdr.h"
#include	"scnhdr.h"
#include	"ldfcn.h"

/* NAMELIST HEADER */
#include	"defs.h"

/* STATIC VARIABLES USED */
static char	*prahead[2] = {
#ifdef PORTAR
			"\n\nSymbols from %s[%.16s]:\n\n",
			"\n\nUndefined symbols from %s[%.16s]:\n\n"
#else
			"\n\nSymbols from %s[%.14s]:\n\n",
			"\n\nUndefined symbols from %s[%.14s]:\n\n"
#endif
};

static char	*prhead[2] = {
			"\n\nSymbols from %s:\n\n",
			"\n\nUndefined symbols from %s:\n\n"
};

/* EXTERNAL VARIABLES DEFINED */
LDFILE		*tagptr;
FILE		*formout,
		*sortout;
char		(*section)[8];

    /*  process(filename)
     *
     *  directs the namelisting of the named file
     *
     *  process verifies that there are in fact symbols in the named file
     *  builds a list of section names to be used in printing the symbols
     *  opens temporaries based on -f and on -v and -n flags
     *  based on -e flag calls either proext or prosym to print the symbol table
     *  based on -v or -n flag invokes system sort routine
     *  based on -f flag invokes awk with the appropriate SGSnmawk program
     *
     *  defines:
     *      - tagptr = ldaopen(filename, ldptr)  used to read tag indexes in
     *		       the symbol table
     *      - formout = fopen(formname, "w")  a temporary for output to be
     *			processed by awk
     *      - sortout = fopen(sortname, "w")  a temporary for output to be
     *			sorted
     *      - section = bldscnlst( )  an array of section names
     *
     *  calls:
     *      - bldscnlst( )  to build array of section names
     *      - prosym(section)  to process all the symbols in the symbol table
     *      - proext(section)  to process only external and static symbols (-e)
     *      - error(file, string, level)  to print error messages
     *		  level indicates kind of clean-up to be associated with error
     *
     *  prints:
     *      - a title for the name list
     *        uses the static format strings in prahead (for archive files) or
     *	      prhead.  The string used is based on value of numbase (HEX or
     *	      OCTAL).
     *
     *	simply returns
     */


process(filename)

char		*filename;

{
    /* UNIX FUNCTIONS CALLED */
    extern FILE		*fopen( );
    extern		fclose( ),
			fflush( ),
			fprintf( ),
			sprintf( );
    extern int		system( );
    extern		free( ),
			exit( );

    /* COMMON OBJECT FILE ACCESS ROUTINES CALLED */
    extern LDFILE	*ldaopen( );
    extern int		ldaclose( );

    /* NAMELIST FUNCTIONS CALLED */
    extern char		**bldscnlst( );
    extern int		prosym( ),
			proext( );
    extern		error( );

    /* EXTERNAL VARIABLES USED */
    extern LDFILE	*ldptr,
			*tagptr;
    extern int		fancyflag,
			vflag,
			nflag,
			eflag,
			uflag,
			hflag,
			numbase;
    extern char		formname[ ],
			sortname[ ],
			formcommand[ ],
			sortcommand[ ];
    extern char		(*section)[8];



    if (HEADER(ldptr).f_nsyms == 0) {
	error(filename, "no symbols", 0);
	return;
    }

    /*	bldscnlst( ) builds list of section names used in printing table */
    if ((section = bldscnlst( )) == NULL) {
	error(filename, "cannot build list of section names", 0);
	return;
    }

    /*	depending on flags open up temporary files for intermediate output */
    if (fancyflag == ON) {
	if ((formout = fopen(formname, "w")) == NULL) {
	    error(filename,
		"cannot open temporary file (-f option); cannot proceed", 1);
	    exit(FATAL);
	}
    } else {
	formout = stdout;
    }

    if (vflag == ON || nflag == ON) {
	if ((sortout = fopen(sortname, "w")) == NULL) {
	    error(filename,
		"cannot open temporary file (-[vn] option); cannot proceed", 1);
	    exit(FATAL);
	}
    } else {
	sortout = formout;
    }


    /*	PRINT HEADING
     *	"S" in Symbol tells awk that output should be formatted in HEX style
     *  "s" in symbol tells awk that output should be formatted in OCTAL style
     */

#ifdef PORTAR
    if (TYPE(ldptr) == ARTYPE) {
#else
    if (TYPE(ldptr) == ARMAG) {
#endif
	ARCHDR	arhead;

	if (ldahread(ldptr, &arhead) != SUCCESS) {
	    error(filename, "cannot read archive header", 2);
	    return;
	}

	if ( hflag == OFF )
	    fprintf(formout, prahead[uflag], filename, arhead.ar_name);

    } else {

	if (hflag == OFF)
	    fprintf(formout, prhead[uflag], filename);
    }

    if ((tagptr = ldaopen(filename, ldptr)) == NULL) {
	fflush(formout);
	error(filename, "cannot open for additional processing", 3);
	return;
    }

    if (eflag == OFF) {
	if (prosym(section) == FAILURE) {
	    fflush(formout);
	    error(filename, "cannot process symbol table (bad format)", 4);
	    return;
	}
    } else {
	if (proext(section) == FAILURE) {
	    fflush(formout);
	    error(filename, "cannot process symbol table (bad format)", 4);
	    return;
	}
    }

    ldaclose(tagptr);

#ifdef PORTAR
    if (uflag == ON)
	return((int) filename);
#endif
    if (fancyflag == ON) {
	fclose(formout);
    } else {
	fflush(formout);
    }

    if (vflag == ON || nflag == ON) {
	fclose(sortout);

	if (system(sortcommand) != OKAY) {
	    error(filename, "abnormal termination during sort", 1);
	    exit(FATAL);
	}
    }

    if (fancyflag == ON) {
	if (system(formcommand) != OKAY) {
	    error(filename, "abnormal termination in -f option", 1);
	    exit(FATAL);
	}
    }

    cfree(section);

    return;
}




    /*  bldscnlst( )
     *
     *  builds an array of section names
     *
     *  bldscnlst allocates memory for the array (each element of the array is
     *  an eight character string)
     *  and then initializes each element to the name of a section
     *
     *  returns a pointer to the array
     */


char **
bldscnlst( )

{
    /* UNIX FUNCTIONS CALLED */
    extern char		*calloc( );

    /* COMMON OBJECT FILE ACCESS ROUTINES CALLED */
    extern int		ldshread( );

    /* EXTERNAL VARIABLES USED */
    extern LDFILE	*ldptr;

    /* LOCAL VARIABLES */
    SCNHDR		secthead;
    char		(*sectname)[8],
			(*scnname)[8];
    unsigned short	sect;
    int			i;


    if ((sectname = (char **) calloc(HEADER(ldptr).f_nscns, 8)) == NULL) {
	return(NULL);
    }

    for (sect = 1, scnname = sectname; sect <= HEADER(ldptr).f_nscns;
	 ++sect, ++scnname) {

	if (ldshread(ldptr, sect, &secthead) != SUCCESS) {
	    return(NULL);
	}

	for (i = 0; i < 8; ++i) {
	    (*scnname)[i] = secthead.s_name[i];
	}
    }

    return(sectname);
}

/*
 *	static char ID[] = "@(#) process.c: 1.5 3/5/83";
 */
