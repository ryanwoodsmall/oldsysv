/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mkshlib:common/t_mklib.c	1.14.1.3"

#include <stdio.h>
#include "filehdr.h"
#include "ldfcn.h"
#include "syms.h"
#include "scnhdr.h"
#include "ar.h"
#include "storclass.h"
#include "shlib.h"
#include "trg.h"
#include "paths.h"
#include "sgs.h"
#include <sys/types.h>
#include <sys/stat.h>

LDFILE	*ldptr;		/* Pointer used to open and read archives */
long 	hval;
char	**libname;
int	nlibs;

extern int errno, sys_nerr;
extern char *sys_errlist[];
mklib()
{
	int	sl;
	usdef	*puslst;
	long	h;
	int	symerr = FALSE;

	/* Create list of possible other shared libraries to be searched */
	libsearch();

	/* Open each archive and look for all undefined symbols */
	for (sl=0; sl<nlibs; sl++)
	{
		/* Find all previously undefined symbols */
		/* that exist in this archive */
		symseek (libname[sl]);	
	}	

	/* Loop through hash table, and check all undefined symbols */
	/* If a symbol is still undefined, quit */
	for (h=0; h<USSIZ; h++)
	{
		puslst = uslst[h];
		while (puslst != NULL)
		{
			if (puslst->found == FALSE)
			{
				fprintf (stderr,"%s still undefined in target after search of other shared libraries\n",puslst->name);
				symerr = TRUE;
			}
			puslst = puslst->next;
		}
		if (symerr)
			fatal ("Undefined symbols - cannot complete build of target");
	}

	/* Print all symbol definitions into an ifile as name=value; */
	usprint();  
}

/*  lookus
	Look up a name in the undefined symbol hash table
									*/
usdef	*lookus(name)
char	*name;
{
	usdef	*puslst;
	
	hval = hash(name,USSIZ);
	puslst = uslst[hval];

	while (puslst != NULL)
	{
		if (strcmp (puslst->name,name) == 0)
			return (puslst);
		puslst = puslst->next;
	}
	return(NULL);
}

static int nextdir = -1;
static char **dirs = NULL;

adddir(directory)
	char *directory;
{
	static int numdirs = 16;	/* Let's start with 16 */
	struct stat statb;
	
	if (directory == NULL)
	{
		fatal("Invocation error - invalid filename given to -L");
		return;
	}
	if (access(directory, 01) == -1)
	{
		extern int errno;
		extern char *sys_errlist[];

		warn("Ignoring directory %s: %s", directory,
		sys_errlist[errno]);
		return;
	}
	if (stat(directory, &statb) == -1 ||
	   (statb.st_mode&S_IFMT) != S_IFDIR)
	{
		warn("Ignoring %s, not a valid directory",
		directory);
		return;
	}
	if (dirs == NULL) {
		dirs = (char **)calloc(sizeof(char *), numdirs);
		nextdir = 0;
	} else if (nextdir == numdirs) {
		numdirs *= 2;
		dirs = (char **)realloc(dirs, sizeof(char *)*numdirs);
		}
	if (dirs == NULL)
		fatal("Out of memory - cannot allocate space for directories");
	dirs[nextdir++] = directory;
}

/*  libsearch

	Create list of shared libraries to be searched for undefined
	symbols.  Two cases:

		1) Path to library input with -L option
		   Name of library under #objects noload
		   -l<libname>
		   Prepend path to <libname>.a (.a appended to name)
		   for each path and archive name
		2) It's a filename.

	We try to mimic ld(1) behavior, which means that the first
	readable file in the "-lX" search is taken, whether it's
	a COFF file or not.
								 	*/
static char nodirs[] = "Cannot find any valid directories in which to find \"#object noload\" files\nCheck default directories and -L arguments carefully.";

libsearch()
{
	char	*lib;
	char 	*templib;
	int	l,nl;

	/* numnold (number of object noload items) is max number of other */
	/* shared libraries that can be searched */
	/* Allocate space for list of search paths */
	if ((libname=(char **)malloc(sizeof(char *) * numnold)) == NULL)
		fatal("Out of space");

	nlibs = 0;		/*  Find number of possible search paths */
	for (l=0; l<numnold; l++)
	{
		lib = objnold[l];	/* Each #objects noload entry */
		if (lib[0] == '-' && lib[1] == 'l')
		{
			if (nextdir < 0)
					fatal(nodirs);
			lib += 2;	/* points to the 'X' in "-lX" now */
			for (nl=0; nl< nextdir; nl++)	
			{
				templib	= strbuild(dirs[nl], "/lib", lib,  ".a", NULL);
				if	(access (templib,04) == 0) /* success */
				{
					libname[nlibs++] = templib;
					break;
				}
				else
					free(templib);
			 }	/* for nl := 0 until nextdir... */
			if (nl == nextdir)
				warn("cannot find lib%s.a in any of the -L or default directories\n",lib);
		} else if (access (lib,04) == 0) /* Can read library */
			libname[nlibs++] = stralloc(lib);
		else
			warn("%s: %s", lib, (errno < 0 || errno >= sys_nerr)
			    ? "cannot read" : sys_errlist[errno]);
	}	/* for */
}


/*  symseek

	Open an archive from list of other shared libraries (libname).
	Look up each symbol from string table in undefined symbol hash
	table (uslst).  If found, store offset into object file and
	record that symbol was found.  Follow each offset to object
	file.  Find symbol and store value and special symbol definition
	from object file.
									*/
symseek(lib)
char	*lib;
{
	extern	long	sgetl( );

	long		num_syms;
	char		*offsets;
	char		num_buf[sizeof(long)];
	struct	ar_hdr	arbuf;
	
	int		iar_size;
	char		*strtab;
	char		*fptr;
	usdef		*puslst; 	/* Pointer to undefined symbol hash list */
	long		ns;		/* Loop counter - number of symbols */
	int		len;

	/* Find an existing archive */
	if ((ldptr = ldopen (lib,(LDFILE *)NULL)) != NULL)
	{
		char armag[SARMAG+1];

		/* Check archive magic number */
		if (fseek(IOPTR(ldptr), 0L, BEGINNING) != 0 ||
		    fread(armag, 1, SARMAG, IOPTR(ldptr)) != SARMAG)
		{
			warn("Ignoring %s: not an archive", lib);
			(void) ldaclose(ldptr);
			return;
		}
		if (strncmp(armag, ARMAG, SARMAG) != 0)
		{
			fprintf (stderr,"\"#objects noload\" directive for %s ignored: not an archive\n",lib); 
			ldaclose(ldptr);
			return;	/* Continue with search of next archive */
		}
		if (FREAD( &arbuf, sizeof( struct ar_hdr ), 1, ldptr) != 1)
		{
			warn ("\"#objects noload\" directive for %s ignored: cannot read archive header\n",lib); 
			(void) ldaclose(ldptr);
			return;
		}

		if ( arbuf.ar_name[0] == '/' )
		{
			int nbytes;
			if (FREAD( num_buf, sizeof( num_buf ), 1, ldptr ) != 1)
			{
				warn ("\"#objects noload\" directive for %s ignored:\ncannot read number of symbols in archive symbol table",lib);
				(void) ldaclose(ldptr);
				return;
			}
			num_syms = sgetl( num_buf );
			nbytes = num_syms * sizeof(long);
			if ((offsets = malloc(nbytes))
				== NULL)
			{
				fatal("Out of space");
			}
			if ( FREAD( offsets, sizeof(long), (int) num_syms,
				ldptr) != (int) num_syms)
			{
				warn ("\"#objects noload\" directive for %s ignored:\ncannot read offsets from archive symbol table",lib);
				(void) ldaclose(ldptr);
				return;
			}

			iar_size = atoi(arbuf.ar_size);
			nbytes = iar_size - (sizeof(long)*(num_syms+1));
			if ((strtab = malloc(nbytes)) == NULL)
				fatal("Out of space");
			if (FREAD (strtab, sizeof(char), nbytes, ldptr) !=
			   (int) nbytes)
			{
				warn ("\"#objects noload\" directive for %s ignored:\ncannot read strings from archive symbol table",lib);
				(void) ldaclose(ldptr);
				return;
			}

			fptr = strtab;				/* Front of symbol */
			for (ns=0; ns<num_syms; ns++)		/* For each symbol */
			{
				/* Look up in undefined symbol hash table */
				/* lookus() returns pointer into hash table */
				puslst = lookus (fptr);	
				if ((puslst != NULL) && (puslst->found == FALSE))
				{
					seekobj(fptr, offsets, puslst, lib);
				} 
				offsets += sizeof(long);	/* And offset */
				fptr += strlen(fptr) + 1 /* For the NULL */;
			}
		}
		else
		{
			warn ("Ignoring %s: no archive string table - use 'ar -ts' to create one\n",lib);
			(void) ldaclose(ldptr);
			return;
		}
		ldaclose(ldptr);		/* Close lib */
	} else if (close(open(lib,0)) == -1)
		warn ("\"#objects noload\" directive for %s ignored:\ncannot open",lib);
	else
		warn ("\"#objects noload\" directive for %s ignored:\nnot a COFF file (possibly an empty archive)", lib);
}


/*  seekobj
	
	Seek to object file for given symbol using offset field of ldptr.
	This routine finds the absolute value of the symbol and stores
	all the special symbols (at least one) found in the object file.
									*/

seekobj(name, offsets, puslst, lib)
char	*name;
char	*offsets;
usdef	*puslst;
char	*lib;
{
	struct	ar_hdr	arbuf;
	SYMENT		*sym;		/* Symbol entry structure */
	SYMENT		**pobjdefs;	/* Array of special symbols defined */
					/* in object file */
	int		nobjs;		/* Number of special symbol definitions */
	long		ns, n;
	char		*symname;	/* Symbol name from object file */

	if (puslst->objdefs != NULL)
		fatal ("Should not be processing undefined symbol %s a second time\nerror in internal mkshlib handling of nested shared libraries",name);

/*
 * Set offset field of ldptr to object file's archive header and
 * read in that header (for error messages, should we need the
 * member name).
 */
	OFFSET(ldptr) = sgetl(offsets);
	if (fseek ( IOPTR (ldptr), OFFSET(ldptr), 0) != 0)
		fatal("%s found in archive, but cannot seek to object file",name);
	if (FREAD((char *)&arbuf, sizeof(arbuf), 1, ldptr) != 1)
		fatal("Cannot read header for archive member in %s", lib);
	trim_arname(arbuf.ar_name, sizeof(arbuf.ar_name));
	OFFSET(ldptr) += sizeof (struct ar_hdr);
/*
 * At this point, OFFSET(ldptr) is the offset into the archive
 * to byte 0 of the object file.
 *
 * We have to initialize HEADER(ldptr), and then it will look
 * to the ldopen(3X) routines like we have been playing by the book.
 */
	if (FREAD((char *)&(HEADER(ldptr)), sizeof(FILHDR), 1, ldptr) != 1)
		fatal ("%s found in archive %s, but can't read object file header",name,lib);
/*
 * Another point where we mimic ld(1):
 *	It's possible that an archive has multiple types of COFF
 *	object files, and ld(1) will ignore all members of the
 *	archive that don't start with the magic number specific
 *	to the CPU type we're producing an object for.
 *
 *	So, we first check for *ANY* COFF magic number, just to
 *	make sure it's an object file. (If not, then the archive
 *	string table shouldn't have pointed to this member, hence
 *	the fatal error.)
 *
 *	Then, we check that it's the magic number specific to the
 *	target machine (ISMAGIC).
 */
	if (!ISCOFF(HEADER(ldptr).f_magic))
		fatal("%s(%s) has bad magic number (0%o)",
			lib, arbuf.ar_name, HEADER(ldptr).f_magic);
	if (!ISMAGIC(HEADER(ldptr).f_magic))
	{
		warn("Ignoring occurrence of %s in %s(%s), magic number (0%o) doesn't correspond to this machine type",
			name, lib, arbuf.ar_name, HEADER(ldptr).f_magic);
		return;
	}
	sym = (SYMENT *)malloc (sizeof (SYMENT));
	if (ldtbseek (ldptr) == SUCCESS)
	{	
		ns = HEADER(ldptr).f_nsyms;	
		pobjdefs = (SYMENT **)malloc(sizeof(SYMENT *) * ns);
		nobjs = 0;
		for (n = 0;n<ns;) 
		{
			if (ldtbread (ldptr,n,sym) == SUCCESS)
			{
				if ((symname = ldgetname(ldptr,sym)) != NULL)
				{
					/* Is this symbol entry */
					if (strcmp (symname,name) == 0)
					{
					/* Check to make sure symbol is external */
						if (sym->n_sclass == C_EXT)
						{
						  if ((sym->n_value >= 0) && (sym->n_scnum >0))
						  {
							error ("relocatable occurrence of \"%s\" in %s(%s)",
							name, lib, arbuf.ar_name);
							puslst->nobjdefs = 0;
							free(pobjdefs);
							puslst->objdefs = NULL;
							return;
						  } else
						  {
						  	if (sym->n_scnum != N_ABS)
						    	  fatal ("Found %s in %s, but it's not a usable symbol",
							    symname);
					/* Take first symbol definition encountered */
						  	puslst->found = TRUE;
						  	puslst->value = sym->n_value;
						  }
						}
					}
					else
					{
					/* Store special symbols defined in this object file */
						if (sym->n_sclass == C_EXT && (sym->n_scnum > 0 || sym->n_scnum == N_ABS))
						{
							if (symname[strlen(symname)-1] == ']')
							{
								pobjdefs[nobjs] = (SYMENT *)malloc(sizeof(SYMENT));
								initsym(pobjdefs[nobjs++], stralloc(symname));
							}
						}
					}

				} else fatal("Cannot read symbol %s in archive %s\n",name, lib);	
			}
			else
				fatal ("%s found in archive %s, but symbol table is unreadable",name, lib);	
			/* Looked at another symbol entry */
			/* Bump n with number of aux. entries */
			n = n + 1 + sym->n_numaux;
		}				/* while loop */

		/* Finished collecting all special symbols in object file */
		puslst->objdefs = pobjdefs;
		if (nobjs == 0)
			fatal("No file definition symbols found for objects in archive %s\n probably referencing old (cplu4.0) shared library", lib);
		puslst->nobjdefs = nobjs;

	}   		
	else
		fatal ("%s found in archive %s, but cannot seek to symbol table",name, lib);
	if (puslst->found == FALSE)
		fatal ("%s found in string table, but not in specified object file in archive %s",name, lib);
	checkforerrors(
"\nOrder of members in archive is suspect - only absolute symbols are\n\
used in \"#objects noload\" processing, and a non-absolute was seen first.");

}


/*  usprint

	Print undefined symbols that are referenced in other shared
	libraries out to an ifile that will resolve their definitions.
	Definitions printed as <name> = <value>; pairs.
									*/

usprint()
{
	usdef	*puslst;	/* Pointer to uslst hash table */
	FILE	*ifil;
	long	h;

	/* open ifil4name */
	if ((ifil = fopen(ifil4name,"w")) == NULL)
		fatal ("Cannot open ifile for printing out symbol definitions");

	/* Print each undefined symbol out to ifile */
	for (h=0; h<USSIZ; h++)
	{
		puslst = uslst[h];
		while (puslst != NULL)
		{
			fprintf (ifil,"%s = 0x%x;\n",puslst->name, puslst->value);
			puslst = puslst->next;
		}
	}

	(void)fclose(ifil);

}

trim_arname(name, sz)
	char *name;
{
	register char *p;

	p = &name[sz];

	while (*--p == ' ')
		if (p == name)	/* Walked off the end */
		{
			(void) strcpy(name, "*unknown*");
			return;
		}
	if (*p == '/')
		*p = '\0';
	else
		*++p = '\0';
}


