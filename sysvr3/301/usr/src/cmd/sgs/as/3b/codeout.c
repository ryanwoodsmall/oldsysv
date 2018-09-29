/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:3b/codeout.c	1.4"
#include <stdio.h>
#include "systems.h"
#include "symbols.h"
#include "codeout.h"
#include "gendefs.h"
#include "temppack.h"
#include "section.h"

extern FILE	*fdsect;	/* file written to by codgen */

extern symbol *dot,
	symtab[];	/* internal symbol table */

extern struct scninfo secdat[];		/* section info table */

long	newdot;		/* up-to-date value of dot */

FILE	*fdcode;	/* temp file containing intermediate language */

#if DCODGEN /* direct code generation of arbitrary number of bytes */

codgen(nbits, val)
	register short nbits;
	long val;
{
	if (nbits) {
		if (nbits % BITSPBY)
			aerror("illegal code generation");
		newdot += nbits / BITSPBY;
		val <<= (sizeof(long)-sizeof(int))*BITSPBY;
		fwrite((char *)&val,(nbits/BITSPBY),1,fdsect);
	};
}

#else

static short
	poscnt = 0,	/* current bit position in outword buffer */
			/* spans from [0 - BITSPOW]	*/
	bitpos = 0;	/* bit position within a byte in outword buffer	*/
			/* spans from [0 - BITSPBY]	*/

static OUTWTYPE	outword;	/* output buffer - see gendefs.h */
static int mask[] = { 0, 1, 3, 7,
		017, 037, 077,
		0177, 0377, 0777,
		01777, 03777, 07777,
		017777, 037777, 077777,
		0177777 };
codgen(nbits, val)
	register short nbits;
	long val;
{
	register short size;	/* space remaining in outword buffer */
	register long value;	/* working buffer for val */

	if (nbits) {
	    start:	value = val;
		/* generate as much of nbits as possible */
		if ((size = BITSPOW - poscnt) > nbits)
			size = nbits;
		value >>= nbits - size;	/* align value to get correct bits */
		value &= mask[size];
		value <<= BITSPOW - poscnt - size;
		outword |= (OUTWTYPE)value;	/* store field in the buffer */
		poscnt += size;
		bitpos += size;
		newdot += bitpos / BITSPBY;
		bitpos %= BITSPBY;
		if (poscnt == BITSPOW) {	/* is buffer full? */
			OUT(outword,fdsect);	/* see gendefs.h */
			poscnt = 0;
			outword = 0;
			if (nbits > size) {	/* more bits? */
				nbits -= size;
				goto start;
			}
		}
	}
}
#endif	/* DCODGEN */

extern int (*(modes[]))();	/* array of action routine functions */

extern upsymins *lookup();

extern short sttop;

codout(file,start,sect)
char *file;
long start;
int sect;
{
	codebuf code;
	register symbol *sym;
	register long	pckword;
	long 		pckbuf;

	dot = (*lookup(".",INSTALL,USRNAME)).stp;
	dot->value = newdot = start;
	dot->styp = secdat[sect].s_typ;
	if ((fdcode = fopen(file, "r")) == NULL)
		aerror("Unable to open temporary file");

#ifndef DCODGEN
	poscnt = 0;
#endif
	while (1) {
		/* get a codebuf structure from the packed temp file */
		if (fread(&pckbuf, sizeof(long), 1, fdcode) != 1) break;
		pckword = pckbuf;
		code.caction = ACTNUM(pckword);
		code.cnbits = NUMBITS(pckword);
		if (pckword & SYMINDEX)
			code.cindex = SYMORVAL(pckword);
		else
			code.cindex = 0;
		if (pckword & VAL0)
			code.cvalue = 0;
		else if (pckword & VAL16)
			code.cvalue = SYMORVAL(pckword);
		else if (fread(&(code.cvalue), sizeof(long), 1, fdcode) != 1)
			aerror("Bad temporary file format");

		if (code.caction != 0) {
			if (code.caction <= NACTION) {
#if VAX
			sym = (code.cindex < 0) ? (symbol*)(-code.cindex)
				: (symbol*)(code.cindex);
#else
			sym = code.cindex ? (symtab + (code.cindex-1))
				: (symbol *) NULL;
#endif
			/* call to appropriate action routine */
			(*(modes[code.caction]))(sym,&code);
			}
			else	aerror("Invalid action routine");
		}
		/* do NOACTION case or handle any end-of-action processing */
		codgen((short)(code.cnbits),code.cvalue);
	}

	if (sttop > 0)
		aerror("Unbalanced Symbol Table Entries-Missing Scope Endings");
	fclose(fdcode);
}


/* get a codebuf structure from the packed temp file */
getcode(codptr)
codebuf	*codptr;
{
	long	pckbuf;

	if (fread(&pckbuf, sizeof(long), 1, fdcode) != 1) return(0);
	codptr->caction = ACTNUM(pckbuf);
	codptr->cnbits = NUMBITS(pckbuf);
	if (pckbuf & SYMINDEX)
		codptr->cindex = SYMORVAL(pckbuf);
	else
		codptr->cindex = 0;
	if (pckbuf & VAL0)
		codptr->cvalue = 0;
	else if (pckbuf & VAL16)
		codptr->cvalue = SYMORVAL(pckbuf);
	else if (fread(&(codptr->cvalue), sizeof(long), 1, fdcode) != 1)
		aerror("Bad temporary file format");
	return(1);
}
