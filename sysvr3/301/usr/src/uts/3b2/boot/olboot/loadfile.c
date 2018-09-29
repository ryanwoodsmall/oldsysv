/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:boot/olboot/loadfile.c	10.3"

#include	"sys/param.h"
#include	"sys/types.h"
#include	"sys/inode.h"
#include	"sys/firmware.h"
#include	"sys/lboot.h"
#include	"a.out.h"

/*
**	loadfile - Load file into memory.
**
**	This routine loads the file, indicated by its inode, into memory
**	at the specified address.
**
**	Return code:  LDPASS if successfully loaded the file;
**		      LDFAIL if failed to load.
*/


unsigned short
loadfile(finode, laddr)
struct inode finode;	/* file's inode */
long laddr;		/* physical address of where to load program */
{
	register int		c,	/* byte count */
				*mp,	/* ptr to memory being loaded */
				tc,	/* byte transfer count for this block */
				*bp;	/* ptr to buffer being copied */
	int 			i,	/* loop count */
				fa;	/* file byte address */
	char			*bmp,	/* byte pointer to memory loaded */
				*bbp;	/* byte pointer to buffer copied */
	extern unsigned short	rb();


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
	if (!(finode.i_size)) {

		/* Zero-length file. */
		PRINTF("File size = 0.\n");
		return(LDFAIL);
	}

	/* Read file into memory at specified address. */

	fa = 0;
	mp = (int *)laddr;

	for (c = finode.i_size; c > 0; c -= tc, fa += tc) {
		bp = (int *)(DATA);
		if (!rb(DATA, fa, &finode))
			return(LDFAIL);
		tc = ((FsBSIZE(fstype) > c) ? c : FsBSIZE(fstype));	/* transfer count */
		for (i = tc / NBPW; i--; *mp++ = *bp++);		/* transfer words */

		/* transfer any remaining bytes */
		bmp = (char *)mp;
		bbp = (char *)bp;
		for (i = tc % NBPW; i--; *bmp++ = *bbp++);

	}
	
	return(LDPASS);
}
