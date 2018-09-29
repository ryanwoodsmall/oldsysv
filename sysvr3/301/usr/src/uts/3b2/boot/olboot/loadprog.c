/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:boot/olboot/loadprog.c	10.5"

#include	"sys/param.h"
#include	"sys/types.h"
#include	"sys/inode.h"
#include	"sys/firmware.h"
#include	"sys/lboot.h"
#include	"a.out.h"

/*
**	loadprog - Load program into memory.
**
**	This routine loads the program, indicated by its inode, into memory
**	at the address in the file header, if:
**	  1. it is a plain, executable file
**	  2. sizes and addresses of text, data, and bss don't overlap or overlay
**		lboot
**	  3. no errors occur on the load
**
**	Return code:  LDPASS if successfully loaded the program;
**		      LDFAIL if failed to load for the reasons mentioned.
*/


unsigned short
loadprog(finode)
struct inode finode;	/* boot program inode */
{
	register int		c,	/* byte count */
				*mp,	/* ptr to memory being loaded */
				tc,	/* byte transfer count for this block */
				*bp;	/* ptr to buffer being copied */
	short			*xmp,	/* ptr to memory being loaded if x86 */
				*xbp;	/* ptr to buffer being copied if x86 */
	register struct scnhdr	*shp;	/* ptr to section header */
	register struct filehdr	*fhp;	/* ptr to file header */
	int			i,	/* loop count */
				nsect,	/* number of sections to check-load */
				fa;	/* file byte address */
	extern unsigned short	rb();
	extern bootstartaddr;


	if (!(finode.i_nlink)) {

		/* Unallocated inode. */
		PRINTF("File link count = 0.\n");
		return(LDFAIL);
	}
	if ((finode.i_mode & IFMT) != IFREG) {

		/* Not regular file. */
		PRINTF("Not a regular file.\n");
		return(LDFAIL);
	}
	if (!(finode.i_mode & 0111)) {

		/* Not executable. */
		PRINTF("File not executable.\n");
		return(LDFAIL);
	}
	if (!(finode.i_size)) {

		/* Zero-length file. */
		PRINTF("File size = 0.\n");
		return(LDFAIL);
	}

	/* Read file and section headers and verify them. */
	if (!rb(AHDR, 0, &finode))
		return(LDFAIL);
	fhp = (struct filehdr *)(AHDR);
	if ((fhp->f_magic != FBOMAGIC) && (fhp->f_magic != X86MAGIC)) {

		/* Not MAC32 or X86 format. */
		PRINTF("Not MAC32 or X86 magic.");
		return(BADMAGIC);
	}

	/* LOAD file sections */
 	bootstartaddr = 0x7fffffff;

	nsect = fhp->f_nscns;	/* number of sections to load */
	fa = sizeof(struct filehdr)+fhp->f_opthdr+fhp->f_nscns * sizeof(struct scnhdr);
	shp = (struct scnhdr *) (((long)(fhp +1)) + fhp->f_opthdr);

	if (fhp->f_magic == FBOMAGIC) {
	    for (nsect = fhp->f_nscns; nsect > 0; nsect--, shp++) {
		/*PRINTF("fhp = %x, shp = %x, fa = %x, nsec = %x\n",fhp,shp,fa,nsect);*/
		/*PRINTF("Section %s, paddr %x, size %x, flags %x\n",shp->s_name,shp->s_paddr,shp->s_size,shp->s_flags);*/
		if ((shp->s_flags & STYP_NOLOAD) != 0 || (shp->s_scnptr == 0)) {
                        continue;
		}
		if ((shp->s_flags & STYP_INFO) != 0)
			continue;

		mp = (int *)shp->s_paddr;	/* physical mem load address */
 		if (((int)mp < bootstartaddr) && ((int)mp > 0x2000000))
 			bootstartaddr = (int)mp;

		for (c = shp->s_size; c > 0; c -= tc, fa += tc) {
			tc = FsBSIZE(fstype) - (fa % FsBSIZE(fstype));	/* transfer count */
			bp = (int *)(DATA + FsBSIZE(fstype) - tc);
			if (!rb(DATA, fa, &finode))
				return(LDFAIL);
			if (tc > c)
				tc = c;
			for (i = tc / NBPW; i--; *mp++ = *bp++);
		}
	    }
	}
	else {
	    for (nsect = fhp->f_nscns; nsect > 0; nsect--, shp++) {
		if ((shp->s_flags & STYP_NOLOAD) != 0 || (shp->s_scnptr == 0)) {
                    continue;
		}
		if ((shp->s_flags & STYP_INFO) != 0)
			continue;

		xmp = (short *)shp->s_paddr;	/* physical mem load address */
 		if ((int)xmp < bootstartaddr)
 			bootstartaddr = (int)xmp;

		for (c = shp->s_size; c > 0; c -= tc, fa += tc) {
		    tc = FsBSIZE(fstype) - (fa % FsBSIZE(fstype));	/* transfer count */
		    xbp = (short *)(DATA + FsBSIZE(fstype) - tc);
		    if (!rb(DATA, fa, &finode))
			return(LDFAIL);
		    if (tc > c)
			tc = c;
		    for (i = tc / 2; i--; *xmp++ = *xbp++);
		}
	    }
	}


	/*PRINTF("Load Complete!\n");*/
	return(LDPASS);
}
