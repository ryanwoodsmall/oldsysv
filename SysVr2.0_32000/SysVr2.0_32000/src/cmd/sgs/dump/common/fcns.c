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
#include	<ar.h>
#include	"filehdr.h"
#include	"scnhdr.h"
#include	"reloc.h"
#include	"linenum.h"
#include	"syms.h"
#include	"ldfcn.h"
#include	"sgs.h"

#define	BADSYMBOL	"<<bad name>>"

int
ahread(ldptr, filename)

LDFILE	*ldptr;
char	*filename;

{
	extern int	printf( ),
			fprintf( ),
			ldahread( );

	extern char	*ctime( );
	extern int	vflag;
	ARCHDR		arhead;
	int		uid,
			gid;

	if (TYPE(ldptr) == (unsigned short) ARTYPE)
	{
		underline(filename);
		if (ldahread(ldptr, &arhead) == SUCCESS)
		{
#if PORTAR || PORT5AR
			uid = arhead.ar_uid;
			gid = arhead.ar_gid;
#else
			uid = (int) arhead.ar_uid & 0xffL;
			gid = (int) arhead.ar_gid & 0xffL;
#endif

			if (!vflag)
			{
				printf("\t%-14.14s  0x%.8lx  %6d  %6d  0%.6ho  0x%.8lx\n\n",
					arhead.ar_name, arhead.ar_date, uid, gid, arhead.ar_mode,
					arhead.ar_size);
			} else {
				printf("\t%-14.14s %.20s %6d %6d 0%.6ho 0x%.8lx\n\n",
					arhead.ar_name, (ctime(&arhead.ar_date) + 4), uid, gid, arhead.ar_mode,
					arhead.ar_size);
			}
		} else {
			fprintf(stderr, "%sdump:  cannot ldahread %s\n", SGS, filename);
			return(FAILURE);
		}
	}

	return(SUCCESS);
}




fhread(ldptr, filename)

LDFILE	*ldptr;
char	*filename;

{
    extern int		printf( ),
			fprintf( ),
			ldfhread( );
    extern  char	*ctime( );
    extern  int	vflag;

    FILHDR		filehead;


    underline(filename);
    if (ldfhread(ldptr, &filehead) == SUCCESS) {

	if (!vflag) {
	   printf("\t0%.6ho  %5hu  0x%8lx  0x%.8lx  %10ld  0x%.4hx  0x%.4hx\n\n",
		filehead.f_magic, filehead.f_nscns, filehead.f_timdat, filehead.f_symptr,
		filehead.f_nsyms, filehead.f_opthdr, filehead.f_flags);
	} else {
		printf("0%.6ho  %5hu  ", filehead.f_magic, filehead.f_nscns);
		printf("%.20s  0x%.8lx  %6ld  0x%.4hx  ",
				(ctime(&filehead.f_timdat) + 4), filehead.f_symptr,
				filehead.f_nsyms, filehead.f_opthdr); 
		if (HEADER(ldptr).f_flags & F_AR16WR){
		  printf("DEC16");
		} else { 
		  if (HEADER(ldptr).f_flags & F_AR32WR){
		    printf("DEC32");
		  } else {
		    printf("NON DEC");
		  }
		}
		if (HEADER(ldptr).f_flags & F_RELFLG){
		  printf("\n\t\t\t\t\t\t\t\t  RELFLG");
		} if (HEADER(ldptr).f_flags & F_EXEC){
		  printf("\n\t\t\t\t\t\t\t\t  EXEC");
		} if (HEADER(ldptr).f_flags & F_LNNO){
		  printf("\n\t\t\t\t\t\t\t\t  LNNO");
		} if (HEADER(ldptr).f_flags & F_LSYMS){
		  printf("\n\t\t\t\t\t\t\t\t  LSYMS");
		}if(HEADER(ldptr).f_flags & F_MINMAL){
		  printf("\n\t\t\t\t\t\t\t\t  MINMAL");
		}if(HEADER(ldptr).f_flags & F_UPDATE){
		  printf("\n\t\t\t\t\t\t\t\t  UPDATE");
		}if(HEADER(ldptr).f_flags & F_SWABD){
		  printf("\n\t\t\t\t\t\t\t\t  SWABD");
		}if(HEADER(ldptr).f_flags & F_PATCH){
		   printf("\n\t\t\t\t\t\t\t\t  PATCH");
		}
	printf("\n\n");
	}  /* else */
	return(SUCCESS);
    }

    fprintf(stderr, "%sdump:  cannot ldfhread %s\n", SGS, filename);
    return(FAILURE);
}



int
ohseek(ldptr, filename)

LDFILE	*ldptr;
char	*filename;

{
    extern int		printf( ),
			fprintf( ),
			fread( );

    unsigned short	buffer;
    int			i,
			count;


    if ((HEADER(ldptr).f_opthdr) != 0) {
	underline(filename);
	if (ldohseek(ldptr) != SUCCESS) {
	    fprintf(stderr, "%sdump:  cannot ldohseek %s\n", SGS, filename);
	    return(FAILURE);
	}

	count = 1;
	printf("\t");
	for (i = HEADER(ldptr).f_opthdr / sizeof(short); i > 0; --i, ++count) {
	    if (FREAD(&buffer, sizeof(short), 1, ldptr) != 1) {
		fprintf(stderr, "%sdump:  cannot FREAD %s\n", SGS, filename);
		return(FAILURE);
	    }
	    printf("%.4hx  ", buffer);

	    if (count == 12) {
		printf("\n\t");
		count = 0;
	    }
	}

	if ((HEADER(ldptr).f_opthdr & 0x1) == 1) {
	    if (FREAD(&buffer, sizeof(char), 1, ldptr) != 1) {
		fprintf(stderr, "%sdump:  cannot FREAD %s\n", SGS, filename);
	    }
	    printf("%.4hx", buffer);
	}
	printf("\n\n");
    }

    return(SUCCESS);
}




int
shread(ldptr, filename)

LDFILE	*ldptr;
char	*filename;

{
	/* FUNCTIONS CALLED */
	extern int	printf( ),
			fprintf( );

	/* LIBLD FUNCTIONS CALLED */
	extern int	ldshread( ),
			ldnshread( );

	/* EXTERNAL VARIABLES USED */
	extern char	*name;
	extern  int	vflag;
	extern long	dstart,
			dstop;

	/* LOCAL VARIABLES USED */
	SCNHDR		secthead;
	unsigned short	first,
			last;
	int		found;

	underline(filename);
	first = 1;
	last = HEADER(ldptr).f_nscns;
	found = 0;

	if (dstart != 0L)
	{
		first = (unsigned short) dstart;
		last = first;
	}

	if (dstop != 0)
	{
		last = (unsigned short) dstop;
		if (dstart == 0L)
			first = 1;
	}

	for (	; first <= last;  ++first)
	{
		if (ldshread(ldptr, first, &secthead) != SUCCESS)
		{
			fprintf(stderr, "%sdump:  cannot ldshread section %d\n", SGS, first);
			return(FAILURE);
		}

		if ((name != NULL) && strncmp(name, secthead.s_name, SYMNMLEN))
			continue;
		else
			found = 1;

		printf("\t%-8.8s  0x%.8lx  0x%.8lx  0x%.8lx  0x%.8lx  0x%.8lx\n",
		secthead.s_name, secthead.s_paddr, secthead.s_vaddr,
		secthead.s_scnptr, secthead.s_relptr, secthead.s_lnnoptr);
		if (!vflag)
		{
			printf("\t          0x%.8lx              0x%.8lx    %5hu       %5hu\n\n",
			secthead.s_flags,secthead.s_size,secthead.s_nreloc,secthead.s_nlnno);
		} else {
			prtflags( secthead.s_flags );
			printf("0x%.8lx     %5hu     %5hu\n\n", secthead.s_size, 
			secthead.s_nreloc, secthead.s_nlnno);
		}

	}

	if ((name != NULL) && !found)
	{
		fprintf( stderr, "%sdump: section %s not in file %s\n", 
			SGS, name, filename );
		return( FAILURE );
	}
	return(SUCCESS);
}



int
sseek(ldptr, filename)

LDFILE	*ldptr;
char	*filename;

{
	/* FUNCTIONS CALLED */
	extern int	printf( ),
			fprintf( ),
			fread( );

	extern int	ldshread( ),
			ldnshread( ),
			ldnsseek( ),
			ldsseek( );

	/* EXTERNAL VARIABLES USED */
	extern char	*name;
	extern char	map[256][2];
	extern long	dstart,
			dstop;

	/* LOCAL VARIABLES */
	SCNHDR		secthead;
	unsigned short	first,
			last,
			buffer;
	long		i;
	int		count;
	int		c;
	int		found;


	underline(filename);
	printf("\n");
	found = 0;
	first = 1;
	last = HEADER(ldptr).f_nscns;

	if (dstart != 0L)
	{
		first = (unsigned short) dstart;
		last = first;
	}

	if (dstop != 0)
	{
		last = (unsigned short) dstop;
		if (dstart == 0L)
			first = 1;
	}

	for (	; first <= last;  ++first)
	{
		if (ldshread(ldptr, first, &secthead) != SUCCESS)
		{
			fprintf(stderr, "%sdump:  cannot ldshread section %d\n", SGS, first);
			return(FAILURE);
		}

		if ((name != NULL) && strcmp( name, secthead.s_name, SYMNMLEN ))
			continue;
		else
			found = 1;

		underline(secthead.s_name);
		if (secthead.s_scnptr == 0L)
		{
			printf("\n");
			continue;
		}

		if (ldsseek(ldptr, first) != SUCCESS)
		{
			fprintf(stderr, "%sdump:  cannot ldsseek section %d\n", 
				SGS, first);
			return(FAILURE);
		}

		count = 1;
		printf("\t");
		for (i = secthead.s_size/sizeof(buffer); i > 0; --i, ++count)
		{
			c = getc(IOPTR(ldptr));
			putchar(map[c][0]);
			putchar(map[c][1]);
			c = getc(IOPTR(ldptr));
			putchar(map[c][0]);
			putchar(map[c][1]);
			printf("  ");
			if (count == 12)
			{
				printf("\n\t");
				count = 0;
			}
		}

		/*
		 * take care of last byte if odd byte section
	 	 */
		if ((secthead.s_size & 0x1L) == 1L)
		{
			c = getc(IOPTR(ldptr));
			putchar(map[c][0]);
			putchar(map[c][1]);
		}
		printf("\n");
	}
	printf("\n");

	if ((name != NULL) && !found)
	{
		fprintf( stderr, "%sdump: section: %s not in file: %s\n",
			SGS, name, filename );
		return( FAILURE );
	}
	return(SUCCESS);
}




int
rseek(ldptr, filename)

LDFILE	*ldptr;
char	*filename;

{
	/* FUNCTIONS CALLED */
	extern int	printf( ),
			fprintf( );

	/* LIBLD FUNCTIONS CALLED */
	extern int	ldshread( ),
			ldnshread( ),
			ldtbread( ),
			ldnrseek( ),
			ldrseek( );
	extern char	*ldgetname( );

	/* EXTERNAL VARIABLES USED */
	extern char	*name;
	extern char	*origname;
	extern long	dstart,
			dstop;
	extern int	vflag;

	/* LOCAL VARIABLES */
	static  char	*rtype[24] = {
				"ABS",
				"DIR16",
				"REL16",
				"IND16",
				"DIR24",
				"REL24",
				"DIR32",
				"OFF8",
				"OFF16",
				"SEG12",
				"DIR32S",
				"AUX",
				"OPT16",
				"IND24",
				"IND32",
				"RELBYTE",
				"RELWORD",
				"RELLONG",
				"PCRBYTE",
				"PCRWORD",
				"PCRLONG",
				"DIR10",
				"REL10",
				"REL32"
		 		};

	SCNHDR		secthead;
	RELOC		relent;
	SYMENT		symbol;
	LDFILE		*ldptr2;
	unsigned short	first,
			last;
	int		i;
	char		*sym_name;
	int		found;


	underline(filename);
	printf("\n");

	if ((vflag) && ((ldptr2 = ldaopen(origname, ldptr)) == NULL))
	{
		fprintf(stderr,"cannot open %s\n", filename);
		return(FAILURE);
	}

	first = 1;
	last = HEADER(ldptr).f_nscns;

	if (dstart != 0L)
	{
		first = (unsigned short) dstart;
		last = first;
	}

	if (dstop != 0)
	{
		last = (unsigned short) dstop;
		if (dstart == 0L)
			first = 1;
	}

	if (name != NULL)
		found = 0;

	for (	; first <= last;  ++first)
	{
		if (ldshread( ldptr, first, &secthead ) != SUCCESS )
		{
			fprintf(stderr, "%sdump:  cannot ldshread section %d\n", SGS, first);
			return(FAILURE);
		}

		if ((name != NULL) && strncmp( name, secthead.s_name, 8))
			continue;
		else
			found = 1;

		underline(secthead.s_name);
		if (secthead.s_nreloc != 0)
		{
			if (ldrseek(ldptr, first) != SUCCESS)
			{
		  		fprintf(stderr, "%sdump:  cannot ldrseek section %d\n", SGS,first);
		  		return(FAILURE);
			}

			for (i = secthead.s_nreloc; i > 0; --i)
			{
				if (FREAD(&relent, RELSZ, 1, ldptr) != 1)
				{
					fprintf(stderr, "%sdump:  cannot FREAD %s\n", SGS, filename);
					return(FAILURE);
				}
				if (!vflag)
				{
					printf("\t0x%.8lx  %10ld  %6d\n",relent.r_vaddr,
						relent.r_symndx, relent.r_type);
				} else { 
					printf("\t0x%.8lx  %10ld  ", relent.r_vaddr, 
						relent.r_symndx);
					if (relent.r_type >= 0 && relent.r_type <= 23)
						printf("%s", rtype[relent.r_type]);
					else
						printf("UNKNOWN");
					if (((ldtbread(ldptr2, relent.r_symndx, &symbol) != SUCCESS) || 
						((sym_name = ldgetname( ldptr2, &symbol )) == NULL )))
						sym_name = BADSYMBOL;
					printf("     %s\n", sym_name);
				} 
			}
		}
		printf("\n");
	}

	if ((name != NULL) && !found)
	{
		fprintf( stderr, "%sdump:  section %s not in file %s", SGS, name, filename );
		return( FAILURE );
	}

	if (vflag)
		ldaclose( ldptr2 );
	return(SUCCESS);
}




int
olseek(ldptr, filename)

LDFILE	*ldptr;
char	*filename;

{
	/* FUNCTIONS CALLED */
	extern int	printf( ),
			fprintf( ),
			ldshread( ),
			ldnshread( ),
			ldnlseek( ),
			ldlseek( ),
			ldtbseek( ),
			ldtbread( );
	extern char	*ldgetname( );

	/* EXTERNAL VARIABLES USED */
	extern int	vflag;
	extern char	*name;
	extern char	*origname;
	extern long	dstart,
			dstop;

	/* LOCAL VARIABLES */
	SCNHDR		secthead;
	LINENO		linent;
	SYMENT		symbol;
	LDFILE		*ldptr2;

	char		*sym_name;
	unsigned short	first,
			last;
	int		i;
	int		found;


	underline(filename);
	printf("\n");

	if ((vflag) && ((ldptr2 = ldaopen(origname, ldptr)) == NULL))
	{
		fprintf(stderr,"cannot open %s\n", filename);
		return (FAILURE);
	}

	found = 0;
	first = 1;
	last = HEADER(ldptr).f_nscns;

	if (dstart != 0L)
	{
		first = (unsigned short) dstart;
		last = first;
	}

	if (dstop != 0)
	{
		last = (unsigned short) dstop;
		if (dstart == 0L)
			first = 1;
	}

	for (	; first <= last;  ++first)
	{
		if (ldshread(ldptr, first, &secthead) != SUCCESS)
		{
			fprintf(stderr, "%sdump:  cannot ldshread section %d\n", SGS, first);
			return(FAILURE);
		}

		if ((name != NULL) && strncmp( name, secthead.s_name, SYMNMLEN))
			continue;
		else
			found = 1;

		underline(secthead.s_name);
		if (secthead.s_nlnno == 0)
		{
			printf("\n");
			continue;
		}

		if (ldlseek(ldptr, first) != SUCCESS)
		{
			fprintf(stderr, "%sdump:  cannot ldlseek section %d\n", SGS,first);
			return(FAILURE);
		}

		for (i = secthead.s_nlnno; i > 0; --i)
		{
			if (FREAD(&linent, LINESZ, 1, ldptr) != 1)
			{
				fprintf(stderr, "%sdump:  cannot FREAD %s\n", SGS, filename);
				return(FAILURE);
			}
			if (linent.l_lnno == 0)
			{
				if (!vflag)
				{
					printf("\t %10ld   %5hu\n", linent.l_addr.l_symndx,
						linent.l_lnno);
				} else {
					printf("\t %10ld   %5hu",
					linent.l_addr.l_symndx, linent.l_lnno );
					if ((ldtbread(ldptr2,linent.l_addr.l_symndx, &symbol) != SUCCESS) ||
						((sym_name = ldgetname( ldptr2, &symbol )) == NULL))
					sym_name = BADSYMBOL;
					printf("	%s\n", sym_name);
				}
			} else {
				printf("\t 0x%.8lx   %5hu\n", linent.l_addr.l_paddr,
					linent.l_lnno);
			}
		}
		printf("\n");
	}

	if (vflag)
		ldaclose( ldptr2 );

	if ((name != NULL) && !found)
	{
		fprintf( stderr, "%sdump: section: %s not in file %s\n",
			SGS, name, filename );
		return( FAILURE );
	}
	return(SUCCESS);
}




int
tbread(ldptr, filename)

LDFILE	*ldptr;
char	*filename;

{
    extern int		fprintf( ),
			printf( ),
			sprintf( ),
			ldtbseek( ),
			strcmp( ),
			ldtbread( );
    extern char		*ldgetname( );

    extern  int		vflag;
    extern char		*name;
    extern long		tstart,
			tstop;

    SYMENT		symbol;
    AUXENT		aux;
    LDFILE		*ldptr2;

    char		*sym_name;
    long		last,
			index;
    int			numaux;


    if ((last = HEADER(ldptr).f_nsyms) == 0) {
	fprintf(stderr,"%s:  no symbols\n", filename);
	printf("\n");
	return(SUCCESS);
    } 
    --last;

    underline(filename);

    if ((tstop == 0) && (tstart != 0)) {
	tstop = tstart;
    }

    if ((tstop != 0L) && (tstop < last)) {
	last = tstop;
    }

    if (ldtbseek(ldptr) != SUCCESS) {
	fprintf(stderr, "%sdump:  cannot ldtbseek %s\n", SGS, filename);
	return(FAILURE);
    }

    for (index = tstart; index <= last; ++index) {
	if (ldtbread(ldptr, index, &symbol) != SUCCESS) {
	    fprintf(stderr, "%sdump:  cannot ldtbread entry %ld\n", SGS, index);
	    return(FAILURE);
	}

	if ((sym_name = ldgetname( ldptr, &symbol )) == NULL)
		sym_name = BADSYMBOL;

	if ((name != NULL) && (strcmp( sym_name, name ) != 0))
	{
		index += symbol.n_numaux;
		continue;
	}

	printf( "[%ld]\t", index );
	print_main( sym_name, symbol );
	numaux = (int) symbol.n_numaux & 0xff;
	for (	; numaux > 0; --numaux, ++index) {
	    if (ldtbread(ldptr, index + 1, &aux) != SUCCESS) {
		fprintf(stderr, "%sdump:  cannot ldtbread entry %d\n", SGS, index + 1);
		return(FAILURE);
	    }

	    printf( "[%ld]\t", index + 1 );
	    if (!vflag) {
		if ( ISFCN(symbol.n_type) ) {
		    printf("a    %8ld    0x%.8lx   0x%.8lx  0x%.8lx  %-5hu\n",
			aux.x_sym.x_tagndx, aux.x_sym.x_misc.x_fsize,
			aux.x_sym.x_fcnary.x_fcn.x_lnnoptr,
			aux.x_sym.x_fcnary.x_fcn.x_endndx, aux.x_sym.x_tvndx);
		} else {
		    printf("a    %8ld   0x%.4hx  0x%.4hx  0x%.8lx  0x%.8lx  %-5hu\n",
			aux.x_sym.x_tagndx, aux.x_sym.x_misc.x_lnsz.x_lnno,
			aux.x_sym.x_misc.x_lnsz.x_size,
			aux.x_sym.x_fcnary.x_fcn.x_lnnoptr,
			aux.x_sym.x_fcnary.x_fcn.x_endndx, aux.x_sym.x_tvndx);
		}
	    } else {
		det_auxent(symbol, aux, ldptr, sym_name);
	    }
	}
    }

    printf("\n");
    return(SUCCESS);
}


print_main( sym_name, symbol )

char	*sym_name;
SYMENT	symbol;

{
	extern	int	vflag;
	int		sclass;
	int		numaux;

#if FLEXNAMES
	if (symbol.n_zeroes == 0L)
		printf( "m2  %-8ld  ", symbol.n_offset );
	else
#endif
		printf( "m1  %-8.8s  ", symbol.n_name );

	printf( "0x%.8lx  %6hd", symbol.n_value, symbol.n_scnum );
	if (vflag) {
		det_type(symbol);
		det_sclass(symbol);
	} else {
		sclass = (int) symbol.n_sclass & 0xff;
		printf( "  0x%.4hx   0x%.2x", symbol.n_type, sclass );
	}

	numaux = (int) symbol.n_numaux & 0xff;
#if FLEXNAMES
	printf("  %3d  %s\n", numaux, sym_name );
#else
	printf("  %3d\n", numaux );
#endif

}


int
zread(ldptr, filename)

LDFILE		*ldptr;
char		*filename;

{
    extern int		printf( ),
			fprintf( ),
			strcmp( ),
			fread( ),
			ldtbseek( ),
			ldtbread( ),
			ldlread( ),
			ldlinit( ),
			ldlitem( );
    extern char		*ldgetname( );

    extern char		*zname;
    extern unsigned short	zstart,
				zstop;

    SYMENT		symbol;
    AUXENT		aux;
    LINENO		line;
    long		fcnindx,
			symindx;
    unsigned short	zitem;
    char		*sym_name;


    underline(filename);
    printf("\n");

    if (ldtbseek(ldptr) != SUCCESS) {
	fprintf(stderr, "%sdump:  cannot ldtbseek %s\n", SGS, filename);
	return(FAILURE);
    }

    symindx = 0L;
    fcnindx = 0L;
    while ((fcnindx == 0L) && (symindx < HEADER(ldptr).f_nsyms)) {
	if (ldtbread(ldptr, symindx, &symbol) != SUCCESS) {
	    fprintf(stderr, "%sdump:  cannot ldtbread entry %ld\n", SGS, symindx);
	    return(FAILURE);
	}

	if ((sym_name = ldgetname( ldptr, &symbol )) == NULL)
		sym_name = BADSYMBOL;

	if ((ISFCN(symbol.n_type)) && (strcmp(zname, sym_name ) == 0)) {
	    fcnindx = symindx;
	} else if (symbol.n_numaux == 1) {
	    if (ISFCN(symbol.n_type) || ISTAG(symbol.n_type)) {
		if (FREAD(&aux, AUXESZ, 1, ldptr) != 1) {
		    fprintf(stderr, "%sdump:  cannot fread aux entry %ld\n", SGS, symindx + 1);
		    return(FAILURE);
		}

		symindx = (symindx < aux.x_sym.x_fcnary.x_fcn.x_endndx) ? aux.x_sym.x_fcnary.x_fcn.x_endndx : symindx + 2;
	    } else {
		symindx += 2;
	    }
	} else {
	    ++symindx;
	}
    }

    if (fcnindx == 0L) {
	fprintf(stderr,"no function \"%s\" in symbol table\n\n", zname);
	return(SUCCESS);
    }

    underline(zname);

    if ((zstart != 0) && (zstart >= zstop)) {
	if (ldlread(ldptr, fcnindx, zstart, &line) == SUCCESS) {
	    printf("\t 0x%.8lx   %5hu\n", line.l_addr.l_paddr, line.l_lnno);
	} else {
	    fprintf(stderr,"\t no line number >= %5hu\n", zstart);
	}
    } else {
	if (ldlinit(ldptr, fcnindx) != SUCCESS) {
	    fprintf(stderr,"\t no line numbers for function\n");
	} else {
	    if ((zitem = zstart) == 0) {
		if (ldlitem(ldptr, zitem, &line) != SUCCESS) {
		    fprintf(stderr, "%sdump:  cannot ldlitem entry %hu\n", SGS, zitem);
		    return(FAILURE);
		}
		printf("\t %10ld   %5hu\n", line.l_addr.l_symndx, line.l_lnno);
		++zitem;
	    }

	    if (zstop == 0) {
		while (ldlitem(ldptr, zitem, &line) == SUCCESS) {
		    printf("\t 0x%.8lx   %5hu\n", line.l_addr.l_paddr, line.l_lnno);
		    zitem = line.l_lnno + 1;
		}
	    } else {
		while ((zitem <= zstop) && (ldlitem(ldptr, zitem, &line) == SUCCESS)) {
		    printf("\t 0x%.8lx   %5hu\n", line.l_addr.l_paddr, line.l_lnno);
		    zitem = line.l_lnno + 1;
		}
	    }
	}
    }

    printf("\n");
    return(SUCCESS);

}




int
underline(string)

char	*string;

{
	extern int		printf( );
	extern	int		uflag;

	if (uflag)
		for (	; *string != '\0'; ++string) {
			printf("_%c", *string);
		}
	else
		printf( string );

	printf(":\n");

	return;
}


/*  det_type is used to find out what type will
    be printed.
*/

int
det_type(syment)

  SYMENT	syment;

{

	extern int	sprintf( ),
			fprintf( );

	static char	*typelist[16] = {
			"null   ",
			"arg    ",
			"char   ",
			"short  ",
			"int    ",
			"long   ",
			"float  ",
			"double ",
			"struct ",
			"union  ",
			"enum   ",
			"enmem  ",
			"Uchar  ",
			"Ushort ",
			"Uint   ",
			"Ulong  "
	};

	unsigned short  type,
			dertype,
			dtype;
	char		*name;


    type = syment.n_type & N_BTMASK;
    name = typelist[type];
    dertype = syment.n_type;
    dtype = ((dertype & N_TMASK) >> N_BTSHFT);

	/* switch on the different cases of types */

	switch(dtype) {

	case DT_PTR:
		printf("  *%s", typelist[type]);
		break;

	case DT_FCN:
		printf(" ()%s", typelist[type]);
		break;

	case DT_ARY:
		printf(" []%s", typelist[type]);
		break;

	default:
		printf("   %s", typelist[type]);
		break;
		} /* end of switch */
return;
}


/*  det_sclass determines what type of storage 
 *  class to use.
 */



int
det_sclass(syment)

    SYMENT	syment;

{
#define SCLAS(x)	x <= (char)C_FIELD ? sclas[((int)x+1)] : sclas[((int)x)- 80]

    extern int		printf( );

    int			class;
    static char		*sclas[24] = {
			"efcn  ",
			"null  ",
			"auto  ",
			"extern",
			"static",
			"reg   ",
			"extdef",
			"label ",
			"ulabel",
			"strmem",
			"argm't",
			"strtag",
			"unmem ",
			"untag ",
			"typdef", 
			"ustat ",
			"entag ",
			"enmem ",
			"regprm",
			"bitfld",
			"block ",
			"fcn   ",
			"endstr",
			"file  "
	};


	class = (int)syment.n_sclass;
	if ((class >= C_EFCN && class <= C_FIELD)
		|| (class >= C_BLOCK && class <= C_FILE)){
	    printf("  %s",SCLAS((int)syment.n_sclass));
	} else 
	if (syment.n_sclass == C_HIDDEN) {
		printf("  hidden");
	} else {
	    printf("  ???????");
	}
return;
}

/* det_auxent determines what kind of aux entry
 * should be printed for that symbol.
 */



int
det_auxent(syment, aux, ldptr, sym_name)

SYMENT		syment;
AUXENT		aux;
LDFILE		*ldptr;
char		*sym_name;

{
	extern int 		printf( ),
				strcmp( );

	int			tmp;
	/* Based on the sclass print a aux entry */
		tmp = 0;
		switch (syment.n_sclass) {
			case C_FILE:
				printf("a1     %14s\n", aux.x_file.x_fname);
				break;
			case C_STAT:
			case C_HIDDEN:
				if (syment.n_type == T_NULL) {
				    printf("a3     0x%.8lx   0x%.4hx  0x%.4hx\n",
					aux.x_scn.x_scnlen, aux.x_scn.x_nreloc,
					aux.x_scn.x_nlinno);
				    break;
				}
			case C_EXT:
				if (syment.n_type == T_NULL &&
					!strcmp( _TV, sym_name )){
				    printf("a2     0x%.8lx  0x%.4hx  0x%.4hx  0x%.4hx\n",
					aux.x_tv.x_tvfill, aux.x_tv.x_tvlen,
					aux.x_tv.x_tvran[0], aux.x_tv.x_tvran[1]);
				    break;
				}
			default:
			    if (ISFCN(syment.n_type))  {
				    tmp = 4;
			    } else {
				tmp = 6;
			    }

			    if (ISARY(syment.n_type) ) 
				tmp += 1;
			    printf("a%d   %10ld", tmp, aux.x_sym.x_tagndx);
					break;
				} /* switch */
		if ( tmp  > 0) {
		    if ( tmp <= 5) {
			printf("  0x%.8lx", aux.x_sym.x_misc.x_fsize);
			if ( tmp == 4) {
			    printf("  0x%.8lx  0x%.8lx",
				aux.x_sym.x_fcnary.x_fcn.x_lnnoptr,
				aux.x_sym.x_fcnary.x_fcn.x_endndx);
			    printf("          %5hu\n", aux.x_sym.x_tvndx);
			} else {
			    printf("  0x%.4hx  0x%.4hx  0x%.4hx  0x%.4hx",
				aux.x_sym.x_fcnary.x_ary.x_dimen[0],
				aux.x_sym.x_fcnary.x_ary.x_dimen[1],
				aux.x_sym.x_fcnary.x_ary.x_dimen[2],
				aux.x_sym.x_fcnary.x_ary.x_dimen[3]);
			    printf("  %5hu\n", aux.x_sym.x_tvndx);
		    }
		} else {
			printf("  0x%.4hx", aux.x_sym.x_misc.x_lnsz.x_lnno);
			printf("  0x%.4hx", aux.x_sym.x_misc.x_lnsz.x_size);

			if(tmp == 6) {
			    printf("  0x%.8lx  0x%.8lx",
				aux.x_sym.x_fcnary.x_fcn.x_lnnoptr,
				aux.x_sym.x_fcnary.x_fcn.x_endndx);
			    printf("      %5hu\n", aux.x_sym.x_tvndx);
			} else {
			    printf("  0x%.4hx  0x%.4hx  0x%.4hx  0x%.4hx",
				aux.x_sym.x_fcnary.x_ary.x_dimen[0],
				aux.x_sym.x_fcnary.x_ary.x_dimen[1],
				aux.x_sym.x_fcnary.x_ary.x_dimen[2],
				aux.x_sym.x_fcnary.x_ary.x_dimen[3]);
			    printf("%5hu\n", aux.x_sym.x_tvndx);
			}
		}
	    }
return;
}

prtflags( flags )
long flags;
{

	int ct = 0;

	printf("\t\t  ");
	switch ((int) (flags & 0x1f)) {
	case STYP_REG :
		printf("REG");
		ct += 3;
		break;
	case STYP_DSECT :
		printf("DSECT");
		ct += 5;
		break;
	case STYP_NOLOAD :
		printf("NOLOAD");
		ct += 6;
		break;
	case STYP_PAD :
		printf("PAD");
		ct += 3;
		break;
	case STYP_COPY :
		printf("COPY");
		ct += 4;
		break;
	}

	if (flags & STYP_TEXT) {
		if (ct) {
			ct += 2;
			printf(", ");
		}
		printf("TEXT");
		ct += 4;
		}

	if (flags & STYP_DATA) {
		if (ct) {
			ct += 2;
			printf(", ");
		}
		printf("DATA");
		ct += 4;
	} else if (flags & STYP_BSS) {
		if (ct) {
			ct += 2;
			printf(", ");
		}
		printf("BSS");
		ct += 3;
	}

	for ( ; ct <= 18; ct++ )
		putchar(' ');
	printf("\t  ");
}

#if FLEXNAMES
int
strseek( ldptr, filename )

LDFILE	*ldptr;
char	*filename;

{
	extern	int	ldahread( );

	ARCHDR	arhbuf;
	long	str_address;
	long	str_size;
	long	offset;
	int	c;
	int	new_string;

	underline( filename );
	str_address = STROFFSET(ldptr);
	if (TYPE(ldptr) == ARTYPE)
	{
		if (ldahread( ldptr, &arhbuf ) != SUCCESS)
		{
			fprintf( stderr, "cannot ldahread archive header\n");
			return( FAILURE );
		}
		if (arhbuf.ar_size <= str_address + 1)
			return( SUCCESS );
	}

	if (FSEEK( ldptr, str_address, BEGINNING ) != OKFSEEK)
	{
		fprintf( stderr, "cannot seek to end of symbol table\n" );
		return( FAILURE );
	}

	if (FREAD( &str_size, sizeof(long), 1, ldptr ) != 1)
		return( SUCCESS );

	offset = 4;
	new_string = 1;
	while (((c = GETC( ldptr )) != EOF) && (offset < str_size))
	{
		if (c == '\0')
		{
			if ( !new_string )
				putchar( '\n' );
			new_string = 1;
		}
		else
		{
			if (new_string)
			{
				printf( "\t[%ld]\t    ", offset );
				new_string = 0;
			}
			putchar( c );
		}
		offset++;
	}

	if (offset < str_size)
	{
		fprintf( stderr, "cannot read string table\n" );
		return( FAILURE );
	}
	else
	{
		putchar( '\n' );
		return( SUCCESS );
	}
}
#endif

#ifdef PORTAR
int
ar_sym_read( ldptr, filename )

LDFILE	*ldptr;
char	*filename;

{
	extern	int	fread( );
	extern	char	*calloc( );
	extern	long	sgetl( );
	extern	char	*calloc( );

	long		num_syms;
	char		*offsets;
	char		c;
	char		num_buf[sizeof(long)];
	struct	ar_hdr	arbuf;

	if (TYPE(ldptr) != ARTYPE)
		return( SUCCESS );

	underline( filename );
	fseek( IOPTR(ldptr), (long) SARMAG, BEGINNING );
	if (FREAD( &arbuf, sizeof( struct ar_hdr ), 1, ldptr) != 1)
	{
		fprintf( stderr, "cannot read symbol table header\n");
		return( FAILURE );
	}

	if ( arbuf.ar_name[0] == '/' )
	{
		if (FREAD( num_buf, sizeof( num_buf ), 1, ldptr ) != 1)
		{
			fprintf( stderr, "cannot read symbol table header\n");
			return( FAILURE );
		}
		num_syms = sgetl( num_buf );
		if ((offsets = calloc( num_syms, sizeof(long) * sizeof(char) ))
			== NULL)
		{
			fprintf( stderr, "cannot allocate space for array\n");
			return( FAILURE );
		}
		if ( FREAD( offsets, sizeof(long) * sizeof(char), num_syms,
			ldptr) != num_syms)
		{
			fprintf( stderr, "cannot read archive symbol table\n");
			return( FAILURE );
		}

		for( ; num_syms; num_syms--, offsets += sizeof(long) )
		{
			printf( "%10ld\t", sgetl( *offsets ) );
			while ((c = putchar( GETC( ldptr ))) != '\0')
				if (c == EOF)
				{
					fprintf( stderr, "premature EOF\n");
					return( FAILURE );
				}
			putchar( '\n' );
		}
	}

	return( SUCCESS );

}
#endif

/*	@(#) fcns.c: 1.15 3/23/83	*/
