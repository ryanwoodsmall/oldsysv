/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mkshlib:common/t_mktarget.c	1.3"

#include <stdio.h>
#include "filehdr.h"
#include "ldfcn.h"
#include "paths.h"
#include "shlib.h"
#include "trg.h"


void
mktarget()
{
	mkplt();	/* make the intermediate partially loaded target, tpltnam */

	/* Build the branch table and create btname which will contain the branch
	 * table as well as the .lib section for the target.
	 * Also, modify tpltnam so that each symbol which has a branch table
	 * entry has its name changed in the symbol table of tpltnam to the
	 * destination label of its corresponding jump in the branch table and 
	 * put this modified file in pltname.
	 */
	mkbt();

	/* Create the target by loading pltname and btname together at the
	 * addresses specified in ifil2name. */
	if (execute(ldname,ldname,"-c","-o",trgname,ifil2name,btname,
								pltname,(char *)0))
		fatal("Internal %s invocation failed",ldname);
}


void
mkplt()
{
	int	i;
	FILE	*ifil;

	/* open ifil1name */
	if ((ifil= fopen(ifil1name,"w")) == NULL)
		fatal("Cannot open temp file");

	/* now place all the input object files in ifil1name */
	for (i=0; i<numobjs; i++)
		(void)fprintf(ifil,"%s\n",trgobjects[i]);
	(void)fclose(ifil);

	if (execute(ldname,ldname,"-r","-o",tpltnam, ifil1name,(char *)0))
		fatal("Internal %s invocation failed",ldname);
}
