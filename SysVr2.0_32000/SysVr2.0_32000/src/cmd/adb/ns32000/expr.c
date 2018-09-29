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
/*	@(#)expr.c	1.2	*/
#include "defs.h"

SCCSID(@(#)expr.c	1.2);

static int  savframe;
static int  savpc;
static char isymbol[BUFSIZ];
static void readsym();
static void chkloc();

static SYMPTR lookupsym(symstr)
	STRING symstr;
{
	SYMPTR symp;

	symset();
	while ( symp = symget() )
	{
		skip_aux(symp);
		if( symp->n_scnum == N_DEBUG )
			continue;
		if( eqsym( symp->n_nptr, symstr, '_' ) ) 
			return(symp);
	}
	return(0);
}

int expr(a)                       /* term | term dyadic expr |  */
{ 
	short rc;
	int   lhs;

	(void) rdc(); 
	lp--; 
	rc = term(a);

	while (rc) {  
		lhs = expv;

	    switch (readchar()) {

	        case '+':
	            (void) term(a|1); 
				expv += lhs; 
				break;

	        case '-':
	            (void) term(a|1); 
				expv = lhs - expv;
				break;

	        case '#':
	            (void) term(a|1);
				expv = round(lhs, expv);
				break;

	        case '*':
	            (void) term(a|1);
				expv *= lhs;
				break;

	        case '%':
	            (void) term(a|1);
				expv = lhs / expv;
				break;
	        
	        case '&':
	            (void) term(a|1);
				expv &= lhs;
				break;

	        case '|':
	            (void) term(a|1);
				expv |= lhs;
				break;
	  
	        case ')':
				if ((a&2) == 0)
					error(BADKET);

	        default:
	            lp--;
	            return(rc);
	    }
	}
	return(rc);
}

static term(a)              /* item | monadic item | (expr) | */
{
	switch (readchar()) {

	    case '*':
	        (void) term(a|1);
			expv = chkget(expv, DSP);
			return(1);

	    case '@':
	        (void) term(a|1);
			expv = chkget(expv, ISP);
			return(1);

	    case '-':
	        (void) term(a|1);
			expv = -expv;
			return(1);

	    case '~':
	        (void) term(a|1);
			expv = ~expv;
			return(1);

	    case '(':
	        (void) expr(2);
	        if (*lp!=')')
				error(BADSYN);
	        else { 
				lp++;
				return(1);
	        }

	    default:
	        lp--;
	        return(item(a));
	}
}

static item(a) /* name [ . local ] | number | . | ^ | 
	              <var | <REG | 'x | | */
	int a;
{
	short  base;
	short  d;
	short  regptr;
	char   savc;
	int    frame;
	int oldframe;
	SYMPTR symp;
	int autonum;
	int argp;
	int word;

	(void) readchar();
	if (symchar(0)) 
	{
		readsym();
		if (lastc == '.') 
		{
			frame = ADBREG(FP);
			callpc = ADBREG(PC);
			while (errflg == NULL)  
			{
				savpc = callpc;
				(void) findsym(callpc, ISYM);
				if( eqsym( symbol.n_nptr, isymbol, '_' ) )
				{
					break;
				}
				callpc = get(frame+4, DSP);
				oldframe = frame; /* for possible adjustment */
				frame = get(frame, DSP) & EVEN;
				if (frame == 0)
					error(NOCFN);
			}
			if(frame!=ADBREG(FP))
				frame = oldframe;
			savframe = frame;
			(void) readchar();
			if(getnum(readchar))
			{
				(void)readchar();
				autonum = expv;
				localsym(frame,frame!=oldframe);
				localval = expv = locadr -(4 * autonum);
				adbpr("%8t%s.%X%  =  ", symbol.n_nptr, autonum);
				adbpr("%R\n",get(localval,DSP));
			} 
			else
				error(BADSYM);
		}
		else if ((symp = lookupsym(isymbol)) == 0)
			error(BADSYM);
		else 
			expv = symp->n_value;
		lp--;

	} 
	else if (getnum(readchar)) 
		;
	else if (lastc == '.') 
	{
		(void)readchar();
		if(getnum(readchar))
		{
			(void)readchar();
			autonum = expv;
			localsym(ADBREG(FP),1);
			localval = expv = locadr - (4*(autonum+1));
			adbpr("%8t%s.%X%  =  ",symbol.n_nptr,autonum);
			adbpr("%R\n",get(localval, DSP));
		}
		else 
			expv = dot;
	    lp--;

	} 
	else if (lastc == '"')
		expv = ditto;

	else if (lastc == '+') 
		expv = inkdot(dotinc);

	else if (lastc == '^')
		expv = inkdot(-dotinc);

	else if (lastc == '<') { 
		savc = rdc();
		{
			struct grres res;
			res = getreg(savc);
			if (res.sf)
				expv = ADBREG(res.roff);
			else if ((base = varchk(savc)) != -1)
				expv = var[base];
			else 
				error(BADVAR);
		}
	} 
	else if (lastc == '\'') { 
	    d = 4; 
	    expv = 0;
	    while (quotchar() != '\0') {  
	        if (d--) { 
				if (d == 1)
	                expv <<= 16; 
	            expv |= ((d&1) ? lastc : lastc << 8);
	        }
			else
				error(BADSYN);
	    }

	} 
	else if (a)
	    error(NOADR);
	else { 
	    lp--; 
	    return(0);
	}
	return(1);
}

/* Service routines for expression reading */
int getnum(rdf) 
	char (*rdf)();
{
	int  base;
	int  d;
	int  frpt;
	BOOL hex = FALSE;
	union {
		float r; 
		int   i;
	} real;

	if (isdigit(lastc) || (hex = TRUE, lastc == '#' &&
	                     hexdigit((*rdf)()))) { 
		expv = 0;
	    	base = (hex ? 16 : radix);
		while ((base > 10 ? hexdigit(lastc) : isdigit(lastc))) {  
			expv *= base;
			if ((d = convdig(lastc)) >= base)
				error(BADSYN);
			expv += d;
			(*rdf)();
			if (expv == 0) { 
				if ((lastc == 'x' || lastc == 'X')) { 
					hex = TRUE;
					base = 16;
					(*rdf)();
				} 
				else if ((lastc == 't' || lastc=='T')) { 
					hex = FALSE; 
					base=10; 
					(*rdf)();
				} 
				else if ((lastc == 'o' || lastc=='O')) { 
					hex = FALSE; 
					base = 8; 
					(*rdf)();
				}
			}
		}
		if (lastc == '.' && (base == 10 || expv == 0) && !hex) { 
			real.r = expv;
			frpt = 0;
			base = 10;
			while (isdigit((*rdf)())) { 
				real.r *= base;
				frpt++;
				real.r += lastc - '0';
			}
			while (frpt--)
				real.r /= base;
			expv = real.i;
		}
		peekc = lastc;
		/*  lp--; */
		return(1);
	}
	else 
		return(0);
}
   
static void readsym()
{
	char *p = isymbol;

	do { 
		if (p < &isymbol[BUFSIZ])
			*p++ = lastc;
	    (void) readchar();
	} while (symchar(1));
	if (p < &isymbol[BUFSIZ])
		*p = 0;
}

static int hexdigit(c)
	char c;
{ 
	return((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'));
}

static int convdig(c)
	char c;
{
	if (isdigit(c)) 
		return(c - '0');
	else if (hexdigit(c))
		return(c - 'a' + 10);
	else
		return(17);
}

static int symchar(dig) /* TRUE if last character read can occur
						   in a symbol in position dig */
{
	if (lastc == '\\') { 
		(void) readchar(); 
		return(TRUE); 
	}
	return(isalpha(lastc) || lastc == '_' || dig && isdigit(lastc));
}

int varchk(name)
{
	if (isdigit(name))
		return(name - '0');
	if (isalpha(name)) 
		return((name & 037) - 1 + 10);
	return(-1);
}


static int
eqsym( s1, s2, c )	/* TRUE if s1 and s2 contain equivalent symbol names */
	STRING s1;
	STRING s2;
	char   c;
{
	if( eqstr( s1, s2 ) )
	    return( TRUE );
	else if ( *s1 == c )
	{ 
		char s3[BUFSIZ];
		int  i;
   
		s3[0] = c;
		for (i = 1; *s2; i++)
	        	s3[i] = *s2++; 
		s3[i] = '\0';
   
	    return( eqstr( s1, s3 ) );
	}
	return(FALSE);
}

calcap( frame, word )
int frame;
int word;
{
	int argp;
	word &= 0xfff;
	argp=frame+20+((word>>14)&3);
	while (word)
	{
		if(word&1)
			argp+=4;
		word >>= 1;
	}
	return(argp);
}
