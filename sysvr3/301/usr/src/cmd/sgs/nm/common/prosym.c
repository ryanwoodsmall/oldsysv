/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nm:common/prosym.c	1.14"
/* UNIX HEADER */
#include	<stdio.h>

/* COMMON SGS HEADERS */
#include	"filehdr.h"
#include	"syms.h"
#include	"ldfcn.h"

/* NAMELIST HEADERS */
#include	"defs.h"
#include	"prosym.h"


#define RFLEN 75  /* maximum length of array buffer (rfname) holding the symbol 
		     name and sometimes the file name (-r option w/out -p) */


    /*  prosym(section, fname)
     *
     *  finds the symbol table (ldtbseek( ))
     *  prints a heading
     *  prints out each symbol table entry
     *  the way that the entry is printed depends mostly on its storage class
     *
     *  calls:
     *      - decotype(outfile, &symbol, &aux) to print the type field
     *      - psect(section, symbol) to print section/class field for pflag 
     *        output corresponding to types C_EXT and C_STAT
     *
     *  prints:
     *      - the symbol name, value, storage class, tv and
     *        (after decotype prints the type) the size, line number and section
     *        name of each symbol
     *      - uses format strings defined in prosym.h
     *
     *  returns SUCCESS or FAILURE
     *  FAILURE is caused by a messed up symbol table
     */

int
prosym(section,fname)

char	(*section)[8];
char	fname[50];


{
    /* UNIX FUNCTIONS CALLED */
    extern		fprintf( );
    extern int		strncmp( );
    extern int		strcmp( );
    extern int		strlen( );

    /* COMMON OBJECT FILE ACCESS ROUTINES CALLED */
    extern int		ldtbseek( );
    extern char		*ldgetname();

    /* NAMELIST FUNCTIONS CALLED */
    extern int		decotype( );
    extern int		psect( );

    /* EXTERNAL VARIABLES USED */
    extern LDFILE	*ldptr;
    extern FILE		*formout,
			*sortout;
    extern int		numbase;
    extern int		fullflag,
			hflag,
			pflag,
			rflag,
			Tflag,
			uflag;

    /* LOCAL VARIABLES */
    FILE		*outfile;
    SYMENT		symbol;
    AUXENT		aux;
    long		symindx;
    int			tvaux;
    char		*ld_name;
    int			name_err = 0;
    char		rfname[RFLEN];
    int			flen;


    if (ldtbseek(ldptr) != SUCCESS) {
	return(FAILURE);
    }

    if (uflag == OFF && hflag == OFF && pflag == OFF)
	fprintf(formout, prtitle[numbase]);

    for (symindx = 0L; symindx < HEADER(ldptr).f_nsyms; ++symindx) {
	outfile = formout;
	tvaux = 0;
	strcpy(rfname,fname);
	if (FREAD(&symbol, SYMESZ, 1, ldptr) != 1) {
	    return(FAILURE);
	}
	if ( symbol.n_numaux == 1 ) {
	    if (FREAD(&aux, AUXESZ, 1, ldptr) != 1) {
		return(FAILURE);
		}
	    ++symindx;
	    }

	/*
	 * if .text, .data, .bss, .init, .comment or .lib and
	 * fullflag is OFF then don't print
	 */
	if (fullflag == OFF)
	    if ( symbol.n_sclass == C_STAT &&
		    symbol.n_type == T_NULL && (
		    strncmp(symbol.n_name, ".text", 8) == 0 ||
		    strncmp(symbol.n_name, ".data", 8) == 0 ||
		    strncmp(symbol.n_name, ".bss",  8) == 0 ||
		    strncmp(symbol.n_name, ".init", 8) == 0 ||
		    strncmp(symbol.n_name, ".comment",  8) == 0 ||
		    strncmp(symbol.n_name, ".lib",  8) == 0) )
		continue;

	if (symbol.n_sclass != C_FILE)  {
	    if ((ld_name = ldgetname(ldptr, &symbol)) == NULL)
	    {
		ld_name="<<bad symbol name>>";
		name_err = 1;
	    }
	} else if (symbol.n_numaux == 1) 
	    ld_name = aux.x_file.x_fname;
	else
	    ld_name = "";

	if (rflag == ON) {
	    if (pflag == OFF)  {
		if (Tflag == ON) {
			flen = 19 - strlen(ld_name);
			if (flen > 0 ) {
	   	 		if (strlen(rfname) > flen) {
					rfname[flen-1] = '*';
					rfname[flen] = '\0';
	   	 		}
			} else {
	    			rfname[0] = '\0';
			}
		}
		strcat(rfname,":");
		flen= RFLEN - (strlen(rfname) + 1);
		strncat(rfname,ld_name,flen);
	    } else
		strcat(rfname,":");
	} else
		strncpy(rfname,ld_name,RFLEN - 1);
	if (Tflag == ON && rfname[20] != '\0')  {
		rfname[19] = '*';
		rfname[20] = '\0';
	}

	if (pflag == ON && rflag == ON)
		fprintf(formout,"%-20s",rfname);

	if ( uflag == ON ) {
	    if ( symbol.n_scnum == 0 && symbol.n_value == 0) {
		fprintf(formout,"    %s\n", rfname);
	    }
	    continue;
	}

	if ( strncmp(symbol.n_name, ".tv", 8) == 0 ) {
	    tvaux = 1;
	}

	/* switch on storage classes */
	switch(symbol.n_sclass) {
	    case C_FILE:
		if (pflag == ON) {
			fprintf(formout,prpfile[numbase],ld_name);
		}
		else
			fprintf(formout, prfile[numbase],rfname);
	    	continue;
		break;

	    case C_REG:
	    case C_REGPARM:
	    case C_AUTO:
	    case C_MOS:
	    case C_ARG:
	    case C_MOU:
	    case C_MOE:
	    case C_FIELD:
		/* print value field as an offset */
		if (pflag == ON) {
			if ( symbol.n_sclass == C_REG || symbol.n_sclass == C_REGPARM )  {
				fprintf(formout,prpoffset[numbase],VALMASK(symbol.n_value),"r",ld_name);
				continue;
			}
			else  {
				fprintf(formout,prpoffset[numbase],VALMASK(symbol.n_value),"a",ld_name);
				continue;
			}
		}
		else
			fprintf(formout, proffset[numbase], rfname,
			    VALMASK(symbol.n_value), sclass[symbol.n_sclass]);
		break;

	    case C_EXT:
		/* external functions stay with their local symbols;
		     * other externals may get sorted (depends on what
		     * process( ) did with sortout).
		     */
		if (ISFCN(symbol.n_type)) {
			outfile = formout;
		} else {
			outfile = sortout;
		}
		/* print value as an address */
		if (pflag == ON) {
			fprintf(outfile,prpaddress[numbase],VALMASK(symbol.n_value),psect(section,symbol),ld_name);
			continue;
		}
		else
			fprintf(outfile, praddress[numbase], rfname,
		    	    VALMASK(symbol.n_value), sclass[symbol.n_sclass]);
		break;

	    case C_STAT:
	    case C_USTATIC:
	    case C_LABEL:
	    case C_BLOCK:
	    case C_FCN:
	    case C_HIDDEN:

		/* print value as an address */
		if (pflag == ON) {
		    if ( symbol.n_sclass == C_STAT || symbol.n_sclass == C_HIDDEN)  {
			fprintf(outfile,prpaddress[numbase],VALMASK(symbol.n_value),psect(section,symbol),ld_name);
			continue;
		    } else {
			fprintf(outfile,prpaddress[numbase],VALMASK(symbol.n_value),'a',ld_name);
			continue;
		    }
		}
		else
		    fprintf(outfile, praddress[numbase], rfname,
			VALMASK(symbol.n_value), SCLASS(symbol.n_sclass));
		break;

	    case C_NULL:
	    case C_EXTDEF:
	    case C_ULABEL:
	    case C_STRTAG:
	    case C_UNTAG:
	    case C_TPDEF:
	    case C_ENTAG:
	    case C_EOS:
		/* value field is meaningless; don't print it */
		if (pflag == ON)  {
			fprintf(formout,prpnoval[numbase],"a",ld_name);
			continue;
		} else
			fprintf(formout, prnoval[numbase], rfname,
		    	    SCLASS(symbol.n_sclass));
		break;

	    default:
		if (pflag == ON)  {
			fprintf(formout,prpoffset[numbase],VALMASK(symbol.n_value),"?",ld_name);
			continue;
		} else
			fprintf(formout, praddress[numbase], rfname,
				VALMASK(symbol.n_value), "??????");
		break;

	}	/* switch */

#ifdef TRVEC
	if ((symbol.n_numaux == 1) && (aux.x_sym.x_tvndx != 0)) {
	    fprintf(outfile, "|tv");
	} else {
	    fprintf(outfile, "|  ");
	}

#endif
	if (decotype(outfile, &symbol, &aux) == FAILURE) {
	    return(FAILURE);
	}

	if (tvaux == 1) {
	    fprintf(outfile, prsize[numbase], ((aux.x_tv.x_tvran[1]
			- aux.x_tv.x_tvran[0]) * sizeof(long)));
	    fprintf(outfile, "|     |\n");
	    continue;
	}

	if ( ISFCN(symbol.n_type) ) {
	    /* print function size information */
	    if ((symbol.n_numaux == 1) && (aux.x_sym.x_misc.x_fsize != 0L)) {
		fprintf(outfile, prfsize[numbase], aux.x_sym.x_misc.x_fsize);
	    } else {
		fprintf(outfile, prnofsize[numbase]);
	    }
	    fprintf(outfile, "|     ");
	} else {
	    /* print regular size information */
	    if ((symbol.n_numaux == 1) && (aux.x_sym.x_misc.x_lnsz.x_size != 0)) {
		fprintf(outfile, prsize[numbase], aux.x_sym.x_misc.x_lnsz.x_size);
	    } else {
		fprintf(outfile, prnosize[numbase]);
	    }

	    if ((symbol.n_numaux == 1) && (aux.x_sym.x_misc.x_lnsz.x_lnno != 0)) {
		fprintf(outfile, "|%5d", aux.x_sym.x_misc.x_lnsz.x_lnno);
	    } else {
		fprintf(outfile, "|     ");
	    }
	}

	if ((symbol.n_scnum > 0) && 
		(symbol.n_scnum <= HEADER(ldptr).f_nscns)) {
	    fprintf(outfile, "|%-.8s\n", section[symbol.n_scnum - 1 ]);
	} else if (symbol.n_scnum == N_ABS) {
	    fprintf(outfile, "|(ABS)\n");
	} else {
	    fprintf(outfile, "|\n");
	}
    }   /* for  */
    if (name_err)
	return (FAILURE);
    return(SUCCESS);
}




    /*  proext(section,fname)
     *
     *  is almost identical to prosym( ) except it only prints information
     *  about static and external symbols.  It also prints the source file
     *  associated with static symbols.
     */


int
proext(section,fname)

char	(*section)[8];
char	fname[50];


{
    /* UNIX FUNCTION CALLED */
    extern		fprintf( );
    extern int		strncmp( );

    /* COMMON OBJECT FILE ACCESS ROUTINE CALLED */
    extern int		ldtbseek( );
    extern char		*ldgetname();

    /* NAMELIST FUNCTION CALLED */
    extern int		decotype( );
    extern int		psect( );

    /* EXTERNAL VARIABLES USED */
    extern LDFILE	*ldptr;
    extern int		numbase;
    extern int		fullflag,
			Tflag,
			hflag;

    /* LOCAL VARIABLES */
    extern FILE		*formout,
			*sortout;

    FILE		*outfile;
    SYMENT		symbol;
    AUXENT		aux,
			faux;
    long		symindx;
    int			tvaux = 0;
    char		*iftv = ".tv";
    char		*ld_name;
    int			name_err = 0;
    char		rfname[RFLEN];
    int			flen;


    if (ldtbseek(ldptr) != SUCCESS) {
	return(FAILURE);
    }

    if (hflag == OFF && pflag == OFF)
    fprintf(formout, pretitle[numbase]);

    faux.x_file.x_fname[0] = '\0';

    for (symindx = 0L; symindx < HEADER(ldptr).f_nsyms; ++symindx) {
	if (FREAD(&symbol, SYMESZ, 1, ldptr) != 1) {
	    return(FAILURE);
	}

	if ( symbol.n_numaux == 1 ) {
	    if ( FREAD(&aux, AUXESZ, 1, ldptr) != 1) {
		return(FAILURE);
	    }
	    ++symindx;
	}

	if (symbol.n_sclass == C_FILE)
		continue;

	if ( strncmp(symbol.n_name,iftv,8) == 0)
	    ++tvaux;

	if ((symbol.n_sclass == C_STAT) || (symbol.n_sclass == C_EXT)) {

	    /*
	     * if .text, .data, .bss, .init, .comment or .lib and
	     * fullflag is OFF then don't print
	     */
	    if (fullflag == OFF)
		if ( symbol.n_sclass == C_STAT &&
			symbol.n_type == T_NULL && (
			strncmp(symbol.n_name, ".text", 8) == 0 ||
			strncmp(symbol.n_name, ".data", 8) == 0 ||
		    	strncmp(symbol.n_name, ".bss",  8) == 0 ||
		    	strncmp(symbol.n_name, ".init", 8) == 0 ||
		    	strncmp(symbol.n_name, ".comment",  8) == 0 ||
		    	strncmp(symbol.n_name, ".lib",  8) == 0) )
		    continue;

	    outfile = formout;
	    strcpy(rfname,fname);
	    if (symbol.n_sclass == C_EXT) {
		/* external symbols may get sorted */
		outfile = sortout;
	    }

	    /* print all values as addresses */
	    if ((ld_name = ldgetname(ldptr, &symbol)) == NULL)
	    {
		ld_name="<<bad symbol name>>";
		name_err = 1;
	    }

	if (rflag == ON) {
	    if (pflag == OFF)  {
		if (Tflag == ON) {
			flen = 19 - strlen(ld_name);
			if (flen > 0 ) {
	   	 		if (strlen(rfname) > flen) {
					rfname[flen-1] = '*';
					rfname[flen] = '\0';
	   	 		}
			} else {
	    			rfname[0] = '\0';
			}
		}
		strcat(rfname,":");
		flen = RFLEN - (strlen(rfname) + 1);
		strncat(rfname,ld_name,flen);
	    } else
		strcat(rfname,":");
	} else
		strncpy(rfname,ld_name,RFLEN - 1);
	if (Tflag == ON && rfname[20] != '\0')  {
		rfname[19] = '*';
		rfname[20] = '\0';
	}

		if (pflag == ON)  {
		    if (rflag == ON) 
			fprintf(outfile,"%-20s",rfname);
		    fprintf(outfile,prpaddress[numbase],VALMASK(symbol.n_value),psect(section,symbol),ld_name);
		    continue;
		} else {
	    		fprintf(outfile, praddress[numbase], rfname,
		    	    VALMASK(symbol.n_value), sclass[symbol.n_sclass]);
		}

#ifdef TRVEC
	    if ((symbol.n_numaux == 1) && (aux.x_sym.x_tvndx != 0)) {
		fprintf(outfile, "|tv");
	    } else {
		fprintf(outfile, "|  ");
	    }
#endif

	    if (decotype(outfile, &symbol, &aux) == FAILURE) {
		return(FAILURE);
	    }

	    if ( ISFCN(symbol.n_type) ) {
		/* print function size information */
		if ((symbol.n_numaux == 1) && (aux.x_sym.x_misc.x_fsize != 0L)) {
		    fprintf(outfile, prfsize[numbase], aux.x_sym.x_misc.x_fsize);
		} else {
		    fprintf(outfile, prnofsize[numbase]);
		}

	    } else {
		/* print regular size information */
		if (symbol.n_numaux == 1 && tvaux) {
		    fprintf(outfile, prsize[numbase], ((aux.x_tv.x_tvran[1] - aux.x_tv.x_tvran[0]) * sizeof(long)));
		} else {
		    if ((symbol.n_numaux == 1) && (aux.x_sym.x_misc.x_lnsz.x_size != 0)) {
			fprintf(outfile, prsize[numbase], aux.x_sym.x_misc.x_lnsz.x_size);
		    } else {
			fprintf(outfile, prnosize[numbase]);
		    }
		}
	    }

	    if ((symbol.n_numaux == 1) && (aux.x_sym.x_misc.x_lnsz.x_lnno != 0)
			&& (! tvaux)) {
		fprintf(outfile, "|%5d", aux.x_sym.x_misc.x_lnsz.x_lnno);
	    } else {
		fprintf(outfile, "|     ");
	    }

	    if ((symbol.n_scnum > 0) &&
		    (symbol.n_scnum <= HEADER(ldptr).f_nscns)) {
		fprintf(outfile, "|%-8.8s", section[symbol.n_scnum - 1 ]);
	    } else {
		fprintf(outfile, "|        ");
	    }

	    if (symbol.n_sclass == C_STAT) {
		fprintf(outfile, "|%.14s\n", faux.x_file.x_fname);
	    } else {
		fprintf(outfile, "|\n");
	    }
	}
    }

    if (name_err)
	return (FAILURE);
    return(SUCCESS);
}

    /*  psect( )
     *
     *  maps types C_EXT and C_STAT to the appropriate section/class in the
     *  parsable style output (pflag)
     *
     *  returns the section/class letter for the pflag output
     */

int
psect(section,symbol)

char 	(*section)[8];
SYMENT	symbol;

{
    /* UNIX FUNCTIONS CALLED */
    extern int	strcmp( );

    /* EXTERNAL VARIABLES USED */
    extern LDFILE	*ldptr;

    /* LOCAL VARIABLES */
    int		sect;

	if (strcmp(section[symbol.n_scnum - 1], ".text") == 0)
		sect = 'T';
	else if (strcmp(section[symbol.n_scnum - 1], ".data") == 0)
		sect = 'D';
	else if (strcmp(section[symbol.n_scnum - 1], ".bss") == 0)
		sect = 'B';
	else if (symbol.n_scnum == -1 && symbol.n_sclass == C_EXT) 
		sect = 'A';
	else if (symbol.n_scnum == 0 && symbol.n_sclass == C_EXT) {
		if (VALMASK(symbol.n_value) == 0)
			sect = 'U';
		else
			sect = 'C';
	}
	else
		sect = 'S';
	if (symbol.n_sclass == C_EXT)
		return(sect);
	else
		return(tolower(sect));
}

/*
 */
