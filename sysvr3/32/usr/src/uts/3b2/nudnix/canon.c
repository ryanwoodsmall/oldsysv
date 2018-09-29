/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:nudnix/canon.c	10.4.1.4"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"

/*
 *convert from canonical to local representation
 */
fcanon(fmt,from,to)
register char *fmt, *from, *to;
{
	register short tmp;
	long ltmp;
	register char *cptr;
	char *lfmt, *tptr;

	tptr = to;
	while(*fmt){
		switch(*fmt++){
		case 's':
			to = SALIGN(to);
			from = LALIGN(from);
			cptr = (char *)&ltmp;
			*cptr++ = hibyte(hiword(*from));
			*cptr++ = lobyte(hiword(*from));
			*cptr++ = hibyte(loword(*from));
			*cptr++ = lobyte(loword(*from));
			*(short *)to = ltmp;
			to = SNEXT(to);
			from = LNEXT(from);
			continue;
		case 'i':
			to = IALIGN(to);
			from = LALIGN(from);
			cptr = (char *)&ltmp;
			*cptr++ = hibyte(hiword(*from));
			*cptr++ = lobyte(hiword(*from));
			*cptr++ = hibyte(loword(*from));
			*cptr++ = lobyte(loword(*from));
			*(int *)to = ltmp;
			to = INEXT(to);
			from = LNEXT(from);
			continue;
		case 'l':
			to = LALIGN(to);
			from = LALIGN(from);
			ltmp = *(long *)from;
			*to++ = hibyte(hiword(ltmp));
			*to++ = lobyte(hiword(ltmp));
			*to++ = hibyte(loword(ltmp));
			*to++ = lobyte(loword(ltmp));
			from = LNEXT(from);
			continue;
		case 'b':
			*to++ = *from++;
			continue;
		case 'c':
			from = LALIGN(from);
			lfmt = fmt;
			ltmp = duatoi(&lfmt);
			fmt = lfmt;
			if(ltmp == 0) {
				ltmp = *(long *)from;
				cptr = (char *)&ltmp;
				*cptr++ = hibyte(hiword(*from));
				*cptr++ = lobyte(hiword(*from));
				*cptr++ = hibyte(loword(*from));
				*cptr++ = lobyte(loword(*from));
			}
			from = LNEXT(from);	
			while(ltmp--)
				*to++ = *from++;	
			continue;
		default:
			return(0);

		}
	}
	return(to - tptr);
}

static char rfs_tbuf[2048];

/*
 *This routine converts from local to cononical representation
 */
tcanon(fmt, from, to, flag)
register char *fmt, *from, *to;
{
	register short tmp;
	long ltmp;
	char *lfmt, *tptr;
	register char *cptr = 0;

	tptr = to;
	if(flag && from == to){
		cptr = to;
		to = rfs_tbuf;
	}
	while(*fmt){
		switch(*fmt++){
		case 's':
			to = LALIGN(to);
			from = SALIGN(from);
			ltmp = *(short *)from;
			*to++ = hibyte(hiword(ltmp));
			*to++ = lobyte(hiword(ltmp));
			*to++ = hibyte(loword(ltmp));
			*to++ = lobyte(loword(ltmp));
			from = SNEXT(from);
			continue;
		case 'i':
			to = LALIGN(to);
			from = IALIGN(from);
			ltmp = *(int *)from;
			*to++ = hibyte(hiword(ltmp));
			*to++ = lobyte(hiword(ltmp));
			*to++ = hibyte(loword(ltmp));
			*to++ = lobyte(loword(ltmp));
			from = INEXT(from);
			continue;
		case 'l':
			to = LALIGN(to);
			from = LALIGN(from);
			ltmp = *(long *)from;
			*to++ = hibyte(hiword(ltmp));
			*to++ = lobyte(hiword(ltmp));
			*to++ = hibyte(loword(ltmp));
			*to++ = lobyte(loword(ltmp));
			from = LNEXT(from);
			continue;

		case 'b':
			*to++ = *from++;
			continue;
		case 'c':
			lfmt = fmt;
			ltmp = duatoi(&lfmt);
			fmt = lfmt;
			if(ltmp == 0){
				ltmp = strlen(from) + 1;
			}
			to = LALIGN(to);
			*to++ = hibyte(hiword(ltmp));
			*to++ = lobyte(hiword(ltmp));
			*to++ = hibyte(loword(ltmp));
			*to++ = lobyte(loword(ltmp));
			while(ltmp--)
				*to++ = *from++;	
			to = LALIGN(to);
			continue;
		default:
			return(0);
		}
	}
	if(cptr){
		bcopy(rfs_tbuf,cptr,(to - rfs_tbuf));
		return(to - rfs_tbuf);
	}
	return(to - tptr);
}

/* takes a numeric char, yields an int */
#define	CTOI(c)		((c) & 0xf)
/* takes an int, yields an int */
#define TEN_TIMES(n)	(((n) << 3) + ((n) << 1))

/* Returns the integer value of the string of decimal numeric
 * chars beginning at **str.
 * Note: updates *str to point at the last character examined.
 */
int
duatoi(str)
register char	**str;
{
	register char	*p = *str;
	register int	n;
	register int	c;

	for (n = 0; (c = *p) >= '0' && c <= '9'; ++p){
		n = TEN_TIMES(n) + CTOI(c);
	}
	*str = p;
	return(n);
}
