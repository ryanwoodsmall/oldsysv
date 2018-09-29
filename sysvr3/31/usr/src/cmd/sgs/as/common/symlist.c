/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:common/symlist.c	1.6.1.4"

#include <stdio.h>
#include <syms.h>
#include "symbols.h"
#include "gendefs.h"

extern upsymins lookup();
/*
 *
 *	"symlist.c" is a file containing functions for keeping track
 *	of symbol table indices.  The following functions are provided:
 *
 *	putindx(sym,sclass,index) Remember that the symol "symb->name"
 *			with storage class "sclass" has symbol table
 *			index "index".
 *
 *	getindx(symb,sclass) Return the symbol table index of the
 *			symbol "symb" with storage class "sclass".
 *
 */

extern char
	*malloc();

/*
 *
 *	"alloc" is a function that permanently allocates blocks of
 *	storage.  The storage allocated by this routine is more
 *	economical than that provided by "malloc", however it cannot be
 *	returned to free storage.  Blocks allocated by "alloc" should
 *	never be passed to "free".
 *
 *	This function operates by obtaining large blocks of storage
 *	from "malloc", and then doles it out in small pieces without
 *	keeping around extra bookkeeping information for each small
 *	block.
 *
 *	The size of the blocks obtained from "malloc" is determined by
 *	the static array "incrs".  This contains successively smaller
 *	sizes of blocks to be obtained.  When a block of a given size
 *	cannot be obtained from "malloc", the next smaller size is
 *	tried.  The static variable "incptr" indexes the element of
 *	"incrs" that is currently being used for the size of the block
 *	to be allocated.
 *
 *	The current block from which storage is being allocated is
 *	addressed by the static variable "block".  The number of bytes
 *	remaining to be allocated in this block is given by the static
 *	variable "length".
 *
 *	Storage is allocated by first rounding the size up to an even
 *	number of bytes.  This avoids alignment problems if the storage
 *	is to be used for something other than characters.  If "length"
 *	is insufficient for allocating "size" bytes, another block of
 *	storage is obtained and "length" is set to the size of that
 *	block.  If necessary, a smaller size will be used to allocate
 *	this new block of storage.  Once a block of sufficient size is
 *	present, the "size" is subtracted from the remaining length,
 *	the variable "block" is incremented by the "size", and the
 *	pevious value of "block" is returned.
 *
 */

char *
alloc(size)
	register int size;
{
	register int mod;
	register char *ptr;

	static unsigned incrs[] = { 2048, 1024, 512, 256, 128, 64, 0 };
	static short
		incptr = 0,
		length = 0;

	static char *block;

	/*
	 * Round up "size" to multiple of the wordsize
	 * of the machine the assembler is running on
	 */
	if ((mod = size % sizeof(int)) != 0)
		size += sizeof(int) - mod;

	if (size > length){
		while((block = malloc(incrs[incptr])) == 0){
			if (incrs[++incptr] == 0)
				aerror("Cannot allocate storage");
		}
		length = incrs[incptr];
	}

	ptr = block;
	block += size;
	length -= size;
	return(ptr);
}

/*
 *
 *	"putindx" is a function that remembers the index of a symbol
 *	table entry.  It first checks to see that the symbol has a
 *	storage class indicating that it is either an external or
 *	static external symbol, a structure, union, enumeration tag,
 *	or a shadow symbol .  If it is not one of these, this
 *	function does nothing 
 *	since no one will ever care what its symbol table index is.
 *	Otherwise, a new structure of type "symlist" is created and
 *	initialized using the parameters to this function.  This is
 *	pushed onto the beginning of a list of such structures pointed
 *	to by the "symlst" field in the symbol entry.
 *
 */

putindx(sym,sclass,index)
	register symbol *sym;
	short sclass;
	long index;
{
	register symlist *temp;
        switch ( sclass ) {
        case C_EXT:
        case C_STAT:
        case C_STRTAG:
        case C_UNTAG:
        case C_ENTAG:
		temp = (symlist *)alloc(SYMSIZ);
		temp->index = index;
		temp->sclass = sclass;
		temp->nextelmt = sym->symlst;
		sym->symlst = temp;
	}
}

/*
 *
 *	"getindx" is a function that gets the symbol table index of
 *	a symbol "symb" with storage class "sclass".  It does this
 *	by doing a lookup on "symb", then searching through the list
 *	of structures of type "symlist", starting at the "symlst"
 *	field for the symbol, for an entry with the right storage class.
 *	If the symbol is found, its index is returned.  Otherwise,
 *	-1 is returned to indicate failure.
 *
 */

long
getindx(symb,stclass)
	char *symb;
	register short stclass;
{
	register symlist *ptr;
	symbol *symptr;
	if ((symptr = lookup(symb,N_INSTALL,USRNAME).stp) == NULLSYM)
		return(-1L);
	for (ptr = symptr->symlst; ptr != NULL; ptr = ptr->nextelmt)
		if (ptr->sclass == stclass)
			return(ptr->index);
	return(-1L);
}
