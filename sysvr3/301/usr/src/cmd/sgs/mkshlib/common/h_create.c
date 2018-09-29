/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mkshlib:common/h_create.c	1.3"

#include <stdio.h>
#include "filehdr.h"
#include "syms.h"
#include "scnhdr.h"
#include "storclass.h"
#include "reloc.h"
#include "ldfcn.h"
#include "shlib.h"
#include "hst.h"


/* mklibdef() creates the library definition file.
 * The lib. def. symbol, libdefsym, will be defined in the lib. def. 
 * file. The purpose of the lib. def. file is to carry the .lib section for
 * the shared library.  Every member of the library will get an unresolved
 * reference to the lib. def. symbol. Thus,
 * if any member of the shared library is linked, the lib. def. file will be linked
 * and the .lib section information for the shared library will be attached to the
 * user's a.out.  It is important that the library definition symbol be given
 * a unique name so that it is not accidently resolved in some file other than the
 * lib. def. file.
 * To make sure that a.outs are not built with conflicting .text or .data spaces,
 * (or any other space conflict)
 * the lib. def. file will also contain section headers which describe
 * sections that are to be loaded at the same virtual address as the .text, .data 
 * or any other loaded
 * sections of the target shared library.  In this way, space conflicts will be
 * caught by the link editor at load time. All information for the lib. def. file
 * is taken from the target shared library (trgname).
 */
void
mklibdef()
{
	LDFILE	*trgfil;	/* pointer to target shared library, trgname */
	FILE	*deffil;	/* pointer to lib. def. file, defname */

	FILHDR	fhead;		/* holds file header for deffil */
	SCNHDR	lib,		/* holds the .lib section header for deffil */
		shead;		/* holds a section header for deffil */
	SYMENT	symbol;		/* holds the current symbol in trgfil */
	long	liboffst,	/* offset of the .lib sect. data in trgfil */
		slength;	/* length of string table for deffil */
	int	len;		/* length of libdefsym */
	unsigned short	nscns;	/* will hold the number of sections in deffil */
	int	libfound=FALSE;	/* tells if .lib section was found in target */
	long	saddr;		/* segment address of current section */
	int	i;

	/* open trgfil */
	if ((trgfil=ldopen(trgname,(LDFILE *)NULL)) == NULL)
		fatal("Cannot open %s",trgname);

	/* open deffil */
	if ((deffil=fopen(defname,"w")) == NULL)
		fatal("Cannot open library definition file");

	/* leave room for file header */
	(void)fseek(deffil, FILHSZ, 1);

	/* Modify the section headers for loaded sections and write them to deffil:
	 * 	1. change the section name to .fk<segment address>
	 *	2. change the section type to STYP_NOLOAD
	 */

	/* seek to the section headers */
	fhead= HEADER(trgfil);
        (void)FSEEK(trgfil,(long)(FILHSZ+fhead.f_opthdr), BEGINNING);

	nscns= fhead.f_nscns;
	for (i=0; i<fhead.f_nscns; i++) {
		if (FREAD(&shead, SCNHSZ, 1, trgfil) != 1)
			fatal("Cannot read section header of %s",trgname);

		/* skip .lib section header but first record the section pointer;
		 * the .lib section header will be written out after the total
		 * number of sections in deffil has been determined so that the
		 * location of .lib section contents can be calculated.
		 */
		if (strncmp(shead.s_name,".lib",sizeof(shead.s_name)) == 0) {
			lib= shead;
			/* save location of .lib section contents in trgfil */
			liboffst= shead.s_scnptr;
			libfound= TRUE;
			continue;
		}

		/* Skip non-loaded and empty sections: we want to check for address
		 * space conflicts and this only aplies to non-empty loaded sections
		 */
		if ((shead.s_size == 0L) ||
			((shead.s_flags & (STYP_TEXT | STYP_DATA | STYP_BSS
						| STYP_NOLOAD)) != shead.s_flags)) {
			nscns--;
			continue;
		}

		/* Dummy section names are derived by printing .fk followed by the 
		 * section's segment address (i.e., the first four hex digits of
		 * the virtual address).
		 */
		saddr= ((shead.s_vaddr & 0xffff0000) >> 16) & 0x0000ffff;
		(void)sprintf(shead.s_name,".fk%x",saddr);
		shead.s_flags |= STYP_NOLOAD;
		shead.s_scnptr=0; /* section contents will no longer exist but
				     the section ptr. should still pt. inside file */
        	/* write the section header to deffil */
        	if (fwrite((char *)&shead, SCNHSZ, 1, deffil) != 1)
                	fatal("Cannot write section header to library definition file");
	}

	/* Now write out the .lib section header and .lib section contents. */
	if (!libfound)
		fatal("Did not find .lib section in %s",trgname);
	lib.s_scnptr= FILHSZ + nscns*SCNHSZ;

	/* write the section header to deffil */
        if (fwrite((char *)&lib, SCNHSZ, 1, deffil) != 1)
        	fatal("Cannot write section header to library definition file");

	/* Copy .lib section contents of trglib to deffil */
	(void)FSEEK(trgfil, liboffst, BEGINNING);
	if (copy(trgfil, deffil, lib.s_size) == FAILURE)
		fatal("Cannot copy from %s to %s",trgname,defname);

	/* modify fhead and write it to deffil  */
	fhead.f_nsyms= 1;	/* only has lib. def. symbol */
	fhead.f_opthdr= 0;	/* no optional header */
	fhead.f_flags &= ~F_EXEC;	/* lib. def. file is not executable */
	fhead.f_nscns= nscns;
	fhead.f_symptr= FILHSZ + nscns*SCNHSZ + lib.s_size;
	(void)fseek(deffil,0L,0);

	/* write the file header to deffil */
	if (fwrite((char *)&fhead, FILHSZ, 1, deffil) != 1)
		fatal("Cannot write file header to library definition file");

	if (fseek(deffil,0L,2) != 0)
		fatal("Cannot seek to end of library definition file");

	/* Create symbol (and possibly string) table for deffil */
	initsym(&symbol, libdefsym);
	symbol.n_scnum= N_ABS;	/* doesn't matter what section it is defined in */
	if (symbol.n_zeroes == 0L)
		symbol.n_offset= 4L;

	if (fwrite((char *)&symbol, SYMESZ, 1, deffil) != 1)
		fatal("Cannot write symbol table to library definition file");
	if (symbol.n_zeroes == 0L) {
		len= strlen(libdefsym);
		slength=  4 + len + 1;
		if ((fwrite((char *)&slength, 4, 1, deffil) != 1)
				|| (fwrite(libdefsym, len+1, 1, deffil) != 1))
			fatal("Cannot write string table to library definition file");
	}

	(void)ldclose(trgfil);
	(void)fclose(deffil);
}


/* create the modified object files */
void
creatobjs()
{
	extern int	errno;

	obj	*curobj;	/* pointer to current object file structure */
	char	*fnam,		/* output file name; if current object file has */
		*outfnam;	/* initialization code then fnam will be an
				 * an intermediate file and outfnam will be the
				 * output file name */
	FILE	*ofil;		/* pointer to current output file */
	int	i, j, k;
	long	symtaboff,	/* offset of symbol table in modified object */
		nsyms;		/* number of symbols in modified object */
	strtbl	stringtab;	/* contains all necessary info about string table */
	long	size;		/* final size of string table */
	SYMENT	*symptr;	/* pointer to current symbol */
	SYMENT	lsymbol;	/* holds library definition symbol info. */
	SYMENT	fsymbol;	/* holds current file definition symbol */
	symlst	*psyml;		/* pointer into the chainlst of the current entry */
	scnlst	*psect;		/* pointer to current section entry */
	long	totsiz;		/* total size of all non-BSS section contents */
	long	scnoffst,	/* offset of start of section contents */
		reloffst;	/* offset of start of relocation entries */
	RELOC	*relptr;	/* pointer to a relocation entry */

	/* set up library definition symbol entry */
	initsym(&lsymbol, libdefsym);

	for (i=0; i<numobjs; i++) {
		curobj= objects[i];

		/* get full pathname for modified object */
		if (curobj->init == NULL)
			fnam= makename(moddir, curobj->objname);
		else {
			fnam= makename(moddir, getbase(tempnam(moddir,"mod")));
			errno= 0; /* reset errno after call to tempnam() */
		}
		
		/* open modified object for writing */
		if ((ofil= fopen(fnam,"w")) == NULL)
			fatal("Cannot open %s",fnam);

		/* save space for file header */
		(void)fseek(ofil, FILHSZ, 0);

		if (curobj->fhead.f_nscns != 0) {
			/* write section headers */
			scnoffst= FILHSZ + curobj->fhead.f_nscns * SCNHSZ;

			for (totsiz=0, psect=curobj->sects;psect;psect=psect->next) {
				if (!(psect->shead.s_flags & STYP_BSS))
					totsiz+= psect->shead.s_size;
			}
			reloffst= scnoffst + totsiz;
			
			/* write the section headers */
			for (psect= curobj->sects; psect; psect=psect->next) {
				/* get file ptr. to section contents */
				psect->shead.s_scnptr= scnoffst;
				if (!(psect->shead.s_flags & STYP_BSS))
					scnoffst+= psect->shead.s_size;
				psect->shead.s_relptr= reloffst;
				reloffst+= RELSZ * psect->shead.s_nreloc;

				/* write section header */
				if (fwrite((char *)&(psect->shead), SCNHSZ, 1, ofil) != 1)
                	        	fatal("Cannot write section header to %s",fnam);
			}

			/* write the section contents */
			for (psect= curobj->sects; psect; psect=psect->next) {
				if (psect->shead.s_size == 0)
					continue;
                        	if (fwrite(psect->contents,(int)psect->shead.s_size,1,ofil)!=1)
                                	fatal("Cannot write section contents to %s",fnam);
			}

			/* write the relocation entries */
			for (psect= curobj->sects; psect; psect=psect->next) {
				if (psect->shead.s_nreloc == 0)
					continue;
				relptr= psect->relinfo;
				for (j=0; j<psect->shead.s_nreloc; j++, relptr++) {
					if (fwrite((char *)relptr,RELSZ,1,ofil)!=1)
						fatal("Cannot write relocation entries to %s",fnam);
				}
			}
		}

		/* write out symbol table; and regenerate and write string table */
		symtaboff= ftell(ofil);

		/* allocate new string table */
		stringtab.strsize= BUFSIZ;
		if ((stringtab.start = malloc(BUFSIZ)) == NULL)
			fatal("Out of space");
		stringtab.next= stringtab.start;

		/* print out the symbol table */
		nsyms= curobj->fhead.f_nsyms;
		for (j=0, symptr= curobj->symtab; j<nsyms; j++, symptr++) {
			writesym(symptr, &stringtab, ofil);
			/* print auxilliary entries */
			for (k=0; k < symptr->n_numaux; k++) {
				symptr++;
				j++;
				if (fwrite((char *)symptr,AUXESZ,1,ofil) != 1)
					fatal("Cannot write symbol to temp file");
			}
		}

		/* print the reference chain linking symbols */
		for (psyml= curobj->chainlst; psyml; psyml= psyml->next) {
			nsyms++;
			writesym(psyml->symptr, &stringtab, ofil);
		}

		nsyms+= 2;
		/* print the file definition symbol */
		fsymbol= *curobj->fdefsym; /* must make copy of symbol since
					    * the chainlst of other structures
					    * point to this entry */
		fsymbol.n_scnum= N_ABS;
		writesym(&fsymbol, &stringtab, ofil);

		/* finally, print library definition symbol */
		writesym(&lsymbol, &stringtab, ofil);

        	/* copy string table */
        	size= stringtab.next - stringtab.start;
        	if (size) {
                	size += 4;
                	if ((fwrite((char *)&size, 4, 1, ofil) != 1) ||
                                (fwrite(stringtab.start,(int)size-4, 1, ofil) != 1))
                        	fatal("failed to write string table to temp file");
        	}


		/* Now go back and write fileheader with correct symbol table info */
		curobj->fhead.f_nsyms= nsyms;
		curobj->fhead.f_symptr= symtaboff;
		(void)fseek(ofil, 0L, 0);
		if (fwrite((char *)&(curobj->fhead), FILHSZ, 1, ofil) != 1)
			fatal("Cannot write file header to %s",fnam);

		(void)fclose(ofil);

		/* add the .init section if one exists */
		if (curobj->init != NULL) {
			outfnam= makename(moddir, curobj->objname);
			if (execute(ldname,ldname,"-r","-o",outfnam,fnam,
						curobj->init->initname,(char *)0))
				fatal("Internal %s invocation failed",ldname);
		}
	}
}

/* This routine writes a symbol table entry to ofil and updates the string table */
void
writesym(symptr, pstrtab, ofil)
SYMENT	*symptr;	/* pointer to symbol to be printed */
strtbl	*pstrtab;	/* pointer to the string table structure */
FILE	*ofil;		/* output file */
{
	long	slength;
	long	len;
	SYMENT	symbol;

	symbol= *symptr;
	if (symbol.n_zeroes == 0) {
		/* put name in string table */
		len = strlen(symbol.n_nptr) + 1;
		slength = (pstrtab->next - pstrtab->start) + len;

		/* make sure string table is big enough */
		if (slength >= pstrtab->strsize) {
			pstrtab->strsize+=
				BUFSIZ * ((slength - pstrtab->strsize)/BUFSIZ+1);
			if ((pstrtab->start = 
				realloc(pstrtab->start,
					(unsigned)pstrtab->strsize)) == NULL)
				fatal("Insufficient memory for string table");
			pstrtab->next= (pstrtab->start + slength) - len;
		}

		(void)strcpy(pstrtab->next, symbol.n_nptr);
		symbol.n_offset= (pstrtab->next - pstrtab->start) + 4;
		pstrtab->next += len;
	}

	if (fwrite((char *)&symbol,SYMESZ,1,ofil) != 1)
		fatal("Cannot write symbol to temp file");
}


/* This routine creates hstname by archiving together all the modified objects and 
 * and defname in the proper order. 
 */
void
archive()
{
	char	*cmd;
	int	i;
	char	*fnam;
	char	*p;
	long	cmdlen,
		maxlen;

	/* set cmd */
	maxlen=strlen(moddir)+strlen(prefix)+strlen(hstname)+34+BUFSIZ;
	if ((cmd= malloc((unsigned)maxlen)) == NULL)
		fatal("Out of space");

	p= cmd;
	p+= sprintf(p,"dir=`pwd` && cd %s && ar r $dir/%s `%slorder ",
		moddir,hstname,prefix);
	cmdlen= strlen(cmd);
	for (i=0; i<numobjs; i++) {
		cmdlen+= strlen(objects[i]->objname) + 1;
		if (maxlen <= cmdlen) {
			maxlen+= BUFSIZ * ((cmdlen-maxlen)/BUFSIZ + 1);
			if ((cmd= realloc(cmd, maxlen)) == NULL)
				fatal("Out of space");
			p= (cmd + cmdlen) - (strlen(objects[i]->objname)+1);
		}
		p+= sprintf(p, "%s ", objects[i]->objname);
	}
	cmdlen+= strlen(defname) + 10;
	if (maxlen <= cmdlen) {
		maxlen+= BUFSIZ * ((cmdlen-maxlen)/BUFSIZ + 1);
		if ((cmd= realloc(cmd, maxlen)) == NULL)
			fatal("Out of space");
		p= (cmd + cmdlen) - (strlen(defname) + 10);
	}
	(void)sprintf(p,"%s | tsort`", defname);

	if (execute("/bin/sh","/bin/sh","-c",cmd,(char *)0))
		fatal("build of host archive failed");
}


/* makename() creates the pathname of the file, fnam, by prepending its
 * directory name, dir.
 */
char *
makename(dir, fnam)
char	*dir;	/* name of directory */
char	*fnam;	/* name of file */
{
	char	*name;
	if ((name= malloc((unsigned)(strlen(dir) + strlen(fnam) + 2))) == NULL)
		fatal("Out of space");
	(void)sprintf(name,"%s/%s",dir,fnam);
	return(name);
}
