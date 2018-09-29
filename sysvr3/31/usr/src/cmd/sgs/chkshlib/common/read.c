/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)chkshlib:common/read.c	1.4"
#ident	"@(#)chkshlib:read.c	1.1"

#include <stdio.h>
#include <signal.h>
#include "filehdr.h"
#include "ldfcn.h"
#include "string.h"
#include "reloc.h"
#include "syms.h"
#include "scnhdr.h"
#include "chkshlib.h"
#include "aouthdr.h"

void
read_sym_tbl()
{
	LDFILE	 *ldptr = NULL;

	do	{

		if ( (ldptr = ldopen(current->f_name, ldptr)) != NULL)
		{
			read_symbols(ldptr);
		}
		else 	printerr ("Cannot open file", current->f_name);

	} while (ldclose(ldptr) == FAILURE);
}

void
read_symbols(ldptr)
LDFILE	 *ldptr;
{
	long	i;
	symrec	*sptr;
	long	hval;
	char	*name;
	SYMENT	symbol;

	/* store the address bounds of the sections occupied by the file's*/
	/* shared library code into bounds structure associated with file */
	current->f_bounds = store_bounds(ldptr);

	/* seek to the beginning of the symbol table */
	if (ldtbseek(ldptr) != SUCCESS)
		printerr("Cannot find symbol table", current->f_name);

	/* loop on number of symbols in file */
	for (i=0; i < HEADER(ldptr).f_nsyms; i++)
	{
		/* read the symbol from the symbol table into "symbol" */
		if (FREAD(&symbol, SYMESZ, 1, ldptr) != 1)
			printerr("Cannot read symbol table", current->f_name);

		/* if there is symbol auxiliary information, seek past it */
		if (symbol.n_numaux > 0) {
			i+= symbol.n_numaux;
			FSEEK(ldptr, symbol.n_numaux*AUXESZ, 1);
		}

		/* get the symbol name */
		if ( (name = ldgetname(ldptr, &symbol)) == NULL)
			printerr("Cannot retrieve symbol name", current->f_name);

		/* reject symbols that need not be checked 	*/
		/* symbols that DO NEED to be checked are: 	*/
		/* for HOSTS:	extern, absolute non-zero	*/
		/* for EXECS:	extern, absolute non-zero 	*/
		/*		EXCEPT "etext" "edata" "end"	*/
		/* for TARGS:	extern, non-zero EXCEPT "etext"	*/
		/*		"edata" "end" and start ".bt"	*/ 
		if (symbol.n_sclass != C_EXT || symbol.n_value == 0
			|| (head->next->f_type != TARG && symbol.n_scnum != N_ABS)
			|| (head->next->f_type != HOST && (strcmp(name,"etext") == 0
			|| strcmp(name,"edata") == 0 || strcmp(name,"end") == 0))
			|| (head->next->f_type == TARG && strncmp(name,".bt",3) == 0))
			continue;

		/* use the hash function for array index */
		hval= hash(name);

		/* store symbol in structure linked list off hash table */
		sptr = (symrec *)chk_malloc((unsigned)sizeof(symrec));

		sptr->name = stralloc(name);
		sptr->addr= symbol.n_value;
		sptr->next= hashtab[hval];
		hashtab[hval]= sptr;
	}
}

/* store the address bounds of the sections occupied by the file's
/* shared library code */

bounds *
store_bounds(ldptr)
LDFILE *ldptr;
{
	AOUTHDR	ohead;
	SCNHDR	secthead;
	short	count_sections = 1;
	bounds 	*top = (bounds *)NULL;

	/* will read the .fk sections of HOST and .text and .data sections of  */
	/* TARG and store in struct linked list address bounds of file 	       */

	if (head->next->f_type == TARG) { /* file is a TARG */
		/* seek past the file header */
		FSEEK(ldptr, FILHSZ, 0);

		/* if there is an optional header and it can be read 	      */
		/* store the bounds of the text and data sections of the file */
		if (HEADER(ldptr).f_opthdr != 0 &&
			FREAD(&ohead,HEADER(ldptr).f_opthdr,1,ldptr) == 1)
		{
			top = fill_bounds (top, ohead.text_start, ohead.text_start+ohead.tsize);
			top = fill_bounds (top, ohead.data_start, ohead.data_start+ohead.dsize);
		}
		else 	printerr("Trouble reading bounds", current->f_name);
	}
	else 	/* file is a HOST */
	{
		/* read the section header of each section by incrementing number*/
		while (ldshread(ldptr,(unsigned)count_sections,&secthead) == 1) 
		{
			if (strncmp(secthead.s_name,".fk",3)== 0) {
				top = fill_bounds (top, secthead.s_vaddr, secthead.s_vaddr + secthead.s_size);
			} /* end if strncmp */

			/* increment section count */
			count_sections++;

		} /*end while */

	} /* end else */
	return (top);
}

/* store the text and data start address and ending address in a linked list	*/
/* of bounds structures off the "f_bounds" bounds pointer in file_data for file */
bounds *
fill_bounds(top, min, max)
bounds *top;
long min, max;
{
	bounds *bptr;
	
	bptr = (bounds *)chk_malloc ((unsigned)sizeof(bounds));

	bptr->b_min = min;
	bptr->b_max = max;
	bptr->next = top;
	top = bptr;

	return(top);

}
