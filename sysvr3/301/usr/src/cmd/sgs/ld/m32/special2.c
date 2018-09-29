/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ld:m32/special2.c	1.23"

#include <stdio.h>

#include "system.h"
#include "aouthdr.h"
#include "structs.h"
#include "ldfcn.h"
#include "tv.h"
#include "slotvec.h"
#include "reloc.h"
#include "extrns.h"
#include "sgsmacros.h"
#include "list.h"
#include "sgs.h"

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
	 * for m32 version of ld01.c, the symbol "_start" does not
	 * need to be undefined
	 */
}

void
dfltsec()
{
/*
 * For the m32, the default action of nm32ld is to generate two m32
 * segments: one for the text and the other for the data (both
 * .data and .bss)
 */

	ADDRESS lint = 0x80000;
	ACTITEM *aiptr, *afaiptr, *grptr;

/*
 * If any SECTIONS directives have been input, they take priority,
 * and no default action is taken
 */

	if( bldoutsc.head )
		return;

#if PAGING
	bond_t_d = 1;	/* setting bond_t_d will tell pboslist to bind text
				and data to the needed addresses */
#endif

/*
 * Generate a series of action items, as if the following had been
 * input to m32ld:
 *
 *	SECTIONS {
 *		.text : {*(.init) *(.text) *(.fini)}
 *		GROUP ALIGN(ALIGNSIZE) : {
 *					.data : {}
 *					.bss  : {}
 *					}
 *		 }
 *
 * If the output file is not an absolute file, don't collapse the .init
 * and .fini sections into the text section.  Instead, use the following
 * directives, and any .init or .fini sections will be grouped together
 * automatically.
 *
 *	SECTIONS
 *	{
 *		.text : {}
 *		GROUP ALIGN(ALIGNSIZE) :
 *		{
 *			.data : {}
 *			.bss  : {}
 *		}
 *	}
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

	memorg = 0x80000000L;
	curfilnm = "*m32.default.file*";

	if (!Nflag){
		lineno = 2;
#if PAGING
		aiptr = dfnscngrp( AIDFNSCN, NULL, NULL, NULL);
		/* for PAGING text will be properly bonded later in ld2.c */
#else
		aiptr = dfnscngrp( AIDFNSCN, cnstnode(textorg), NULL, NULL );
#endif
		aiptr->dfnscn.aisctype = STYP_REG;
		copy( aiptr->dfnscn.ainame, _TEXT, 8 );
		aiptr->dfnscn.aifill = 0;
		aiptr->dfnscn.aifillfg = 0;
		if ( aflag )
		{
			afaiptr = bldadfil((char *)0, aiptr);
			bldadscn(_INIT, (char *)0, aiptr);
			bldadscn(_TEXT, (char *)0, aiptr);
			bldadscn(_FINI, (char *)0, aiptr);
			afaiptr->adfile.ainadscs = 3;
		}
		listadd( l_AI, &bldoutsc, aiptr );

		lineno = 3;
		grptr = dfnscngrp( AIDFNGRP, NULL, cnstnode( lint ), NULL );
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
	} else {
		lineno = 2;
		grptr = dfnscngrp( AIDFNGRP, cnstnode( textorg ), NULL, NULL );
		copy( grptr->dfnscn.ainame, "*group*", 7);

		lineno = 3;
		aiptr = dfnscngrp( AIDFNSCN, NULL, NULL, NULL );
		aiptr->dfnscn.aisctype = STYP_REG;
		copy( aiptr->dfnscn.ainame, _TEXT, 8 );
		aiptr->dfnscn.aifill = 0;
		aiptr->dfnscn.aifillfg = 0;
		if ( aflag )
		{
			afaiptr = bldadfil((char *)0, aiptr);
			bldadscn(_INIT, (char *)0, aiptr);
			bldadscn(_TEXT, (char *)0, aiptr);
			bldadscn(_FINI, (char *)0, aiptr);
			afaiptr->adfile.ainadscs = 3;
		}
		listadd( l_AI, &grptr->dfnscn.sectspec, aiptr );

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

	return;
}

void
procspecobj(fdes, filename)
LDFILE *fdes;
char *filename;
{
	/*
	 * No more additional files to check 
	 */

	lderror(1, 0, NULL,
		"file %s is of unknown type: magic number = %06.1x",
		filename, TYPE(fdes));
}

void
adjsize(osp)
OUTSECT *osp;
{
	/*
	 * No special cases, no need to adjust size of output section
	 */
}

void
adjaout(aout)
AOUTHDR *aout;
{
	/*
	 * Indicate, via the MAGIC NUMBER, when the data segment starts
	 * on a m32 segment (512K bytes) boundary
	 */

	if( (aout->data_start & 0x7ffffL) == 0 )
		aout->magic = (short) AOUT2MAGIC;
}

void
relocate(ifd, infl, isp, fdes, rdes, sect_buf, buffer_size)
register LDFILE *ifd;
INFILE *infl;
INSECT *isp;
FILE *fdes, *rdes;
register char *sect_buf;
long buffer_size;
{
	long vaddiff;
	register long rdif;
	RELOC rentry;
	register SLOTVEC *svp;
	long sect_size, chunk_size, bytes_so_far;
	long byte_offset, indx;
	int reloc_read;

	union
	{
		long	 l;
		char	 c[4];
	} value;

	vaddiff = isp->isnewvad - isp->ishdr.s_vaddr;
	sect_size = isp->ishdr.s_size;
	chunk_size = min( sect_size, buffer_size );
	if (fread( sect_buf, chunk_size, 1, ifd ) != 1)
		lderror( 2, 0, NULL, "cannot read section %.8s of file %s",
			isp->ishdr.s_name, infl->flname );
	sect_size -= chunk_size;
	bytes_so_far = 0;
	fseek( ifd, isp->ishdr.s_relptr + infl->flfiloff, 0 );

	for ( reloc_read = 1; reloc_read <= isp->ishdr.s_nreloc; reloc_read++ )
	{
		if (fread( &rentry, RELSZ, 1, ifd ) != 1)
			lderror( 2, 0, NULL, "cannot read relocation entries of section %.8s of %s",
				isp->ishdr.s_name, infl->flname );

		if ((svp = svread( rentry.r_symndx)) == NULL)
		{
			lderror(1, 0, NULL, "relocation entry found for non-relocatable symbol in %s",
				infl->flname);
			continue;
		}

		if (rentry.r_type == R_ABS)
			continue;

		rdif = svp->svnvaddr - svp->svovaddr;
		byte_offset = rentry.r_vaddr - isp->ishdr.s_vaddr;

		if (byte_offset < bytes_so_far){
			if(chunk_size != isp->ishdr.s_size)
				lderror(2,0,NULL, "Reloc entries out of order in section %.8s of file %s",
					isp->ishdr.s_name, infl->flname);
			else
				lderror(1,0,NULL, "Reloc entries out of order in section %.8s of file %s",
					isp->ishdr.s_name, infl->flname);
			}
		while (byte_offset > bytes_so_far + chunk_size)
		{
			bytes_so_far += chunk_size;
			fwrite( sect_buf, chunk_size, 1, fdes );
			chunk_size = min( sect_size, buffer_size );
			fseek( ifd, isp->ishdr.s_scnptr + bytes_so_far + infl->flfiloff, 0 );
			if (fread( sect_buf, chunk_size, 1, ifd ) != 1)
				lderror( 2, 0, NULL, "cannot read section %.8s of %s", isp->ishdr.s_name, infl->flname );
			sect_size -= chunk_size;
			fseek( ifd, infl->flfiloff + isp->ishdr.s_relptr + reloc_read * RELSZ, 0);
		}

		if ((byte_offset + 4 > bytes_so_far + chunk_size) && sect_size)
				/* 4 bytes is the address size, must be sure all
				   address is in buffer */
		{
			bytes_so_far += chunk_size - 4;
			fwrite( sect_buf, chunk_size - 4, 1, fdes );
			sect_buf[0] = sect_buf[chunk_size - 4];
			sect_buf[1] = sect_buf[chunk_size - 3];
			sect_buf[2] = sect_buf[chunk_size - 2];
			sect_buf[3] = sect_buf[chunk_size - 1];
			chunk_size = min( sect_size, buffer_size  - 4);
			fseek( ifd, isp->ishdr.s_scnptr + bytes_so_far + 4 + infl->flfiloff, 0 );
			if (fread( sect_buf + 4, chunk_size, 1, ifd ) != 1)
				lderror( 2, 0, NULL, "cannot read section %.8s of %s", isp->ishdr.s_name, infl->flname );
			sect_size -= chunk_size;
			chunk_size +=4;
			fseek( ifd, infl->flfiloff + isp->ishdr.s_relptr + reloc_read * RELSZ, 0);
		}

		indx = byte_offset - bytes_so_far;
		value.c[0] = sect_buf[indx];
		value.c[1] = sect_buf[indx + 1];
		value.c[2] = sect_buf[indx + 2];
		value.c[3] = sect_buf[indx + 3];

		switch( rentry.r_type ) {

		case R_DIR32:
			value.l = swapb4(swapb4(value.l,0)+rdif,0);
			break;

		case R_DIR32S:
			value.l = swapb4(swapb4(value.l,1)+rdif,1);
			break;

		default:
			lderror( 2, 0, NULL, "Illegal relocation type %d found in section %.8s in file %s",
				rentry.r_type, isp->ishdr.s_name, infl->flname);
		}

		/* put the relocated address back in the buffer */
		sect_buf[indx] = value.c[0];
		sect_buf[indx + 1] = value.c[1];
		sect_buf[indx + 2] = value.c[2];
		sect_buf[indx + 3] = value.c[3];

		/*
		 * Preserve relocation entries
		 */
		if( rflag )
		{
			rentry.r_vaddr += vaddiff;
			rentry.r_symndx = svp->svnewndx;
			fwrite(&rentry, RELSZ, 1, rdes);
		}
	}

	while (sect_size){
		bytes_so_far += chunk_size;
		fwrite( sect_buf, chunk_size, 1, fdes );
		chunk_size = min( sect_size, buffer_size );
		fseek( ifd, infl->flfiloff + isp->ishdr.s_scnptr + bytes_so_far, 0 );
		if (fread( sect_buf, chunk_size, 1, ifd ) != 1)
			lderror( 2, 0, NULL, "cannot read section %.8s of %s", isp->ishdr.s_name, infl->flname );
		sect_size -= chunk_size;
		}
	fwrite( sect_buf, chunk_size, 1, fdes ); 

}

long
swapb4(val, type)
long val;
int type;
{
	register long result;


/*
 * Reorder the bytes of a long, converting to/from UNIX/M32 format
 *
 *	val	: w x y z
 *
 *	11/70	:0:	xwzy
 *		:1:	yzwx
 *	vax	:0:	zyxw
 *		:1:	no change
 *
 *	other	:0:	no change
 *		:1:	zyxw
 */

	if( type ) {
#if AR16WR || AR32W
		result = (val >> 16) & 0x0000FFFFL;
		result |= (val << 16) & 0xFFFF0000L;
#if AR32W
		val = result;
		result = (val >> 8) & 0x00FF00FFL;
		result |= (val << 8) & 0xFF00FF00L;
#endif
#else
		result = val;
#endif
		}
	else {
#if AR16WR || AR32WR
		result = (val >> 8) & 0x00FF00FFL;
		result |= (val << 8) & 0xFF00FF00L;
#if AR32WR
		val = result;
		result = (val >> 16) & 0x0000FFFFL;
		result |=(val << 16) & 0xFFFF0000L;
#endif
#else
		result = val;
#endif
		}

	return( result );

}
