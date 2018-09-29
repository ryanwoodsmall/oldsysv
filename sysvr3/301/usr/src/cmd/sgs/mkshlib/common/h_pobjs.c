/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mkshlib:common/h_pobjs.c	1.4"

#include <stdio.h>
#include "filehdr.h"
#include "scnhdr.h"
#include "syms.h"
#include "ldfcn.h"
#include "reloc.h"
#include "aouthdr.h"
#include "sgs.h"
#include "shlib.h"
#include "hst.h"



/* This function initializes the hash table containing all the external symbols
 * in the target's symbol table, trgsymtab. This information is placed in a
 * hash table so that the absolute addresses of external symbols defined in
 * the constituent object files can be easily found.
 */
void
init_trgsymtab()
{
	long	i;
	LDFILE	*ldptr;		/* pointer to target shared library file */
	AOUTHDR	ohead;		/* a.out header of target */
	long	nsyms;		/* number of symbols in target */
	SYMENT	symbol;		/* current symbol table entry */
	stab	*pstab;		/* pointer to a hash table entry for trgsymtab */
	long	hval;		/* hash value */
	char	*name;		/* name of current symbol */

	/* open trgname and make sure it is of the appropriate type */
        if ((ldptr= ldopen(trgname,(LDFILE *)NULL)) == NULL)
                fatal("Cannot open %s", trgname);
        if (!ISCOFF(HEADER(ldptr).f_magic))
                fatal("Bad magic: %s",trgname);
        if (ISARCHIVE(TYPE(ldptr)))
                fatal("%s is an archive",trgname);

	(void)FSEEK(ldptr, (long)FILHSZ, BEGINNING);
	if (HEADER(ldptr).f_opthdr == 0 ||
			FREAD(&ohead, (int)HEADER(ldptr).f_opthdr, 1, ldptr) != 1)
		fatal("Cannot read optional header of %s",trgname);
	if (ohead.magic != LIBMAGIC)
		fatal("%s is not a target shared library",trgname);

	/* scan symbol table of trgname and initialize trgsymtab */
	if ((nsyms= HEADER(ldptr).f_nsyms) == 0) {
		(void)fprintf(stderr,"WARNING: %s has no symbols\n",trgname);
		return;
	}
	if (ldtbseek(ldptr) == FAILURE)
		fatal("Cannot seek to symbol table of %s",trgname);
	for (i=0; i<nsyms; i++) {
		if (ldtbread(ldptr, i, &symbol) == FAILURE)
			fatal("Cannot read symbol table of %s",trgname);
		i+= symbol.n_numaux;

		/* make sure symbol is external */
		if (symbol.n_sclass != C_EXT) 
			continue;

		if ((name= ldgetname(ldptr, &symbol)) == NULL)
			fatal("Cannot get name of symbol in %s",trgname);

		/* get a new structure for the target symbol hash table, trgsymtab,
		 * and initialize it */
		hval= hash(name,TRSIZ);
		pstab= newstab(stralloc(name),(long)symbol.n_value, trgsymtab[hval]);

		/* Now put the new symbol entry in the hash table. */
		trgsymtab[hval]=pstab;
	}

	(void)ldclose(ldptr);
}
	

/* This function reads all relevent pieces of object file, onam, into the structure
 * pointed to by pobj.  In particular it stores the file header, the section header,
 * section contents and relocation entries for all sections except .text, .data and
 * .bss, and the symbol and string tables.
 * At the same time, the archive symbol table, arsymtab, is initialized.
 * The archive symbol table is a hash table containing all the defined symbols
 * in the archive and a pointer to the object in which each symbol is defined.
 */
void
readobj(idx)
long	idx;	/* index of object file in trgobjects */
{
	char	*onam;		/* current input object file name */
	LDFILE	*ldptr;		/* pointer to current object file */
	scnlst	*pscn;
	SCNHDR	shead;
	int	nscns;		/* number of sections in the current object */
	int	numsects;	/* number of sections in the modified object */
	long	hdoffst;	/* offset of next section header */
	RELOC	*relptr;	
	SYMENT 	*symptr;
        obj     *pobj;          /* pointer to the obj structure holding info. for
                                 * the current object file. */
        obj     *objptr;        /* used to scan through objlst[] for repeated
                                 * filenames. */
	arstab	*parstab;	/* pointer to an entry to be placed in the archive
				 * symbol table. */
	char	*name;
	long	nsyms;		/* number of symbols in object file */
	long	strsize;	/* holds size of string table of object file */
	long	i, 
		j,
		hval;

	/* allocate pobj */
        pobj=newobj();		/* get a new obj structure */
        objects[idx]= pobj;	/* set member of objects array to point to
                                 * this structure. */
	onam= trgobjects[idx];
        pobj->objname= getbase(onam);

        /* insert the new obj structure in objlst */
        hval= hash(pobj->objname, OBSIZ);

        /* first make sure that no other input file has the same basename */
        objptr= objlst[hval];
        while (objptr != NULL) {
                if (strcmp(pobj->objname, objptr->objname) == 0)
                	fatal("File %s supplied more than once", pobj->objname);
                objptr= objptr->next;
        }

        pobj->next= objlst[hval];
        objlst[hval]= pobj;

	/* Open object file, onam, and make sure it is the proper type. */
	if ((ldptr= ldopen(onam,(LDFILE *)NULL)) == NULL)
		fatal("Cannot open %s", onam);
	if (!ISCOFF(HEADER(ldptr).f_magic) || ISARCHIVE(TYPE(ldptr)))
		fatal("%s is not an object file",onam);

	/* read in file header */
	pobj->fhead= HEADER(ldptr);
	
	/* scan through sections and store information for all sections except
	 * .text, .data and .bss */
	(void)FSEEK(ldptr,(long)(FILHSZ + HEADER(ldptr).f_opthdr),0);
	nscns= HEADER(ldptr).f_nscns;
	numsects=0;
	for (i=0; i<nscns; i++) {
		if (FREAD((char *)&shead, SCNHSZ, 1, ldptr) != 1)
			fatal("Cannot read section header in %s",onam);

		/* skip .text, .data and .bss sections */
		if (strncmp(shead.s_name,".text",sizeof(shead.s_name)) == 0 ||
			strncmp(shead.s_name,".data",sizeof(shead.s_name)) == 0 ||
			strncmp(shead.s_name,".bss",sizeof(shead.s_name)) == 0)
			continue;

		/* allocate new structure to hold section information and put it
		 * on the section list for the object, pobj->sects */
		pscn= newscnlst();
		pscn->next= pobj->sects;
		pobj->sects= pscn;

		numsects++;
		hdoffst= FTELL(ldptr);	/* save current file position */

		shead.s_nlnno=0;	/* eliminate any line number entries 
					 * (they won't be right anyway) */
		pscn->shead= shead;	/* record section header */
		if (shead.s_size != 0) {
			/* record section contents */
			(void)FSEEK(ldptr,shead.s_scnptr,BEGINNING);
			if ((pscn->contents= malloc((unsigned)shead.s_size)) == NULL)
				fatal("Out of space");
			if (FREAD(pscn->contents,(int)shead.s_size,1,ldptr) != 1)
				fatal("Cannot read section contents of %s",onam);
		}
		if (shead.s_nreloc != 0) {
			/* record section's relocation entries */
			(void)FSEEK(ldptr,shead.s_relptr,0);
			if ((pscn->relinfo= (RELOC *)
				malloc(shead.s_nreloc*sizeof(struct reloc)))==NULL)
                                fatal("Out of space");
			relptr= pscn->relinfo;
			for (j=0; j<shead.s_nreloc; j++, relptr++) {
				if (FREAD(relptr, RELSZ, 1, ldptr) != 1)
					fatal("Cannot read relocation entries of %s",onam);
			}
		}
		(void)FSEEK(ldptr,hdoffst,0);
	}
	pobj->fhead.f_nscns= numsects;

	/* Read in symbol and string tables.
	 * First read in the string table. */
        (void)FSEEK(ldptr, STROFFSET(ldptr), 0);
        if (FREAD( (char *)&strsize, 4, 1, ldptr) != 1)
                strsize = 0;
        else {
                if ((pobj->strtab = malloc((unsigned)strsize)) == NULL)
                        fatal("Out of space");
                if (FREAD( (pobj->strtab)+4, (int)strsize-4, 1, ldptr) != 1)
                        fatal("Cannot read string table of %s",onam);
        }

        /* allocate space for the symbol table and read it into memory */
        nsyms= HEADER(ldptr).f_nsyms;
	if ((pobj->symtab= (SYMENT *)malloc((unsigned)(nsyms*sizeof(SYMENT))))==NULL)
		fatal("Out of space");

	(void)FSEEK(ldptr, HEADER(ldptr).f_symptr, BEGINNING);
        for (i=0, symptr=pobj->symtab; i<nsyms; i++, symptr++) {
                if (FREAD(symptr,SYMESZ,1,ldptr) != 1)
                        fatal("Cannot read symbol table of %s",onam);
	}

	/* adjust pointers into the string table; change entries of defined
	 * symbols so their value reflects their absolute address as found
	 * in the target; and add defined symbols to the archive symbol table. */
	for (i=0, symptr=pobj->symtab; i < nsyms; ) {
		/* adjust pointers into the string table */
		if (symptr->n_zeroes == 0)
			symptr->n_nptr= pobj->strtab + symptr->n_offset;
	
		/* now look at the defined symbols */
		if (symptr->n_sclass == C_EXT && 
				(symptr->n_scnum > 0 || symptr->n_scnum == N_ABS)) {
			/* For each defined symbol, get its absolute address
			 * (from trgsymtab) and add it to the archive
		 	 * symbol table, arsymtab */
			name= getname(symptr);
			symptr->n_scnum= N_ABS;	/* make the symbol absolute */
			symptr->n_value= getabs(name);	/* get absolute value of
							 * symbol from trgsymtab */

			/* Stick symbol in archive symbol table, arsymtab.
			/* First make sure symbol is not already defined. */
			hval= hash(name,ARSIZ);
			parstab= arsymtab[hval];
			while (parstab != NULL) {
				if (strcmp(parstab->name, name) == 0)
					fatal("Multipy defined symbol, %s, found in shared library",name);
				parstab= parstab->next;
			}
			parstab= newarstab(stralloc(name),pobj,arsymtab[hval]);
			arsymtab[hval]=parstab;
		} else if (symptr->n_scnum > 0) {
			/* section numbers will not be valid in the modified object;
			 * section numbers are therefore changed to N_DEBUG so
			 * that programs which examine these symbols will at least
			 * see predictable results */
			symptr->n_scnum= N_DEBUG;
		}
		i+= 1 + symptr->n_numaux;
		symptr+= 1 + symptr->n_numaux;
	}

	/* add file definition symbol */
	if ((pobj->fdefsym= (SYMENT *)malloc(sizeof(SYMENT))) == NULL)
		fatal("Out of space");
	name= mkdefsym(pobj->objname, defsuffix);
	initsym(pobj->fdefsym, name);

	(void)ldclose(ldptr);
}

/* mkdefsym() creates a definition symbol name and returns a pointer to the string */
char *
mkdefsym(fnam, suffix)
char	*fnam,
	*suffix;
{
	char	*name;

	if ((name= malloc((unsigned)(strlen(fnam) + strlen(suffix) + 2))) == NULL)
		fatal("Out of space");
	(void)sprintf(name,"%s.%s",fnam, suffix);
	return(name);
}

/* initsym() initializes the symbol entry pointed to by symptr */
void
initsym(symptr, name)
SYMENT	*symptr;
char	*name;
{
	if (strlen(name) <= SYMNMLEN)
                (void)strncpy(symptr->n_name,name,SYMNMLEN);
        else {
                symptr->n_zeroes= 0L;
                symptr->n_nptr= name;
        }
        symptr->n_value=0;
        symptr->n_scnum=0;
        symptr->n_type=0;
        symptr->n_sclass= C_EXT;
	symptr->n_numaux=0;
}

/* This routine gets the absolute address of a symbol as found in the target
 * shared library.  The routine looks up the symbol's value in trgsymtab.
 */
long
getabs(name)
char	*name;
{
	long	hval;
	stab	*pstab;

	hval= hash(name,TRSIZ);
	pstab=trgsymtab[hval];
	while (pstab != NULL) {
		if (strcmp(pstab->name, name) == 0)
			return(pstab->absaddr);
		pstab= pstab->next;
	}
	fatal("Target is out of synch with supplied objects");
}



/* refchain() firms up the reference chains between archive members by placing
 * a reference to the file definition symbol of the resolving object in the
 * chainlst of the unresolved object */
void
refchain()
{
	int	i, j;
	obj	*curobj;	/* pointer to current object file structure */
	SYMENT	*symptr;	/* pointer to current undefined symbol */
	arstab	*parstab;	/* pointer into archive symbol table */
	char	*name;		/* name of current undefined symbol */
	long	hval;		/* hash value of name in arsymtab */
	long	nsyms;		/* number of symbols in current object */

	/* Scan through objects */
	for (j=0; j<numobjs; j++) {
		curobj= objects[j];
		/* scan through list of undefined symbols  and find out where
		 * each symbol is defined */
		nsyms= curobj->fhead.f_nsyms;
		for (i=0, symptr= curobj->symtab; i < nsyms; ) {
			if (symptr->n_sclass == C_EXT && symptr->n_scnum == 0 &&
				symptr->n_value == 0) {
				/* symbol is undefined */
				name= getname(symptr);
				hval= hash(name, ARSIZ);
	
				/* search archive symbol table */
				parstab= arsymtab[hval];
				while (parstab != NULL) {
					if (strcmp(name, parstab->name) == 0)
						break;
					parstab= parstab->next;
				}
				if (parstab == NULL)
					fatal("Undefined symbol %s in file %s not defined in archive",name,curobj->objname);

				/* add the file definition symbol of the object in
				 * which the undefined symbol is defined to the
				 * chainlst of the current object */
				curobj->chainlst= merge(curobj->chainlst,
							parstab->objptr->fdefsym);
			}
	
			i+= 1 + symptr->n_numaux;
			symptr+= 1 + symptr->n_numaux;
		}
	}
}


/* This function merges symptr into usymlst and returns a pointer to the merged
 * list.  */
symlst *
merge(usymlst, symptr)
symlst	*usymlst;	/* pointer to list of file definition symbols */
SYMENT	*symptr;	/* new file definition symbol to be added to usymlst */
{
	symlst	*tptr;		/* pointer into usymlst */
	char	*name1, *name2;

	/* scan through symptr */
	if (symptr != NULL) {
		name1= getname(symptr);
		tptr= usymlst;
		while (tptr != NULL) {
			name2= getname(tptr->symptr);
			if (strcmp(name1,name2) == 0) 
				break;
			tptr= tptr->next;
		}
		/* symbol is not in usymlst; add it to usymlst */
		if (tptr == NULL) {
			tptr= newsymlst(symptr, usymlst);
			usymlst= tptr;
		}
	}
	return(usymlst);
}


/* This function associates initialization code with the proper objects */
void
checkinits()
{
	initinfo	*pinit;
	obj		*objptr;
	char		*str;
	long		hval;

	for (pinit= inits; pinit; pinit=pinit->next) {
		/* first make sure there is such an object file in host */
		str= getbase(pinit->objname);
		hval= hash(str, OBSIZ);
		objptr= objlst[hval];
		while (objptr != NULL) {
			if (strcmp(objptr->objname,str) == 0)
				break;
			objptr= objptr->next;
		}
		if (objptr == NULL)
			fatal("Initialization specifications for non-supplied object file %s", str);

		if (objptr->init != NULL)
			fatal("Multiple initialization specifications for object file %s", str);

		objptr->init= pinit;
	}
}
