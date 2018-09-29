/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mkshlib:common/t_mktarget.c	1.5.1.1"

#include <stdio.h>
#include "filehdr.h"
#include "syms.h"
#include "ldfcn.h"
#include "paths.h"
#include "shlib.h"
#include "trg.h"


void
mktarget()
{
	char *mkehsyms();

	mkplt();	/* make the intermediate partially loaded target, tpltnam */

	/* Build the branch table and create btname which will contain the branch
	 * table as well as the .lib section for the target.
	 * Also, modify tpltnam so that each symbol which has a branch table
	 * entry has its name changed in the symbol table of tpltnam to the
	 * destination label of its corresponding jump in the branch table and 
	 * put this modified file in pltname.
	 */
	mkbt();

	/* Create ifile containing export and hide directives for loader */
	ifil3name = mkehsyms();
	
	/* Find undefined symbols (uslst) in other shared libraries and create 
	/* ifile with name=value; pairs	*/
	if (usflag)	/* At least one undefined symbol exists */
		mklib();

	/* Create the target by loading pltname and btname together at the
	 * addresses specified in ifil2name. */
	if (usflag)	/* ifil4name will exist */
	{
		if (execute(ldname,ldname,"-c","-o",trgname,ifil2name,btname,
					pltname,ifil4name,ifil3name,(char *)0))
			fatal("Internal %s invocation failed",ldname);
	}
	else
	{
		if (execute(ldname,ldname,"-c","-o",trgname,ifil2name,btname,
					pltname,ifil3name,(char *)0))
			fatal("Internal %s invocation failed",ldname);
	}
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

/*  mkehsyms

	Create separate ifile holding exported and hidden symbols.
	This file is passed to ld in mktarget routine. 
	Ifile will be saved for host processing.

									*/
char *mkehsyms()
{
	int 	e,h;
	FILE	*ifil;

	/* open ifil3name */

	if ((nexpsyms > 0) || (nhidesyms > 0) || (allhide == TRUE) || (allexp == TRUE))
	{
		if ((ifil = fopen(ifil3name,"w")) == NULL)
			fatal("Cannot open ifile for export and hide directives - temp file");
	
		/* Place all exported and hidden symbols in ifile.  This file */
		/* will be saved and used to hide and export symbols in the host */

		if (allexp == TRUE)
		{
			fprintf (ifil,"EXPORT { \* }\n");
		}
		for (e=0; e<nexpsyms; e++)
		{
			fprintf (ifil,"EXPORT { %s }\n", expsyms[e]);
		}

		if (allhide == TRUE)
		{
			fprintf (ifil,"HIDE { \* }\n");
		}
		for (h=0; h<nhidesyms; h++)
		{
			fprintf (ifil,"HIDE { %s }\n", hidesyms[h]);
		}
		(void)fclose(ifil);

		return (ifil3name);	/* Return name of ifile */
	}
	else
	{
		return ((char *)0);	/* No ifile created, this will null */
					/* terminate line early		    */
	}
}
