/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:common/symbols.c	1.9.1.1"
/*
 */

#include <stdio.h>
#include <string.h>
#include "syms.h"
#include "systems.h"
#include "symbols.h"
#include "symbols2.h"
#include <sys/types.h>
#include <sys/stat.h>
/*
 *	"symbols.c" is a file containing functions for accessing the
 *	symbol table.  The following functions are provided:
 *
 *	newent(string)
 *		Creates a new symbol table entry for the symbol with name
 *		"string".
 *
 *	lookup(string,install_flag,user_name_flag)
 *		Looks up the symbol whose name is "string" in the symbol
 *		table and returns a pointer to the entry for that symbol.
 *		Install_flag is INSTALL if symbol is to be installed,
 *		N_INSTALL if it is only to be looked up.  User_name_flag
 *		is USRNAME if the symbol is user-defined, MNEMON if it is
 *		an instruction mnemonic.
 *
 *	traverse(func)
 *		Goes through the symbol table and calls the function "func"
 *		for each entry.
 *
 *	creasyms()
 *		Enters the instruction mnemonics found in instab[] into
 *		the instruction hash table.  Then it malloc()'s the
 *		symbol table and hash symbol table.
 *
 *	addstr(string)
 *		Enters the "string" into the string table.  Called by
 *		newent().  Space for the string table is initially
 *		malloc()-ed in strtabinit().  If "string" would exceed
 *		the available space, then the string table is realloc()-ed
 *		with a larger size.  This procedure is only used in the
 *		flexnames version of the assembler.
 *
 *	strtabinit()
 *		Sets up the string table, with space malloc()-ed.  This
 *		procedure is only used in the flexnames version of the
 *		assembler.
 *
 */


/* The symbol table is actually an array of pointers to tables. 
 * "tablesize" is the number of symbols each table can hold.
 */
unsigned long tablesize;
#define NTABLES 20
symbol *symtab[NTABLES];

short tabletop = 0; /* index in symtab of next table to allocate */

unsigned long numsyms = NSYMS;	/* current max. number of symbols */
unsigned long numhash = NSYMS;	/* size of hash table */

unsigned long symcnt;	/* index of next free symbol entry */

symbol **symhashtab;	/* hash table for symbols */
/* instr **insthashtab;	hash table for instructions */

/* static allocations for tables - may turn out too small */
symbol statsymtab[NSYMS];
symbol *statsymhashtab[NSYMS];

char *malloc(),
	*calloc();
#if FLEXNAMES
char	*strtab;
long	currindex;

char	*realloc();
#endif

symbol *
newent(strptr)
	register char *strptr;
{
	register symbol *symptr;
	register char *ptr1;
	if (symcnt >= numsyms)
		incr_symtab();
	GETSYMPTR(symcnt,symptr);
	symcnt++;
#if FLEXNAMES
	if (strlen(strptr) > SYMNMLEN)
	{
		symptr->_name.tabentry.zeroes = 0L;
		symptr->_name.tabentry.offset = currindex;
		addstr(strptr);
	}
	else
#endif
		for (ptr1 = symptr->_name.name; *ptr1++ = *strptr++; )
			;
	return(symptr);
}

#if STATS
unsigned long numids,	/* number of identifiers entered */
	numretr,	/* number of retrieval lookups */
	numlnksretr,	/* number of links traversed (collisions)
			in retrievals */
	numins,		/* number of insertion lookups */
	numlnksins,	/* number of links traversed (collisions)
			in insertions */
	numreallocs,	/* number of reallocs of symbol table */
	inputsz;	/* number of bytes of input */
#endif

upsymins
lookup(sptr,install,usrname)
	char *sptr;
	BYTE install;
	BYTE usrname;
{
	register symbol *p, *q = NULL;
	register char
		*ptr1,
		*ptr2;
	unsigned long register
		ihash = 0,
		hash;
	upsymins retval;

/* 
if (!usrname) {
search thru insthashtab[].  The m32 and 3b20 assemblers have been
reworked so that instruction lookups aren't done here, so we don't
need to worry about this.  We can wait for cplu 5.0 to add this code
for other assemblers.
}
*/

/* search for user name in symhashtab[] */
#if STATS
if (install)
	numins++;
else
	numretr++;
#endif

	/* hash sptr */
	ptr1 = sptr;
	while (*ptr1) {
		ihash = ihash*4 + *ptr1++;
	}
	ihash += *--ptr1 * 32;
	hash = ihash % numhash;

	/* Search thru chain.  If symbol with correct name is found,
	 * return pointer to it.  Otherwise fall thru. */
	for (p = symhashtab[hash]; p!=NULL; p=p->next) {
#if FLEXNAMES
		/* Compare the string given with the symbol string.	*/
		if (strcmp(sptr,(p->_name.tabentry.zeroes != 0) ? (p->_name.name) : (&strtab[p->_name.tabentry.offset])) == 0)
#else
		for (ptr1=sptr,ptr2=p->_name.name; *ptr2==*ptr1++; )
			if (*ptr2++ == '\0')
#endif
			{
				retval.stp = p;
				return(retval);
			}
		q = p;
#if STATS
		if (install)
			numlnksins++;
		else
			numlnksretr++;
#endif
	} /* for (p */

	/* Not found, so install new symbol entry if flag set. */
	if (install) {
#if STATS
		numids++;
#endif
		if (q) 
			q->next = p = newent(sptr);
		else
			symhashtab[hash] = p = newent(sptr);
	} else
		p = NULL;
	retval.stp = p;
	return(retval);
}

traverse(func)
	int (*func)();
{
	register short table, index;

	for (table=0; table<tabletop-1; table++)
		for (index=0; index < tablesize; ++index)
			(*func)(&symtab[table][index]);
	for (index=0; index <= (symcnt-1) % tablesize; ++index)
		(*func)(&symtab[table][index]);
}

/* creasyms() sets the initial size of the symbol table and symbol
hash table to either (1) a statically determined initial value, or
(2) an estimated size, based on the input file size, if (1) appears
too small.  We bias the calculation to use the statically allocated
tables by a factor of 3 so that we malloc() new initial tables only
if we are sure that the static sizes are way off.
*/
extern instr instab[];
extern FILE *fdin;

/* SYMFACTOR is an empirically determined average ratio of number of
 * bytes of input to number of symbols. */
#define SYMFACTOR 75
creasyms()
{
	struct stat buf;
	fstat(fileno(fdin),&buf);
#if STATS
	inputsz = buf.st_size;
#endif
	tablesize = buf.st_size/SYMFACTOR;
	if (tablesize > NSYMS*3) {
		/* reinitialize tables to larger values */
		/* make tablesize odd for hashing */
		numsyms = numhash = (tablesize += tablesize % 2 + 1);
		if ((symtab[tabletop++] = (symbol *)calloc(numsyms,sizeof(symbol))) == NULL)
			aerror("Cannot malloc symbol table");
		if ((symhashtab = (symbol **)calloc(numhash,sizeof(symbol *))) == NULL)
			aerror("Cannot malloc symbol hash table");
	} else {
		/* just use statically allocated tables */
		tablesize = NSYMS;
		symtab[tabletop++] = statsymtab;
		symhashtab = statsymhashtab;
	}
}

#if FLEXNAMES
long	size,
	basicsize = 4 * BUFSIZ;

addstr(strptr)
	register char	*strptr;
{
	register int	length;

	length = strlen(strptr);
	if (length + currindex >= size)
		if ((strtab = realloc(strtab,(unsigned)(size += basicsize))) == NULL)
			aerror("cannot realloc string table");
	strcpy(&strtab[currindex],strptr);
	currindex += length + 1;
}	/* addstr(strptr) */

strtabinit()
{
	if ((strtab = malloc((unsigned)(size = basicsize))) == NULL)
		aerror("cannot malloc string table");
	currindex = 4;
}	/* strtabinit() */
#endif

/* Increase size of symbol table by tablesize. */
static incr_symtab()
{
#if STATS
	numreallocs++;
#endif
	if (tabletop >= NTABLES) {
		aerror("Out of symbol table");
	}
	if ((symtab[tabletop++] = (symbol *)calloc(tablesize,sizeof(symbol))) == NULL)
		aerror("Cannot malloc more symbol table");
	numsyms += tablesize;
}
