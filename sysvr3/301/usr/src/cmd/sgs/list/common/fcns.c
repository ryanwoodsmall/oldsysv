/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)list:common/fcns.c	1.4"
/* UNIX HEADER */
#include	<stdio.h>

/* COMMON SGS HEADERS */
#include	"filehdr.h"
#include	"ldfcn.h"

/* SGS SPECIFIC HEADER */
#include	"sgs.h"

/* LISTER HEADER */
#include	"defs.h"

    /*  error(file, message)
     *
     *  prints the error message "message" identifying the lister and 
     *    the offending source or object file "file"
     */


error(file, message)

char	*file;
char	*message;

{
    /* UNIX FUNCTIONS CALLED */
    extern		fflush( ),
			fprintf( ),
			fclose( );


    /*  because setbuf( ) is used to buffer the standard output and stderr
     *  is used to print error messages, the error messages might appear in an
     *  inappropriate place unless stdout is fflush'd before the error message
     *  is printed
     */

    fflush(stdout);
    fprintf(stderr, "%slist:  %s:  %s\n", SGS, file, message);

    return;
}

/*
 *	fatal()
 *
 *	print the error message, close the input files, and exit
 */

fatal( message )
char	*message;

{
	/* OBJECT FILE ACCESS ROUTINES CALLED */
	extern int		ldaclose( );

	/* EXTERNAL VARIABLES USED */
	extern LDFILE	*ldptr;
	extern FILE	*srcptr;
	extern char	*objname;

	fflush( stdout );
	fprintf( stderr, "%slist: %s: %s\n", SGS, objname, message );

	fclose(srcptr);
	ldaclose(ldptr);

	exit(1);
}

/*
*/
