/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)list:common/lnnofcns.c	1.3"
#include	<stdio.h>
#include	"filehdr.h"
#include	"scnhdr.h"
#include	"linenum.h"
#include	"ldfcn.h"
#include	"defs.h"


unsigned short
findlnno(fcndx, section, lnnoptr)

long		fcndx;
unsigned short	section;
long		lnnoptr;

{
    extern int		fseek( ),
			fread( );

    extern int		ldshread( );

    extern unsigned short	ndxlnno( );

    extern LDFILE	*ldptr;

    SCNHDR		secthead;
    LINENO		line;
    long		endptr;


    if (ldshread(ldptr, section, &secthead) == SUCCESS) {
	if (lnnoptr == 0) {
	    return(ndxlnno(fcndx, section, secthead.s_nlnno));
	}

	endptr = secthead.s_lnnoptr + secthead.s_nlnno * LINESZ;
	if ((lnnoptr >= secthead.s_lnnoptr) && (lnnoptr < endptr)) {
	    if (((endptr - lnnoptr) % LINESZ) == 0) {
		if (FSEEK(ldptr, lnnoptr, BEGINNING) == OKFSEEK) {
		    if (FREAD(&line, LINESZ, 1, ldptr) == 1) {
			if (line.l_lnno == 0) {
			    return((unsigned short)((endptr-lnnoptr)/LINESZ)-1);
			}
		    }
		}
	    }
	}
    }

    return((unsigned short) 0);
}




unsigned short
ndxlnno(fcndx, section, nlnnos)

long		fcndx;
unsigned short	section;
unsigned short	nlnnos;

{
    extern int		fseek( ),
			fread( );

    extern int		ldlseek( );

    extern LDFILE	*ldptr;

    LINENO		line;


    if (nlnnos != 0) {
	if (ldlseek(ldptr, section) == SUCCESS) {
	    do {
		if (FREAD(&line, LINESZ, 1, ldptr) != 1) {
		    return((unsigned short) 0);
		}
		if (line.l_lnno == 0) {
		    if (line.l_addr.l_symndx == fcndx) {
			return(--nlnnos);
		    }
		}
	    } while (--nlnnos != 0);
	}
    }

    return((unsigned short) 0);
}




unsigned short
getlnno( )

{
    extern int		fread( );

    extern		error( );

    extern LDFILE	*ldptr;
    extern char		*objname;

    LINENO		line;


    if (FREAD(&line, LINESZ, 1, ldptr) != 1) {
	error(objname, "cannot read line number entries", FATAL);
    }

    return(line.l_lnno);
}

/*
*/
