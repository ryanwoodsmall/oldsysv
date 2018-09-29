/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-m32:gen/atof.c	1.6"
/*LINTLIBRARY*/
/*
 *	C library - ascii to floating (atof) and string to double (strtod)
 *
 *	This version compiles both atof and strtod depending on the value
 *	of STRTOD, which is set in the file and may be overridden on the
 *	"cc" command line.  The only difference is the storage of a pointer
 *	to the character which terminated the conversion.
 */
#ifndef STRTOD
#define STRTOD	0
#endif
#include "shlib.h"		/* For shared libraries*/
#include <ctype.h>
#include "values.h"

extern double _ptod();

#if STRTOD
#define STORE_PTR	(*ptr = p)
#define DEC_PTR	(*ptr = p - 1)
#define GOT_DIGIT	(got_digit++)
#define RET_ZERO(val)	if (!got_digit) return (0.0)

double
strtod(p, ptr)
register char *p;
char **ptr;
#else
#define STORE_PTR
#define DEC_PTR
#define GOT_DIGIT
#define RET_ZERO(val)	if (!val) return (0.0)

double
atof(p)
register char *p;
#endif
{
	register int c, exp = 0;
	long high, low = 0;
	int lowlen = 0;
	int neg_val = 0;
#if STRTOD
	int got_digit = 0;
	char *dummy;
	if (ptr == (char **)0) 
		ptr = &dummy; /* harmless dumping place */
	STORE_PTR;
#endif
	while (isspace(c = *p)) /* eat leading white space */
		p++;
	switch (c) { /* process sign */
	case '-':
		neg_val = 1;
	case '+': /* fall-through */
		p++;
	}
	{	/* accumulate value */
		register long temphigh = 0;
		register int decpt = 0;

		while (isdigit(c = *p++) || c == '.' && !decpt++ &&
							isdigit(c = *p++)) {
			GOT_DIGIT;
			exp -= decpt; /* decr exponent if decimal pt. seen */
			c -= '0';
			if (temphigh < MAXLONG/10) {
				temphigh *= 10;
				temphigh += c;
			} else if (++lowlen <= 9) {
				low = 10*low + c;
			} else {
				exp++;
				lowlen--;
			}
		}
		RET_ZERO(temphigh);
		high = temphigh;
	}
	DEC_PTR; /* in case there is no legitimate exponent */
	if (c == 'E' || c == 'e') { /* accumulate exponent */
		register int e_exp = 0, neg_exp = 0;
		STORE_PTR;

		switch (*p) { /* process sign */
		case '-':
			neg_exp++;
		case '+': /* fall-through */
		case ' ': /* many FORTRAN environments generate this! */
			p++;
		}
		if (isdigit(c = *p)) { /* found a legitimate exponent */
			do {
				/* limit outrageously large exponents */
				if (e_exp < DMAXEXP)
					e_exp = 10 * e_exp + c - '0';
			} while (isdigit(c = *++p));
			if (neg_exp)
				exp -= e_exp;
			else
				exp += e_exp;
			STORE_PTR;
		}
	}

	return(_ptod(high, low, lowlen, exp, neg_val));
}
