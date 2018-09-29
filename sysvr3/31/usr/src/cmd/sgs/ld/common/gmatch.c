/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ld:common/gmatch.c	1.2.1.1"
#include "system.h"
#include <stdio.h>
#include "attributes.h"
#include "list.h"
#include "structs.h"

typedef char BOOL;

#ifndef MINUS
#define	MINUS	'-'
#endif

int nexpr = 0x10;	/* Start out with 16 slots for
			 * stored r.e.'s, grow as needed...
			 */
static struct re {
	char *s_expr;
	enum scope s_how;
} *exprs;
int nextexpr = 0;
static char nospace[] = "No space for regular expression %s";
	
store_re(s, how)
	char *s;
	enum scope how;
{
	extern char *myalloc();
	int len;

/*
 * Note that malloc/realloc is used --- realloc gets
 * awfully confused when handy an address that didn't
 * come from malloc!
 */
	if (exprs == NULL)
		exprs = (struct re *)malloc(nexpr*sizeof(*exprs));
	else if (nextexpr == nexpr) {
		nexpr *= 2;
		exprs = (struct re *)realloc(exprs, nexpr*sizeof(*exprs));
		}
	if (exprs == NULL)
		lderror(2,0,NULL,nospace, s);
	len = strlen(s) + 1;
	exprs[nextexpr].s_expr = myalloc(len);
	(void) strcpy(exprs[nextexpr].s_expr, s);
	exprs[nextexpr++].s_how = how;
}

/*
 * Return 1 if "how" seems valid for "s".
 * For example, if there's an expression that says "hide
 * all symbols of the form 'yy*'", then this returns:
 *	action_re("yydebug", __hidden) == 1
 *	action_re("yydebug", __exported) == 0
 *	action_re("zyz", __hidden) == 0
 *	action_re("zyz", _exported) == 0
 *
 * What's the logic here?
 * a) Find the first r.e. that "s" matches, remember its
 *    directive (hide/export).
 * b) Now look for any remaining matching r.e.'s with conflicting
 *    directives.
 * c) Error if we found such a conflict,
 *    "no change to symbol" if we found nothing,
 *    otherwise check to see what we do with the symbol and
 *    tell the caller whether it's okay to make the change or not.
 */

static char conflict[] = "Conflict on symbol %s - exported and hidden!";

action_re(s, how)
	char *s;
	enum scope how;
{
	int i;
	enum scope first = __undefined;

	if (nextexpr == 0) return(0);
	for (i = 0; i < nextexpr; i++) {
		if (first == __undefined && gmatch(s, exprs[i].s_expr))
			first = exprs[i].s_how;
		else if (first == exprs[i].s_how)
			continue;
		else if (gmatch(s, exprs[i].s_expr))
			lderror(2,0,NULL, conflict, s);
		}
	if (first == __undefined)
		return(0);
	if (how == first)
		return(1);
	else
		return(0);
}

/*
 * Return 1 if "s" contains special characters that
 * make "s" a r.e.;
 * return -1 if it's a malformed r.e.;
 * return 0 if it contains no metacharacters.
 */

hasmeta(s)
	char *s;
{
	int foundmeta = 0;
	int foundescape = 0;
	enum { inbracket, notinbracket} state = notinbracket;

	do
	{
		switch (*s)
		{
		    case '*':
		    case '?':
			foundmeta = 1;
			break;
		    case '\\':	
			foundescape = 1;
			s++; /* Ignore next character */
			if (*s == '\0')
				return(-1);
			break;
		    case '[':
			if (state == inbracket) return(-1);
			state = inbracket;
			foundmeta = 1;
			break;
		    case ']':
		    case '-':
		    case '!':
			if (state == notinbracket) return(-1);
			foundmeta = 1;
			if (*s == ']') state = notinbracket;
			break;
		}
	} while (*s++ != '\0');
	if (state == inbracket) return(-1);
	if (foundmeta) return(1);
	if (foundescape) return(-2);
	return(0);
}
	
gmatch(s, p)
register unsigned char	*s, *p;
{
	register unsigned char scc;
	unsigned char c;

	scc = *s++;
	switch (c = *p++)
	{
	case '[':
		{
			BOOL ok;
			int lc = -1;
			int notflag = 0;

			ok = 0;
			if (*p == '!')
			{
				notflag = 1;
				p++;
			}
			while (c = *p++)
			{
				if (c == ']')
					return(ok ? gmatch(s, p) : 0);
				else if (c == MINUS && lc > 0 && *p!= ']')
				{
					if (notflag)
					{
						if (scc < lc || scc > *(p++))
							ok++;
						else
							return(0);
					}
					else
					{
						if (lc <= scc && scc <= (*p++))
							ok++;
					}
				}
				else
				{
					if(c == '\\') /* skip to quoted character */
						c = *p++;
					lc = c;
					if (notflag)
					{
						if (scc && scc != lc)
							ok++;
						else
							return(0);
					}
					else
					{
						if (scc == lc)
							ok++;
					}
				}
			}
			return(0);
		}

	case '\\':	
		c = *p++; /* skip to quoted character and see if it matches */
	default:
		if (c != scc)
			return(0);

	case '?':
		return(scc ? gmatch(s, p) : 0);

	case '*':
		while (*p == '*')
			p++;

		if (*p == 0)
			return(1);
		--s;
		while (*s)
		{
			if (gmatch(s++, p))
				return(1);
		}
		return(0);

	case 0:
		return(scc == 0);
	}
}
