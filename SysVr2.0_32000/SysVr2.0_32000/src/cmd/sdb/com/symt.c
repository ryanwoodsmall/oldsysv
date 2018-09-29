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
 	/* @(#) symt.c: 1.14 10/14/83 */

/*
 *  MACHINE DEPENDENT and OPERATING SYSTEM DEPENDENT
 */

#include "head.h"
#include "coff.h"

/* set to FIXIT when know it is not right but do not know what is */
#define FIXIT	(-2)
/* set to NEEDVAL to signal value to be filled in later */
#define NEEDVAL	(-3)

extern SCNHDR *scnhdrp;
extern FILHDR filhdr;
extern int magic;		/* ISMAGIC(magic) ==> a ".o" file */
extern MSG		NOPCS;

long rdsym(), rdlntry();
SYMENT syment;			/* rdsym() stores symbol table entry */
AUXENT auxent[MAXAUXENT];	/* rdsym() stores auxiliary entry(s) */
LINENO linent;			/* rdlntry() stores lineno entry */
static char	*strtab;	/* Character string for storing string table. */
struct	sh_name {		/* SHort_Name string for symbol table entries */
	char	name[ 9 ];
};
static struct sh_name *shstrtab; /* table of SHort_Name entries.	*/
static long	strtablen;	/* Length of string table */

int gflag = 0;
long docomm();		/* made return offset, but not using ! */

/* initialize file and procedure tables */
initfp()
{
	int compar();
	SYMENT *syp = &syment;
	AUXENT *axp = &auxent[0];
	struct proct *procpbf;
	register struct proct *procp;
	register struct filet *filep;
	struct stat stbuf;

	long soffset, noffset;
	int i, notstb;
	short class, numaux;
	unsigned short type;
	register char *p, *q;
	int nbf = 0;
	
	firstdata = MAXPOS;
	sbuf.fd = txtmap.ufd;	binit(&sbuf);
	soffset = ststart;
	filep = files = badfile = (struct filet *) sbrk(sizeof filep[0]);
	procp = procs = badproc = (struct proct *) sbrk(sizeof procp[0]);
	
			/* assuming symbol table at end of a.out ! */
	for( ; ; soffset = noffset) {	/* stop when rdsym() returns ERROR */
		if ((noffset = rdsym(soffset)) == ERROR)
			 break;
		notstb = 0;
		type = syp->n_type;
		class = syp->n_sclass;
		numaux = syp->n_numaux;
#if DEBUG > 1
		if(debugflag > 2)
			prstentry(syp);
		if(debugflag > 3 && numaux > 0)
			prprocaux(axp,syp->n_nptr);
#endif

		if (class == C_FILE)	/* C_LINE ?? */
		{

		/* note:
	 	*  start with one filet and one proct;
	 	*  when need another filet, get FILEINCR (10) more;
	 	*  and move proct's down after end of filet's.
	 	*/
			if( filep == badfile )
			{
				p = sbrk(FILEINCR*sizeof filep[0]);
				if( p < 0 )
				{
					perror("sdb(sbrk)");
					exit(4);
				}
				q = p + FILEINCR*sizeof filep[0];
				while (p > (char *) procs)
					*--q = *--p;
				badfile += FILEINCR;
				procp = (struct proct *)
				    ((char *) procp +
						FILEINCR*sizeof filep[0]);
				procs = (struct proct *)
				    ((char *) procs +
						FILEINCR*sizeof filep[0]);
				badproc = (struct proct *)
				    ((char *)badproc +
						FILEINCR*sizeof filep[0]);
			}
			/* in coff, do not know core address for .file
			 * first proc in file will set faddr
			 */
			filep->faddr = NEEDVAL;
			filep->lineflag = 0;	/* (class == C_LINE); ?? */
			filep->stf_offset = soffset;
			/*  in coff, statics are after last proc in file */
			filep->f_statics = NEEDVAL;	/* set when get .ef */
			filep->f_type = OTHERTYPE;  /* set to F77 if ".f" */
			/*  "value" for .file is
				symbol table index for next .file; or
				symbol table entry of first global (last)
			*/
			extstart = filhdr.f_symptr + syp->n_value * SYMESZ;
#if DEBUG > 1
			if(debugflag > 1 )
				fprintf(FPRT2, " extstart=%#o;\n",extstart);
#endif
			p = filep->sfilename;

			/*  change; now always just 1 auxiliary entry */
#if DEBUG > 1
			if(debugflag > 1)
				fprintf(FPRT2,
				"debug: p=%#x; &syp->n_nptr[0]=%#x;\n",
						p, &syp->n_nptr[0]);
#endif
			/*  from original; stack ovflw ?? */
			if (p < (char *)&syp - sizeof syp->n_nptr)
			{
				for(i = 0; i < AUXESZ; i++)
				{
				    if((*p++ = axp->x_file.x_fname[i])=='.'
						&& i < AUXESZ - 1)
				    switch(*p++=axp->x_file.x_fname[++i])
				    {
					case 'f': filep->f_type = F77;
						  break;
					default:  filep->f_type = OTHERTYPE;
						  break;
				    }
				}
			}

			q = filep->sfilename;
			for (p=fwp; *q; *p++ = *q++)
				;	/* initializes filework[] */
			*p = '\0';
			/*  now stat file and test when get .bf */
			nbf = 0;
			filep++;
		}

		/*  assembly language global text symbols */
		if (syp->n_scnum > 0) {
			q = scnhdrp[syp->n_scnum -1].s_name;
			if((class == C_EXT || class == C_STAT) &&
			    numaux == 0 && eqstr(q,_TEXT) &&
			    /* kludge for prof.h MARK macro */
			    (syp->n_nptr[1] != '.' || syp->n_nptr[0] != 'M'))
				notstb++;
		}

		if (ISFCN(type) || notstb)
		{
			if (procp == badproc)
			{
				if (sbrk(PROCINCR*sizeof procp[0]) < 0)
				{
					perror("sdb(sbrk)");
					exit(4);
				}
				badproc += PROCINCR;
			}
			procp->pname = syp->n_nptr;
			procp->paddress = syp->n_value;
			if((filep-1)->faddr == NEEDVAL)
				(filep-1)->faddr = procp->paddress;
			procp->st_offset = soffset;
	/* old ?	procp->sfptr = filep == badfile ? filep : filep -1; */
			procp->sfptr = notstb ? badfile : filep -1;
			procp->lineno = 0;	/*  will set if get .bf */
			procp->entrypt = 0;	/*  alternate entry ?? */
			procp->notstab = notstb;
			if(numaux < 1 && notstb == 0) {
				error("Proc Aux entry missing;\n");
			}
			procpbf = procp++;
		}

		if ( eqstr( syp->n_nptr, ".bf" ) )
		{
			gflag++;	/*  "cc -g ..." ==> .bf entries */
			/* first proc in file and warnings not disabled */
			if ( nbf++ == 0 && !Wflag )
			{
				if ( ( q = findfl( filework ) ) == NULL ||
				    ( stat( q, &stbuf ) == -1 ) )
				{
				    fprintf( FPRT1, "Warning: `%s' not found\n",
						filework );
				}
				else if ( stbuf.st_mtime > symtime )
					fprintf( FPRT1,
					      "Warning: `%s' newer than `%s'\n",
						q, symfil );
				if( q )
				{
					strcpy( filework, q );
				}
			}
			procpbf->lineno = axp->x_sym.x_misc.x_lnsz.x_lnno;
			procpbf->sfptr->f_statics = noffset;
#if DEBUG > 1
			if( debugflag > 1 )
				fprintf(FPRT2, "%s() in %.14s line %d;\n",
				     procpbf->pname,
				     procpbf->sfptr->sfilename,procpbf->lineno);
#endif
			curstmt.lnno = curstmt.stno = 0;
			procpbf->notstab = 0;
		}

		if (eqstr(syp->n_nptr, ".ef")) {	/* only 3B needs */
			procpbf->ef_line = axp->x_sym.x_misc.x_lnsz.x_lnno +
						procpbf->lineno -1;
#if DEBUG > 1
			if(debugflag)
				fprintf(FPRT2, ".ef: lnno=%d;\n",
						procpbf->ef_line);
#endif
		}

		if (class == C_EXT && syp->n_scnum > 0) {
/*   ?			if (!extstart)	extstart = soffset;*/
			q = scnhdrp[syp->n_scnum -1].s_name;
#if DEBUG > 1
			if(debugflag > 1)
				fprintf(FPRT2, "scnhdr name=%s; scnum=%d;\n",
						q, syp->n_scnum);
#endif
			if((eqstr(q,_DATA) || eqstr(q,_BSS)) &&	/* ?? */
					syp->n_value < firstdata) {
				firstdata = syp->n_value;
#if DEBUG > 1
				if(debugflag > 1)
					fprintf(FPRT2, " firstdata=%#lx\n;",
								firstdata);
#endif
			}
		}
	}
	qsort((char *)procs, (int)(procp-procs), sizeof procs[0], compar);
	badproc->st_offset = soffset;
	badproc->sfptr = procp->sfptr = badfile;
	badproc->pname = procp->pname = "";
	badfile->sfilename[0] = filep->sfilename[0] = '\0';

	if (!gflag) {
		if(filhdr.f_nsyms == 0)
			fprintf(FPRT1, "Warning: '%s` has no symbols\n",symfil);
		else
			fprintf(FPRT1, "Warning: `%s' not compiled with -g\n",
							symfil);
	}

	/* initialize adsubn, adsubc, and adargs to the addresses
	 * of dbsubn, dbsubc, and dbargs respectively (use ERROR
	 * if not found).
	 */
	adargs = extlookup("__dbargs", extstart) == ERROR ? ERROR :
			sl_addr;
	adsubc = extaddr("__dbsubc");
	adsubn = extaddr("__dbsubn");
	setcur(1);
}

/* returns current procedure from state (curfile, curstmt) */
struct proct *
curproc()
{
	register ADDR addr;
	register struct proct *procp;

	addr = getaddr( "", curstmt );
#if DEBUG > 1
	if(debugflag)
	{
		fprintf(FPRT2, "curproc(): addr=%#lx\n", addr);
	}
#endif
	if (addr == ERROR)
	{
		return(badproc);
	}
	procp = adrtoprocp( addr );
#if DEBUG > 1
	if(debugflag > 1)
	{
		fprintf(FPRT2, "	procp->pname=%s;\n", procp->pname);
	}
#endif
	return( procp );

}

/* returns procedure s, uses curproc() if s == NULL */

struct proct *
findproc( s )
char *s;
{
	register struct proct *p, *altproc;
	
#if DEBUG > 1
	if(debugflag)
		fprintf(FPRT2, "findproc(s=%s);\n", s);
#endif
	if( s[ 0 ] == '\0' )
	{
		return( curproc() );
	}
	altproc = badproc;
	
	for( p = procs; p->pname[ 0 ]; p++ )
	{
		if( eqpatu( s, p->pname ) )
		{
#if DEBUG > 1
		if(debugflag > 1)
			fprintf(FPRT2, " : pname=%s; notstab=%d;\n",
					p->pname, p->notstab);
#endif
			if (p->notstab)
			{
				altproc = p;
			}
			else
			{
				return(p);
			}
		}
	}
#if DEBUG > 1
	if(debugflag)
	{
		fprintf(FPRT2, " : altproc=%s\n",
			altproc == badproc ? "badproc" : altproc->pname);
	}
#endif
	return( altproc );
}

/* returns filet containing filename s */
struct filet *
findfile( s )
char *s;
{
	register struct filet *f;
	char *strrchr();
	register char *p = strrchr(s,'/');

	if(p)
	{
		s = ++p;
	}

#if DEBUG > 1
	if( debugflag )
	{
		fprintf( FPRT2, "findfile( s = %s )\n", s );
	}
#endif
#ifdef ns32000
	/* this isn't specific just to the ns32000 !!?	*/
	for ( f = files; f < badfile; f++ )
#else
	for ( f = files; f->sfilename[ 0 ]; f++ )
#endif
	{
#if DEBUG > 1
		if( debugflag > 3 )
		{
			fprintf( FPRT2, "  : f->sfilename = %.14s;\n",
				f->sfilename );
		}
#endif
		if ( strcmp( f->sfilename, s ) == 0 )
		{ 
#if DEBUG > 1
			if( debugflag > 1 )
			{
				fprintf( FPRT2, " : sfilename = %.14s;\n",
					f->sfilename );
				fprintf( FPRT2, "   lineflag = %d;\n",
					f->lineflag );
				fprintf( FPRT2, "   faddr = %#lx;\n",
					f->faddr );
				fprintf( FPRT2, "   stf_off = %#lx;\n",
					f->stf_offset );
			}
#endif
			for( ; f->lineflag; f-- )
				;

			if( f < files ) error( "Bad file array" );
			return( f );
		}
	}
	return( f );
}

/* offstoproc no longer needed, as slooknext is now called with
**	the parameter procp.  It is thus commented out here.
**-----------------------------------------------------------------
**
** offset in a.out to process (i.e. which function offset is in)
**  added offstoproc() to implement slooknext()
*/

/* ************** offstoproc commented out
/*struct proct *
/*offstoproc(offs)
/*long offs; {
/*	register struct proct *pp, *pps = badproc;
/*	register long off1 = 0;
/*	
/*	if(offs <= 0)
/*		return(badproc);
/*	/* note: the proct's are sorted by address in core, not by offset */
/*	for (pp = procs; pp->pname[0]; pp++) {
/*#if DEBUG > 1
/*		if(debugflag > 1) fprintf(FPRT2,
/*			"offstoproc(%#o): pp->stf_offset=%#o; off1=%#o;\n",
/*					offs, pp->st_offset, off1);
/*#endif
/*		if (off1 <= pp->st_offset && pp->st_offset <= offs) {
/*			off1 = pp->st_offset;
/*			pps = pp;
/*		}
/*	}
/*#if DEBUG > 1
/*	if(debugflag > 1) fprintf(FPRT2,
/*			"offstoproc(%#lo): pname=%s;	st_offset=%#lo;\n",
/*					offs, pps->pname, pps->st_offset);
/*#endif
/*	return(pps);
/*}
 **************** end of comment (offstoproc) */

/* slookup() is no longer called.  The usual calling sequence to find
 * local symbols should be:
 *	adtroprocp - get pointer into proc table, given an instruction address
 *	adtostoffset - get offset in a.out of .eb or .ef up to which to search
 *		for symbol.  This also sets "level" to the block level
 *		corresponding to the instruction address, and sets up
 *		the blklist linked list structure to tell which blocks
 *		are active for each level of nesting.
 *	slooknext - starts looking in the symbol table at the first entry
 *		for the given proc, and searches up to offset given by
 *		adtostoffset (or previous call to slooknext), looking for
 *		last valid (with respect to scoping rules) occurence of
 *		the specified symbol.
 *
 * Old comments follow, which no longer apply, but are left to give an
 * idea of the original design, especially for common symbols.
 *
 * slookup():
 * Looks up variable matching pat starting at poffset in a.out,
 *  searching backwards (on original Vax version);
 * Ignoring nested blocks to beginning of procedure.
 * in COFF, search foward from preceding procedure and remember last
 *	poffset is offset in a.out to ".eb" or ".ef"
 *	Not using slookup(); in prvar.c outvar(), slookinit() and slooknext()
 * Returns its offset and symbol table entries decoded in sl_*
 *
 * If comblk == "*" then match both within and outside common blocks,
 * if comblk == ""  then match only outside common blocks,
 *                  else match only within comblk.
 */
 
/********** slookup and slookinit no longer compiled
long
slookup(pat, poffset, stelt)
long poffset; char *pat; {
	slookinit();
	slooknext(pat, poffset, stelt, "*");
}

static int clevel, fnameflag, comfound, incomm;

slookinit() {
	clevel = level = fnameflag = comfound = incomm = 0;
}
********** end of commented section */

long tagoff = -1;	/* structure tag offset; prvar.c outvar() uses */

struct blklist {
	int blknum;		/* block number corresponding to the nesting
				 * level implicit in the position of the 
				 * structure element within the block list
				 */
	struct blklist *next;	/* forward pointer */
	struct blklist *prev;	/* backward pointer */
};

/* zeroth level element in list of block numbers for local varaible search */
static struct blklist zeroblk = {0, (struct blklist *)0, (struct blklist *)0};
static int level;	/* block level for given address; adtostoffset sets */
static long offstatics;	/* offset for statics; slooknext set for staticlookup*/

long
slooknext(pat, poffset, procp)
long poffset;
char *pat;
struct proct *procp;
{
	register int i;
	register long offset, found;
	register int class;
	register int curlevel = 0;	/* block level of current symbol */
	register int curblk = 0;	/* block number of current symbol */
	register struct blklist *curptr
			= &zeroblk;	/* pointer to blklist entry
					 * for current level */
	long noffset;
	SYMENT *syp = &syment;
	AUXENT *axp = &auxent[0];
	
#if DEBUG > 1
	if (debug || debugflag)
		fprintf(FPRT2, "slooknext(pat=%s, poffset=%ld(%#lo))\n",
					pat, poffset, poffset);
#endif

	/* procp now passed; don't use offstoproc
	 * procp = offstoproc(poffset);
	 */

	offset = procp->st_offset;

	/* use to search backwards here;
		now get preceeding function and
		search foward remembering the last
	 */
	found = ERROR;
	while( (noffset = rdsym(offset)) != ERROR) {
		class = syp->n_sclass;
#if DEBUG > 1
		if(debugflag > 1) {
			fprintf(FPRT2, "	offset=%#lo;	class=%d(%#o);",
						offset, class,class);
			fprintf( FPRT2, "	n_name=%s;\n", syp->n_nptr );
		}
#endif
		if (offset >= poffset)
			return(found);
		if (class == C_FCN && eqstr(syp->n_nptr , ".ef"))
			return(found);

		if(class == C_BLOCK) {
			if( eqstr( syp->n_nptr , ".bb" ) )
			{
				curlevel++;
				curblk++;
				if ( curptr = curptr->next ) /* advance level */
					;
				else
				{
					return(found);  /* adtostoffset error */
				}
			}
			else if( eqstr( syp->n_nptr, ".eb" ) )
			{
				curlevel--;
				if ( curptr = curptr->prev ) /* back up */
					;
				else
				{
					return( found );   /* level SGS error */
				}
			}
			offset = noffset;
			continue;
		}

		/* if the symbol is in the correct block number for the
		 * nesting level in which the symbol is declared, match
		 */
		if ((curlevel <= level && curblk == curptr->blknum)
			&& eqpatu(pat, syp->n_nptr)
			&& syp->n_nptr[0] && class)
	/* 		&& (eqstr("*", comblk) ||
			      (comblk[0] == '\0' && incomm == 0) ||
			      comfound)
			&& (stelt == (ISTELT(class)) ))
	 */
			{
			sl_size = 0;
			sl_class = syp->n_sclass;
			sl_addr = syp->n_value;
			sl_procp = procp;	/* for stackreg to know */
			strcpy( sl_name, syp->n_nptr );
	/* 	if (clevel != 0)
				docomm(offset);
	*/

			found = offset;

			/* kludge for common storage.  Must look for
			 * global (external) symbol that actually
			 * allocates storage, since that has correct
			 * value (address).  However, this local
			 * symbol has correct tag index.
			 */
			if ((sl_type = syp->n_type) == (ushort)-1) {
				sl_type = T_STRUCT;
				if (syp->n_numaux <=0) return(ERROR);
				tagoff = ststart +
				    axp->x_sym.x_tagndx*SYMESZ;
				if ((sl_addr = rdcom(sl_name)) <= 0)
				    return(ERROR);
			}
			else {
			/* fill in sl_size, sl_dimen[], tagoff.
			 * arrays and structs are complex, all else
			 * simply get sl_dimen[0] = 0, and remember old tagoff.
			 */
			arystrdata();
			}
#if DEBUG > 1
			if (debugflag > 1)
				fprintf(FPRT2, "level=%d; found=%#lo;\n",
						level, found);
#endif
		}
		offset = noffset;
	}
	return(found);
}

/* globallookup and staticlookup (which hasn't been copied into this
 *	verstion) have been superceeded by sglookup and strlookup.
 *	The former matches a pattern in the static symbols or global
 *	symbols, depending on the offset it is given into the symbol
 *	table.  It searches from the given offset until the pattern is
 *	matched, a C_FILE symbol is reached, the beginning of the
 *	global symbols is reached (for static variables), or the end
 *	of the symbol table is reached (for globals).  Strlookup
 *	is given an offset into the symbol table pointing to a structure
 *	tag, or one of the members defined under that tag.  It searches
 *	forward until the pattern is matched, .eos is reached, or
 *	the end of the symbol table is reached (an SGS error).
 *	The offset to the structure tag is found by sglookup
 *	(if the tag is applied to a static or global variable), or
 *	slooknext (if the defined structure is a local).
 *	Thus sglookup should be called to locate non-structure symbols,
 *	and strlookup will locate the desired element(s).
 */
/* 
 * Look up global variable matching 'pat` (starting at filestart).
 * Return its symbol table entries decoded in sl_*
 * globallookup() -- changes for auxent and more; now calls sglookup()
 *	Changes to handle members of structures, unions.
	Returns offset of symbol matching pattern, or ERROR (-1) for no match.
 *	Split into two parts, so staticlookup can use same code.
 */
/* commented out globallookup ***************
long
globallookup(pat, filestart, stelt)
char *pat; long filestart; {
/*	register int i, class, clevel;
/*	SYMENT *syp = &syment;
/*	AUXENT *axp = &auxent[0];
/*	register long offset;
/*	static long nextoff = 0;	/* 3B: next start for externals */
/*	
/*#if DEBUG
/*	if (debug)
/*		fprintf(FPRT2, "globallookup(%s,%ld(%#lo),%d)\n",
/*				pat, filestart, filestart, stelt);
/*#endif
/*	/* on 3B, globals at end of symbol table; do not know in which file */
/*	/*	except external structure member definitions !! */
/*	if (stelt) {	/* a structure member */
/*		if (ststart < filestart && filestart < extstart)
/*			offset = filestart;	/* use prev return value */
/*		else				/* abs number structures */
/*			/* if 2 struct's have same member name,  will get 1st */
/*			offset = ststart;
/*	}
/*	else {		/* a real external */
/*		if (filestart == extstart)
/*			offset = extstart;
/*		else
/*			offset = nextoff > 0 ? nextoff : extstart;
/*	}
/*	clevel = 0;
/*	while( (offset = rdsym(offset)) != ERROR) {
/*		class = syp->n_sclass;
/*#if DEBUG > 1
/*		if(debugflag > 1) {
/*			prstentry(syp);
/*			if(syp->n_numaux >= 1)
/*				prprocaux(axp,syp->n_name);
/*		}
/*#endif
/*		if(class == C_BLOCK) {	/* Vax Common --> 3B Blocks ?? */
/*			if(eqstr(syp->n_name, ".bb"))
/*				clevel++;
/*			else if(eqstr(syp->n_name, ".eb"))
/*				clevel--;
/*			else
/*				fprintf(FPRT1, "Warn: C_BLOCK error;\n");
/*		}
/*		else if (eqpatu(pat, syp->n_name) && syp->n_name[0]) {
/*			sl_class = syp->n_sclass;
/*			if (stelt != ISTELT(syp->n_sclass))
/*				continue;
/*	/* this routine only to get .data and .bss symbols ??
/*					structure members have scnum = -1 !! */
/*			if(!stelt && (syp->n_scnum <= 0 ||
/*				eqstr(scnhdrp[syp->n_scnum -1].s_name, _TEXT)) )
/*				continue;
/*	
/* /* ??		sl_size = 0; */
/*			sl_type = syp->n_type;
/*			sl_addr = syp->n_value;
/*			sl_scnum = syp->n_scnum;
/*			for (i=0; i<SYMNMLEN; i++)
/*				sl_name[i] = syp->n_name[i];
/*			if (clevel != 0)
/*				docomm(offset);
/*			break;
/*		}
/*	}
/*
/*	if(offset == ERROR)
/*		return(ERROR);
/*
/*#if DEBUG > 1
/*	if(debugflag)
/*		fprintf(FPRT2, "globallook: sl_size=%d, numaux=%d;\n",
/*				sl_size, syp->n_numaux);
/*#endif
/*	sl_size = (syp->n_numaux > 0 ? axp->x_sym.x_misc.x_lnsz.x_size : 0);
/*	if (sl_class == C_EXT && (clevel == 0)) {
/*			if (eqpatu(sl_name, syp->n_name))
/*				sl_addr = syp->n_value;
/*	}
/*	nextoff = offset;
/*	i = BTYPE(syp->n_type);
/*	/* for external structure, elements not at end with externs */
/*	if (i == T_STRUCT || i == T_UNION || i == T_ENUM)
/*		return(ststart + axp->x_sym.x_tagndx * SYMESZ);
/*	return(offset);		/*  > extstart ==> extern for next call */
/*}
*/

/*  sglookup(): static/global symbol table lookup
 *	was just part of globallookup();
 *	Now (with strlookup) replaces globallookup and staticlookup.
 *
 *	Returns offset of last pattern matched (like slooknext),
 *	ERROR (-1) for no match.
 */

sglookup(pat, offset)
char *pat;
register long offset;
{
	register int i, class;
	SYMENT *syp = &syment;
	AUXENT *axp = &auxent[0];

	while( (offset = rdsym(offset)) != ERROR) {
		class = syp->n_sclass;
#if DEBUG > 1
		if (debugflag > 1) {
			prstentry(syp);
			if (syp->n_numaux > 0)
			{
				prprocaux( axp, syp->n_nptr );
			}
		}
#endif

		if (class == C_FILE || offset == extstart)	/* statics */
			return(ERROR);

		if( eqpatu( pat, syp->n_nptr ) && syp->n_nptr[ 0 ] )
		{
			/* don't want structure members */
			if (ISTELT(class)) continue;

			/* don't want type 0 symbols (labels, .ef, etc.) */
			if ( ( sl_type = syp->n_type ) < 0 )
			{
				continue;
			}
			if( sl_type == 0 && ( syp->n_scnum != 2 &&
			      syp->n_scnum != 3 ) )
			{
				continue;
			}
			if( sl_type == 0 && !args[0] && cmd != '=' )
			{
				printf("Assuming %s is int.\n", syp->n_nptr);
			}

			/*  this routine only to get .data and .bss
			 * 	symbols??
			 * 	(structure members and tags have negative
			 *	 section numbers)
			 */
			if ((sl_scnum = syp->n_scnum) <= 0 ||
				eqstr(scnhdrp[syp->n_scnum -1].s_name, _TEXT))
					continue;

			sl_class = class;
			sl_addr = syp->n_value;
			strcpy( sl_name, syp->n_nptr );
			break;
		}
	}

	if (offset == ERROR) return(ERROR);

	/* the following code was (?) to find globals in a separate
	 *	area, once the symbol was located in the collection of
	/*	all symbols (in the original version) */
	/*
	/* if (sl_class == C_EXT) if (eqpatu(sl_name, syp->name))
	/*				sl_addr = syp->n_value;
	 */

	/* fill in sl_size, sl_dimen[], tagoff.
	 * arrays and structs are complex, all else
	 * simply get sl_dimen[0] = 0, and remember old tagoff.
	 */
	arystrdata();

	return(offset);
}

/* strlookup - given an offset into a.out which points to a structure
 *		tag or a member following it, this function reads
 *		the entry.  If it is a tag, it skips that entry.  It then
 *		proceeds to read through the structure until the pattern
 *		is matched, or a .eos is reached (or the end of the symbol
 *		table, in case of an SGS error).
 *		The offset to the next stucture entry is returned,
 *		unless .eos (or end of table) is reached, whence ERROR
 *		(-1) is returned.
 *
 *		The offset provided should come from a prior call to
 *		strlookup, or sglookup, or slooknext.  The latter
 *		two functions locate an actual structure, and return
 *		the tag offset in the global variable "tagoff".
 */
strlookup(pat, offset)
char *pat;
register long offset;
{
	SYMENT *syp = &syment;
	AUXENT *axp = &auxent[0];
	register int i;

	if ((offset = rdsym(offset)) == ERROR) return(ERROR);

	/* check if entry is structure tag */
	if (syp->n_scnum == N_DEBUG) {
		/* sl_size = axp->x_sym.x_misc.x_lnsz.x_size; */
		if ((offset = rdsym(offset)) == ERROR) return(ERROR);
	}

	do {
		/* check if .end of structure was reached */
		if ( eqstr( ".eos", syp->n_nptr ) )
		{
			return(ERROR);
		}

		/* pattern match */
		if ( eqpatu( pat, syp->n_nptr ) && syp->n_nptr[ 0 ] )
		{
			sl_type = syp->n_type;
			sl_addr = syp->n_value;	/* relative to structure */
			sl_scnum = syp->n_scnum;
			sl_class = syp->n_sclass;
			strcpy( sl_name, syp->n_nptr );
			if ( sl_class == C_FIELD )
			     sl_size = axp->x_sym.x_misc.x_lnsz.x_size;
			else sl_size = 0;
			break;
		}
	} while ((offset = rdsym(offset)) != ERROR);

	if (offset == ERROR) return(ERROR);

	/* fill in sl_size, sl_dimen[], tagoff.
	 * arrays and structs are complex, all else
	* simply get sl_dimen[0] = 0; sl_size remains unchanged
	 * (in case it has the size of a bit field, otherwise
	 * it will not even be looked at, so its value need not be cleared)
	 * and tagoff remains unchanged (pointing to this structure template,
	 * for use in number.member request later).
	 */
	arystrdata();
	return(offset);
}

/* arystrdata() - sets sl_dimen[] (in case the symbol is an array, OR
 *		some level of pointers to an array [in which case ISARY fails]).
 *
 *		In the case of a structure, or array of structures,
 *		sl_size is set to the structure size.  Otherwise,
 *		sl_size is unchanged.  Tagoff is set to the offset
 *		of the structure tag or the offset of the first structure
 *		element, or unchanged if the symbol is not a structure.
 *
 *		This function assumes that the symbol table entry buffer
 *		and the auxiliary entry buffer are current, and that sl_type
 *		has the type of the associated symbol.  Arystrdata must
 *		be called after all data from the symbol table entry has
 *		been saved, as it calls rdsym(), which wipes out
 *		the symbol table buffers.
 */
arystrdata()
{
	SYMENT *syp = &syment;
	AUXENT *axp = &auxent[0];
	register int i;

	/* if the number of auxiliary entries is zero, this is not
	 * a structure or array
	 */
	if (syp->n_numaux <= 0)
	{
		sl_dimen[0] = 0;
		return;
	}

	/* copy dimension information, even if this is not an array
	 * (for pointers to pointers to arrays, etc. it is not worth
	 * the effort to determine whether they are arrays)
	 */
	for (i = 0; i < DIMNUM; i++)
		if ((sl_dimen[i] = axp->x_sym.x_fcnary.x_ary.x_dimen[i]) == 0)
			break;

#if DEBUG > 1
	if (debugflag > 1)
	{
		int numdim = i;
		fprintf(FPRT2, "arystrdata: numdim = %d\n", numdim);
		for (i = 0; i < numdim; i++)
			fprintf(FPRT2, "sl_dimen[%d]=%d\t", i, sl_dimen[i]);
		fprintf(FPRT2, "\n");
	}
#endif

	/* for arrays of structures, or just structures, the structure
	 * tag has the size of the structure.  In the case of a pointer
	 * to an array of structures, the size of the symbol is
	 * sizeof(pointer), and it is not easy to tell that this is
	 * an array, meaning that the size of the structure must
	 * be obtained from the structure tag.  Thus, in all cases
	 * the structure tag is read to get the size of the structure,
	 * even though in the case of a struct, or pointer to struct,
	 * the size of the symbol is the size of the struct.
	 */
	i = BTYPE(sl_type);
	if (ISTRTYP(i))
	{
		tagoff = rdsym(ststart + axp->x_sym.x_tagndx * SYMESZ);
		if (tagoff <= 0) 	/* error */
		{
			sl_size = 0;
		}
		else {
			sl_size = axp->x_sym.x_misc.x_lnsz.x_size;
		}
#if DEBUG > 1
		if (debugflag > 0)
		{
		   fprintf(FPRT2, "arystrdata: sl_size=%d, tagoff=%#xn",
			sl_size, tagoff);
		}
#endif
	}
	else ;

}

/* rdcom(name) - read global storage, looking for common name that
 *		matches "name"
 */
long
rdcom(name)
char *name;
{
	register int i;
	register long offset;
	SYMENT *syp = &syment;
	register char *ptr;

	offset = extstart;
	while((offset = rdsym(offset)) != ERROR) {

#if DEBUG > 1
		if (debugflag > 1) prstentry(syp);
#endif

		if ((ptr = syp->n_nptr)[0] != '_')
		{
			continue;
		}
		i = 0;

		/* match as many as seven extra characters */
		do
		{
			if( !( *++ptr ) )
			{
				break;	/* no match */
			}
			if ( *ptr != name[ i ] )
			{
				break;
			}
		} while (++i < 7);
		
		/*
		 * If FLEXNAMES are allowed this section makes sure that the
		 * last one(s) is a _\0.  Else if fewer than 7 characters
		 * matched, remaining one(s) must be _\0 to match
		 */
		switch(i) {
		default:
			if (*ptr++ != '_')
			{
				continue;
			}
			if (*ptr != '\0')
			{
				continue;
			}
			if (name[i])
			{
				continue;
			}
			break;
#ifndef FLEXNAMES
		case 6:
			if (*ptr != '_' )
			{
				continue;
			}
			if( name[ i ] )
			{
				continue;
			}
			break;
		case 7:
			break;
#endif
		}

		/* have found correct global for common, so set value */
		return(syp->n_value);

	}
	return((long)ERROR);
}

/* gets scale factor for indexing, given a dereferenced ype,
 * the number of "true" dimensions (i.e. not pointers) already
 * dereferenced, and the type of file (F77 arrays in column order,
 * others in row order)
 */
int
scaleindx(type, subflag, filetyp)
register short type;
register int subflag;
int filetyp;
{
	register int scalefctr;
	if (filetyp == F77) {
		/* for F77 arguments, get size of type pointed to */
		scalefctr = ISPTR(type) ? typetosize(DECREF(type),sl_size) :
			typetosize(type, sl_size);
		/* multiply by all previous dimensions */
		while (--subflag >= 0)
			scalefctr *= sl_dimen[subflag];
	}
	else {
		/* subflag points to dimension corresponding to current
		 * index.  It will be necessary to increment when the
		 * next array dimension is found.
		 */
		if (!ISARY(type)) {	/* allow pointer arithmetic */
			subflag--;	/* subflag must point to
					   previous "true" dimension */
		}
		scalefctr = 1;

		/* while next type still derived */
		while ((type = DECREF(type)) & ~N_BTMASK) {
		    if (!ISARY(type)) break;
		    scalefctr *= sl_dimen[++subflag];
		}
		scalefctr *= typetosize(type, sl_size);
	}
	return(scalefctr);
}


/* getbnd - gets bounds for the subflag'th true dimension, i.e.
 *		subflag is the index into the list of dimensions
 *		(it is incremented in outvar each time a real dimension
 *		is indexed)
 */
int
getbnd(lbnd, ubnd, subflag)
int *lbnd;
int *ubnd;
int subflag;
{
	AUXENT *axp = &auxent[0];
	
	/* lbnd will be 0 until symbol table has true lower bound;
	 * outvar, where it calls getbnd, will adjust lbnd and ubnd
	 * by adding one for F77 arrays.
	 */
	*lbnd = 0;
	*ubnd = *lbnd + sl_dimen[subflag] - 1;
}
	
/* core address to procedure (pointer to proc array) */
struct proct *
adrtoprocp(addr) 
ADDR addr; {
	register struct proct *procp, *lastproc;
	register ADDR ladr;
	static struct {		/* save previous arg and return value */
		ADDR sadr;
		struct proct *sproc;
	} savelast = { (ADDR)-1, (struct proct *)0 };

#if DEBUG > 1
	if(debugflag)
		fprintf(FPRT2, "adrtoprocp(addr=%#lx);\n", addr);
#endif
	/* gets called a lot with same address */
	if(addr == savelast.sadr) {
#if DEBUG > 1
		if(debugflag)
			fprintf(FPRT2, "  : SAME proc;\n");
#endif
		return(savelast.sproc);
	}
	else
		savelast.sadr = addr;
	lastproc = badproc;
	for (procp=procs; procp->pname[0]; procp++) {
#if DEBUG > 1
		if(debugflag > 1) {
			fprintf(FPRT2, "	pname=%s;	",procp->pname);
			fprintf(FPRT2, "	paddress=%#lx;	",procp->paddress);
			fprintf(FPRT2, "	entrypt=%d;\n", procp->entrypt);
		}
#endif
		if (procp->paddress > addr)
			return(savelast.sproc = lastproc);
		if (procp->entrypt == 0)
			lastproc = procp;
	}
#if DEBUG > 1
	if(debugflag)
		fprintf(FPRT2, "	: pname=%s;\n",
			lastproc == badproc ? "'badproc`" : lastproc->pname);
#endif
	/*  after last proc ? -- so prisploc() will test for external */
	if(lastproc != badproc && rdsym(lastproc->st_offset) != ERROR) {
		ladr = lastproc->paddress + auxent[0].x_sym.x_misc.x_fsize;
#if DEBUG > 1
		if(debugflag)
			fprintf(FPRT2, "	: ladr=%#x, st_off=%#x;\n",
				ladr, lastproc->st_offset);
#endif
		if(addr >= ladr)
			return(savelast.sproc = badproc);
	}
	return (savelast.sproc = lastproc);
}
 
/* core address to file (pointer to file array) */
/*  changed algorithm for 3B: not assuming in order */
/*  commented out adrtofilep *****************
/*  struct filet *
/*  adrtofilep(addr) 
/*  ADDR addr; {
/*	register struct filet *filep, *filer;
/*	register ADDR fa;
/*#if DEBUG > 1
/*	if(debugflag)
/*		fprintf(FPRT2, "adrtofilep(addr=%#lx); files->sfilename=%s;\n",
/*					addr, files->sfilename);
/*#endif
/*	filer = badfile;
/*	for (filep=files; filep->sfilename[0]; filep++) {
/*		fa = filep->faddr;
/*#if DEBUG > 1
/*		if(debugflag > 1)
/*			fprintf(FPRT2, "	sfilename=%s; faddr=%#lx;\n",
/*					filep->sfilename, fa);
/*#endif
/*		if ((filer == badfile || filer->faddr <= fa) && fa <= addr)
/*			filer = filep;
/*	}
/*#if DEBUG > 1
/*	if(debugflag) fprintf(FPRT1,
/*		" file=%s;\n", filer != badfile ? filer->sfilename : "BADFILE");
/*#endif
/*	return (filer);
/*}
*************************** end of comment (adrtofilep) */
 
/*
 * core address to statement 
 * Sets external exactaddr to addr if addr is NOT the first instruction
 * of a line, set to -1 (ERROR) otherwise.
 * Sets external lnfaddr to address of statement.
 *  major changes for COFF
 */
/* static long lastoffset = -1;		/*  useless for COFF */

struct stmt
adtostmt(addr, procp) 
ADDR addr;
register struct proct *procp;
{
	register long offset; 
	register long lna;
	register LINENO *lnp = &linent;
	AUXENT *axp = &auxent[0];
	struct stmt stmt;
	
	exactaddr = addr;
	stmt.lnno = -1;
	stmt.stno = 0;
	if (procp == badproc) return(stmt);
	if(procp->lineno <= 0)
		return(stmt);		/* function not compiled with -g */
	stmt.lnno = 1;
	lnfaddr = procp->paddress;

#if DEBUG > 1
	if(debugflag > 0)
		fprintf(FPRT2, 
		    "adtostmt(addr=%#x, procp=%#lx); pname=%s; n_value=%#x\n",
		    addr, (long)procp, procp->pname, syment.n_value);
#endif
	offset = procp->st_offset;
	if( rdsym(offset) == ERROR)
		return(stmt);

#if u3b || u3b5 || ns32000
	/*  On 3B, functions begin at first address in procedure,
	 *	so the line number of that instruction is meaningful.
	 *	On the VAX, the first short is not an instruction,
	 *	so don't return a line number for it.
	 */
	if(addr == syment.n_value) {
		if(gflag > 0)	/* in coff, lnno relative to function { */
			stmt.lnno += procp->lineno -1;
		return(stmt);
	}
#else
#if vax
	if(addr == syment.n_value) {
		stmt.lnno = -1;
		return(stmt);
	}
#endif
#endif

	offset = (long) axp->x_sym.x_fcnary.x_fcn.x_lnnoptr;
	if(offset == 0)		/*  sgs bug ? */
		offset = getlnnoptr(procp);	/* for SGS .o files */
	if( (offset = rdlntry(offset)) == ERROR || lnp->l_lnno != 0) {
#if DEBUG > 1
		if(debugflag)
			fprintf(FPRT2, "Cannot rdlntry(%#o) for %s;\n",
					offset-LINESZ, procp->pname);
#endif
		return(stmt);
	}
	/*  many changes to adtostmt() and --
		assuming line number info followed by symbol table
		assuming line number entries in increasing order of address
	*/
	while( (offset = rdlntry(offset)) != ERROR) {
		lna = lnp->l_addr.l_paddr;
#if DEBUG > 1
		if(debugflag > 1)
			fprintf(FPRT2, "l_paddr=%#lx;	l_lnno=%#d;\n",
					lna, lnp->l_lnno);
#endif
		if(lna > addr || lnp->l_lnno == 0)
			break;
		if(lnp->l_lnno == stmt.lnno)
			stmt.stno++;
		else
			stmt.stno = 1;
		stmt.lnno = lnp->l_lnno;
		lnfaddr = lna;
		if(lna == addr)
			exactaddr = -1;		/* => 1st instr of line */
	}
	if(gflag > 0)		/* in coff, lnno relative to function { */
		stmt.lnno += procp->lineno -1;
#if DEBUG > 1
	if(debugflag > 0)
		fprintf(FPRT2, "	stmt.lnno=%d; stmt.stno=%d;\n",
				stmt.lnno, stmt.stno);
#endif
	return (stmt);
}

adrtolineno(addr, procp) 
ADDR addr;
struct proct *procp;
{
	struct stmt stmt;

#if DEBUG > 1
	if(debugflag)
		fprintf(FPRT2, "adrtolineno(addr=%#lx, procp=%#lx);\n",
			addr, (long)procp);
#endif
	stmt = adtostmt(addr, procp);
	return(stmt.lnno);
}


/* address to a.out offset */
/*  new adtostoffset()
 *	only called from prvar.c outvar();
 *	Use to get offset to start looking in symbol table
 *	end of block or end of function will do
 */
long
adtostoffset(addr, procp) 
ADDR addr;
struct proct *procp;
{
	SYMENT *syp = &syment;
	AUXENT *axp = &auxent[0];
	register int class;
	register char *name;
	register long offset, lastoffs;
	register struct blklist *tail = &zeroblk;
	register int block;

#if DEBUG > 1
	if(debugflag)
		fprintf(FPRT2, "adtostoffset(addr=%#lx, procp=%#lx);\n",
			addr, (long)procp);
#endif
	level = 0;
	block = 0;
	lastoffs = offset = procp->st_offset;
	for ( ; ; ) {
		if( (offset = rdsym(offset)) == ERROR)
			return(ERROR);
		class = syp->n_sclass;
		name = syp->n_nptr;
		if (class == C_BLOCK)
		{
			if (eqstr(name, ".bb")) {
			    if (syp->n_value > addr) break;
			    if (!(tail->next)) {
				if ((tail->next = (struct blklist *)
				    sbrk(sizeof(struct blklist)))
					< 0) return(ERROR);
				(tail->next)->prev = tail;
				(tail->next)->next = (struct blklist *)0;
			    }
			    tail = tail->next;
			    tail->blknum = ++block;
			    level++;
			}
			else if (eqstr(name, ".eb"))
			{
			    if (syp->n_value > addr) break;
			    if (!(tail = tail->prev))
				return(ERROR);		/* internal error */
			    level--;
			}
		}
		else if (class == C_FCN && eqstr(name, ".ef") &&
			syp->n_value >= addr) break;

		lastoffs = offset;
	}
#if DEBUG > 1
	if(debugflag > 1)
		fprintf(FPRT2, "	lastoffs=%#lo, block=%d, level=%d;\n",
			lastoffs, block, level);
#endif
	return(lastoffs);
}

/*
 * Set (curfile, curstmt) from core image or uarea of running process.
 * Returns 1 if there is a core image and /or a process, 0 otherwise.
 *
 * Print the current line iff verbose is set.
 */
setcur(verbose) {
	register struct proct *procp;

#if DEBUG > 1
	if(debugflag > 1)
		prprstk();
#endif
	
				/* if allow break at 0, dot will = 0 */
	if (!pid) {
		if (fcor < 0) {		/*  was "if (dot == 0)" */
			fprintf(FPRT1, "No core image\n");
			goto setmain;
		}
		if (fakecor) {    /* arbitrary (not core) core file */
			goto setmain;
		}
	}
	dot = USERPC;
	procp = adrtoprocp(dot-(signo ? NOBACKUP : 0));
	if ((procp->sfptr != badfile) && !procp->notstab) {
		struct stmt stmt;
#ifdef mc68000sdb
/* temporary kludge to correct for compiler bug */
		finit((procp->sfptr)->sfilename);
#else
/* 		finit(adrtofilep(procp->paddress)->sfilename); */
 		finit((procp->sfptr)->sfilename);
#endif
		stmt = adtostmt(dot-(signo ? NOBACKUP : 0), procp);
		ffind(stmt.lnno);
		curstmt.stno = stmt.stno;
		if (verbose) {
			if ((!(signo ? NOBACKUP : 0)) && exactaddr != -1)
				printf("%#lx in ", exactaddr);
#if u3b || u3b5
			printf("%s:", procp->pname);
#else
#if vax || ns32000
			if (procp->pname[0] == '_')
			{
				printf("%s:", (procp->pname)+1 );
			}
			else
			{
				printf("%s:", procp->pname);
			}
#endif
#endif
			fprint();
		}
#if DEBUG > 1
		if(debugflag > 1)
			fprintf(FPRT2,
				"setcur(): stmt=%d(%d);\n",stmt.lnno,stmt.stno);
#endif
		return(1);
	}
	if (verbose) {
#if u3b || u3b5
		printf("%s: address %#lx\n", procp->pname, dot);
#else
#if vax || ns32000
		if (procp->pname[0] == '_') 
			printf("%s: address %#lx\n", procp->pname+1, dot);
		else
			printf("%s: address %#lx\n", procp->pname, dot);
#endif
#endif
	}
	
setmain:
	procp = findproc("MAIN__");
	if (procp->sfptr == badfile) {
#ifdef ns32000
		/* this should also apply to the vax !!?	*/
		procp = findproc("_main");
#else
		procp = findproc("main");
#endif
		if (procp->sfptr == badfile) {
		    if(!ISMAGIC(magic)) {	/* not a .o file */
			nolines = 1;
			/* printf("main not compiled with debug flag\n"); */
			dot = 0;	/* 0?? - is dot needed? */
			return(0);
		    }
		    else
			procp = procs;	/*  for .o files, just first proc */
		}
	}
	finit(procp->sfptr->sfilename);
	ffind(procp->lineno);
	dot = procp->paddress;
	return(0);
}

compar(a, b)
struct proct *a, *b; {
	if (a->paddress == b->paddress) {
		if (a->notstab) return(-1);
		if (b->notstab) return(1);
		return(0);
	}
	return(a->paddress < b->paddress ? -1 : 1);
}

/*
 * in nametooffset() and nametofile()
 * assuming a file name has a '.' in it; else assume a procedure name
 */

/* gets offset of file or procedure named s */
nametooffset(s)
register char *s; {
	register struct filet *f;
	register struct proct *p;
	
	if (*s == '\0')
		return(ERROR);
#if DEBUG > 1
	if(debugflag > 1)
		fprintf(FPRT2, "nametooffset(s=%s);\n", s);
#endif
	if (eqany('.', s)) {
		f = findfile(s);
		return(f->sfilename[0] ? f->stf_offset : ERROR);
	}
	p = findproc(s);
	return(p->pname[0] ? p->st_offset : ERROR);
}

/* returns s if its a filename, its file otherwise */
/*  using in stmttoaddr() -- was not use in VAX sdb */
char *
nametofile(s)
register char *s; {
	register struct proct *p;
	
	if (eqany('.', s)) {
		return(s);
	}
	p = findproc(s);
/* return(adrtofilep(p->paddress)->sfilename); */
	return((p->sfptr)->sfilename);
}

/*
 * statement to process (i.e. which function statement is in)
 *  added stmttoproc() to implement stmttoaddr()
 */
struct proct *
stmttoproc(stmt, filename)
struct stmt stmt; char *filename; {
	register struct proct *pp, *pps = badproc;
	register unsigned short ln, ll = 0;
	register struct filet *sfptr;
	
	if((ln = stmt.lnno) <= 0)
		return(badproc);
	if((sfptr=findfile(filename))->sfilename[0] == '\0')
		return(badproc);
	if(procs->lineno > ln && procs->sfptr == sfptr)
		return(procs);		/* at name line of 1st fun */
	/* note: the proct's are sorted by address in core, not by lineno */
	for (pp=procs; pp->pname[0]; pp++) {
#if DEBUG > 1
		if(debugflag > 2) {
			fprintf(FPRT2,
				"stmttoproc(%d): pp->lineno=%d; lineno=%d;\n",
						ln, pp->lineno, ll);
			fprintf(FPRT2, "	pp=%#o, pp->pname=(%#o)%s\n",
						pp, pp->pname[0], pp->pname);
		}
#endif
		if(pp->sfptr != sfptr || pp->lineno == 0)
			continue;	/* wrong source file or no lineno */
		if (ll <= pp->lineno && pp->lineno <= ln) {
			ll = pp->lineno;
			pps = pp;
#if DEBUG > 1
			if(debugflag > 1) fprintf(FPRT2,
					"  : pps->pname=%s;\n", pps->pname);
#endif
		}
	}
	if(pps->ef_line < ln && pps != badproc)	/* function lineno is for '{' */
		pps++;		/* really next function name line -- kludge! */
#if DEBUG > 1
	if(debugflag)
		fprintf(FPRT2,
			"stmttoproc: pps->pname=%s; pps->st_offset=%#o;\n",
					pps->pname, pps->st_offset);
#endif
	return(pps);
}

/*
 * stmt number to address;
 * offset should be within file; not using offset on 3B;
 *  on 3B, completely redid stmttoaddr()
 *	assuming line number entries in increasing order of address
 */
stmttoaddr(stmt, offset, file) 
struct stmt stmt; long offset; char *file; {
	SYMENT *syp = &syment;
	AUXENT *axp = &auxent[0];
	LINENO *lnp = &linent;
	long laddr;
	register long offl, offn;
	register unsigned short ll;
	register int  stcnt = 0;
	register struct proct *procp;
	
#if DEBUG
	if(debug) fprintf(FPRT2, 
		"\n	stmttoaddr(stmt=%d(%d),offset=%d(%#o),file=%s);\n",
			stmt.lnno, stmt.stno, offset, offset, file);
#endif
	/* 'file` may actually be a proc name */

	if( ( procp = stmttoproc( stmt, nametofile( file ) ) ) == badproc ||
	      procp->notstab )
	{
		return( ERROR );
	}

	if(stmt.lnno < procp->lineno)		/* line with function name */
	{
		return(procp->paddress);		/* before opening '{' */
	}
	laddr = procp->paddress; /* else may be problem when a.out optimized */
	offl = procp->st_offset;
	if( rdsym(offl) == ERROR)
		return(ERROR);
	offl = axp->x_sym.x_fcnary.x_fcn.x_lnnoptr;

	offn = filhdr.f_symptr + axp->x_sym.x_fcnary.x_fcn.x_endndx *SYMESZ;
	if(offl == 0)		/*  sgs bug ? */
		offl = getlnnoptr(procp);	/* for SGS .o files */
	if( rdsym(offn) == ERROR)
		return(ERROR);
	if(ISFCN(syp->n_type)) {
		offn = axp->x_sym.x_fcnary.x_fcn.x_lnnoptr;
		if(offn == 0)		/*  sgs bug ? */
			offn = getlnnoptr(procp +1);	/* for SGS .o files */
	}
	else		/* in case last function */
		offn = filhdr.f_symptr;		/* symbol table follows */
#if DEBUG > 1
	if(debugflag > 1)
		fprintf(FPRT2, "stmttoaddr: offl = %#o; offn = %#o;\n",
						offl, offn);
#endif

	ll = stmt.lnno - procp->lineno +1;	/* in coff, rel. to proc { */
	if( (offl = rdlntry(offl)) == ERROR)
		return(ERROR);
	if( lnp->l_addr.l_symndx !=
	    ( procp->st_offset-filhdr.f_symptr ) / SYMESZ )
	{
		fprintf(FPRT1, "sttmttoaddr: Bad lineno for %s;\n",
							procp->pname);
		return(ERROR);
	}
	while(offl < offn) {
		if( (offl = rdlntry(offl)) == ERROR)
			break;
#if DEBUG > 1
		if(debugflag > 2)
			fprintf(FPRT2, "l_paddr=%#lx; l_lnno=%d; ll=%d;\n",
					lnp->l_addr.l_paddr, lnp->l_lnno, ll);
#endif
		if(lnp->l_lnno <= ll) {		/* line numbers increase */
			laddr = lnp->l_addr.l_paddr;	/* save previous */
#if DEBUG > 1
			if(debugflag > 2)
				fprintf(FPRT2, " : l_lnno=%d; stcnt=%d;\n",
							lnp->l_lnno, stcnt);
#endif
			if(lnp->l_lnno == ll) {
				if(++stcnt >= stmt.stno)
					return(lnp->l_addr.l_paddr);
			}
		}
		else		/* lnp->l_lnno > ll ==> went past */
			return(laddr);	/* on 3B, some lineno's missing ! */
	}
#if DEBUG > 1
	if(debugflag)
		fprintf(FPRT2, "stmttoaddr: Cannot find stno=%d, lnno=%d;\n",
					stmt.stno, stmt.lnno);
#endif
	return (ERROR);
}


/* line number to address, starting at offset in a.out */
/* assumes that offset is within file */
				/*  no longer using lntoaddr() */
lntoaddr(lineno, offset, file) 
long offset; char *file; {
	struct stmt stmt;
	stmt.lnno = lineno;
	stmt.stno = 1;
	return(stmttoaddr(stmt, offset, file));
}

/* gets address of proc:number */
getaddr(pnam, stmt) 
char *pnam; struct stmt stmt; {
	register long offset;
	register ADDR addr;
	register char *s;
	
#if DEBUG > 1
	if(debugflag > 1) {
		fprintf(FPRT2, "getaddr: pnam=%s; curfile=%s;\n", pnam,curfile);
		fprintf(FPRT2, "	stmt.lnno=%d;\n", stmt.lnno);
		fprintf(FPRT2, "	stmt.stno=%d;\n", stmt.stno);
	}
#endif
	s = pnam[0] ? pnam : curfile;
	if (*s == '\0')
		return(ERROR);
	offset = nametooffset(s);
#if DEBUG
	if (debug) fprintf(FPRT2, "getaddr() computed offset %#lo\n", offset);
#endif
	if (offset == ERROR) {
		addr = extaddr(pnam);
		if (addr != ERROR) addr += PROCOFFSET;
#if DEBUG
		if (debug) fprintf(FPRT2, " extaddr computed %#lx\n", addr);
#endif
		return(addr);
	}
	if (stmt.lnno)
		addr = stmttoaddr(stmt, offset, s);
	else {
		addr = findproc(pnam)->paddress + PROCOFFSET;
	}
#if DEBUG
	if (debug) fprintf(FPRT2, " and addr %#lx\n", addr);
#endif
	return(addr);
}

/* returns address of function (name is for historical reasons) */
ADDR
extaddr(name)
char *name;
{
	register struct proct *procp;

#if DEBUG > 1
	if(debugflag)
		fprintf(FPRT2, "extaddr(name=%s);\n", name);
#endif

	if( *name == '\0' )	/*  prevent recursion when not in a.out */
		return( ERROR );
	if( ( procp = findproc( name ) ) != badproc )
	{
		return( procp->paddress );
	}
	
	return(ERROR);
}

/*  getlocv() to call getreg() or getval(), if loc is a reg or not */
/*	only called in xeq.c; could probably use elsewhere also */
/* sl_class must be set, because this routine uses it to determine
 *	whether the location is a register on the stack.  For the 3B,
 *	all data not in registers is left justified, which getval expects.
 *	If the data is a register image, it is right justified, and
 *	getreg will extract the value correctly.
 *
 *	getlocv returns the value as a right justified long int.
 */
long
getlocv(loc, d, space)
ADDR loc;
char *d;
{
	if (ISREGV(sl_class))
		return(getreg(loc,d));
	else
		return(getval(loc,d,space));
}

/* 
 * Look up external data symbol matching pattern starting at 
 *  (filestart+SYMESZ)
 * Return its address in sl_addr and name in sl_name.
 */
long
extlookup(pat, filestart)
char *pat; long filestart; {
	register int i, class, n;
	register long offset;
	register char *q;
	SYMENT *syp = &syment;
	AUXENT *axp = &auxent[0];
	
#if DEBUG > 1
	if(debugflag)
		fprintf(FPRT2, "extlookup(pat=%s,filestart=%#o);\n",
						pat,filestart);
#endif
	offset = filestart;
	while( (offset = rdsym(offset)) != ERROR) {
		class = syp->n_sclass;
		if (syp->n_scnum <= 0) continue;
		q = scnhdrp[syp->n_scnum -1].s_name;
		if ((class == C_EXT) &&
		    (eqstr(q,_DATA) || eqstr(q,_BSS)) &&
		    eqpatu(pat, syp->n_nptr)) {
			sl_addr = syp->n_value;
			sl_type = syp->n_type;
			sl_size = axp->x_sym.x_misc.x_lnsz.x_size;
			sl_class = class;
			q = &syp->n_nptr[0];
		    /* no special treatment of leading '_' on 3B
		    *	if (syp->n_nptr[0] == '_')
		    *	{
		    *		q = &syp->n_name[1];
		    *		n = SYMNMLEN -1;
		    *	}
		    */
			strcpy( sl_name, *q );
			return(offset);
		}
	}
	return(ERROR);
}

/* find enclosing common blocks and fix up addresses */
/*  fixed docomm(), but do not expect to be called */
/*  docomm was to adjust for common symbol table kludges in old
 *	symbol table format; is not used in COFF, and is commented out
long
docomm(offset)
long offset; {
/*	SYMENT *syp = &syment;
/*	AUXENT *axp = &auxent[0];
/*	register long noffset;
/*
/*	fprintf(FPRT1, "Warn:  docomm() called;\n");
/*	for( noffset = offset ; ; ) {
/*		if( (noffset = rdsym(noffset)) == ERROR) {
/*			error("Bad common block");
/*			return(offset);
/*		}
/*		sl_class = C_EXT;
/*		if(syp->n_sclass==C_BLOCK && eqstr(syp->n_name,".eb")) {
/*			sl_addr += extaddr(syp->n_name);
/* /* 				blseek(&sbuf, offset, 0);*/
/*			return(offset);
/*		}
/*	}
/*}
************************ end of comment (docomm) */

/* Determine if class is that of a variable */
/* for coff, need type to tell that proc's (C_EXT) are not variables */

char pctypes[] = {C_EXT, C_STAT, C_REGPARM, C_REG, C_AUTO,	/* 3B coff */
			C_ARG, C_MOS, C_MOU, C_MOE, C_FIELD, 0};

varclass(class, type)
int class, type; {		/* added type as arg to varclass() */
	register char *p;

	for (p=pctypes; *p; p++) {
		if (class == *p)
			if(!ISFCN(type)) return(1);
			else return(0);
	}
	return(0);
}

/*
 * address to external name 
 * returns difference between addr and address of external
 * name returned in sl_name
 */
adrtoext(addr) 
ADDR addr; {
	SYMENT *syp = &syment;
	struct proct *procp;
	register int i, nq;
	register long prevdiff = MAXPOS, diff;
	register long offs = extstart;
	register char *q;

#if DEBUG > 1
	if(debugflag)
		fprintf(FPRT2, "adrtoext(addr=%#lx);\n", addr);
#endif
	if((procp = adrtoprocp(addr)) != badproc)
	{
		prevdiff = addr - procp->paddress;
		strcpy( sl_name, procp->pname );
	}
	while ((offs = rdsym(offs)) != ERROR) {
		if (syp->n_sclass == C_EXT) {
			diff = addr - syp->n_value;
			if (diff >= 0 && diff < prevdiff) {
				q = &syp->n_nptr[0];
			    /* no special treatment of leading '_' on 3B
			    *	if (syp->n_nptr[0] == '_') {
			    *		q = &syp->n_nptr[1];
			    *		nq = SYMNMLEN -1;
			    *	}
			    */
				strcpy( sl_name, *q );
				sl_scnum = syp->n_scnum;
#if DEBUG > 1
				if(debugflag)
					fprintf(FPRT2, "  : sl_name=%s;\n",
								sl_name);
#endif
				if (diff == 0)
					return(0);
				prevdiff = diff;
			}
		}
	}
#if DEBUG > 1
	if(debugflag)
		fprintf(FPRT2, "  : prevdiff=%#x; sl_name=%s;\n",
					prevdiff,sl_name);
#endif
	return (prevdiff != MAXPOS ? prevdiff : -1);
}

/*
 * The following conversions from addresses to symbol names
 *	are only used when disassebling in 'i' format
 *	(see psymoff() in opset.c)
 */

/*
 * address to local name in procp
 * returns difference between addr and address of local
 * returned in sl_name
 */
adrtolocal(addr, procp) 
ADDR addr; struct proct *procp; {
	SYMENT *syp = &syment;
	register int i, prevdiff = MAXPOS, diff;
	register long offl;

	offl = rdsym(procp->st_offset);
	while( (offl = rdsym(offl)) != ERROR) {
		if (ISFCN(syp->n_type))
			break;
		if (syp->n_sclass == C_AUTO) {
			diff = addr - syp->n_value;
			if (diff >= 0 && diff < prevdiff) {
				strcpy( sl_name, syp->n_nptr );
				if (diff == 0)
					return(0);
				prevdiff = diff;
			}
		}
	}
	return(prevdiff != MAXPOS ? prevdiff : -1);
}

/*
 * address to parameter name in procp
 * returns difference between addr and address of local
 * returned in sl_name
 */
adrtoparam(addr, procp) 
ADDR addr; struct proct *procp; {
	SYMENT *syp = &syment;
	register int i, prevdiff = MAXPOS, diff;
	register long offl;

	offl = rdsym(procp->st_offset);
	while( (offl = rdsym(offl)) != ERROR) {
		if (ISFCN(syp->n_type))
			break;
		if (ISARGV(syp->n_sclass)) {
			diff = addr - syp->n_value;
			if (diff >= 0 && diff < prevdiff) {
				strcpy( sl_name, syp->n_nptr );
				if (diff == 0)
					return(0);
				prevdiff = diff;
			}
		}
	}
	return(prevdiff != MAXPOS ? prevdiff : -1);
}

/*
 * register number to register variable name in procp
 * returned in sl_name
 */
adrtoregvar(regno, procp) 
ADDR regno; struct proct *procp;
{
	SYMENT *syp = &syment;
	register int i;
	register long offl;

	offl = rdsym(procp->st_offset);
	while( (offl = rdsym(offl)) != ERROR) {
		if (ISFCN(syp->n_type))
			return(-1);
		if (ISREGV(syp->n_sclass)) {
			if (syp->n_value == regno) {
				strcpy( sl_name, syp->n_nptr );
				return(0);
			}
		}
	}
	return(-1);
}

/* sets file map for M command */
setmap(s)
char *s; {
	union {
		MAP *m;
		L_INT *mp;
	} amap;
	int starflag = 0;

	amap.mp = 0;
#if DEBUG > 1
	if(debugflag)
		fprintf(FPRT2, "setmap(s=%s)\n", s);
#endif
	for (; *s; s++) {
		switch (*s) {
		case '/':
			amap.m = &datmap;
			break;
		case '?':
			amap.m = &txtmap;
			break;
		case '*':
			starflag++;
			break;
		case ' ':		/* allow space before '*' */
			break;
		default:	
			goto sout;
		}
	}
	
sout:	if (amap.mp == 0) {
		error("Map `?' or `/' must be specified");
		return;
	}
	if (starflag)
		amap.mp = amap.m == &datmap ?	(L_INT *)&datmap.b2 :
						(L_INT *)&txtmap.b2;
	for (; *s; s++) {
		if (*s >= '0' && *s <= '9')
			*(amap.mp)++ = readint(&s);
	}
}


/* added rdsym() and rdlntry()
 * long curoffs -- shared by rdsym(), rdlntry(), and access() (in access.c)
 * These routines share a common file descriptor, buffer and offset
 * Must always call them with correct offset as argument
 * since they decide whether or not to do a seek.
 */

long curoffs = (ERROR -1);

/* added long rdsym(offset)
 *	Stores SYMENT and AUXENT in external syment and auxent
 *	Returns offset for next syment
 *	Seeks done here if necessary; removed from other symt.c code
 *	for COFF, always just 0 or 1 auxiliary entries
 */

long
rdsym(offset)
long offset;
{
	register int numaux;
	register SYMENT *symep;
	register AUXENT *auxep;

	symep = &syment;
	auxep = &auxent[0];
	if(offset <= 0)
	{
		return(ERROR);
	}
	if( offset >= ( filhdr.f_symptr + filhdr.f_nsyms * SYMESZ ) )
	{
#ifdef	DEBUG
		if( debugflag > 1 )
		{
			printf( "offset = %d, filhdr.f_symptr = %d, ",
				offset, filhdr.f_symptr );
			printf( "filhdr.f_nsyms = %d\n", filhdr.f_nsyms );
			printf("filhdr.f_nsyms * SYMESZ + filhdr.f_symptr = %d\n",
				 filhdr.f_nsyms * SYMESZ + filhdr.f_symptr );
		}
#endif
		return( ERROR );
	}
	if(offset != curoffs)
	{
		if(filhdr.f_nsyms == 0)		/* in case "strip" file */
			return(ERROR);
		blseek(&sbuf, offset, 0);
		curoffs = offset;
	}
	if (bread(&sbuf, (char *)symep, SYMESZ) < SYMESZ)
	{
		curoffs = ERROR -1;
		return(ERROR);
	}
#ifdef FLEXNAMES
	if( symep->n_zeroes == 0 )
	{
		if (symep->n_offset < 4 || symep->n_offset > strtablen)
		{
			printf("Bad string table offset @ 0x%lx\n",
				curoffs - SYMESZ);
			return (ERROR);
		}
		symep->n_nptr = &strtab[ symep->n_offset ];
	}
	else
#endif
	{
		long	i = offset - filhdr.f_symptr;
		i /= SYMESZ;
		strncpy( shstrtab[ i ].name, symep->n_name, SYMNMLEN );
		shstrtab[ i ].name[ 8 ] = '\0';
		symep->n_zeroes = 0;
		symep->n_offset = 0;
		symep->n_nptr = shstrtab[ i ].name;
	}
/*
**	printf("Found name `%s'\n", symep->n_nptr);
*/
	curoffs += SYMESZ;
	if( ( numaux = symep->n_numaux ) > MAXAUXENT )
	{
		fprintf(FPRT1, "Warn: [%.8s](%d): n_numaux(=%d) > %d;\n",
			symep->n_nptr, (int)symep->n_sclass, numaux,MAXAUXENT);
		numaux = MAXAUXENT;
	}
	while(numaux-- > 0)
		if( bread( &sbuf, ( char * ) auxep++, AUXESZ ) < AUXESZ )
		{
			curoffs = ERROR -1;
			return(ERROR);
		}
	curoffs += (AUXESZ * symep->n_numaux);
	return(curoffs);
}

/* added long rdlntry(offset)
 *	stores LINENO in external linent;
 *	returns offset for next LINENO
 *	for COFF, lineno entries separate from symbol table entries.
 */

long
rdlntry(offset)
long offset;
{

	if(offset <= 0)
		return(ERROR);
	/* LINENO entries followed by symbol table */
	if(offset >= filhdr.f_symptr)
		return(ERROR);
	if(offset != curoffs) {
		blseek(&sbuf, offset, 0);
		curoffs = offset;
	}
	if (bread(&sbuf, (char *)&linent, LINESZ) < LINESZ) {
		curoffs = ERROR -1;
		return(ERROR);
	}
	curoffs += LINESZ;
	return(curoffs);
}

/*
 * getlnnoptr(): to compensate for sgs 'bug` with ".o" files
 *	auxent for a proc in a .o file has lnnoptr always 0,
 *	even though lineno information is present!
 */
long
getlnnoptr(procp)
struct proct *procp;
{
	register long offl;
	register LINENO *lnp = &linent;
	int n, nz = 0;
		
	if(!ISMAGIC(magic))	/* only need for .o files */
		return(ERROR);
	n = procp - procs;	/* function number; assume proct order */
	offl = scnhdrp[0].s_lnnoptr;	/* assume text, first section */
#if DEBUG > 1
	if(debugflag)
		fprintf(FPRT2, "procs=%#x; procp=%#x; n=%d; offl=%#x;\n",
				procs, procp, n, offl);
#endif
	/* count through line number entries til get function wanted */
	while( (offl = rdlntry(offl)) != ERROR) {
#if DEBUG > 1
		if(debugflag > 1)
			fprintf(FPRT2, "l_paddr=%#lx; l_lnno=%d; nz=%d;\n",
					lnp->l_addr.l_paddr, lnp->l_lnno, nz);
#endif
		if (lnp->l_lnno == 0)
			if(nz++ == n)
				return(offl -LINESZ);
	}
	return(ERROR);
}

int
readstrtbl( fd )
int  fd;
{
	extern char *sbrk();
	long home;
	register int i;

	home = lseek( fd, 0L, 1 );
	if (filhdr.f_symptr == 0)
	{
		return;
	}
	if ( lseek( fd, filhdr.f_symptr, 0 ) == -1 )
	{
		(void) lseek( fd, home, 0 );
		return;
	}
	if ( ( shstrtab = (struct sh_name *)sbrk( ( ( filhdr.f_nsyms *
		sizeof( struct sh_name) ) + 3 ) & ~3 ) ) ==
		( struct sh_name * )( -1 ) )
	{
		fprintf(FPRT2,"Cannot allocate space for internal symbol table; Goodbye\n");
		exit(2);
	}
	if ( lseek( fd, filhdr.f_nsyms * SYMESZ + filhdr.f_symptr, 0 ) == -1 )
	{
		(void) lseek( fd, home, 0 );
		return;
	}
	if( read( fd, (char *) &strtablen, sizeof( long ) ) != sizeof( long ) )
	{
		(void) lseek( fd, home, 0 );
		return;
	}
	if (strtablen <= 4L)
	{
		(void) lseek(fd,home,0);
		return;
	}
	if ( ( strtab = sbrk( ( ( strtablen ) + 3 ) & ~3 ) ) ==
		( char * )( -1 ) )
	{
		fprintf(FPRT2,"Cannot allocate space for string table; Goodbye\n");
		exit(2);
	}
	if ( read( fd, &strtab[ sizeof( long ) ], strtablen - sizeof( long ) )
		!= strtablen - sizeof( long ) )
	{
		fprintf(FPRT2,"Cannot read string table; Goodbye\n");
		exit(2);
	}
	if ( strtab[ strtablen - 1 ] != '\0' )
	{
		(void) lseek( fd, home, 0 );
		return;
	}
	lseek( fd, home, 0 );
	/*
	**	Used for debugging the reading in of string table for FLEXNAMES.
	**	printf("STRING TABLE\n");
	**	for (i = 4; i < strtablen; i++)
	**	{
	**		if (strtab[i] == '\0')
	**			printf("^@\n");
	**		else if (strtab[i] < ' ')
	**			printf("^%c", strtab[i]|0100);
	**		else if (strtab[i] == '\177')
	**			printf("^?");
	**		else if (strtab[i] > '~')
	**			printf("M-%c", strtab[i]&0177);
	**		else
	**			printf("%c", strtab[i]);
	**	}
	*/
	return;
}
