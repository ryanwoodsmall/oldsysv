/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)list:common/initfcns.c	1.8"
/* UNIX HEADER */
#include	<stdio.h>

/* COMMON SGS HEADERS */
#include	"filehdr.h"
#include	"ldfcn.h"

/* SGS SPECIFIC HEADERS */
#include	"paths.h"
#include	"sgs.h"

/* LISTER HEADER */
#include	"defs.h"

/* EXTERNAL VARIABLES DEFINED */
int		h_flag = 0;
char		*objname = NULL;
LDFILE		*ldptr;
int		fflag = 0;
char		*ffunction[MAXFUNCS];
#if DMERT
int		numsame = 0;
int		tflag = 0;
char		Oname[BUFSIZ/2];
#endif

/* EXTERNAL VARIABLES USED */
extern		int	ok_to_print;
#if DMERT
extern		char	srcname[];
#endif

/* STATIC VARIABLES */
static char	*aoutname = A_OUT;

    /*	initfcns.c is a collection of functions called before the Lister
     *  begins processing C source files
     *
     *	setflags(flagc, flagv)
     *
     *	eliminates non-C source file names from flagv 
     *	      flagv = --argv (argv without the command name)
     *	as far as setflags is concerned flagv contains 3 types of args:
     *	    - C source file names (these are saved in filev)
     *	    - flag arguments (arguments beginning with '-' are ignored)
     *	    - object file names (non-C source file names, the last one is used)
     *	determines the name of the object file to be used
     *
     *	defines:
     *	    - char	*objname = *flagv if *flagv is neither a C file name
     *				   nor a flag argument
     *				 = aoutname if not set to some command line arg
     *
     *	calls:
     *	    - cname(*flagv) to determine if a command line file name is a
     *	      valid C source file name
     *	    - error(objname, string) if more than one non-C source file
     *	      is found in the command line
     *
     *	prints:
     *	    - an error message for any command line argument that begins with
     *	      a '-'
     *
     *	returns filec, the number of C source file name arguments remaining
     *  in argv (they are repositioned in argv through the local variable filev)
     */


int
setflags(flagc, flagv)

int	flagc;
char	**flagv;

{
    /* UNIX FUNCTIONS CALLED */
    extern	fprintf( );

    /* LISTER FUNCTIONS CALLED */
    extern int	cname( );
    extern	error( );

    /* EXTERNAL VARIABLES USED */
    extern char	*objname;

    /* LOCAL VARIABLES */
    int		filec;
    char	**filev;


    for (filec = 0, filev = flagv; flagc > 0; --flagc, ++flagv) {

	if (**flagv == '-') {
	    *++*flagv;
	    switch (**flagv) {

		case 'h':
		    h_flag = 1;
		    break;

		case 'F':
			if (((*flagv)+1) == '\0')
				fprintf( stderr, "%slist: -F must specify a function name", SGS );
			else
			{
				ffunction[fflag] = (*flagv) + 1;
				fflag++;
				if (fflag >= MAXFUNCS)
				{
					fprintf( stderr, "%slist: too many -F options", SGS );
					break;
				}
				ok_to_print = 0;
			}
			break;

#if DMERT
		case 's':
			numsame = atoi((*flagv)+1);
			break;

		case 't':
			tflag = 1;
			sprintf( srcname, "%s", ((*flagv)+1));
			break;

		case 'O':
			sprintf(Oname,"%s",((*flagv)+1));
			break;
#endif

		case 'V':
		    fprintf(stderr,"%slist -%s\n",SGS,RELEASE);
		    break;

		default:
		    fprintf(stderr,"%slist: unknown option \"%s\" ignored\n",
		    SGS,*flagv);
		    break;
	    }
	} else if (cname(*flagv) == FAILURE) {
	    if (objname != NULL) {
		/*  it is an error if more than one object file is specified */
		error(objname, "invalid C source name");
	    }
	    objname = *flagv;
	} else {
	    *filev++ = *flagv;
	    ++filec;
	}
    }

    if (objname == NULL) {
	objname = aoutname;
    }

    return(filec);
}


    /*	openobject( )
     *
     *	opens the object file whose name has been determined by setflags( )
     *  checks to make sure that the object file can be used
     *	      the object file must have the right magic number,
     *	      it must have symbol table and line number entries
     *
     *	defines:
     *	     - LDFILE	*ldptr = ldopen(objname, NULL)
     *
     *  calls:
     *	     - checkflags( ) to determine if the object file has symbols and
     *	       line number entries
     *	     - error(objname, string) if the object file cannot be opened
     *	       or if it doesn't have the right magic number
     *
     *  returns SUCCESS or FAILURE 
     */


int
openobject( )

{
    /* OBJECT FILE ACCESS ROUTINES CALLED */
    extern LDFILE	*ldopen( );

    /* LISTER FUNCTIONS CALLED */
    extern int		checkflags( );
    extern		error( );

    /* EXTERNAL VARIABLES USED */
    extern LDFILE	*ldptr;
    extern char		*objname;


    if ((ldptr = ldopen(objname, NULL)) != NULL) {
	if (ISCOFF(TYPE(ldptr))) {
	    return(checkflags( ));
	} else {
	    error(objname, "bad magic");
	}
    } else {
	error(objname, "cannot open");
    }

    return(FAILURE);
}


    /*	checkflags( )
     *
     *	determines whether the object file has symbols and line numbers
     *	by checking the file header
     *
     *  calls:
     *	    - error(objname, string) if there are either no symbols or
     *	      no line number entries
     *
     *	returns SUCCESS or FAILURE
     */


int
checkflags( )

{
    /* LISTER FUNCTIONS CALLED */
    extern	error( );

    /* EXTERNAL VARIABLES USED */
    extern LDFILE	*ldptr;
    extern char		*objname;


    if (HEADER(ldptr).f_nsyms == 0L) {
	error(objname, "symbols have been stripped; cannot proceed");
	return(FAILURE);
    }

    if ((HEADER(ldptr).f_flags & F_LNNO) != 0) {
	error(objname, "line number entries have been stripped; cannot proceed");
	return(FAILURE);
    }

    return(SUCCESS);
}


    /*	cname(filename)
     *
     *  determines if filename is a valid C source file name
     *  by checking if the name ends in ".c"
     *
     *  returns SUCCESS or FAILURE
     */


int
cname(filename)

char	*filename;

{
    /* LOCAL VARIABLES */
    int		count;

    for (count = 0; *++filename != '\0'; ++count);

    if (count > 1) 
#if DMERT
	if (((*--filename == 'h') || (*filename == 'c')) && (*--filename == '.') )
#else
	if ((*--filename == 'c') && (*--filename == '.')) 
#endif
	    return(SUCCESS);

    return(FAILURE);
}

/*
*/
