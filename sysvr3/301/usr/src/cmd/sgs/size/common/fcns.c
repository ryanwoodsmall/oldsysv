/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)size:common/fcns.c	1.5"
/* UNIX HEADER */
#include	<stdio.h>

/* COMMON SGS HEADERS */
#include	"filehdr.h"
#include	"scnhdr.h"
#include	"ldfcn.h"

/* SGS SPECIFIC HEADER */
#include	"sgs.h"

/* SIZE HEADER */
#include	"defs.h"


    /*  error(file, string)
     *
     *  simply prints the error message string
     *  simply returns
     */


error(file, string)

char	*file;
char	*string;

{
     /* UNIX FUNCTIONS CALLED */
    extern	fprintf( ),
		fflush( );

    fflush(stdout);
    fprintf(stderr, "%ssize:  %s:  %s\n", SGS, file, string);

    return;
}


/*
*/
