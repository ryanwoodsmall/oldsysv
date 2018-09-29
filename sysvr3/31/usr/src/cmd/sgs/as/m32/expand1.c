/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:m32/expand1.c	1.6.1.3"

#include <stdio.h>
#include "systems.h"
#include "symbols.h"
#include "codeout.h"
#include "section.h"
#include "expand.h"
#include "gendefs.h"
#include "scnhdr.h"
#include <memory.h>

extern rangetag range[];
extern char idelta[];
extern char pcincr[];
extern struct scninfo secdat[];
extern struct scnhdr sectab[];

#ifndef	MAXSS
#define MAXSS	400	/* maximum number of Selection set entries */
#endif
#ifndef	MAXSDI
#define MAXSDI	1000	/* initial maximum number of SDI's we can handle */
#endif
#ifndef	MAXLAB
#define MAXLAB	500	/* init. max. no. labels whose address depends on SDI's */
#endif

#define	ABS_MAXSDI 65000	/* absolute maximum value of unsigned short */
#define	ABS_MAXLAB 32500	/* absolute maximum value of short */

short maxsdi = MAXSDI,		/* dynamic max. for SDI's */
	maxlab = MAXLAB;	/* dynamic max. for labels */

char firstislongsdi[MAXSDI];	/* initial allocation for islongsdi */
char *islongsdi = firstislongsdi;

ssentry selset[MAXSS];

symbol *firstlabset[MAXLAB];	/* initial allocation for labset */
symbol **labset = firstlabset;

unsigned short sdicnt = 0;
static short ssentctr = -1,
		labctr = -1;

#if STATS
short maxssentctr, maxlabctr;
#endif

unsigned short PCmax[NSECTIONS];

extern symbol *dot;
extern long newdot;

update(ssptr,sditype)
	ssentry *ssptr;
	short sditype;
{
	register short cntr;
	register symbol *lptr;
	register short sssectnum;
	register unsigned short delta,
		sdipos;
	long instaddr;

	delta = idelta[ssptr->itype];
	sdipos = ssptr->sdicnt;
	instaddr = ssptr->minaddr;
	sssectnum = ssptr->sectnum;
	PCmax[sssectnum] -= delta;
	if (dot->sectnum == sssectnum)
		dot->maxval -= delta;
	else
		secdat[sssectnum].s_maxval -= delta;
	if (sditype) { /* nonzero is short */
		instaddr += ssptr->maxaddr;
		{ register ssentry *ptr;
		for (cntr = ssentctr, ptr = &selset[0]; cntr-- >= 0; ++ptr) {
			if ((ptr->sdicnt > sdipos) 
				&& (ptr->sectnum == sssectnum))
				ptr->maxaddr -= delta;
		}}
		{ register symbol **ptr2;
		for (cntr = labctr, ptr2 = &labset[0]; cntr-- >= 0; ) {
			lptr = *ptr2;
			if ((lptr->value + lptr->maxval > instaddr) 
				&& (lptr->sectnum == sssectnum)) {
				lptr->maxval -= delta;
				if (lptr->maxval == 0) {
					*ptr2 = labset[labctr--];
					continue;
				}
			}
			ptr2++;
		}}
	}
	else {	/* long */
		if (dot->sectnum == sssectnum) {
			dot->value += delta;
			newdot += delta;
		}
		else
			sectab[sssectnum].s_size += delta;
		islongsdi[sdipos] = (char)delta;
		{ register ssentry *ptr;
		for (cntr = ssentctr, ptr = &selset[0]; cntr-- >= 0; ++ptr) {
			if ((ptr->sdicnt > sdipos)
				&& (ptr->sectnum == sssectnum)) {
				ptr->minaddr += delta;
				ptr->maxaddr -= delta;
			}
		}}
		{ register symbol **ptr2;
		for (cntr = labctr, ptr2 = &labset[0]; cntr-- >= 0; ) {
			lptr = *ptr2;
			if ((lptr->value > instaddr)
				&& (lptr->sectnum == sssectnum)) {
				lptr->value += delta;
				lptr->maxval -= delta;
				if (lptr->maxval == 0) {
					*ptr2 = labset[labctr--];
					continue;
				}
			}
			ptr2++;
		}}
	}
}

/*
 *	"notdone" is used to indicate when pass 1 has been completed.
 *	This helps to detect undefined externals.
 */

static short notdone = YES;

/*
 *	"overflow" is used to indicate when the maximum number of
 *	SDI's have been received (ABS_MAXSDI). When "overflow" becomes
 *	non-zero, then only "expand" is called to optimize the
 *	SDI's that have already been received.
 */

static short overflow = NO;

sdiclass(sdiptr)
	register ssentry *sdiptr;
{
	register symbol *lptr;
	register short itype;
	register short ltype;
	register long span;

	lptr = sdiptr->labptr;
	itype = sdiptr->itype;
	if ((ltype = lptr->styp & TYPE) != UNDEF) {
		if ((ltype != TXT) || (lptr->sectnum != sdiptr->sectnum))
			/* generate long form if label is not of
			 * type TXT or if label and instruction are
			 * in different sections */
			return(L_SDI);
		span = lptr->value;
	}
	else {
		if (notdone == NO)
			return(L_SDI);
		if (dot->sectnum != sdiptr->sectnum)
			return(U_SDI);
		span = (dot->value != sdiptr->minaddr) ? dot->value :
			sdiptr->minaddr + pcincr[itype];
	}
	span += sdiptr->constant - (sdiptr->minaddr + pcincr[itype]);
	if ((span < range[itype].lbound) ||
		(span > range[itype].ubound))	/* definitely long */
		return(L_SDI);
	else {
		if (ltype != UNDEF) {
			span += (int)(lptr->maxval) -
				((lptr->value > sdiptr->minaddr) ?
				(sdiptr->maxaddr + idelta[itype]) : sdiptr->maxaddr);
			if ((span >= range[itype].lbound) &&
				(span <= range[itype].ubound))	/* definitely short */
				return(S_SDI);
		}
	}
	return(U_SDI);
}

expand()
{
	register short change = YES;
	register short cntr;
	register ssentry *current;
	register short sditype;

	while (change) {
		change = NO;
		for (cntr = ssentctr, current = &selset[0]; cntr-- >= 0; ) {
			if ((sditype = sdiclass(current)) != U_SDI) {

				change = YES;
				update(current,sditype);
				*current = selset[ssentctr--];
			}
			else
				current++;
		}
	}
}

punt()
{
	register short cntr;
	register ssentry *current;
	register unsigned firstsdi;
	register ssentry *outptr;

	firstsdi = sdicnt;
	for (cntr = ssentctr, current = &selset[0]; cntr-- >= 0; ++current) {
		if (current->sdicnt < firstsdi) {
			firstsdi = current->sdicnt;
			outptr = current;
		}
	}
	update(outptr,L_SDI);
	*outptr = selset[ssentctr--];
	expand();
}

newlab(sym)
	register symbol *sym;
{
	static short labwarn = YES;

	if (++labctr == maxlab) {
		if (maxlab == MAXLAB) {
			if ((labset = (symbol **)calloc((maxlab *= 2),sizeof(symbol *))) == NULL)
				aerror("Cannot calloc more labels table for span-dependent optimization");
			memcpy(labset,firstlabset,MAXLAB*(sizeof(symbol *)));
		} else if (maxlab <= ABS_MAXLAB/2) {
			if ((labset = (symbol **)realloc(labset,(maxlab *= 2)*sizeof(symbol *))) == NULL)
				aerror("Cannot realloc more labels table for span-dependent optimization");
		} else {
			if (labwarn == YES) {
				werror("Table overflow: some optimizations lost (Labels)");
				labwarn = NO; /* don't warn again */
			}
			labctr--;	/* gone too far, back up */
			while (labctr == maxlab - 1) {
				punt();
			}	/* continue to punt until we free a label */
			labctr++;	/* now point to a free area */
		}
	}
#if STATS
	if (labctr > maxlabctr) maxlabctr = labctr;
#endif
	labset[labctr] = sym;
}

deflab(sym)
	register symbol *sym;
{
	sym->maxval = PCmax[sym->sectnum];
	if (ssentctr >= 0) {
		newlab(sym);
		expand();
	}
}

#if !ONEPROC
extern FILE *fdlong;
#endif

fixsyms()
{
	notdone = NO;
	expand();
#if !ONEPROC
	fwrite(islongsdi,sizeof(*islongsdi),(int)(++sdicnt),fdlong);
#endif
}

sdi(sym,const,itype)
	register symbol *sym;
	long const;
	register short itype;
{
	register long span;

	if (sym) {
		if ((sym->styp & TYPE) == UNDEF)
			return(U_SDI);
		if (((sym->styp & TYPE) != TXT) || 
			(sym->sectnum != dot->sectnum))
			/* generate long form if label is not of
			 * type TXT or if label and instruction are
			 * in different sections */
			return(L_SDI);
	}
	else
		return(L_SDI);

	span = sym->value + const - (dot->value + pcincr[itype]);
	if ((span < range[itype].lbound) ||
		(span > range[itype].ubound))	/* definitely long */
		return(L_SDI);
	else {
		/*
		 *	Now determine if the instruction is definitely
		 *	short. This calculation is different from the
		 *	similar calculation in "sdiclass" since, as an
		 *	optimization, the long form of the instruction
		 *	is not included in the calculation. This prevents
		 *	determining that the size of an instruction
		 *	is uncertain based on its maximum length. Note
		 *	any previous uncertain instruction lengths
		 *	are reflected by "dot->maxval".
		 */
		span += (long)((int)(sym->maxval) - dot->maxval);
		if ((span >= range[itype].lbound) &&
			(span <= range[itype].ubound))	/* definitely short */
			return(S_SDI);
	}
	return(U_SDI);
}

newsdi(sym,const,itype)
	register symbol *sym;
	long const;
	short itype;
{
	register ssentry *nsdi;
	if ((++sdicnt + 1) >= maxsdi) {
		if (maxsdi == MAXSDI) {
			if ((islongsdi = (char *)calloc((maxsdi *= 2),sizeof(char))) == NULL)
				aerror("Cannot calloc more sdi table for span-dependent optimization");
			memcpy(islongsdi,firstislongsdi,MAXSDI*(sizeof(char)));
		} else if (maxsdi <= ABS_MAXSDI/2) {
			if ((islongsdi = (char *)realloc(islongsdi,maxsdi*2)) == NULL)
				aerror("Cannot realloc more sdi table for span-dependent optimization");
			memset(islongsdi+maxsdi,0,maxsdi);
			maxsdi *= 2;
		} else {
			overflow = YES;
		}
	}
	if (++ssentctr == MAXSS) {
		ssentctr--;	/* gone too far, back up */
		punt();
		ssentctr++;	/* one sdi was removed, now point to free area */
	}
#if STATS
	if (ssentctr > maxssentctr) maxssentctr = ssentctr;
#endif
	nsdi = &selset[ssentctr];
	nsdi->sdicnt = sdicnt;
	nsdi->itype = itype;
	nsdi->minaddr = dot->value;
	nsdi->maxaddr = PCmax[dot->sectnum];
	nsdi->constant = const;
	nsdi->labptr = sym;
	nsdi->sectnum = dot->sectnum;
}

shortsdi(sym,const,itype)
	register symbol *sym;
	long const;
	register short itype;
{
	register int sditype=L_SDI;

	if (!overflow) {
		if (sym && ((sym->styp & TYPE) == UNDEF)) {
			if (dot->styp != TXT)
				return(L_SDI);
			sditype = U_SDI;
			newsdi(sym,const,itype);
			dot->maxval = PCmax[dot->sectnum] += idelta[itype];
				/* here we have a forward reference.	*/
			/* shortsdi() used to call expand() in 	*/
			/* all cases, now we exempt the forward */
			/* references since they will stay in	*/
			/* the selection set anyway.		*/
		}
		else {
			if ((sditype = sdi(sym,const,itype)) == U_SDI) {
				if (dot->styp != TXT)
					return(L_SDI);
				if (dot == sym) {
					return(S_SDI);
				}
				newsdi(sym,const,itype);
				dot->maxval =
					(PCmax[dot->sectnum] += idelta[itype]);
				if (ssentctr > 0) expand();
			}
		}
	} else if (ssentctr > 0)
		expand();
	return(sditype);
}
