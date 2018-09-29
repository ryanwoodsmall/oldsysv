/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mkshlib:m32/t_btassem.c	1.2"

#include <stdio.h>
#include "filehdr.h"
#include "ldfcn.h"
#include "shlib.h"
#include "trg.h"


/* This function prints the leading lines of assemble code for the branch table */
void
prlbtassem(assembt)
FILE	*assembt;
{
	/* Print leading lines of assembly file.
	 * The beginning of the assemble file should look like:
	 *
	 *	.file	"<filename>"
	 *	.ident	"<ident string>"  #if ident string exists
	 *	.text
	 */
	(void)fprintf(assembt,".file\t\"branchtab\"\n");
	if (idstr != 0)
		(void)fprintf(assembt,".ident\t\"%s\"\n",idstr);
	(void)fprintf(assembt,".text\n");
}


/* This function generates the assembly code for a label in the branch table */
void
labelassem(symname, assembt)
char	*symname;
FILE	*assembt;
{
	(void)fprintf(assembt,".globl\t%s\n%s:\n",symname,symname);
}

/* This function generates the assembly code for the branch table */
void
genbtassem(label, assembt)
char	*label;
FILE	*assembt;
{
	/* Each entry in the branch table will have the following format:
	 *
	 *		jmp	<label name>
	 */
	(void)fprintf(assembt,"\tjmp\t%s\n",label);
}


/* This function generates the assembly code for the .lib section */
void
genlibassem(assembt)
FILE	*assembt;
{
	int	size;	/* length of target pathname */
	int	i;
	char	*p;
	int	slong,
		len;

	/* The .lib contents will have the following format:
	 *
	 *	.section	.lib,"l"
	 *	.word	num,2	# where num is the size of the current entry
	 *			# i.e. length of the target pathname + other info.
	 *	.byte	<target pathname in hex>
	 */
	(void)fprintf(assembt,".section\t.lib,\"l\"\n");
	len= strlen(trgpath)+1;
	slong= sizeof(long);
	size= (len+(slong-1))/slong + 2; 	/* the last term represents the two
						   leading words- the 1st describes
						   the total length and the 2nd
						   describes the offset of the
						   non-optional information */
	(void)fprintf(assembt,".word\t%d,2\n",size);
	(void)fprintf(assembt,".byte\t");
	for (p=trgpath;*p!='\0';p++) {
		(void)fprintf(assembt,"0x%x,",*p);
	}
	(void)putc('0',assembt);
}
