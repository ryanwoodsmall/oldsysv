/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)list:common/main.c	1.9"
/* UNIX HEADER */
#include	<stdio.h>

/* COMMON SGS HEADERS */
#include	"filehdr.h"
#include	"ldfcn.h"

/* SGS SPECIFIC HEADER */
#include	"sgs.h"

/* LISTER HEADER */
#include	"defs.h"

/* EXTERNAL VARIABLES DEFINED */
FILE		*srcptr;
char		outbuf[BUFSIZ];
#if DMERT
char		srcname[BUFSIZ/2];
#endif

    /*	The Lister (a.k.a. b16list or n3blist)
     *	operates on:
     *	    - one or more C source files specified as command line arguments
     *	    - one Common object file
     *           - taken to be the last non-C source command line file argument
     *	         - or the default object file for the given sgs (b16a.out or
     *	           n3ba.out)
     *
     *	main(argc, argv)
     *
     *  parses the command line (setflags( ))
     *	opens and checks the object file (openobject( ))
     *	processes each C source file (process( ))
     *	closes the object file (ldaclose( ))
     *
     *	defines the global variable:
     *	    - FILE	*srcptr = fopen(*argv, "r") for each C source file, *argv
     *
     *  calls:
     *	    - setflags(--argc, ++argv)	to eliminate non-C source file name
     *	      arguments from argv and to determine the name of the object file
     *	    - openobject( ) to open the object file 
     *	    - process(*argv) to direct the listing of the C source file *argv
     *      - error(*argv, string) to print a standard format lister error
     *        message (string)
     *
     *	prints:
     *	    - a usage message if there are no C source file command line args
     *
     *	exits successfully always
     */


int
main(argc, argv)

int	argc;
char	**argv;

{
    /* UNIX FUNCTIONS CALLED */
    extern	fprintf( );
#if DMERT
    extern FILE	*vfopen( );
#else
    extern FILE	*fopen( );
#endif
    extern	fclose( ),
		exit( );
    extern int	fflush( );

    /* OBJECT FILE ACCESS FUNCTIONS CALLED */
    extern int	ldaclose( );

    /* LISTER FUNCTIONS CALLED */
    extern int	setflags( ),
		openobject( );
    extern	process( ),
		error( );

    /* EXTERNAL VARIABLES USED */
    extern FILE		*srcptr;
    extern LDFILE	*ldptr;
    extern char		outbuf[ ];
#if DMERT
    extern int		tflag;
#endif


    /*  setflags( ) eliminates all non C source file arguments from argv;
     *  recall that argv[0] is the name of the command itself
     */
    if ((argc = setflags(--argc, ++argv)) == 0) {
	fprintf(stderr, "usage:  %slist [-h] [-V] [-Ffunction ] sourcefile [sourcefile ... ] [objectfile]\n", SGS);
	exit(0);
    }

    if (openobject( ) == SUCCESS) {
	/*  buffer standard output 
	 *  normally output directed to a terminal is not buffered and
	 *  they say buffered output is better
	 *  but be forwarned that buffer has to be flushed before error output
	 *  is printed since it too may be directed to a terminal
	 */
	setbuf(stdout, outbuf);
	for (	; argc > 0; --argc, ++argv) {
#if DMERT
		if ((srcptr = vfopen(*argv, "r")) != NULL) {
			if (!tflag)
				sprintf( srcname, "%s", *argv );
			process( srcname );
#else
		if ((srcptr = fopen(*argv, "r")) != NULL) {
			process(*argv);
#endif
			fclose(srcptr);
		} else {
			error(*argv, "cannot open");
		}
	}
	ldaclose(ldptr);
    }

    exit(0);
}

/*
*/
