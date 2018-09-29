/*	@(#)expand2.c	2.2	*/

#include "symbols.h"
#include "expand.h"
#include "expand2.h"

/*
 *	This array contains the upper and lower bounds of the span
 *	permitted for a short form of a span dependent instruction
 *	(sdi).  This array must be initialized in the same units
 *	as dot,	 e.g. bytes. Also don't forget that the structure
 *	expects long values.
 */

rangetag range[NITYPES] = {
		{-125L,		124L},		/* SDIBR1 */
		{-125L,		124L},		/* SDIBR2 */
		{-121L,		120L},		/* SDIBR3: a little safer */
		{-121L,		120L}		/* SDIBR4: a little safer */
};

/*
 *	This array contains the size of the short form of a sdi.
 *	The size must be in the same units that are used by dot,	
 *	e.g. bytes.
 */

char pcincr[NITYPES] = {
	2, /* SDIBR1 */
	2, /* SDIBR2 */
	7, /* SDIBR3 */
	6  /* SDIBR4 */
};

/*
 *	This array contains the difference between the size of
 *	the long form of a sdi and the short form. Again the difference
 *	must be in the same units as dot.
 */

char idelta[NITYPES] = {
	6, /* SDIBR1 */
	4, /* SDIBR2 */
	6, /* SDIBR3 */
	6  /* SDIBR4 */
};
