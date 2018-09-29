/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
#include "sys/mmu.h"
#include "sys/page.h"

extern char *pmavad;
extern long *pmapte;

int
piget(pad)
{
	register b;

	b = pad & PFNMASK;
	*pmapte = b|(PG_V|PG_KR);
	lmr(EIA, pmavad);
	return (pmavad[pad&0x1ff]);
}

piput(pad, val)
char val;
{
	register b;

	b = pad & PFNMASK;
	*pmapte = b|(PG_V|PG_KW);
	lmr(EIA, pmavad);
	pmavad[pad&0x1ff] = val;
}
