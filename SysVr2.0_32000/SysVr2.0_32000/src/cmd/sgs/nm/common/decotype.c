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
/* UNIX HEADER */
#include	<stdio.h>

/* COMMON SGS HEADERS */
#include	"filehdr.h"
#include	"syms.h"
#include	"ldfcn.h"

/* NAMELIST HEADERS */
#include	"defs.h"
#include	"decotype.h"


    /*  decotype(outfile, symbol, aux)
     *
     *  decodes the type and derived type field of the given symbol table entry
     *  using information in the given auxiliary entry for arraydimensions
     *  or to find the tag name of a structure, union or enumeration;
     *  writes the decoded information on outfile using format strings defined
     *  in decotype.h
     *
     *  calls the namelist functions prepend( ) and paren( ) to build the
     *    decoded type string
     *
     *  returns SUCCESS or FAILURE
     *    decotype will fail only if it cannot read a structure, union or
     *    enumeration tag entry that is supposed to be there
     */


int
decotype(outfile, symbol, aux)

FILE	*outfile;
SYMENT	*symbol;
AUXENT	*aux;

{
    /* UNIX FUNCTIONS CALLED */
    extern char		*strcat( );
    extern		sprintf( ),
			fprintf( );

    /* OBJECT FILE ACCESS ROUTINES CALLED */
    extern int		ldtbread( );

    /* NAMELIST utility FUNCTIONS CALLED */
    extern		prepend( ),
			paren( );

    /* EXTERNAL VARIABLES USED */
    extern LDFILE	*ldptr,
			*tagptr;
    extern int		numbase,
			eflag,
			fancyflag;

    /* LOCAL VARIABLES */
    char		typestr[TYPELEN],
			tmpstr[TMPLEN];
    unsigned short	type,
			dertype,
			dtype,
			lastdtyp;
    SYMENT		tag;
    int			arydim;


    /* "typestr[ ]" is used to build up the decoded type and derived type string
     * typestr will remain null ('\0') if the type/derived type field is
     * undefined (0)
     */

    typestr[0] = '\0';

    type = symbol->n_type & N_BTMASK;
    strcat(typestr, typelist[type]);

    if ((type == T_STRUCT || type == T_UNION || type == T_ENUM)
	&& (symbol->n_numaux == 1)) {

	if (aux->x_sym.x_tagndx != 0) {
	    if (ldtbread(tagptr, aux->x_sym.x_tagndx, &tag) != SUCCESS) {
		return(FAILURE);
	    }

	    sprintf(tmpstr, "-%.8s", tag.n_name);
	    strcat(typestr, tmpstr);
	}
    }

    /*  the derived type indicators are decoded from right to left until
     *  a null field is found (DT_NON)
     *  derived type pointer (DT_PTR) is indicated by a prepended "*"
     *  derived type array (DT_ARY) is indicated by an appended "[ ]"
     *    (the array dimesion is filled in if it is present in an aux entry)
     *  derived type function (DT_FCN) is indicated by an appended "( )"
     *  since derived types function and array associate more closely with
     *    a basic type than does derived type pointer, parenthesis are used
     *    to associate pointer more closely with the basic type if it 
     *    appears to the right of an array or function derived type
     */

    dertype = symbol->n_type;
    arydim = -1;
    lastdtyp = DT_NON;

    while ((dtype = (dertype & N_TMASK) >> N_BTSHFT) != DT_NON) {
	if (dtype == DT_PTR) {
	    prepend("*", typestr);
	} else {
	    if (lastdtyp == DT_PTR) {
		paren(typestr);
	    }

	    if (dtype == DT_FCN) {
		strcat(typestr, "( )");
	    } else {
		if ((++arydim < 4) && (symbol->n_numaux == 1)) {
		    sprintf(tmpstr, "[%hd]", aux->x_sym.x_fcnary.x_ary.x_dimen[arydim]);
		    strcat(typestr, tmpstr);
		} else {
		    strcat(typestr, "[ ]");
		}
	    }
	}

	lastdtyp = dtype;
	dertype >>= N_TSHIFT;
    }

    /*  the format of namelist's output is dependent on the "-e" option and
     *  the "-o" option (See prosym.h)
     *  the "-e" option determines whether "pretype" or "prtype" is used
     *  to format the output type string
     *  the "-o" flag (numbase) is used to choose the particular format
     *  string used
     *
     *  (the only difference is the number of characters printed)
     */

    if ((fancyflag == ON) && (eflag == OFF)) {
	fprintf(outfile, prtype[numbase+1], typestr);
    } else {
	if (eflag == OFF) {
	    fprintf(outfile, prtype[numbase], typestr);
        } else {
	    fprintf(outfile, pretype[numbase], typestr);
	}
    }

    return(SUCCESS);
}



    /*  prepend(new, base)
     *
     *  prepends the string "new" to the string "base"
     *  the result is left in "base"
     */

prepend(new, base)

char	*new;
char	*base;

{
    int		i;
    int		j;

    for (i = 0; base[i] != '\0'; ++i);

    for (j = 0; new[j] != '\0'; ++j);

    for ( ; i >= 0; --i) 
	base[i + j] = base[i];

    for (i = 0; j > 0; --j, ++i)
	base[i] = new[i];

    return;
}



    /*  paren(base)
     *
     *  puts parenthesis around the string "base"
     */

paren(base)

char	*base;

{
    int 	i;

    for (i = 0; base[i] != '\0'; ++i);
    base[i] = ')';
    base[++i] = '\0';

    for (	; i >= 0; --i) {
	base[i + 1] = base[i];
    }

    base[0] = '(';

    return;
}

/*
*     static char	ID[ ] = "@(#) decotype.c: 1.2 4/14/82";
*/
