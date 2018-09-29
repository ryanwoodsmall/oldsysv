/*    @(#)special.c	2.10 5/27/83	*/
#include <stdio.h>

#include "system.h"
#include "params.h"
#include "aouthdr.h"
#include "structs.h"
#include "ldfcn.h"

#include "slotvec.h"
#include "reloc.h"
#include "extrns.h"
#include "list.h"
#include "sgs.h"
#include "sgsmacros.h"

char Xflag = 1;		/* generate optional header with old */
unsigned short magic = (unsigned short) NS32WRMAGIC;


set_spec_syms()
{

/*
 * For non-relocatable link edits, set up the special symbols.
 *
 *
 *         _ETEXT : next available address after the end of the output
 *                      section _TEXT
 *         _EDATA : next available address after the end of the output
 *                      section _DATA
 *         _END   : next available address after the end of the output
 *                      section _BSS
 *
 */

        register OUTSECT *osptr;
        register long base;
        SYMTAB  *sym;
        AUXENT aux;

	if (Nflag) /* squash the text and data together into one
		      writeable segment */
		magic = (unsigned short)NS32WRMAGIC;

        if ( ! aflag )
                return;


        osptr = (OUTSECT *) outsclst.head;
        while (osptr) {
                if( equal(osptr->oshdr.s_name,_TEXT, 8))
                        creatsym(_ETEXT,
                                osptr->oshdr.s_paddr + osptr->oshdr.s_size);
                else if( equal(osptr->oshdr.s_name,_DATA, 8))
                        creatsym(_EDATA,
                                osptr->oshdr.s_paddr + osptr->oshdr.s_size);
                else if( equal(osptr->oshdr.s_name,_BSS, 8)) {
                        base = osptr->oshdr.s_paddr + osptr->oshdr.s_size;
                        creatsym(_END, base);
                        }
                osptr = osptr->osnext;
                }
}


void
adjneed(need, osp, sap)

ADDRESS	*need;
OUTSECT	*osp;
ANODE	*sap;
{
	/*
	 * adjust amount of memory allocated for a particular
	 * section;  special processing step now used only
	 * for DMERT with TRVEC
	 */
}

void
undefine()
{
	/*
	 * Undefine any symbols necessary prior to resolving
	 *	e.g.	"_start" in ld01.c
	 */
}

void
dfltsec()
{
	ADDRESS albound = ALIGNSIZE;
	ADDRESS *lptr = &albound;
	ACTITEM *aiptr, *grptr;

	if (Nflag)
		return;


/*
 * If any SECTIONS directives have been input, they take priority,
 * and no default action is taken
 */

	if( bldoutsc.head )
		return;

#if PAGING
	bond_t_d = 1;	/* set bond_t_d for PAGING so that text and data will
			be at the proper addresses. pboslist will build the
						bond ACTITEMS if this is 1 */
#endif

/*
 * Generate a series of action itmes, as if the following had been
 * input to vaxld:
 *
 *	SECTIONS {
 *		.text : {}
 *		GROUP ALIGN(ALIGNSIZE) : {
 *					.data : {}
 *					.bss  : {}
 *					}
 *		 }
 *
 *
 * Steps:
 *	1. Define background variables
 *	2. Process the .text definition
 *	3. Process the GROUP definition
 *	4. Process the .data definition
 *	5. Process the .bss definition
 *
 *
 * The logic used was obtained by simulating the action of the parser.
 */

	curfilnm = "*vax.default.file*";

	lineno = 2;
	aiptr = dfnscngrp( AIDFNSCN, NULL, NULL, NULL );
	aiptr->dfnscn.aisctype = STYP_REG;
	copy( aiptr->dfnscn.ainame, _TEXT, 8 );
	aiptr->dfnscn.aifill = 0;
	aiptr->dfnscn.aifillfg = 0;
	listadd( l_AI, &bldoutsc, aiptr );

	lineno=3;
	grptr = dfnscngrp( AIDFNGRP, NULL, lptr, NULL );
	copy( grptr->dfnscn.ainame, "*group*", 7);

	lineno = 4;
	aiptr = dfnscngrp( AIDFNSCN, NULL, NULL, NULL );
	aiptr->dfnscn.aisctype = STYP_REG;
	copy( aiptr->dfnscn.ainame, _DATA, 8 );
	aiptr->dfnscn.aifill = 0;
	aiptr->dfnscn.aifillfg = 0;
	listadd( l_AI, &grptr->dfnscn.sectspec, aiptr );

	lineno = 5;
	aiptr = dfnscngrp( AIDFNSCN, NULL, NULL, NULL );
	aiptr->dfnscn.aisctype = STYP_REG;
	copy( aiptr->dfnscn.ainame, _BSS, 8 );
	aiptr->dfnscn.aifill = 0;
	aiptr->dfnscn.aifillfg = 0;
	listadd( l_AI, &grptr->dfnscn.sectspec, aiptr );
	
	listadd( l_AI, &bldoutsc, grptr );
}

void
procspecobj(fdes, ap)
LDFILE *fdes;
ACTITEM *ap;
{
	/*
	 * Process non-standard or illegal files
	 */

	lderror(1, ap->ldlbry.aiinlnno, ap->ldlbry.aiinflnm,
		"file %s is of unknown type: magic number = %06.1x",
		ap->ldlbry.aifilnam, TYPE(fdes));
}

void
adjsize(osp)
OUTSECT *osp;
{
	/*
	 * Adjust size of output section in special cases
	 */
}

void
adjaout(aout)
AOUTHDR *aout;
{

	/*
	 * additions for special a.out header
	 */
#ifdef ns32000
	aout->msize = 16;
	aout->mod_start = 0L;
	aout->entry_mod = 0;
#endif

}

#ifdef ns32000
unsigned long
reverse(s,n)
short s;
unsigned long n;
{
	/* reverse byte ordering in integer */
	switch (s) {
		case 1:	/* byte */
			return n;
		case 2: /* word */
			return (n<<8) & 0xff00 | (n>>8) & 0xff;
		case 4: /* long */
			return ((n<<24) | (n<<8)&0xff0000 | (n>>8)&0xff00 | (n>>24));
		}
}
#else
#if AR32W
#define SWAP4(x)	swap4(x)
#else
#define SWAP4(x)	x
#endif
#endif

/* reverse byte ordering in integer */
unsigned long int swap4(n) unsigned long int n;
{
return((n<<24) | (n<<8)&0xFF0000 | (n>>8)&0xFF00 | (n>>24));
}
void
relocate(ifd, rfd, infl, isp, osp, fdes, rdes, paddiff, relocfg, buf)
LDFILE *ifd, *rfd;
INFILE *infl;
INSECT *isp;
OUTSECT *osp;
FILE *fdes, *rdes;
long *paddiff;
register int *relocfg;
register char *buf;
{
		/*
		 * Process the section
		 */

		register long ndx;
		register long rdif;
		register SLOTVEC *svp;

		long highaddr, vaddiff;
		int numbytes;
		union {
			long	 l;
			unsigned short u[2];
			short	 i[2];
			char	 c[4];
			} value;
		RELOC rentry;

		highaddr = isp->ishdr.s_vaddr + isp->ishdr.s_size;
		vaddiff = isp->isnewvad - isp->ishdr.s_vaddr;
		*paddiff = isp->isnewpad - isp->ishdr.s_paddr;
		for( ndx = isp->ishdr.s_vaddr; ndx < highaddr; ) {

			/*
			 * Read a relocation entry, if any left, and
			 * determine the size of the relocation field, in bytes
			 */
			if( *relocfg ) {
				if (fread(&rentry, RELSZ, 1, IOPTR(rfd)) != 1)
					lderror(2,0,NULL,"fail to read the reloc entries of section %s of %s",
					isp->ishdr.s_name,infl->flname);
				(*relocfg)--;
#ifdef ns32000
				if ( (rentry.r_type & R_ADDRTYPE) == R_NOTHING) {
#else
				switch (rentry.r_type) {

				case R_ABS:
#endif
					/*
					 * No relocation, so skip to the
					 * next entry
					 */
					if( rflag )
					    if( (svp = svread(rentry.r_symndx)) == NULL ) {
						lderror(1,0,NULL, "reloc entry (%10.1lx, %ld %d) found for non-relocatable symbol, in section %s of file %s",
							rentry, isp->ishdr.s_name, infl->flname);
						goto nextentry;
						}
					    else
						goto nullentry;
					goto nextentry;
#ifdef ns32000
					}
				switch ( rentry.r_type & R_SIZESP ) {


				case R_S_08: numbytes = 1; break;

				case R_S_16: numbytes = 2; break;

				case R_S_32: numbytes = 4; break;
#else
				case R_PCRBYTE:
				case R_RELBYTE: numbytes = 1; break;

				case R_PCRWORD:
				case R_RELWORD: numbytes = 2; break;

				case R_PCRLONG:
				case R_RELLONG: numbytes = 4; break;
#endif

				default:
					lderror(2,0,NULL,
						"Illegal relocation type %d found in section %s in file %s",
						rentry.r_type, isp->ishdr.s_name, infl->flname);
				}
				}
			/*
			 * There are no (more) relocation entries, so
			 * copy the rest of the section
			 */
			else {
				while( (rdif = highaddr - ndx) > 0 ) {
					rdif = min(BUFSIZ, rdif);
					if (FREAD(buf,(int) rdif,1,ifd) != 1 ||
					    fwrite(buf,(int) rdif,1,fdes) != 1)
						lderror(2,0,NULL,"fail to copy the rest of section %s of file %s",
						isp->ishdr.s_name,infl->flname);
					ndx += rdif;
					}
				continue;
				}


			/*
			 * Copy bytes that need no relocation
			 */
			while( (rdif = rentry.r_vaddr - ndx) > 0 ) {
				rdif = min(BUFSIZ, rdif);
				if (FREAD(buf,(int) rdif,1,ifd) != 1 ||
				    fwrite(buf,(int) rdif,1,fdes) != 1)
					lderror(2,0,NULL,"fail to copy the bytes that need no reloc of section %s of file %s",
					isp->ishdr.s_name,infl->flname);
				ndx += rdif;
				}
			if( rdif < 0 ) {
				lderror(2,0,NULL, "Reloc entries out of order in section %s of file %s",
					isp->ishdr.s_name, infl->flname);
				}

			/*
			 * Read and process the field to be relocated
			 */
			if (fread(&value.l, sizeof(char), numbytes, IOPTR(ifd)) != numbytes)
				lderror(2,0,NULL,"fail to read the field to be relocated of section %s of file %s",
				isp->ishdr.s_name,infl->flname);

			if (rentry.r_symndx >= 0)
			   if( (svp = svread(rentry.r_symndx)) == NULL ) {
				lderror(1,0,NULL, "No reloc entry found for symbol: index %ld, section %s, file %s",
					rentry.r_symndx, isp->ishdr.s_name, infl->flname);
					goto nextentry;
					}

			rdif = svp->svnvaddr - svp->svovaddr;

			/*
			 * Special processing for COPY sections:
			 *  do not touch the raw data (no relocation)
			 *  but process relocation entries and lineno
			 *  entries as usual
			 */

			if ( osp->oshdr.s_flags & STYP_COPY )
				goto copysect;

#ifdef ns32000
			/* R_ADDRESS | R_ABS | ? | ? */
			switch( rentry.r_type & (R_FORMAT | R_SIZESP) ) {

			case R_IMMED|R_S_08:
				value.c[0] += rdif; break;
			case R_IMMED|R_S_16:
				value.i[0] = reverse(2,reverse(2,value.i[0])+rdif); break;
			case R_IMMED|R_S_32:
				value.l = reverse(4,reverse(4,value.l)+rdif); break;
			case R_DISPL|R_S_08:
				value.c[0] += rdif; value.c[0] &= 0x7f; break;
			case R_DISPL|R_S_16:
				value.i[0] = reverse(2,value.i[0]);
				value.i[0] = ((value.i[0]+rdif) & 0x3fff) | 0x8000;
				value.i[0] = reverse(2,value.i[0]);
				break;
			case R_DISPL|R_S_32:
				value.l = reverse(4,value.l);
				value.l = ((value.l+rdif) & 0x3fffffff) | 0xc0000000;
				value.l = reverse(4,value.l);
				break;
			case R_NUMBER|R_S_08:
				value.c[0] += rdif; break;
			case R_NUMBER|R_S_16:
				value.i[0] += rdif; break;
			case R_NUMBER|R_S_32:
				value.l += rdif; break;
#else
			switch( rentry.r_type ) {

			case R_RELBYTE:
			case R_RELWORD:
			case R_RELLONG:
				/* nothing special */
				value.l = SWAP4( SWAP4(value.l) + rdif );
				break;

			case R_PCRBYTE:
			case R_PCRWORD:
			case R_PCRLONG:
				if (rentry.r_symndx >= 0)
					value.l = SWAP4( SWAP4(value.l) + rdif );
				value.l = SWAP4( SWAP4(value.l) - vaddiff );
				break;
#endif
			}

	copysect:	ndx += numbytes;
			fwrite(&value.l, numbytes, 1, fdes);

	nullentry:	/*
			 * Preserve relocation entries
			 */
			if( rflag ) {
				rentry.r_vaddr += vaddiff;
				rentry.r_symndx = svp->svnewndx;
				fwrite(&rentry, RELSZ, 1, rdes);
				}

	nextentry:	;
		}

}

void
procext(tvndx, secnum, sltval1, sltval2, ovaddr, nvaddr, gsp, sm)
int *tvndx, secnum;
long *sltval1, *sltval2, *ovaddr, *nvaddr;
register SYMTAB *gsp;
register SYMENT *sm;
{
	/*
	 * Create slot vector for C_EXT and C_EXTDEF variables
	 * in output.c
	 */
	if ((sm->n_sclass == C_EXT) && (sm->n_scnum == 0)) {
		*sltval1 = 0L;
		if (aflag || gsp->sment.n_scnum != 0)
			*sltval2 = gsp->smnewvirt;
		else
			*sltval2 = 0L;
	} else {
		*sltval1 = sm->n_value;
		*sltval2 = gsp->smnewvirt;
	}
}

void
proclocstat(sm, infd, segment, newscnum, tvndx, ovaddr, nvaddr, opaddr, npaddr, ndx, sltval1, sltval2, count)
register SYMENT *sm;
LDFILE *infd;
int *segment, *newscnum, *tvndx;
long *ovaddr, *nvaddr, *opaddr, *npaddr, ndx, *sltval1, *sltval2, count;
{
	/*
	 * create slot vector for C_STAT variable in output.c
	 */
			*sltval2 = sm->n_value;
			*sltval1 = *sltval2 - (npaddr[sm->n_scnum] - opaddr[sm->n_scnum]);
			svcreate(count, *sltval1, *sltval2, ndx, newscnum[sm->n_scnum],
				(*tvndx != -1) ? SV_TV : 0);
}

void
specflags(flgname, argptr)
char *flgname;
char **argptr;
{
	/*
	 * process flag specifications that have fallen through
	 * to default on switch of flgname in ld00.c
	 */

	switch ( *flgname ) {

/*
 * process special flag specification for vax link editor (ld)
 * these flags have fallen through switch of argname in ld00.c
 */

		case 'x':
			xflag++;
			break;

		case 'n':
			break;

		default:
			yyerror("unknown flag: %s", flgname);

		}


}
