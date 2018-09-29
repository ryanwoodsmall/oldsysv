/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ld:common/ld00.c	1.39"
#include "system.h"

#include <stdio.h>

#if TS || RT
#include <ar.h>
#else
#include <archive.h>
#endif

#include "structs.h"
#include "extrns.h"
#include "list.h"
#include "ldfcn.h"
#include "sgs.h"
#include "sgsmacros.h"
#include "paths.h"
#include "y.tab.h"

extern FILE *yyin;	/* current lex input file */
extern char inline[];	/* current input line up to current token */
extern int yychar;	/* current yacc lookahead token number */

extern int in_y_exp;	/*
			 *	See explanation in ld.yac.
			 *	We must initialize this var. to zero
			 *	before parsing an ifile.
			 */

#if FLEXNAMES
#	define STRINCRE 2048
#else
#	define STRINCRE 1024
#endif
#define MAXINFLS 16
#define MAXDLEN 64	/* maximum lib search directory length*/

#ifndef LLIBDIR
#define LLIBDIR LIBDIR
#endif
#ifndef NDELDIRS
#define NDELDIRS	1
#endif

extern char *realloc();

static int nldirs = NDELDIRS;  /* number of -L options used    */
static char *libdirs[MAXLDIRS] = {	/* directories searched for libraries */
			LLIBDIR
#ifdef LLIBDIR1
			,LLIBDIR1
#endif
			};


/*
 * A stack is kept of input ifile discriptors so that
 * ifiles may include other ifiles.
 *
 * NOTE: currently, this only works if the ifile name
 *		is the last statement in an ifile, because YACC does
 *		not return from a recursive call properly.
 */
typedef struct instr INSTR;
struct instr {
	FILE *infildes;		/* file descriptor */
	char *infilnm;		/* ptr to file name */
	int lastlnno;		/* last line number read in the file */
	};
char whichin = 0;		/* index int instack */
INSTR instack[MAXINFLS+1];	/* stack of input ifiles */



filespec( filename )
	char *filename;
{

/*
 * This routine is called when there is an input directive to load a 
 * file. It checks the type of file, to distinguish between an ifile
 * and an object file
 */

	FILE *filedes;
#if PORTAR || PORT5AR
	unsigned short	firstword;
	char		atype[SARMAG];
#else
	unsigned short	firstword,
			atype;
#endif

	if ((filedes = fopen(filename,"r")) == NULL) {
		lderror(2,0,sname(curfilnm),"Can't open file %s for input", filename);
		return;
		}

	in_y_exp = 0;

	if (fread( (char *) &firstword, sizeof(firstword), 1, filedes) != 1)
		lderror(2,0,NULL,"Can't read 1st word of file %s",filename);
#if PORTAR || PORT5AR
	fseek(filedes,0L,0);
	if (fread(atype,SARMAG,1,filedes) != 1)
		atype[0] = '\0';
#else
	if ( sizeof(firstword) != sizeof(ARMAG) )
		if ( fread( (char *)&atype, sizeof(atype), 1, filedes) != 1)
			atype = firstword;
#endif

	if (ISMAGIC(firstword))
	{
#ifdef IAPX		
		if ( magic == (unsigned short) 0 ) {
			if ( ISTV(firstword) && (tvflag != 1) )
				yyerror("TV file in NON-TV run");
				
		initvars(firstword);	/* initialize variables for 
					 * either 16 or 20-bit addressing.
					 */
		} else if (firstword != magic) {
			lderror(2,0,NULL,"The magic number for file %s does not match previous objects",filename);
		}
#endif
		ploadfil( filename, filedes, firstword );
		fclose(filedes);
	}

#if PORTAR || PORT5AR
	else if (strncmp(atype,ARMAG,SARMAG) == 0)
#else
	else if ( (firstword == (unsigned short) ARMAG) ||
			((sizeof(firstword) != sizeof(ARMAG)) &&
			 (atype == (unsigned short) ARMAG)) )
#endif
	{
		ploadfil( savefn(filename), filedes, USH_ARTYPE );
		fclose(filedes);
	}

	else if ( BADMAGIC(firstword) )
	{
		yyerror("file %s is of unknown type: magic number = %6.2x",
			filename, firstword);
		fclose(filedes);
	}

	else {
		/*
		 * Assume that the file is an ifile
		 */
		instack[whichin].infilnm = curfilnm;
		instack[whichin].lastlnno = lineno;
		instack[whichin].infildes = filedes;
		curfilnm = savefn(filename);
		lineno = 1;
		if( ++whichin == MAXINFLS ) {
			yyerror("ifile nesting limit exceeded with file %s", filename);
			fclose(filedes);
			curfilnm = instack[--whichin].infilnm;
			lineno = instack[whichin].lastlnno++;
			return;
			}

		if (fseek(filedes, 0L, 0) != 0)
			lderror(2,0,NULL,"Can't seek to the beginning of file %s",
			filename);
		yyin = filedes;
		yyparse();
		close(filedes);

		yyin = instack[--whichin].infildes;
		curfilnm = instack[whichin].infilnm;
		lineno = instack[whichin].lastlnno++;
		yychar = -1;
		}
}



bldadscn(scname, fname, sp)
char *scname;
char *fname;
ACTITEM *sp;		/* ptr to DFNSCN act_item */
{

/*
 * Build a ADDSCN action item
 */

	ACTITEM *p;

	p = (ACTITEM *) myalloc(sizeof(ACTITEM));

	p->addscn.aitype = AIADDSCN;
	p->addscn.aiinflnm = curfilnm;
	p->addscn.aiinlnno = lineno;
	copy(p->addscn.ainame, scname, sizeof(p->addscn.ainame));
	p->addscn.aiscfile = fname;
	p->addscn.aifill = 0;

	listadd(l_AI, &(sp->dfnscn.sectspec), p);
}




ACTITEM *
bldadfil(fname, sp)
char *fname;
ACTITEM *sp;		/* ptr to DFNSCN act_item */
{

/*
 * Build a ADFILE action item
 */

	ACTITEM *p;

	p = (ACTITEM *) mycalloc(sizeof(ACTITEM));

	p->adfile.aitype = AIADFILE;
	p->adfile.aiinflnm = curfilnm;
	p->adfile.aiinlnno = lineno;
	p->adfile.aifilnam = fname;

	listadd(l_AI, &(sp->dfnscn.sectspec), p);

	return (p);
}




bldexp(ep, lp)
ENODE *ep;
LISTOWN *lp;
{

/*
 * Build an EVEXPR action item
 */

	ACTITEM *p;

	p = (ACTITEM *) myalloc(sizeof(ACTITEM));

	p->evexpr.aitype = AIEVEXPR;
	p->evexpr.aiinlnno = lineno;
	p->evexpr.aiinflnm = curfilnm;
	p->evexpr.aiexptr = ep;

	listadd(l_AI, lp, p);
}




pflags(flgname, ifile) 
char *flgname;		/* ptr to x, extracted from -x */
int ifile;		/* set TRUE iff the flag comes from an ifile */
{

/*
 * Process flag specifications
 *
 * Note that flags can come from two sources:
 *	command line : everything is character strings
 *	       ifile : parser has already converted tokens to binary
 */

	register int type;


/*
 *	-a		: make the output file "executable":
 *				1. complain about unresolved references
 *				2. define several "_xxxx" symbols
 *
 *	-cn		: increment the partial execution flag.  'n'
 *				defaults to 1:
 *				0 : normal, complete ld execution
 *				1 : execute parser of PASS 1 only
 *				2 : execute PASS 1 only
 *				3 : execute through allocation (up
 *					to output())
 *				4 : complete ld execution, with
 *					/dev/null as the output
 *					file name
 *
 *	-dn (-b for u3b): increment the internal ld debug flag. 'n'
 *				defaults to 1
 *
 *	-e name		: generate a UNIX a.out header in the optional
 *				header field of the output file, and
 *				set the "entry point" entry to the 
 *				address of the symbol "name".  This
 *				flag forces the flag -X
 *
 *	-f 0xaabb	: use the two-byte value aabb to:
 *				1. fill "holes" between sections
 *				2. fill uninitialized .bss sections
 *
 *	-h nnn		: make the optional header field nnn bytes in
 *				size. Zero fill any unused bytes of this
 *				header
 *
 *	-i		: ***** b16ld and x86ld only *****
 *				use two regions, to give separate I and
 *				D space (64 K each)
 *
 *	-lx		: search the library libx.a in directory
 *				libdirs[n] for any symbols which can
 *				satisfy outstanding unresolved references.
 *
 *	-m		: generate a memory map
 *
 *	-o name		: use "name" as the name of the output file.
 *				The default value is given by the
 *				constant A.OUT, defined in the include
 *				file paths.h
 *
 *	-p[nnn]		: generate a patch list. The size of the
 *				optional header is increased by the
 *				size of the patch list
 *			  the output section size is set to nnn
 *				after allocation and patch list
 *				construction to save physical memory
 *
 *	-r		: keep relocation information in the output file
 *
 *	-s		: strip all relocation and symbol table 
 *				information from the output file
 *
 *	-t		: ****** UNIX only ******
 *			        do type checking

 *	-tv		: ***** b16ld/x86ld/n3bld only *****
 *				generate TV function linkage
 *
 *	-x 		: ***** UNIX only ******
 *				do not preserve symbols in the output symbol
 *				table except static or external symbols
 *
 *	-u name		: add "name" to ld's symbol table, as an
 *				undefined symbol
 *
 *	-z		: ****** UNIX only ******
 *				leave memory around zero unconfigured ro catch
 *			  null pointers
 *
 *	-B nn		: generate "padding" output sections of nn bytes
 *				of zero following any output section
 *				which is either:
 *				1. of zero length
 *				2. all of uninitialzed .bss 
 *
 *	-F		: ****** UNIX only ******
 *				indicate in header file for paging, block
 *				for paging, memory configure for paging, etc.
 *
 *	-H		: change the type of all global symbol table
 *				entries to "static"
 *
 *	-L<path>	: provides alternate directories and orders
 *			  search path for libraries (equivalent to
 *			  -I flag for cpp includes). -L must precede
 *			  -l to be effective.
 *
 *	-M		: ****** UNIX only *****
 *				output message if external variables are
 *				multiply defined
 *
 *	-N		: squash text and data together
 *
 *	-S		: "silent" mode: only fatal errors will be 
 *				listed - not warnings or diagnostics
 *
 *	-V		: output current ld environment definition. '-V'
 *				flags are cumulative:
 *				1 : describe the ld version
 *				2 : describe the machine and os version
 *
 *	-VS nn		: put nn as version stamp in the optional header
 *
 *	-X		: generate, in the optional header field, a
 *				standard UNIX a.out header
 *				(except for u3b, where it means
 *				 elimate compiler generated labels)
 *
 *	-ild		: generate an "extra" output section for each
 *				unallocated area of configured memory
 *
 */

	switch (flgname[0]) {
#if !UNIX || M32
		case 'a':
			aflag++;
			break;
#endif
		case 'c':
			cflag++;
			break;
		case 'e':
			if( ifile )
				if (yylex() != NAME)
					yyerror("-e flag does not specify a legal symbol name: %s", inline);
				else
#if FLEXNAMES
					{
					epsymbol = myalloc(strlen(yylval.sptr) 
								+1);
					strcpy(epsymbol, yylval.sptr);
					}
#else
					copy(epsymbol, yylval.sptr, 8);
#endif
			else {
#if FLEXNAMES
				epsymbol = *++argptr;
#else
				copy(epsymbol, *++argptr, 8);
#endif
				argcnt--;
				}
			Xflag++;
			break;
		case 'f':
			if( ifile )
				if (yylex() != LONGINT)
					yyerror("-f flag does not specify a two-byte number: %s", inline);
				else
					globfill = (short) *yylval.lptr;
			else {
				if( (globfill = (short) stoi(*++argptr)) == (short) 0x8000 ) {
					yyerror("-f flag does not specify a two-byte number: %s", *argptr);
					globfill = 0;
					}
				argcnt--;
				}
			break;
#if !UNIX
		case 'h':
			if( ifile )
				if (yylex() != LONGINT)
					yyerror("-h flag does not specify a numeric value: %s", inline);
				else 
					hflag = *yylval.lptr;
			else {
				if( (hflag = stoi(*++argptr)) == 0x8000 ) {
					yyerror("-h flag does not specify a numeric value: %s", *argptr);
					hflag = 0;
					}
				argcnt--;
				}
			if( hflag < 0 ) {
				yyerror("invalid value on -h flag: %.2x", hflag);
				hflag = 0;
				}
			break;
#endif
		case 'i':
#if IANDD
			if (flgname [1] == '\0')
				{
				iflag++;
				break;
				}
#endif
#if ILDOPT
			if (flgname [1] == 'l' && flgname [2] == 'd')
				{
				ildflag++;
				break;
				}

#endif
			break;
		case 'l':
			if( flgname[1] == '\0' )
				lderror( 0, 0, NULL, "-l requires an argument");
			else 
				(void) library( flgname );
			break;
		case 'm':
			mflag++;
			break;
		case 'n':
			break;
#if !UNIX
		case 'p':
			pflag = -1;	/* use default patch size */
			if( flgname[1] ) {
				if( (pflag = stoi(&flgname[1])) == 0x8000 ) {
					yyerror("-p flag does not specify a number: %s", &flgname[1]);
					pflag = -1;
				} else {
					if (pflag <= 0) {
						yyerror("-p flag size out of range: %s", &flgname[1]);
						pflag = -1;
						}
					} /* else */
				} /* if flgname */
			break;
#endif
		case 'o':	
			if( ifile ) {
				type = yylex();
				if( (type == FILENAME)  ||  (type == NAME) )
					copy(outfilnm, yylval.sptr, 128);
				else
					yyerror("-o flag does not specify a valid file name: %s", inline);
				}
			else {
				copy(outfilnm, *++argptr, 128);
				argcnt--;
				}
			if( outfilnm[127] != '\0' ) {
				outfilnm[127] = '\0';
				yyerror("-o file name too large (> 128 char); truncated to (%s)", outfilnm);
				}
			break;
		case 'r':
			rflag++;
			break;
		case 's':
			sflag++;
			break;
		case 't':
#if TRVEC
			if (flgname[1] == 'v') {
				tvflag = 1;
#if !IAPX
				magic = (unsigned short) TVMAGIC;
#endif
				break;
				}
#endif
#if COMMON
			tflag++;
#endif
			break;
		case 'u':	
			if( ifile )
				if( yylex() != NAME )
					yyerror("-u flag does not specify a legal symbol name: %s", inline);
				else
					undefsm( yylval.sptr );
			else {
				undefsm( *++argptr );
				argcnt--;
				}
			break;
		case 'x':
			xflag++;
			break;
#if PAGING
		case 'z':
			zflag++;
			break;
#endif
#if !UNIX || !DMERT
		case 'B':
			if( ifile )
				if (yylex() != LONGINT)
					yyerror("-B flag does not specify a number: %s", inline);
				else
					Bflag = *yylval.lptr;
			else {
				if( (Bflag = stoi(*++argptr)) == 0x8000 ) {
					yyerror("-B flag does not specify a number: %s", *argptr);
					Bflag = 0;
					}
				argcnt--;
				}
			break;
		case 'H':
			Hflag++;
			break;
#endif
#if COMMON
		case 'M':
			Mflag++;
			break;
#endif
		case 'N':
			Nflag++;
			break;
		case 'L':
			if ( nldirs >= MAXLDIRS )
				yyerror("too many -L options, %d allowed", MAXLDIRS-1);
			else if ( flgname[1] == NULL )
				yyerror("no directory given with -L");
			else if ( strlen(&flgname[1]) > MAXDLEN )
				yyerror("-L path too long(%s)", &flgname[1]);
			else {
				libdirs[nldirs] = myalloc(size(&flgname[1]));
				strcpy(libdirs[nldirs++], &flgname[1]);
				}
				break;
		case 'S':
			Sflag++;
			break;
		case 'V':
			if (flgname[1] == 'S')  {
				if( ifile )
					if (yylex() != LONGINT)
						yyerror("-VS flag does not specify a numeric value: %s", inline);
					else 
						VSflag = *yylval.lptr;
				else {
					if( (VSflag = stoi(*++argptr)) == 0x8000 ) {
						yyerror("-VS flag does not specify a numeric value: %s", *argptr);
						VSflag = 0;
						}
					argcnt--;
					}
				}
			else  {
				Vflag++;
				}
			break;
#if !UNIX
		case 'X':
			Xflag++;
			break;
#endif
		case 'Y':
			{
			int YLflag = 0, YUflag = 0;
			char *dirname, *tempname;
			if (ifile) {
				yylex();
				if (yylval.sptr[0] == 'L'){
					YLflag++;
					if ((yylval.sptr[1] == 'U') && 
						yylval.sptr[2]== NULL)
						YUflag++;
					else if (yylval.sptr[1] != NULL)
						goto Yflagerr;
					}
				else if (yylval.sptr[0] == 'U') {
					YUflag++;
					if ((yylval.sptr[1] == 'L') && 
						(yylval.sptr[2]== NULL))
						YLflag++;
					else if (yylval.sptr[1] != NULL)
						goto Yflagerr;
					}
				else
					goto Yflagerr;
				if (yylex() != COMMA)
					goto Yflagerr;
				if (yylex() != FILENAME)
					lderror(2,0,NULL, "Second argument to Yflag must be a directory name");
				else
					dirname = yylval.sptr;
				}
			else {
				argcnt--;
				if ((*++argptr)[0] == 'L') {
					YLflag++;
					if(((*argptr)[1] == 'U') && 
						((*argptr)[2] == ',')) {
						YUflag++;
						dirname = &((*argptr)[3]);
						}
					else if((*argptr)[1] == ',')
						dirname = &((*argptr)[2]);
					else
						goto Yflagerr;
					}
				else if ((*argptr)[0] == 'U') {
					YUflag++;
					if(((*argptr)[1] == 'L') && 
						((*argptr)[2] == ',')) {
						YLflag++;
						dirname = &((*argptr)[3]);
						}
					else if((*argptr)[1] == ',')
						dirname = &((*argptr)[2]);
					else
						goto Yflagerr;
					}
				else
					goto Yflagerr;
				}	
			tempname = myalloc(strlen(dirname) + 1);
			strcpy(tempname,dirname);
			if (YLflag)
				libdirs[0] = tempname;
			if (YUflag)
#ifdef LLIBDIR1
				libdirs[1] = tempname;
#else
				lderror(0,0,NULL, "ld was built with only one default search library directory so U argument to -Y is ignored");
#endif
			}
			break;

Yflagerr:			lderror(2,0,NULL, "Yflag must be followed by U or L or both, followed by ,dirname");

		default:
			specflags(flgname, argptr);
		}
}


char *
library(fname)
	char *fname;
{

/*
 * For a library flag of the form "-lx", build a file name of the form
 *
 *		"libdirs[n]/libx.a"
 *
 * where libdirs[0] is LIBDIR (defined in paths.h) and libdirs[n>0]
 * are defined using the -L option. Then build an INFILE structure
 * and return the file name.
 */

	static char libname[MAXDLEN+16];	/* -lx file name */
	int dndx;
	char *p;
	FILE *lfd;
	char *nameptr;
	unsigned short firstword;
#if PORTAR || PORT5AR
	char atype[SARMAG];
#else
	unsigned short atype;
#endif

	if ( *(++fname) == '\0' ) {
		yyerror("the -l flag (specifying a default library) is not supported");
		return( NULL );
		}
	else
		p = fname;

	/*
	 *  Loop thru libdir[NDELDIRS] thru libdir[nldirs-1]
	 */
	for ( dndx = NDELDIRS; dndx < nldirs; dndx++)
	{
		sprintf(libname, "%s%s%.8s%s", libdirs[dndx], "/lib", p, ".a");
		if ( (lfd = fopen(libname,"r")) != NULL)
			goto foundit;
	}

	/*
	 *  loop thru libdirs[0] thru libdirs[NDELDIRS-1]
	 */
	for ( dndx = 0; dndx < NDELDIRS; dndx++)
	{
		sprintf(libname, "%s%s%.8s%s", libdirs[dndx], "/lib", p, ".a");
		if ( (lfd = fopen(libname,"r")) != NULL)
			goto foundit;
	}

	yyerror("can't find library lib%s.a", p);
	return( NULL );

foundit:
	nameptr = savefn( libname );
	if (fread( (char *) &firstword, sizeof( firstword ), 1, lfd ) != 1)
		lderror(2,0,NULL, "Can't read 1st word of file %s", libname );

#if PORTAR || PORT5AR
	fseek( lfd, 0L, 0 );
	if (fread( atype, SARMAG, 1, lfd ) != 1)
		atype[0] - '\0';
#else
	if (sizeof( firstword ) != sizeof( ARMAG ))
		if (fread( (char *) &firstword, sizeof( atype ), 1, lfd ) != 1)
			atype = firstword;
#endif

	if (ISMAGIC( firstword ))
		ploadfil( nameptr, lfd, firstword );
#if PORTAR || PORT5AR
	else if (strncmp( atype , ARMAG, SARMAG ) == 0)
#else
	else if (( firstword == unsigned short) ARMAG)
		|| (( sizeof( firstword ) != sizeof( ARMAG ))
		&& ( atype == (unsigned short) ARMAG)))
#endif
		ploadfil( nameptr, lfd, USH_ARTYPE );
	else
		yyerror( "file %s is of unknown type: magic number = %6.2x",
			nameptr, firstword );

	fclose( lfd );
	return( nameptr );
}



char *
savefn(name)
char *name;
{

/*
 * Save the name of an ifile or input *.o or archive file
 */

	char *strbeg;
	int length;

	length = strlen(name) + 1;
	if (strnext + length > strlimit)
	{
		strbase = myalloc( STRINCRE );
		strnext = strbase;
		strlimit = strbase + STRINCRE;
	}
	strbeg = strnext;
	strncpy( strbeg, name, length );
	strnext += length;

	return( strbeg );
}
