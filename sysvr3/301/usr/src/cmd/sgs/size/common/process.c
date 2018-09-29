/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)size:common/process.c	1.11"
/* UNIX HEADER */
#include	<stdio.h>
#include	<ar.h>

/* COMMON SGS HEADERS */
#include	"filehdr.h"
#include	"scnhdr.h"
#include	"ldfcn.h"
#include	"sgs.h"

/* SIZE HEADERS */
#include	"defs.h"
#include	"process.h"


    /*  process(filename)
     *
     *  prints out the sum total of section sizes along with the size
     *  information (size, physical and virtual addresses) for each section
     *  in the object file
     *  uses static format strings to do the printing (see process.h).
     *
     *  calls:
     *      - error(file, string) if the object file is somehow messed up
     *
     *  simply returns
     */


process(filename)

char	*filename;

{
    /* UNIX FUNCTIONS CALLED */
    extern		printf( );

    /* COMMON OBJECT FILE ACCESS ROUTINE CALLED */
    extern int		ldshread( );

    /* SIZE FUNCTIONS CALLED */
    extern		error( );

    /* EXTERNAL VARIABLES USED */

    extern int		fflag,
			nflag;
    extern int		numbase;
    extern LDFILE	*ldptr;

    /* LOCAL VARIABLES */
    long		size;
    unsigned short	section;
    SCNHDR		secthead;
    ARCHDR		arhead;
    extern int		ldahread( );
    long		ssize[10];
    char		*sname[10];
    long		flags;
    int			notfirst=0;
    long		textsize=0,	/* holds total size of all text sections */
			datasize=0,	/* holds total size of all data sections */
			bsssize=0;	/* holds total size of all bss sections */

    if (ISARCHIVE(TYPE(ldptr))) {
    	if (ldahread(ldptr, &arhead) == SUCCESS) {
    		printf("%s:", arhead.ar_name);
    	} else {
    		printf("%s: ", filename);
	}
    }

    if (!fflag) {
	/* Scan through section headers and see if ouput can be printed in the
	 * default format.  If a "regular" section (loaded and allocated) or
	 * NOLOAD section (if the -n flag is on) is found
	 * which is not solely text, data or bss, then the output must be printed
	 * in the full format. */
	for (section = 1; section <= HEADER(ldptr).f_nscns; ++section) {
		if (ldshread(ldptr, section, &secthead) != SUCCESS) {
	    		error(filename, "cannot read section header");
	    		return;
		}
		flags= secthead.s_flags;
		if (((flags & 0x1f) == STYP_REG && 
			!((flags & STYP_INFO) || (flags & STYP_OVER) || 
			  (flags & STYP_LIB)  || (flags & STYP_NOLOAD))) ||
			(nflag && (flags & STYP_NOLOAD))) {
	
			/* This section will be included in the size; check to see
			 * if it is solely text, data or bss. */
			if ( !( (flags & STYP_TEXT) ||
				(flags & STYP_DATA) || 
				(flags & STYP_BSS)  ||
				(strncmp(secthead.s_name,".text",8) == 0) ||
				(strncmp(secthead.s_name,".data",8) == 0) ||
				(strncmp(secthead.s_name,".bss",8)  == 0) ) ||

				((flags & STYP_TEXT) && (flags & STYP_DATA)) ||
			   	((flags & STYP_TEXT) && (flags & STYP_BSS )) ||
			   	((flags & STYP_DATA) && (flags & STYP_BSS )) ) {

				fflag++; /* output will be in full format */
				break;
			}
		}
    	}
    }

    size = 0L;
    for (section = 1; section <= HEADER(ldptr).f_nscns; ++section) {
	if (ldshread(ldptr, section, &secthead) != SUCCESS) {
	    error(filename, "cannot read section header");
	    return;
	}
	flags= secthead.s_flags;
	if (((flags & 0x1f) == STYP_REG && 
		!((flags & STYP_INFO) || (flags & STYP_OVER) || 
		  (flags & STYP_LIB)  || (flags & STYP_NOLOAD))) ||
		(nflag && (flags & STYP_NOLOAD)) || fflag) {

		if (fflag) {
			if (notfirst)
				printf(" + ");
			printf(prusect[numbase], secthead.s_size);
	    	    	printf("\(%.8s\)", secthead.s_name);
		} else {
			if ((flags & STYP_TEXT) || 
					(strncmp(secthead.s_name,".text",8) == 0)) 
				textsize+= secthead.s_size;
			else if ((flags & STYP_DATA) || 
					(strncmp(secthead.s_name,".data",8) == 0)) 
				datasize+= secthead.s_size;
			else if ((flags & STYP_BSS) || 
					(strncmp(secthead.s_name,".bss",8) == 0)) 
				bsssize+= secthead.s_size;
		}
		size += secthead.s_size;
		notfirst++;
	}
    }

    if (!fflag) {
	printf(prusect[numbase],textsize);
	printf(" + ");
	printf(prusect[numbase],datasize);
	printf(" + ");
	printf(prusect[numbase],bsssize);
    }

    printf(prusum[numbase], size);
    return;
}
