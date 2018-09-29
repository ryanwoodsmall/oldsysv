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
#include	<stdio.h>
#include	"filehdr.h"
#include	"ldfcn.h"
#include	"filedefs.h"
#include	"dumpmap.h"
#include	"sgs.h"


main(argc, argv) 

int	argc;
char	**argv;

{
    extern		fprintf( ),
			exit( ),
			cfree( );

    extern int		ldaclose( );

    extern int		setflags( );
    extern FILELIST	*openobject( );
    extern 		process( );

    FILELIST		*listhead,
			*listptr,
			*nextptr;


    if ((argc = setflags(--argc, ++argv)) == 0) {
	fprintf(stderr, "usage:  %sdump [flags] _f_i_l_e ...\n", SGS);
	exit(0);
    }

    listhead = NULL;

    for (	; (argc > 0) && ((listhead = openobject(*argv)) == NULL);
	 --argc, ++argv);

    --argc;
    ++argv;

    for (listptr = listhead; argc > 0; --argc, ++argv) {
	if ((listptr->nextitem = openobject(*argv)) != NULL) {
	    listptr = listptr->nextitem;
	}
    }

    if (listhead != NULL) {
	process(listhead);

	for (listptr = listhead; listptr != NULL; listptr = nextptr) {
	    nextptr = listptr->nextitem;

	    ldaclose(listptr->ldptr);
	    cfree(listptr);
	}
    }

    exit(0);
}

/*
*     static char	ID[ ] = "@(#) main.c: 1.4 3/31/83";
*/
