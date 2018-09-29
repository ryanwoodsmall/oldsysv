/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nm:common/setflags.c	1.13"
/* UNIX HEADER */
#include	<stdio.h>

/* SGS SPECIFIC HEADERS */
#include	"paths.h"
#include	"sgs.h"

/* NAMELIST HEADER */
#include	"defs.h"


int		numbase = DECIMAL; /* number base */
int		Tflag = OFF;	/* ON if names are to be truncated if needed */
int		vflag = OFF;	/* ON if sort externals by value */
int		nflag = OFF;	/* ON if sort externals by name */
int		eflag = OFF;	/* ON if only externals and statics wanted */
int		uflag = OFF;    /* ON if undefined symbol output is wanted */
int		hflag = OFF;	/* ON if no header is to be printed	  */
int		fullflag = OFF;	/* ON if full output is desired */
int		pflag = OFF;    /* ON if parsable style format (similar to
				   PDP-11 format) is desired */
int		pprflag=OFF;	/* ON if header is to be printed with pflag-
				   not user defined */
int		rflag = OFF;	/* ON if filename is to be prepended to each
				 * variable name */

char		*sortname;	/* name of tmp for sort	*/

char		sortcommand[MAXLEN];	/* command line to invoke sort	*/

char		outbuf[BUFSIZ];


    /*  setflags(flagc, flagv)
     *
     *  scans the command line for flag arguments, setting appropriate flags
     *  eliminates flag arguments from the command line
     *  forms temporary file names and command strings for -f and -n or -v flags
     *  buffers standard output if output is not to be directed to a temp file
     *
     *  defines:
     *      - numbase = HEX (non-UNIX default)
     *		      = DECIMAL (UNIX default or -d flag)
     *		      = OCTAL (-o flag)
     *      - vflag, Tflag, nflag, eflag, fullflag, 
     *			uflag, hflag, pflag, pprflag, rflag = OFF (default)
     *		      = ON (-v flag) (-n flag) (-e flag) (-f flag)
     *      - sortname = TMPDIR/nms?????  if -v or -n flag is on
     *      - sortcommand = sort -t'|' sortfield sortname  if -v or -n flag on
     *	        sortfield is set to name field (+0 -1) if -n flag is on
     *		sortfield is set to value field (+1 -2) if -v flag is on
     *          sortfield is set to both (concatenated) if both flags are on
     *
     *  prints:
     *      - an error message for any unrecognized flag
     *
     *  returns filec, the number of non flag arguments in the command line
     *  (each is assumed to be an object file or archive file name)
     */


int
setflags(flagc, flagv)

int	flagc;
char	**flagv;

{
    /* FUNCTIONS called */
    extern 		fprintf( ),
			sprintf( );
    extern char		*strcat( );
    extern int		getpid( );

    extern int		numbase,
			Tflag,
			vflag,
			nflag,
			eflag,
			uflag,
			hflag,
			fullflag,
			pflag,
			pprflag,
			rflag;

    extern char		outbuf[ ];


    static char	sortfield[16] = "";
    int		filec;
    char	**filev;


    filec = 0;

    for (filev = flagv; flagc > 0; --flagc, ++flagv) {

	if (**flagv == '-') {
	    while(*++*flagv != '\0') {
		switch (**flagv) {
		    case 'd':
			numbase = DECIMAL;
			break;
		    case 'o':
			numbase = OCTAL;
			break;
		    case 'x':
			numbase = HEX;
			break;
		    case 'T':
			Tflag = ON;
			break;
		    case 'v':
			if (vflag == OFF) {
			    if (pflag == OFF || rflag == ON)
			    	strcat(sortfield, "+1 -2 ");
			    else
				strcat(sortfield, "+0 -1 ");
			    vflag = ON;
			}
			break;
		    case 'n':
			if (nflag == OFF) {
			    if (pflag == OFF)
			    	strcat(sortfield, "+0 -1 ");
			    else if (rflag == OFF)
			        strcat(sortfield, "+2 -3 ");
			    else
				strcat(sortfield, "+3 -4 ");
			    nflag = ON;
			}
			break;
		    case 'e':
			eflag = ON;
			break;
		    case 'f':
			fullflag = ON;
			break;
		    case 'u':
			uflag = ON;
			break;
		    case 'h':
			hflag = ON;
			break;
		    case 'V':
			fprintf(stderr,"%snm - %s\n",SGS,RELEASE);
			break;
		    case 'p':
			strcpy(sortfield,"");
			if (vflag == ON)
				strcat(sortfield, rflag ? "+1 -2 " : "+0 -1 ");
			if (nflag == ON)
				strcat(sortfield, rflag ? "+3 -4 " : "+2 -3 ");
			pflag = ON;
			break;
		    case 'r':
			if (pflag == ON && (vflag == ON || nflag == ON))  {
				strcpy(sortfield,"");
				strcat(sortfield, nflag ? "+3 -4 " : "+1 -2 ");
			}
			rflag = ON;
			break;
		    default:
			fprintf(stderr,"%snm:  unknown option \"%c\" ignored\n",
				SGS, **flagv);
			break;
		}
	    }
	} else {
	    *filev++ = *flagv;
	    ++filec;
	}
    }
    if (filec > 1 && pflag == ON)
	pprflag = ON;

    if (uflag == ON) {
	vflag = OFF;	/* no values for undefined symbols so no sort	*/
	nflag = OFF;	/* sort on name not compatible			*/
	eflag = OFF;	/* you get this for free; no need to duplicate	*/
	fullflag = OFF;	/* full not compatible with undefined	*/
    }

    setbuf(stdout, outbuf);

    if (vflag == ON || nflag == ON) {
	sortname= tempnam(TMPDIR, "nms");
	if (pflag == ON)
	   sprintf(sortcommand, "sort %s %s", sortfield, sortname);
	else
	   sprintf(sortcommand, "sort -t'|' %s %s", sortfield, sortname);
    }

    return(filec);
}

/*
 */
