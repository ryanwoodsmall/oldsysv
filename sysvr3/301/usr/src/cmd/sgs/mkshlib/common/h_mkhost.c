/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mkshlib:common/h_mkhost.c	1.2"

#include <stdio.h>
#include "filehdr.h"
#include "reloc.h"
#include "syms.h"
#include "scnhdr.h"
#include "ldfcn.h"
#include "shlib.h"
#include "hst.h"


/* mkhost() creates the host shared library.  The creation of the host shared library
 * requires essentially 3 steps:
 *	1. Creation of the library definition object.
 *	2. Modification of the library's constituent .o files, including the 
 *	   routine.
 *	3. Actual creation of the host archive.
 */

mkhost()
{
	long	i;

	init_trgsymtab();	/* initialize the target symbol hash table */

	defsuffix= getdsuf();	/* get the suffix for the file definition symbols */
	libdefsym= mkdefsym(defname, defsuffix);  /* create libdefsym */
	mklibdef();	/* make the library definition file */

	/* allocate the objects array */
	if ((objects= (obj **)malloc((unsigned)(sizeof(obj *) * numobjs))) == NULL)
		fatal("Out of space");

	/* for each object file used to build the target record all pertinent
	 * information in objlst */
	for (i=0; i<numobjs; i++)
		readobj(i);

	refchain();	/* trace the unresolved reference chains and firm up the
			 * links with file definition symbols */

	checkinits();	/* associate .init sections with the proper objects */
	creatobjs();	/* create the modified constituent object files */
	archive();	/* archive the mess */
}
