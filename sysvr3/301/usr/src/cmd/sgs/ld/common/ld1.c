/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ld:common/ld1.c	1.39"
#include "system.h"

#include <stdio.h>

#if TS || RT
#if PORTAR || PORT5AR
#define TABSZ	2000
#endif
#include <ar.h>
#else
#include <archive.h>
#endif

#include "structs.h"
#include "extrns.h"
#include "list.h"
#include "ldfcn.h"
#include "params.h"
#include "sgsmacros.h"
#include "reloc.h"
#if DMERT
#include "aouthdr.h"
#include "magic.h"
#endif
#include "ldmacros.h"
#include "instr.h"

#if AR32W && !UNIX
#define MAXSYMBS	120
#else
#if AR16WR
#define	MAXSYMBS	30
#else
#define MAXSYMBS	60
#endif
#endif

#if TRVEC
#if IAPX
#include "sgs.h"
#endif IAPX
#define OKMAGIC(x)	((unsigned short) x == (unsigned short) magic)
#else
#include "sgs.h"
#define OKMAGIC		ISMAGIC
#endif

#if M32
unsigned short macflags = F_BM32RST;	/* F_BM32RST remains on iff all input files
					   have it on */
#endif

pmemreg()
{

/*
 * Input the MEMLIST and REGLIST built during PASS 1
 */

#if USEREGIONS
	register REGION *rptr;
#endif
	register MEMTYPE *mptr;
	ADDRESS length;

/*
 * If no MEMORY directives are supplied, use a default, otherwise check
 * for proper usage
 */

	if (memlist.head == NULL )  {
		mptr = (MEMTYPE *) myalloc(sizeof(MEMTYPE));
		copy(mptr->mtname, "*dfltmem", 8);
#if PAGING
		if (zflag)
			memorg = max(memorg, NONULLORG);
#endif
		mptr->mtorig = memorg;
		mptr->mtlength = (MEMSIZE - memorg);
		mptr->mtattr=15;		/* all attrs set */
		listadd( l_MEM, &memlist, mptr );
		}
	else {
		mptr = (MEMTYPE *) memlist.head;
		while ( mptr->mtnext ) {
			length = mptr->mtorig + mptr->mtlength;
			if( length > mptr->mtnext->mtorig ) {
				lderror(1,0,NULL, "memory types %.8s and %.8s overlap",
					mptr->mtname, mptr->mtnext->mtname);
				free( mptr->mtnext );
				mptr->mtnext = mptr->mtnext->mtnext;
				}
			mptr = mptr->mtnext;
			}
		}

#if USEREGIONS
/*
 * If no REGIONS directives are supplied, use a default, otherwise check
 * for proper usage
 */

#if IAPX
    if( IAPX16MAGIC(magic) ) {
#endif
	if( reglist.head == NULL )  {
		if( (! iflag)  &&  (! tvflag) ) {
			/*
			 * No separate I and D space. No tv
			 */
			rptr = (REGION *) myalloc(sizeof(REGION));
			copy(rptr->rgname, "*dfltrgn", 8);
			rptr->rgorig = 0L;
			rptr->rglength = REGSIZE;
			rptr->rgvaddr = 0L;
			listadd(l_REG, &reglist, rptr);
			}
		else if( ! tvflag ) {
			/*
			 * Separate I and D space. No tv
			 */
			rptr = (REGION *) myalloc(sizeof(REGION));
			copy(rptr->rgname, "RTEXT", 8);
			rptr->rgorig = 0L;
			rptr->rglength = REGSIZE;
			rptr->rgvaddr = 0L;
			listadd(l_REG, &reglist, rptr);
			rptr = (REGION *) myalloc(sizeof(REGION));
			copy(rptr->rgname, "RDATA", 8);
			rptr->rgorig = REGSIZE + 2;
			rptr->rglength = REGSIZE - 2;
			rptr->rgvaddr = 2L;
			listadd(l_REG, &reglist, rptr);
			}
		}
	else {
		rptr = (REGION *) reglist.head;
		while( rptr->rgnext ) {
			length = rptr->rgorig + rptr->rglength;
			if( length > rptr->rgnext->rgorig ) {
				lderror(2,0,NULL,"Can't allocate space for overlap regions %.8s and %.8s",
					rptr->rgname, rptr->rgnext->rgname);
				}
			rptr = rptr->rgnext;
			}
		}
#if IAPX
    }	/* END IF_IAPX16MAGIC */
#endif
#endif

}



ploadfil( filename, filedes, ftype )
	char		*filename;
	FILE		*filedes;
	unsigned short	ftype;
{
	extern long	sgetl();
	ARCHDR	ahdr;
	SYMENT	symbuf[MAXSYMBS];
	LDFILE	ldbuf;
	register LDFILE *ldptr;
#if AR16WR
	char	fbuf[BUFSIZ];
#endif
	register int done, count;

	/*
	 * The same file can appear many times in an ifile,
	 * since the input sections from it can go
	 * into many different output sections
	 *
	 * Process a NON-ARCHIVE file exactly ONCE; but,
	 * process an ARCHIVE file each time it is given
	 */
	if ((ftype != USH_ARTYPE) && fndinfil( filename ))
		return;

	/*
 	 *	1. Make sure the file is of valid type (i.e., "good magic")
 	 *	2. Input the file and section headers
 	 *	3. Scan the symbol table, counting globals, etc.
 	 */

	ldptr = &ldbuf;
	IOPTR( ldptr ) = filedes;
	if (ftype == USH_ARTYPE)
	{
		TYPE( ldptr ) = ftype;
#if PORTAR || PORT5AR
		OFFSET( ldptr ) = 0L;
		FSEEK( ldptr, 0L, 0 );
#else
		OFFSET( ldptr ) = ARCHSZ + sizeof( ARMAG );
		if (getrec1( arhdr, ldptr ) == FAILURE)
			lderror( 2, 0, NULL, "cannot read archive header of %s",
				filename );
#endif
	}
	else
	{
		TYPE( ldptr ) = ftype;
		OFFSET( ldptr ) = 0L;
		FSEEK( ldptr, 0L, 0 );
		if (FREAD( (char *) &HEADER( ldptr ), sizeof( HEADER( ldptr )), 1, ldptr ) != 1)
			lderror( 2, 0, NULL, "cannot read file header of %s",
				filename );
	}

	if (OKMAGIC(TYPE(ldptr)))
	{
#if DMERT
		if (HEADER(ldptr).f_opthdr >= sizeof(AOUTHDR))
		{
			short o_magic;
			if (FREAD(&o_magic,sizeof(o_magic),1,ldptr) != 1)
				lderror(2,0,NULL,"fail to read magic number of pfile %s",
				filename );
			if (o_magic == (short) PLIBMAGIC)
				dmertplib = 1;
		}
#endif

		loadobjf( ldptr, filename, -1, NULL );
#if DMERT
		dmertplib = 0;
#endif
		fclose(IOPTR(ldptr));
		}

	else if( TYPE(ldptr) == USH_ARTYPE )
	{
#if PORTAR
		/* Read in the archive symbol table and search it
		 * for unresolved symbol references.  Keep passing
		 * over the symbol table, loading in archive members
		 * as you go until one complete pass is made with no
		 * additional loads of an archive member.
		 */
			
		typedef union
		{
			long l;
			char c[sizeof(long)/sizeof(char)];
		} longchar;

		longchar arsymtab[TABSZ], sym_count;
		char * arsymnmptr[TABSZ];
		char found_one;
		long  sym_ar_size;
		int strtabsz;
		char *j,*strtabptr;
		int i;
		struct ar_hdr arheader;

		/* read archive header */
		if (fseek(IOPTR(ldptr),(long)SARMAG,0) != OKFSEEK
		   || FREAD(&arheader,sizeof(struct ar_hdr),1,ldptr) != 1
		   || strncmp(arheader.ar_fmag,ARFMAG,sizeof(arheader.ar_fmag))
		   || sscanf(arheader.ar_size, "%ld", &sym_ar_size) !=1)
			lderror(2,0,NULL,
				"can't read archive header from archive %s",
				filename );

		if (arheader.ar_name[0] != '/')
			lderror(2,0,NULL,
				"archive symbol directory is missing from archive %s\nexecute 'ar ts %s' to restore archive symbol directory",
				filename, filename );
		
		/* figure out number of archive symbols */
		if (FREAD(sym_count.c,sizeof(sym_count.c),1,ldptr) != 1)
			lderror(2,0,NULL,
				"can't read archive symbol directory number of symbols from archive %s",
				filename );

		sym_count.l = sgetl( sym_count.c );
		/* check to make sure that you have a symbol table */
		if ( sym_count.l <= 0 )
			lderror(2,0,NULL,
				"archive symbol table is empty in archive %s\nexecute 'ar ts %s' to restore archive symbol table",
				filename, filename );

		if ( sym_count.l > TABSZ )
			lderror(2,0,NULL, "archive symbol directory in archive %s is too large",
				filename );

		/* read in the archive symbol directory */

		if (FREAD(arsymtab,sizeof(longchar),sym_count.l,ldptr)
			!= sym_count.l)
			lderror(2,0,NULL, "can't read archive symbol directory of archive %s",
				filename );

		/* read in the symbol names string table */

		strtabsz = sym_ar_size - (sym_count.l + 1) * sizeof(longchar);
		strtabptr = myalloc(strtabsz);
		if ((FREAD(strtabptr,strtabsz,1,ldptr) != 1) ||
			strtabptr[strtabsz - 1] != '\0')
			lderror(2,0,NULL,
				"can't read archive string table of archive %s",
				filename );

		j = strtabptr; /* ptr for walking thru string table */
		for (i=0; i<sym_count.l; i++)
		{
			if (j > (strtabptr + strtabsz - 1))
				lderror(2,0,NULL,
					"too few symbol names in string table for archive %s",
					filename );

			arsymtab[i].l = sgetl(arsymtab[i].c);
			arsymnmptr[i] = j;
			while (*++j != '\0');
			j++;
		}
		
		/* loop thru symbol table until you can't find any more
		   unresolved symbols which are satisfied by this 
		   archive */
		found_one = 1;
		while (found_one && unresolved)
		{
			struct ar_hdr memberhdr;
			SYMTAB *symptr;
			int i;

			if ((unresolved == 1) && (symptr = findsym( _END ))
					&& (symptr->sment.n_scnum == 0)
					&& (symptr->sment.n_value == 0))
				break;
			found_one = 0;
			for (i=0; i < sym_count.l; i++)
			{
				symptr = findsym(arsymnmptr[i]);
				if (symptr != NULL) /* found one ?? */
					if ( symptr->sment.n_value == 0
						&& symptr->sment.n_scnum==0)
					{
						/* seek to the archive location */
						OFFSET(ldptr) = arsymtab[i].l
							+ sizeof(struct ar_hdr);
						/* read this member's archive header */
						if (FSEEK(ldptr,-(int)(sizeof(struct ar_hdr)),BEGINNING)
							!= OKFSEEK ||
							FREAD(&memberhdr,sizeof(struct ar_hdr),1,ldptr) != 1 ||
							strncmp(memberhdr.ar_fmag,ARFMAG,
							sizeof(memberhdr.ar_fmag)))
							lderror(2,0,NULL,
								"can't read archive header of archive %s",
								filename );
						if (sscanf(memberhdr.ar_size,"%ld",&ar_size) != 1)
							lderror(2,0,NULL,
								"invalid archive size for file %s",
								filename );
						/* read this member's header */
						if (FREAD(&(HEADER(ldptr)),FILHSZ, 1, ldptr) != 1)
						lderror(2,0,NULL,
							"can't read file header of archive %s",
							filename );
						if (OKMAGIC(HEADER(ldptr).f_magic))
						{
							/* bind this member */
							loadobjf( ldptr, filename, -1, NULL );
							found_one = 1;
						}
					}
			} /* end of for */
		} /* end of while */

		/* we are done with this archive */
		fclose(IOPTR(ldptr));
#endif
#if PORT5AR
		/* Read in the archive symbol table and search it
		 * for unresolved symbol references.  Keep passing
		 * over the symbol table, loading in archive members
		 * as you go until one complete pass is made with no
		 * additional loads of an archive member.
		 */
		struct ar_sym arsymtab[TABSZ];
		struct ar_hdr arheader;
		long sym_count;
		char found_one;

		/* read archive header */
		fseek(IOPTR(ldptr),0L,0);
		if (FREAD(&arheader,sizeof(struct ar_hdr),1,ldptr) != 1)
			lderror(2,0,NULL,
				"can't read archive header from archive %s",
				filename );
		
		/* figure out number of archive symbols */
		sym_count = sgetl( arheader.ar_syms );
		/* check to make sure that you have a symbol table */
		if (sym_count <= 0)
			lderror(2,0,NULL,
				"archive symbol table is empty in archive %s\nexecute 'ar ts %s' to restore archive symbol table",
					filename, filename );

		/* read in the archive symbol table */
		if (FREAD(arsymtab,sizeof(struct ar_sym),sym_count,ldptr)
			!= sym_count)
			lderror(2,0,NULL,
				"can't read archive symbol table of archive %s",
				filename );

		found_one = 1; /* loop thru symbol table until you can't
				  find any more unresolved symbols which
				  are satisfied by this archive */
		while (found_one && unresolved)
		{
			SYMTAB *symptr;
			int i;
			short j;
			long location;
			char sym_name[sizeof(arsymtab->sym_name)+1];

			found_one = 0;
			for (i=0; i<=sym_count; i++)
			{
				strncpy(sym_name,arsymtab[i].sym_name,
					sizeof(arsymtab->sym_name));
				j = sizeof(arsymtab->sym_name);
				while (sym_name[j-1] == ' ' && j >= 1) j--;
				sym_name[j] = '\0';
				symptr = findsym(sym_name);

				if (symptr != NULL)  /* found one ?? */
					if (symptr->sment.n_value == 0 && symptr->sment.n_scnum == 0)
					{
						/* seek to the archive location */
						location = sgetl( arsymtab[i].sym_ptr );
						OFFSET(ldptr) = location +
							sizeof(struct arf_hdr);
						/* read this member's header */
						FSEEK(ldptr,0L,BEGINNING);
						if (FREAD(&(HEADER(ldptr)),
							FILHSZ,1,ldptr) != 1)
						lderror(2,0,NULL,
							"can't read file header of archive %s",
							filename );

						if (OKMAGIC(HEADER(ldptr).f_magic))
						{
							/* bind this member */
							loadobjf( ldptr, filename, -1, NULL);
							found_one = 1;
						}
					} 
			};
		};

		/* we are done with this archive */
		fclose(IOPTR(ldptr));
#endif
#if !PORTAR && !PORT5AR
		/*
		 * Search each member of the archive library
		 * to see if it will resolve any unsatisfied
		 * references
		 */
		done = count = 0;
		while( ! done )
		{
			if ( OKMAGIC(HEADER(ldptr).f_magic))
			{
				if (HEADER(ldptr).f_nsyms > MAXSYMBS ||
				    getrec3(symbuf,ldptr) == FAILURE)
				{
					if( libsrch(ldptr,NULL) )
					     loadobjf(ldptr, filename, count,NULL);
				}
				else
				{
					if( libsrch(ldptr,symbuf) )
					     loadobjf(ldptr, filename, count,symbuf);
				}
			}
			else
			{
				if (FSEEK(ldptr,HEADER(ldptr).f_symptr+
					(HEADER(ldptr).f_nsyms * SYMESZ),BEGINNING) != OKFSEEK)
					lderror(2,0,NULL,"fail to seek to the end of library %s member %d",
						filename, count);
			}
			done = myldclose(&ahdr,ldptr);
			count++;
		}
#endif
	}
	else
		procspecobj( ldptr, filename );
}


#if !PORTAR && !PORT5AR
libsrch(fdes,symbuf)
LDFILE	*fdes;
SYMENT	*symbuf;
{

/*
 * Return true if current member of archive file defines an
 *  undefined symbol
 */

	register int i;
	register SYMTAB *symp;
	SYMENT	*symbol;
	SYMENT	symbolbuf;

	if ( !OKMAGIC(HEADER(fdes).f_magic) )
		return (0);

	if (symbuf == NULL)  {
		if (FSEEK(fdes,HEADER(fdes).f_symptr,0) != OKFSEEK)
			lderror(2,0,NULL,"fail to seek to symbol table when search libraries");
		symbol = &symbolbuf;
		}

	for( i = 0; i < HEADER(fdes).f_nsyms; i++ ) {
		if (symbuf == NULL)  {
			if (FREAD(symbol,SYMESZ,1,fdes) != 1)
				lderror(2,0,NULL,"fail to read symbol table when search libraries");
			}
		else  {
			symbol = symbuf++;
			}
		if( (symbol->n_sclass == C_EXT)  &&  (symbol->n_scnum > 0) ) {
			if( (symp = findsym(PTRNAME(symbol))) != NULL ) {
				if( symp->sment.n_scnum == 0 ) {  /*undefined*/
					return(1);
					}
				}
			}
		/*
		 * skip auxilliary entries for efficiency and
		 * to avoid misinterpreting garbage sment
		 */

		if ( (int) symbol->n_numaux > 0 ) {
			if (symbuf == NULL)  {
				if (fseek(IOPTR(fdes),(long) (SYMESZ * (int) symbol->n_numaux), 1) != OKFSEEK)
					lderror(2,0,NULL,"fail to skip aux entries when search libraries");
				}
			else  {
				symbuf++;
				}
			i += (int) symbol->n_numaux;
			}
		}

	return(0);
}
#endif
/*eject*/
loadobjf(fdes,fnamp,filndx,symbuf)
LDFILE	*fdes;			/* file descriptor		    */
char	*fnamp;			/* name of the file		    */
int	filndx;			/* member number within library    */
SYMENT	*symbuf;
{

/*
 * Given a file descriptor which has been "seeked" to the beginning
 * of the file header, build the INFILE and INSECT structures
 * and process the symbol table.
 */

#if FLEXNAMES
	char *strtabptr;
	int strtabsz;
#endif
	register int s;
	register INSECT *sptr;
	register INFILE *filptr;

	filptr = (INFILE *) mycalloc(sizeof(INFILE));

	curfilnm = fnamp;
	filptr->flname = fnamp;
	filptr->flfilndx = filndx;
	filptr->flfiloff = OFFSET(fdes);
	filptr->flsymptr = HEADER(fdes).f_symptr;
	filptr->flscns = HEADER(fdes).f_nscns;
	filptr->flnsyms = HEADER(fdes).f_nsyms;

#if DMERT
	if (dmertplib)
		filptr->flplib = 1;
	else
		filptr->flplib = 0;
#endif

	/*
	 * Build an INSECT data structure for every section in the
	 * input file.  Check for two special cases:
	 *
	 *	1. Sections with illegal names - names that conflict
	 *		predefined ld section names - are not processed,
	 *		and yield an error message
	 *	2. Sections which are "padding sections" (see add_pad())
	 *		are skipped. They exist only to provide certain
	 *		physical alignment in the file
	 */

	if (FSEEK(fdes,(long) (FILHSZ + HEADER(fdes).f_opthdr), BEGINNING) != OKFSEEK)
		lderror( 2, 0, NULL, "fail to seek to section headers of file %s", fnamp);

	for( s = 1; s <= HEADER(fdes).f_nscns; s++ ) {
		sptr = (INSECT *) mycalloc(sizeof(INSECT));
		if (FREAD(&sptr->ishdr,SCNHSZ,1,fdes) != 1)
			lderror(2,0,NULL,"fail to read section headers of file %s",fnamp);

		if( ! OKSCNNAME(sptr->ishdr.s_name) ) {
			lderror(1,0,NULL, "file %s has a section name which is a reserved ld identifier: %.8s",
				fnamp, sptr->ishdr.s_name);
			filptr->flscns--;
			continue;
			}
                if ( equal(sptr->ishdr.s_name, _TV, 8) )
			++usrdeftv;

		/* COPY, LIB and INFO sections require the same treatment
			 as DSECT sections */
		if( sptr->ishdr.s_flags & (STYP_COPY | STYP_INFO | STYP_LIB))
			sptr->ishdr.s_flags |= STYP_DSECT;
		if (sptr->ishdr.s_scnptr)
			max_section = max( sptr->ishdr.s_size, max_section );

		if( sptr->ishdr.s_flags & STYP_PAD ) {
			free( sptr );
			filptr->flscns--;
			continue;
			}
		sptr->isecnum = (short) s;
#if DMERT
		if (dmertplib)  {
			sptr->ishdr.s_relptr = 0;
			sptr->ishdr.s_nreloc = 0;
			sptr->ishdr.s_lnnoptr = 0;
			sptr->ishdr.s_nlnno = 0;
			}
#endif
		listadd(l_IS,filptr,sptr);
		}
#if FLEXNAMES
	strtabsz = 0L;
	strtabptr = strtabread(IOPTR(fdes), filptr, &strtabsz);
	if (strtabptr && (strtabsz <= MAXKEEPSIZE))
		filptr->flstrings = strtabptr;
	else
		filptr->flstrings = NULL;
#endif

	if (symbuf == NULL)
		if (FSEEK(fdes,HEADER(fdes).f_symptr,BEGINNING) != OKFSEEK)
			lderror(2,0,NULL,"fail to seek to symbol table of file %s",fnamp);
#if FLEXNAMES
	filptr->flnlsyms = psymtab(IOPTR(fdes),HEADER(fdes).f_nsyms,filptr,symbuf,strtabptr);
#else
	filptr->flnlsyms = psymtab(IOPTR(fdes),HEADER(fdes).f_nsyms,filptr,symbuf);
#endif
	listadd(l_IF,&infilist,filptr);
	
#if FLEXNAMES
	if (strtabptr && (strtabsz > MAXKEEPSIZE))
		free( strtabptr );
#endif

#if M32
	/* macflags is used to calculate BM32 specific filehdr flags */
	macflags |= HEADER(fdes).f_flags & F_BM32ID ;
	/* turn F_BM32RST off if any input file has it off */
	if ((HEADER(fdes).f_flags & F_BM32RST) == 0)
		macflags &= ~F_BM32RST;
#endif

}

#if FLEXNAMES
char *
strtabread( fdes, infl, sizeptr )
	FILE *fdes;			/* file descriptor		*/
	INFILE	*infl;			/* other infomation needed about the file */
	int   *sizeptr;			/* ptr to size of string table read */
{
/* 
 * Given the file descriptor and the name of the file read the string table
 * if there is one and return a pointer to it, otherwise return null.
 */
	long symtabend;
	char *strtabptr;
	if (infl->flsymptr == 0)
		return(NULL);
	symtabend = infl->flsymptr + SYMESZ * infl->flnsyms;
	if (infl->flfiloff) {
		if (ar_size > symtabend) {
			if (fseek(fdes, symtabend + infl->flfiloff, BEGINNING) != OKFSEEK ||
				fread(sizeptr,sizeof(*sizeptr),1,fdes) != 1)
				lderror(2,0,NULL,
				"fail to read string table of file %s", infl->flname);
			if (*sizeptr > 4) {
				strtabptr = myalloc(*sizeptr);
				if (fread(&strtabptr[4], *sizeptr - 4, 1, fdes) != 1)
					lderror(2, 0, NULL,
					"fail to read string table of file %s",
					infl->flname);
				else
					return(strtabptr);
				}
			else
				return(NULL);
			}
		else 
			return(NULL);
		}
	else {
		/* not a member of an archive so it must be a
		 * regular object file or a DMERT public library
		 * (both of which are processed the same way
		 */

		if (fseek(fdes,symtabend,BEGINNING) == OKFSEEK &&
			fread(sizeptr,sizeof(*sizeptr),1,fdes) == 1) {
			/* read string table */
			if (*sizeptr > 4) {
				strtabptr = myalloc(*sizeptr);
				if (fread(&strtabptr[4], *sizeptr - 4, 1, fdes) != 1)
					lderror(2,0,NULL,
					"fail to read string table of file %s",
					infl->flname);
				else
					return(strtabptr);
				}
			else
				return(NULL);
			}
		else
			return(NULL);
		}
}
#endif
#if COMMON
/*eject*/
commspec()
{
/* process list of COMFILE structures, i.e. the files whose common symbols
 * are being put into a particular output section because of sections
 * directives which include [COMMON]
 * This routine is called prior to pf77comm
 */
 	register COMFILE *comptr;
	register SYMTAB *symp;
	register long symsize;
	register INSECT *sptr;
	register long i;
	INFILE	*fptr;
	LDFILE	*ldptr,ldbuf;
#if FLEXNAMES
	int strtabsz;
	char *strtabptr;
#endif

	ldptr = &ldbuf;
	comptr=(COMFILE *)commlist.head;
	while (comptr) {
		fptr = fndinfil(comptr->fnamptr);
		if ((IOPTR(ldptr)=fopen(comptr->fnamptr,"r")) == NULL)
			lderror(2,0,NULL,"fail to open file %s ",comptr->fnamptr);
		while (fptr){	/* loop to process all library members used in
				   this load; if file is not a library loop
				   is only executed once
				*/
			OFFSET(ldptr) = fptr->flfiloff;
#if FLEXNAMES && PORTAR
			if (fptr->flstrings)
				strtabptr = fptr->flstrings;
			else {

				/* if file is archive read archive header */
				if (fptr->flfiloff){
					struct ar_hdr memberhdr;
					if(FSEEK(ldptr,-(int)(sizeof(struct ar_hdr)), BEGINNING) != OKFSEEK ||
						FREAD(&memberhdr,sizeof(struct ar_hdr),1,ldptr)!=1 ||
						strncmp(memberhdr.ar_fmag,ARFMAG, sizeof(memberhdr.ar_fmag)))
						lderror(2,0,NULL,
							"fail to read archive header of %s",
							fptr->flname);
					if (sscanf(memberhdr.ar_size,"%ld",&ar_size) != 1)
						lderror(2,0,NULL,"invalid archive size for file %s",
							fptr->flname);
					}
				strtabsz = 0L;
				strtabptr = strtabread(IOPTR(ldptr),fptr,&strtabsz);
				}
#endif
			if (FSEEK(ldptr,0,BEGINNING) != OKFSEEK)
				lderror(2,0,NULL,"fail to seek to file header of %s",
					fptr->flname);
			if (FREAD(&(HEADER(ldptr)),FILHSZ,1,ldptr) != 1)
				lderror(2,0,NULL, "fail to read file header of %s",
					fptr->flname);
			/* create dummy common section for the input file */
			sptr = (INSECT *) mycalloc(sizeof(INSECT));
			listadd(l_IS,fptr,sptr);
			strcpy(sptr->ishdr.s_name,".common");
			sptr->ishdr.s_flags = STYP_BSS;
			sptr->isecnum = ++fptr->flscns;
			if (FSEEK(ldptr,HEADER(ldptr).f_symptr,BEGINNING) != OKFSEEK)
				lderror(2,0,NULL,"cannot seek to symbol table of file %s",comptr->fnamptr);
			for(i=0; i < HEADER(ldptr).f_nsyms; i++) {
				SYMENT symbol;
				SYMTAB *gsym;
				if (FREAD(&symbol,SYMESZ,1,ldptr) != 1)
					lderror(2,0,NULL,
					"fail to read symbol %d of %s symbol table",
					i,comptr->fnamptr);
				if (symbol.n_numaux){
					i += symbol.n_numaux;
					if (FSEEK(ldptr,HEADER(ldptr).f_symptr + 
						(i +1) * SYMESZ, BEGINNING) != OKFSEEK)
						lderror(2,0,NULL, "fail to skip aux entries of %s symbol table",
						comptr->fnamptr);
					}
				if (symbol.n_scnum == 0 && symbol.n_value != 0) {
					/* found a common symbol */
#if FLEXNAMES
					if (symbol.n_zeroes == 0L){
						if (strtabptr == NULL)
							lderror(2,0,NULL, "no string table in file %s",
							comptr->fnamptr);
						symbol.n_nptr = strtabptr + symbol.n_offset;
						}
#endif
					gsym = findsym(SYMNAME(symbol));
					if (gsym->sment.n_scnum != 0){
						if (gsym->smnewval == 0) {
							lderror(0,0,NULL,"symbol %s from file %s is initialized elsewhere",
							SYMNAME(symbol),comptr->fnamptr);
							continue;
							}
						else if (gsym->smnewval != (long) comptr->dfnscnptr)
							lderror(2,0,NULL,"symbol %s from file %s already bound to an output section",
							SYMNAME(symbol),comptr->fnamptr);
						else
							continue;
						}
					else {
						gsym->smscnptr = sptr;
						gsym->sment.n_scnum = sptr->isecnum;
						gsym->smnewval = (long) comptr->dfnscnptr;
						}
	
					/* align common symbol on optimal boundary by
					   increasing section size appropriately */
	
					symsize = gsym->sment.n_value;
					if (symsize == 2) {
						sptr->ishdr.s_size += 1;
						sptr->ishdr.s_size &= (~1);
						}
					else if (symsize > 2) {
						sptr->ishdr.s_size += COM_ALIGN;
						sptr->ishdr.s_size &= (~COM_ALIGN);
						}
					gsym->sment.n_value = sptr->ishdr.s_size;
	
					/* increase size of section */
	
					sptr->ishdr.s_size += symsize;
					}	/* end if common symbol */
				}	/* end for loop */
 			/* round up the section size to be a multiple of 4 bytes */
 			sptr->ishdr.s_size += 03L;
 			sptr->ishdr.s_size &= (~03L);

			if (fptr->flfiloff == 0)	/*if file is not archive
							   break out of while(fptr)*/
				break;
			
			/* find next .o from this archive used in this load */
			while (fptr = fptr->flnext)
				if (!strcmp(fptr->flname,comptr->fnamptr))
					break;
			}	/* end while(fptr) */

		fclose(IOPTR(ldptr));
		comptr = comptr->comnext;
		}	/* end while(comptr) */
}
pf77comm()
{
/* process F77 common data declarations, undefined (i.e., unitialized
 * common) data is placed into the bss section.  common data is indicated
 * by an undefined external variable with a non zero value.  the value
 * indicates the size of the common data region.
 */

	register SYMTAB *symp;
	register long symsize;
	register INSECT *sptr;
	register long i;
	INFILE *fptr;

	/* look thru the symbol table to see if there are some symbols
	 * which are undefined and have non-zero values.  allocate all
	 * such symbols to an imaginary bss segment defined in the 
	 * COMF77 file.
	 */

	/* initialize the dummy section to which common data is allocated */

	fptr = (INFILE *)mycalloc(sizeof(INFILE));
	fptr->flname = COMF77;
	
	sptr = (INSECT *)mycalloc(sizeof(INSECT));

	listadd(l_IS,fptr,sptr);
	listadd(l_IF,&infilist,fptr);
	
	strcpy(sptr->ishdr.s_name,".bss");
	sptr->isecnum = 1;


	i = 1;
	while ( i <= numldsyms )
	{
		symp = getsym(i++);
		if (symp->sment.n_sclass == C_EXT &&
		    symp->sment.n_scnum == N_UNDEF &&
		    symp->sment.n_value > 0) { /* we have a common symbol */
			
			/* assign this common symbol to the dummy section */

			symp->smscnptr = sptr;
			symp->sment.n_scnum = 1;

			/* align common symbol on optimal boundary */

			symsize = symp->sment.n_value;
			if (symsize == 2) {
				sptr->ishdr.s_size += 1;
				sptr->ishdr.s_size &= (~1);
				}
			else if (symsize > 2) {
				sptr->ishdr.s_size += COM_ALIGN;
				sptr->ishdr.s_size &= (~COM_ALIGN);
				}

			/* increase size of dummy bss section */

			symp->sment.n_value = sptr->ishdr.s_size;
			sptr->ishdr.s_size += symsize;
		}
	}
 	/* round up the section size to be a multiple of 4 bytes */
 	sptr->ishdr.s_size += 03L;
 	sptr->ishdr.s_size &= (~03L);
}
#endif



#if !PORTAR && !PORT5AR
/*eject*/
myldclose(arhdr,fdes)
ARCHDR *arhdr;
LDFILE *fdes;
{
	OFFSET(fdes) += arhdr->ar_size + ARCHSZ;

	if (arhdr->ar_size & 01)  {
		OFFSET(fdes) += 1;
		if (fseek(IOPTR(fdes),1L,1) != 0)  {
			fclose(IOPTR(fdes));
			return(SUCCESS);
			}
		}

	if (getrec1(arhdr,fdes) == SUCCESS)
		return(FAILURE);

	fclose(IOPTR(fdes));
	return(SUCCESS);
}
/*eject*/
getrec1(arhdr,fdes)
ARCHDR *arhdr;
LDFILE *fdes;
{

/*
 * if the file is an arch file, read in arch header and file
 * header.  Otherwise just read in file header.
 */

	if (TYPE(fdes) == USH_ARTYPE)
		if (FREAD(arhdr,ARCHSZ,1,fdes) != 1)
			return(FAILURE);

	if (FREAD(&(HEADER(fdes)),FILHSZ,1,fdes) != 1)
		return(FAILURE);

	return(SUCCESS);
}



getrec3(symbuf,fdes)
SYMENT *symbuf;
LDFILE *fdes;
{

/*
 * read in the symbol table and store it in the buffer.
 */

	int i, lim;

	if (FSEEK(fdes,HEADER(fdes).f_symptr,BEGINNING) != OKFSEEK)
		return(FAILURE);

	lim = (HEADER(fdes).f_nsyms < MAXSYMBS ? HEADER(fdes).f_nsyms : MAXSYMBS);
	for (i = 0; i < lim; i++)  {
		if (FREAD(symbuf++,SYMESZ,1,fdes) != 1)
			return(FAILURE);
		}

	return(SUCCESS);
}
#endif
