/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)chkshlib:common/check.c	1.3.1.1"

#include <stdio.h>
#include <signal.h>
#include "filehdr.h"
#include "ldfcn.h"
#include "string.h"
#include "reloc.h"
#include "syms.h"
#include "scnhdr.h"
#include "chkshlib.h"

void
do_check()
{
	LDFILE	 *ldptr = NULL;

	do	{
		/* if verbose not set, stop after first incompatibility */
		if ( (!(vflag)) && (incompatible_flag) ) break;

		if ( (ldptr = ldopen(head->f_name, ldptr)) != NULL)
		{
			checks(ldptr);
		}
		else 	printerr ("Cannot open file", head->f_name);

	} while (ldclose(ldptr) == FAILURE);

	if (!(incompatible_flag)) print_result(SUCCESS);
	else 	print_result(FAILURE);
}

void
checks(ldptr)
LDFILE	*ldptr;
{
	/* check file's symbols against hash table (other file's symbols) */

	long	i;
	symrec	*sptr;
	char	*name;
	SYMENT	symbol;
	int 	bounds_ok;
	bounds 	*bptr;

	/* seek to the beginning of the symbol table */
	if (ldtbseek(ldptr) != SUCCESS)
		printerr("Cannot find symbol table", head->f_name);

	/* loop on number of symbols in file */
	for (i=0; i < HEADER(ldptr).f_nsyms; i++)
	{
		/* read the symbol from the symbol table into "symbol" */
		if (FREAD(&symbol, SYMESZ, 1, ldptr) != 1)
			printerr("Cannot read symbol table", head->f_name);

		/* if there is symbol auxiliary information, seek past it */
		if (symbol.n_numaux > 0)
		{
			i+= symbol.n_numaux;
			FSEEK(ldptr, symbol.n_numaux*AUXESZ, 1);
		}

		/* get the symbol name */
		if ((name= ldgetname(ldptr, &symbol)) == NULL)
			printerr("Cannot retrieve symbol name", head->f_name);

		/* reject symbols that need not be checked 	*/
		/* symbols that DO NEED to be checked are: 	*/
		/* for HOSTS:	extern, absolute non-zero	*/
		/* for EXECS:	extern, absolute non-zero 	*/
		/*		EXCEPT "etext" "edata" "end"	*/
		/* for TARGS:	extern, non-zero EXCEPT "etext"	*/
		/*		"edata" "end" and start ".bt"	*/ 
		if (symbol.n_sclass != C_EXT || symbol.n_value == 0
			|| (head->f_type != TARG && symbol.n_scnum != N_ABS)
			|| (head->f_type != HOST && (strcmp(name,"etext") == 0
			|| strcmp(name,"edata") == 0 || strcmp(name,"end") == 0))
			|| (head->f_type == TARG && strncmp(name,".bt",3) == 0))
			continue;

		/* look up name in the hash table	 */
		sptr = look_up (name);

		if (sptr == NULL) 	/* current symbol not in hash table */
		{
			/* check current symbol is within bounds of other file */
			bounds_ok = FALSE;
			for (bptr=current->f_bounds; bptr != NULL; bptr=bptr->next) {
				if ((bptr->b_min<= symbol.n_value ) &&
					(symbol.n_value <= bptr->b_max)) 
				{
					/* is within bounds */
					bounds_ok = TRUE;
					break;
				}
			}
			if (bounds_ok)	{	/* WITHIN BOUNDS needs to match */

				if (sptr == NULL) {
					print1("Unmatched defined symbol",name,symbol.n_value);
					/* if not verbose, stop after first error */
					if (!(vflag)) 	return;
					else 	continue;
				}
			}			
			else {	/* OUT OF BOUNDS need not match */
				if (bflag) printwarn("Symbol out of bounds", name, symbol.n_value);
				continue;
			}

		} /* end if current symbol not in hash table */

		/* current symbol within hash table - CHECK VALUE */
		if (sptr->addr != symbol.n_value)
		{
			print1_2("Incompatibility",name,symbol.n_value, sptr->addr);

			/* if not verbose, return after first incompatiblity */
			if (!(vflag)) return;
		}
	}
}

/* look_up the symbol name in the hashtab */
symrec *
look_up (name)
char *name;
{
	symrec *sptr;
	long	hval;

	/* use the hash function for array index */
	hval= hash(name);

	/* search the linked list til you find the name */
	for (sptr= hashtab[hval]; sptr != (symrec *)0; sptr= sptr->next)
	{
		if (strcmp(name,sptr->name) == 0)
			break;
	}
	return(sptr);
}
