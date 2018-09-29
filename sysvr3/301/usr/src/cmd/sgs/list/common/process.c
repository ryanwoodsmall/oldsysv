/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)list:common/process.c	1.7"
/* UNIX HEADER */
#include	<stdio.h>

/* SGS SPECIFIC HEADER */
#include	"sgs.h"

/* LISTER HEADER */
#include	"defs.h"

/* EXTERNAL VARIABLES DEFINED */
int ok_to_print = 1;

    /*	process(filename)
     *
     *	finds the named source file (filename) in the object file sym table
     *	and for each function in the source file
     *	    - finds the named function (fcnname) in the sym table
     *      - lists the function with line numbers from the obj file
     *
     *	calls:
     *	    - findfile(filename) to find the named file in the object file
     *	      symbol table
     *	    - findname(filename) to find a function in the source file
     *	    - findfcn(fcnname) to find the function in the symbol table
     *	    - printfcn(filename, fcndx) to print out the function with line
     *	      numbers
     *	    - error(string, string) to print an error message
     *        when any function called fails
     *      - getline( ) to initialize a buffer for reading the source file
     *
     *  simply returns
     */


int
process(filename)

char	*filename;

{
    /* UNIX FUNCTIONS CALLED */
    extern			printf( );

    /* LISTER FUNCTIONS CALLED */
    extern int			findfile( ),
				getline( ),
				printfcn( );
    extern char			*findname( );
    extern long			findfcn( );
    extern			error( );

    /* EXTERNAL VARIABLES USED */
    extern int			h_flag;
    extern char			*objname;
    extern char			buffer[];
    extern int			bufndx;
    extern short		inif;
    extern char			*ffunction[];
    extern int			fflag;

    /* LOCAL VARIABLES */
    char			*fcnname;
    long			fcndx;
#if DMERT
    char			dfunc[10];
#endif
    int				i;
    int				print_code;


    if (findfile(filename) == FAIL) {
	error(filename, "not in symbol table");
	return;
    }

    /*  title */
    if (h_flag != 1)	/* suppress page heading ? */
	printf("\n\n%s with line numbers from %s:\n", filename, objname);

    if (getline( ) == EOF) {
	error(filename, "unexpected end-of-file");
	return;
    }

    while ((fcnname = findname(filename)) != NULL) {

	if (fflag)
	{
		for ( i = 0; i < fflag; i++ )
			if (strcmp( fcnname, ffunction[i] ) == 0)
				break;
		if (i == fflag)
			continue;
	}

	if ((fcndx = findfcn(fcnname)) == LONGFAIL) {
#if DMERT
		sprintf(dfunc, "$%s", fcnname);
		dfunc[8] = NULL;
		if ((fcndx = findfcn(dfunc)) == LONGFAIL) {
#else
		{
#endif
			if ( inif == 0 ) {
				fflush( stdout );
				fprintf(stderr, "%slist: %s: %s: cannot find function\n", SGS, filename, fcnname);
			} else if (fflag)
				ok_to_print = 0;
			continue;
		}
	}

	if (fflag)
		ok_to_print = 1;
	if ((print_code = printfcn(filename, fcndx)) == FAIL) 
		return;
	else if (print_code == NOTFOUND)
		continue;

	if (fflag)
		ok_to_print = 0;

#if DMERT
	for ( i=strlen(buffer)+1; i > bufndx; --i) {
		buffer[i+2] = buffer[i];
	}
	buffer[++bufndx] = FUNCEND;
	buffer[++bufndx] = '\n';
#endif
    }
    return;
}


/*
*/
