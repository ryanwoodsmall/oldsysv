/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nm:common/main.c	1.9"
/* UNIX HEADER */
#include	<stdio.h>

/* COMMON SGS HEADERS */
#include	"filehdr.h"
#include	"ldfcn.h"

/* SGS SPECIFIC HEADER */
#include	"sgs.h"

/* NAMELIST HEADER */
#include	"defs.h"

/* EXTERNAL VARIABLES DEFINED */
LDFILE		*ldptr;

    /*
     *  main(argc, argv)
     *
     *  parses the command line (setflags( ))
     *  prepares to field interrupts  (catchsig( ))
     *  opens, processes and closes each command line object file argument 
     *      (ldopen( ), process( ), ldclose( ))
     *  cleans up after itself (cleanup( ))
     *
     *  defines:
     *      - LDFILE	*ldptr = ldopen(*argv, ldptr) for each obj file arg
     *
     *  calls:
     *      - setflags(--argc, ++argv) to set flags and associated variables
     *      - catchsig( ) to set up interrupt catching mechanism
     *      - process(*argv) to direct the namelisting of the obj file *argv
     *      - cleanup( ) to unlink temporary files
     *
     *  prints:
     *      - a usage message if there are no command line object file args
     *      - an error message if it can't open a command line obj file arg
     *        or if the opened object file doesn't have the right magic number
     *
     *  exits successfully always
     */


int
main(argc, argv)

int	argc;
char	**argv;

{
    /* UNIX FUNCTIONS CALLED */
    extern 		fprintf( ),
			sprintf( ),
    			exit( );

    /* OBJECT FILE ACCESS ROUTINES CALLED */
    extern LDFILE	*ldopen( );
    extern int		ldclose( ),
			ldaclose( );

    /* NM FUNCTIONS CALLED */
    extern int		setflags( );
    extern		process( ),
			catchsig( ),
			cleanup( );

    /* EXTERNAL VARIABLES USED */
    extern LDFILE	*ldptr;



    /*  setflags eliminates flag arguments from argv;
     *  recall that argv[0] is the command name itself
     */
    if ((argc = setflags(--argc, ++argv)) == 0) {
	fprintf(stderr, "usage:  %snm [-o|x|d] [-V] [-T] [-v] [-h] [-n] [-e] [-f] [-u] [-p] [-r] file ...\n",SGS);
        exit(0);
    }

    catchsig( );

    for (	; argc > 0; --argc, ++argv) {
	ldptr = NULL;
	do {
	    if ((ldptr = ldopen(*argv, ldptr)) != NULL) {
		if (ISCOFF(HEADER(ldptr).f_magic)) {
		    process(*argv);
		} else {
		    fprintf(stderr, "%snm:  %s:  bad magic\n", SGS, *argv);
		    ldaclose(ldptr);
		}
	    } else {
		fprintf(stderr, "%snm:  %s:  cannot open\n", SGS, *argv);
	    }
	} while (ldclose(ldptr) == FAILURE);
    }

    cleanup( );
    exit(0);
}

/*
 */
