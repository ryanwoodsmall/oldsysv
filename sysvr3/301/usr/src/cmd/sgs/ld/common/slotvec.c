/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ld:common/slotvec.c	1.14"
#include "system.h"

#include <stdio.h>
#include "structs.h"
#include "paths.h"
#include "slotvec.h"
#include "extrns.h"


/********************************************************************
 *
 *  This module administers the slot vector used during output of the
 *  object file.  A slot is used to store the information needed to permit
 *  the relocation of all references to a single symbol.
 *
 *  Using a slot vector prevents the link editor from having to look
 *  up each symbol in the global symbol table in order to relocate it.
 *
 *  The size of the slot vector is dependant on the size of the largest
 *  input symbol table, since it is rebuilt for every input object file.
 *
 ********************************************************************/

SLOTVEC *svpageptr = NULL;		/* pointer to "page-in-core"
						array of slots */
unsigned svhighslt = 0;			/* index of next available slot */
static long svhighndx = 0L;		/* sym. tbl index of symbol assigned to
					    last assigned slot */
/*eject*/
svinit(numslots)
long numslots;		/* number of slots that are needed */
{
    char * calloc();

/*
 * Allocate a slot vector.
 */

	register int numpages;

		if( (svpageptr = (SLOTVEC *) calloc((unsigned)numslots, (unsigned)SLOTSIZ)) == NULL ) {
			lderror(2,0,NULL,"fail to allocate %ld bytes for slotvec table",
			(long) numslots * (long) SLOTSIZ);
			}
}


svcreate(symindex, ovaddr, nvaddr, newindex, secnum, flags )
long symindex, ovaddr, nvaddr, newindex;
int secnum,flags;
{

/*
 * Get and initialize the next available slot in the slot vector
 */

	register SLOTVEC *p;

	p = GETSLOT(svhighslt++, WRTN);

	svhighndx = symindex;

	p->svsymndx = symindex;
	p->svovaddr = ovaddr;
	p->svnvaddr = nvaddr;
	p->svnewndx = newindex;
	p->svsecnum = secnum;
	p->svflags = flags;

}
/*eject*/
SLOTVEC *
svread(symndx)
long symndx;
{

/*
 * Return the pointer to the slot belonging to the symbol whose 
 * old (e.g., input) symbol table index is given by "symndx"
 *
 * Return NULL upon a failure to find a slot
 */

	register SLOTVEC *p;
	register unsigned high, low, diff, guess;

/*
 * Optimize: see if asking for the slot of the last symbol given a slot
 */

	if( symndx == svhighndx )
		return(GETSLOT(svhighslt-1, RNLY));

/*
 * Perform a binary search on the slot vector, looking for the 
 * requested slot
 */

	high = svhighslt - 1;
	low = 0;

	while( (diff = high - low) != 0 ) {
		guess = low + (diff / 2);
		p = GETSLOT(guess, RNLY);
		if( p->svsymndx == symndx )
			return(p);
		if( p->svsymndx > symndx ) { 
			if( high == guess )
				break;		/* not found */
			high = guess;
			}
		else {
			if( low == guess )
				break;		/* not found */
			low = guess;
			}
		}

/*
 * ERROR: the specified symbol was never assigned a slot
 */

	return( NULL );

}
