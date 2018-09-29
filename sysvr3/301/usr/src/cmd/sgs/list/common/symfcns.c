/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)list:common/symfcns.c	1.6"
/* UNIX HEADER */
#include	<stdio.h>

/* COMMON SGS HEADERS */
#include	"filehdr.h"
#include	"syms.h"
#include	"ldfcn.h"

/* LISTER HEADER */
#include	"defs.h"

/*  NEXTSYM is used to increment a symbol table index (x).  If the index grows
 *  beyond the size of the symbol table it may not be necessary to look at
 *  the start of the symbol table, only at the symbols beginning at y
 */
#define NEXTSYM(x, y)	((x) < HEADER(ldptr).f_nsyms - 1) ? (x) + 1 : (y)

/* STATIC VARIABLES */
static long	lastfilndx = 0,
		symndx = 0,
		lastfcndx = 0;

    /*  symfcns.c contains a pair of functions that search the symbol table
     *  of the object file for source file or function names
     *
     *  the following static variables are used to localize the search
     *	    - symndx  is the symbol table index of the NEXT symbol table
     *		entry to be read.
     *	    - lastfilndx  is the symbol table index of the LAST FILE entry
     *		processed.
     *		If symndx ever grows (mod HEADER(ldptr).f_nsyms) equal
     *		to lastfilndx:  the named file is not in the symbol table.
     *	    - lastfcndx  is the symbol table index of the LAST FUNCTION entry
     *		processed.
     *		It is set when the named file is found in the symbol table
     *		If, in findfcn( ), symndx ever grows (bounded by lastfilndx
     *		and the next FILE entry in the symbol table) to lastfcndx:
     *		the named function is not in the symbol table.
     *
     *
     *	findfile(filename)  simply discovers whether or not the named file 
     *	is entered in the symbol table of the object file (objname).
     *
     *	The variables symndx and lastfcndx are used only to make
     *	searching the symbol table a little bit quicker.
     *
     *	The variable lastfilndx is more important if there happen
     *	to be two functions of the same name (one or more static)
     *	in the same object file.
     *
     *  returns:
     *      - SUCCEED when the appropriate filename is found
     *      - FAIL otherwise
     *
     *  calls fatal( string ) if the symbol table is messed up.
     */


int
findfile(filename)

char	*filename;

{
    /* UNIX FUNCTIONS CALLED */
    extern int		fread( ),
			strncmp( );

    /* OBJECT FILE ACCESS FUNCTION CALLED */
    extern int		ldtbread( );

    /* LISTER FUNCTION CALLED */
    extern		fatal( );

    /* EXTERNAL VARIABLES USED */
    extern LDFILE	*ldptr;
#if DMERT
   extern int		numsame;
#endif

    /* LOCAL VARIABLES */
    char		*fileonly;
    SYMENT		symbol;
    AUXENT		aux;
#if DMERT
    int			samefound;

    samefound = 0;
#endif

    /* set fileonly to simple file name without any path name */
    for (fileonly = filename; *filename != '\0'; ++filename) {
	if (*filename == '/') {
	    fileonly = filename + 1;
	}
    }

    do {
	if (ldtbread(ldptr, symndx, &symbol) != SUCCESS) {
	    fatal( "cannot read symbol table" );
	}

	/* all "interesting" symbols have auxiliary entries */
	if (symbol.n_numaux == 1) {
	    if (FREAD(&aux, AUXESZ, 1, ldptr) != 1) {
		fatal( "cannot read symbol table" );
	    }

	    if (symbol.n_sclass == C_FILE) {
		if (strncmp(fileonly, aux.x_file.x_fname, FILNMLEN) == 0) 
#if DMERT
		    if (samefound++ == numsame ) {
#else
		    {
#endif
			/*  found the file name:
			 *  set lastfilndx, symndx and lastfcndx
			 *    lastfilndx  to the index of the file name entry
			 *    symndx  to the index of the next symbol to look at
			 *    lastfcndx  to the place to start looking for a fcn
			 */
			lastfilndx = symndx;
			symndx = NEXTSYM(symndx + 1, 0);
			lastfcndx = symndx;
			return(SUCCEED);
		    }
		symndx = NEXTSYM(symndx + 1, 0);
	    } else if ((ISFCN(symbol.n_type) || ISTAG(symbol.n_sclass)) &&
		       (aux.x_sym.x_fcnary.x_fcn.x_endndx > (symndx + 1))) {
		/*  the auxiliary entry has a pointer around the end of
		 *  the function or the structure (union or enum)
		 */
		symndx = aux.x_sym.x_fcnary.x_fcn.x_endndx;
	    } else {
		/*  external symbols that are not functions are not associated
		 *  with any specific source file:  don't need to look at them
		 */
		symndx = NEXTSYM(symndx + 1, 0);
	    }
	} else {
	    if (symbol.n_sclass == C_EXT) {
		symndx = 0;
	    } else {
		symndx = NEXTSYM(symndx, 0);
	    }
	}
    } while (symndx != lastfilndx);
    /*  if symndx grows to the index of the last file processed, the file
     *  we are looking for is not in the symbol table
     */

    return(FAIL);
}




    /*	findfcn(fcnname)  discovers whether the named function is
     *	entered in the symbol table.
     *	If it is, then findfcn returns its index in the symbol table
     *  otherwise findfcn returns LONGFAIL
     *
     *  calls fatal( string ) if the symbol table is messed up
     */


long
findfcn(fcnname)

char		*fcnname;

{
    /* UNIX FUNCTION CALLED */
    extern int		strcmp( );

    /* COMMON OBJECT FILE ACCESS FUNCTION CALLED */
    extern int		ldtbread( );
    extern char		*ldgetname( );

    /* LISTER FUNCTION CALLED */
    extern		fatal( );

    /* EXTERNAL VARIABLES USED */
    extern LDFILE	*ldptr;

    /* LOCAL VARIABLES */
    SYMENT		symbol;
    AUXENT		aux;
    char		*symname;


    if (symndx >= HEADER(ldptr).f_nsyms )  /*symndx out of range */
	symndx = lastfilndx + 2;
    do {
	if (ldtbread(ldptr, symndx, &symbol) != SUCCESS) {
	    fatal( "cannot read symbol table" );
	}

	/*  again all interesting symbols have auxiliary entries */
	if (symbol.n_numaux == 1) {
	    if (FREAD(&aux, AUXESZ, 1, ldptr) != 1) {
		fatal( "cannot read symbol table" );
	    }

	    if (ISFCN(symbol.n_type)) {
		if (((symname = ldgetname( ldptr, &symbol)) != NULL)
			&& (strcmp( fcnname, symname ) == 0)) {
		    /*  found the function 
		     *  set lastfcndx and symndx
		     *    lastfcndx  to the index of the function entry
		     *    symndx  to the next symbol to look at
		     */
		    lastfcndx = symndx;
		    symndx = aux.x_sym.x_fcnary.x_fcn.x_endndx;

		    return(lastfcndx);
		}
		symndx = aux.x_sym.x_fcnary.x_fcn.x_endndx;
	    } else if (ISTAG(symbol.n_sclass)) {
		symndx = aux.x_sym.x_fcnary.x_fcn.x_endndx;
	    } else if (symbol.n_sclass == C_FILE) {
		/*  don't want to look at the symbols associated with another
		 *  file -- so wrap around to the entries associated with this
		 *  file, and don't bother to look at file entry and it's aux
		 *  (therefore look at the entry at lastfilndx + 2)
		 */
		symndx = lastfilndx + 2;
	    } else {
		symndx = NEXTSYM(symndx + 1, lastfilndx + 2);
	    }
	} else {
		symndx = NEXTSYM(symndx, lastfilndx + 2);
	}
    } while ( (symndx != lastfcndx) && (symndx < HEADER(ldptr).f_nsyms) );
    /*  if symndx grows to the index of the last function processed, the
     *  function we are looking for is not in the symbol table
     */

   if (symndx >= HEADER(ldptr).f_nsyms)
	symndx = lastfilndx + 2;

    return(LONGFAIL);
}

/*
*/
