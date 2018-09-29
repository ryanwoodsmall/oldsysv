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
static char Sccsid[]="@(#)bal.c	3.1";
#
/*
	Function to find the position, in str, of the first of the char-
	acters in end occurring outside a balanced string.  A balanced string
	contains matched occurrences of any character in open and the corres-
	ponding character in clos.  Balanced strings may be nested.  The null
	at the end of str is considered to belong to end.  Unmatched members
	of open or clos result in an error return.
*/

#define ifany(x) for (p=x; *p; p++) if (c == *p)
#define matching_clos clos[p-open]
#define error -1
#define position s-str-1

balbrk(str,open,clos,end)
char *str,*open,*clos,*end;
{
	register char *p, *s, c;
	char opp[2];
	opp[1] = '\0';
	for (s = str; c = *s++;  ) {
		ifany(end) return position;
		ifany(clos) return error;
		ifany(open) {
			opp[0] = matching_clos;
			s += balbrk(s,open,clos,opp);
			if (*s++ != matching_clos) return error;
			break;
		}
	}
	return position;
}
