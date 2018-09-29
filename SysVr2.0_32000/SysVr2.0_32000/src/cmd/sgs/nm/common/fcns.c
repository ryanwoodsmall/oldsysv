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
#include	<signal.h>

/* COMMON SGS HEADERS */
#include	"filehdr.h"
#include	"ldfcn.h"

/* SGS SPECIFIC HEADER */
#include	"sgs.h"

/* NAMELIST HEADER */
#include	"defs.h"


    /*  error(file, message, level)
     *
     *  prints the given error message indicating the offending file (given)
     *  and then depending on level takes appropriate clean-up action
     *
     *  simply returns
     */


error(file, message, level)

char	*file;
char	*message;
int	level;

{
    /* UNIX FUNCTIONS CALLED */
    extern		fprintf( ),
			fclose( ),
			free( );

    /* COMMON OBJECT FILE ACCESS ROUTINES CALLED */
    extern int		ldaclose( );

    /* EXTERNAL VARIABLES USED */
    extern LDFILE	*ldptr,
			*tagptr;

    extern FILE		*formout,
			*sortout;
    extern int		fancyflag,
			vflag,
			hflag,
			nflag;
    extern char		(*section)[8];

    fprintf(stderr, "%snm:  %s:  %s\n", SGS, file, message);

    switch(level) {
	case 0:
	    /* an error occurred before any temporary files were opened
	     * and before the object file was again opened for tagptr
	     */
	    break;

	case 1:
	    /* a FATAL error:
	     * simply call onintr( ) to clean-up and exit
	     * it doesn't matter what files are opened, onintr( ) closes
	     * everything
	     */
	    onintr( );
	    break;

	case 2:
	    /* a problem with an archive file
	     * close the archive file so that its next member won't be processed
	     * (and the error repeated)
	     * also close temporary files (if flags say that they must have been
	     * opened)
	     */
	    free(section);
	    ldaclose(ldptr);
	    if (fancyflag) {
		fclose(formout);
	    }
	    if (vflag || nflag) {
		fclose(sortout);
	    }
	    break;

	case 3:
	    /* can't open object file a second time for tagptr */
	    free(section);
	    if (fancyflag) {
		fclose(formout);
	    }
	    if (vflag || nflag) {
		fclose(sortout);
	    }
	    break;

	case 4:
	    /* messed up symbol table */
	    free(section);
	    if (fancyflag) {
		fclose(formout);
	    }
	    if (vflag || nflag) {
		fclose(sortout);
	    }
	    ldaclose(tagptr);
	    break;

	default:
	    break;
    }

    return;
}




    /*  catchsig( )
     *
     *  prepares to field interrupts with the routine onintr( )
     *
     *  namelist fields interrupts so that it can remove the temporary files
     *  it has created if it should be interrupted 
     *
     * catchsig simply returns
     */


catchsig( )

{
    /* UNIX FUNCTION CALLED */
    extern int	(*signal( ))( );

    /* EXTERNAL VARIABLE USED */
    extern		onintr( );

    if ((signal(SIGINT, SIG_IGN)) == SIG_DFL)
	signal(SIGINT, onintr);

    if ((signal(SIGHUP, SIG_IGN)) == SIG_DFL)
	signal(SIGHUP, onintr);

    if ((signal(SIGQUIT, SIG_IGN)) == SIG_DFL)
	signal(SIGQUIT, onintr);

    if ((signal(SIGTERM, SIG_IGN)) == SIG_DFL)
	signal(SIGTERM, onintr);

    return;

}




    /*  onintr( )
     *
     *  cleans up after an interrupt happens (or after a FATAL error)
     *  ignores signals (interrupts) during its work
     *
     *  exits FATALly always
     */


onintr( )

{
    /* UNIX FUNCTIONS CALLED */
    extern int	(*signal( ))( );
    extern		exit( ),
			fprintf( ),
			fclose( ),
			unlink( ),
			free( );

    /* OBJECT FILE ACCESS ROUTINE CALLED */
    extern		ldaclose( );

    /* EXTERNAL VARIABLES USED */
    extern LDFILE	*ldptr,
			*tagptr;
    extern FILE		*formout,
			*sortout;
    extern char		formname[ ],
			sortname[ ];
    extern char		(*section)[8];


    signal(SIGINT, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);

    fprintf(stderr, "\n");

    ldaclose(ldptr);
    ldaclose(tagptr);
    fclose(formout);
     unlink(formname);
    fclose(sortout);
     unlink(sortname);
    free(section);

    exit(FATAL);

}




    /*  cleanup( )
     *
     *  is called by main( ) when namelist terminates normally
     *  cleanup simply unlinks temporary files
     */


cleanup( )

{
    /* UNIX FUNCTION CALLED */
    extern		unlink( );

    /* EXTERNAL VARIABLES USED */
    extern char		formname[ ],
    			sortname[ ];

    unlink(formname);
    unlink(sortname);

}

/*
 *	static char ID[] = "@(#) fcns.c: 1.3 4/14/82";
 */
