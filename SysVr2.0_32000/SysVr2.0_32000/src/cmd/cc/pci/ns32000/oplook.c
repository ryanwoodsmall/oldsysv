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
/*	@(#)oplook.c	1.3	*/
#include	"optim.h"
#include	"extdec.h"
#include	<ctype.h>

#define	OP(instr,code,nargs,arg1,arg2,arg3,arg4) \
	{instr,code,nargs,arg1,arg2,arg3,arg4}

struct	instruc instruc[] =
{
#include	"instrs.h"
	0
};

int	instcomp(p0, p1)
USERTYPE p0;
USERTYPE p1;
{
	return (strcmp(p0->instr, p1->instr));
}

struct	instruc	*instlook(name)
char	*name;
{
	static	struct	instruc	i;

	i.instr = name;
	return
	(
		(struct instruc *)
		bsearch
		(
			(char *) &i,
			(char *) &instruc[0],
			sizeof instruc / sizeof instruc[0],
			sizeof instruc[0],
			instcomp
		)
	);
}

chgop(p, pop, popcode)				/* change an instruction */
register struct	node	*p;
register int	pop;
register char	*popcode;
{
	if (!p) abort(-2);
	p->op = pop;
	p->opcode = popcode;
	p->userdata = instlook(popcode);
	return;
}

/*
**	getreg(s): if "s" is a character string that represents a register
**		name, place that register's bit mask in regmask and return a
**		pointer to the first character not part of the register name.
**		Otherwise return the argument untouched with regmask cleared.
*/

static	int	regmask;

static	char *	getreg(arg)
char *	arg;
{
	register char *	s = arg;

	regmask = 0;
	switch (*s++)
	{
	case 'a':
	case 's':
		if (*s++ != 'p' || isalnum(*s))
			return (arg);
		return (s);
   case 't':
      if (*s++ == 'o' && *s++ == 's' && ! isalnum(*s))
         return s;
      return arg;

	case 'r':
	case 'f':
		if (*s == 'p' && ! isalnum(s[1]) )
			return &s[1];
		if (!isdigit(*s))
			return (arg);
		regmask = 1 << (*s++ - '0');
		if (*s && isalnum(*s))		/* No register, just a name */
			regmask = 0, s = arg;
		return (s);
	}
	return (arg);
}

int	r, w;	/* for debugging... */

#ifdef LIVEDEAD
int	uses(p)
struct	node	*p;
{
	r = scanops(p, 0);
	return (r);
}

int	sets(p)
struct	node	*p;
{
	w = scanops(p, 1);
	return (w);
}

int	scanops(p, rw)
register struct	node	*p;		/* instruction node */
int			rw;		/* 0: read access; 1: write access */
{
	char *	s;		/* operand string */
	register char *	t;		/* fast temp str. */
	register int	tmp;		/* fast temporary */
	register int	n;		/* operand number */
	int	rregs = 0;		/* registers read */
	int	wregs = 0;		/* registers writ */
	boolean		indxbl;		/* index mode ok? */

	if (p->op == MISC)
		p->userdata = NULL;
	if (p->userdata == NULL)
		return (0);

	if (p->userdata < &instruc[0] || p->userdata > (USERTYPE) ((char *) &instruc[0]) + sizeof instruc)
		p->userdata = instlook(p->ops[0]);

		/* temps are scrogged, but continue parsing instruction */
	if (p->op == ENTER || p->op == EXIT)
		return 0;

	if (p->op == STRING)
		wregs |= (R0|R1|R2);
	else
	if (isret(p))
		rregs |= RETREG;
	else
	if (p->op == JSR)
		wregs |= (R0|R1|R2|R3);		/* temps are scrogged */
	else
	if (p->op == JSR && strcmp(p->ops[0], "mcount"))
		rregs |= (R0|R1);
	else
	{
		for (n = 0; n < p->userdata->nargs; n++)
		{
			s = p->ops[n + 1];
			indxbl = true;

			/* rN -> N: opacc(p->userdata->args[n]) */
			if ((t = getreg(s)) != s)     /*operand starts w/register name*/
			{
				register int	tmp2;
				register int	tmp3;

				s = t;
				tmp = regmask;
				tmp2 = opacc(p->userdata->args[n]);
				tmp3 = optyp(p->userdata->args[n]);
				if (tmp3 == TD || tmp3 == TQ)
					tmp |= tmp << 1;	/* 2 regs for doubles */
				if (tmp2 == AR || tmp2 == AM)
					rregs |= tmp;
				if (tmp2 == AW || tmp2 == AM)
					wregs |= tmp;
			}
			else

			/* $X -> none */
			if (*s == '$')
			{
				indxbl = false;
				++s;
				isdisp(&s);
			}
			else

			/* D -> none ; D(rN) -> N: r ; D(D(rN)) -> N: r */
			if ( isdisp(&s) )
			{
				if (*s && *s != '[')
				{
					assert(*s++ == '(');
					if ((t = getreg(s)) == s)
					{
						if (! isdisp(&s))
							error("Seen 'D('. Found [%s]\n", s);
						assert(*s++ == '(');
						if ((t = getreg(s)) == s)
							error("Seen 'D(D('. Found [%s]\n", s);
						s = t;
						assert(*s++ == ')');
					}
					else
						s = t;
					assert(*s++ == ')');
					rregs |= regmask;
				}
			}
			/* NO ELSE HERE! */

			/* [rM:s] -> M: r */
			if (indxbl && *s)
			{
				assert(*s++ == '[');
				if ((t = getreg(s)) == s)
					error("Seen `[', found [%s]\n", s);
				s = t;
				assert(*s++ == ':');
				if (*s != 'b' && *s != 'w' && *s != 'd')
					error("Seen '[rn:', found [%s]\n", s);
				s++;
				assert(*s++ == ']');
				rregs |= regmask;
			}
			if (*s)
			{
				fprintf(stderr, "Operand failure: [%s]\n", s);
				fflush(stderr);
			}
			/*
			**	This assert is required to acknowledge
			** that the entire operand has been parsed.  If
			** the assert fails, more code needs to be
			** added above (to parse the hairier
			** addressing mode).
			*/
			assert(*s == '\0');
		}
	}
	if (rw == 0)
		return (rregs);
	else
		return (wregs);
}
#endif

boolean	wwchanged;


int	RETREG;
int	isret(p)
register struct	node	*p;
{
        switch (p->op)
        {
	case RXP:
	case RETI:
	case RETT:
        case RET:
                RETREG = R0;
                return true;
	default:
		return (false);
	}
	/* NOTREACHED */
}

char	*destarg(p)
register struct	node	*p;
{
	register int	i;

	for (i = p->userdata->nargs; i > 0; --i)
	{
		register int	acc;

		acc = opacc(p->userdata->args[i - 1]);
		if (acc == AW || acc == AM)
			return (p->ops[i]);
	}
	return (NULL);
}

typesize(t)
register int	t;
{
	switch (t)
	{
	case TB:	return (1);
	case TW:	return (2);
	case TF:
	case TL:	return (4);
	case TD:
	case TQ:	return (8);
	}
	return (0);
}

boolean	noside(s)	/* does the addressing mode of s have no side effects?*/
register char	*s;
{
	register char	*c = s;

        if (! strcmp(s,"tos") )
                return false;
        return true;
}

boolean	isreg(s)
register char	*s;
{
	switch (*s)
	{
	case 'r':
		if (isdigit(s[1]))
		{
			if (s[1] == '1' && isdigit(s[2]))
				return (s[3] ? false : true);
			else
				return (s[2] ? false : true);
		}
		return (false);

	case 'a':
	case 'f':
	case 's':
		if (*++s == 'p' && !*++s)
			return (true);
	}
	return (false);
}

argtype(p, n)
register struct	node	*p;
register int		n;
{
	if (!p) abort(-2);
	return (optyp(p->userdata->args[--n]));
}

#ifdef LIVEDEAD
boolean	isdead(s, p)			/* is register s dead after node p? */
register char		*s;
register struct	node	*p;
{
	register char *	t;

	if ((t = getreg(s)) != s && *t == '\0' && (regmask < (1<<4)) && (p->nlive & regmask) == 0)
		return (true);
	return (false);
}
#endif

void	windstats()
{
	register int	i;

	for (i = 0; i < w1size - 1; i++)
	{
		if (w1stats[i])
			fprintf(stderr, "%d %s\n", w1stats[i], w1opts[i]);
	}
	for (i = 0; i < w2size - 1; i++)
	{
		if (w2stats[i])
			fprintf(stderr, "%d %s\n", w2stats[i], w2opts[i]);
	}
	for (i = 0; i < w3size - 1; i++)
	{
		if (w3stats[i])
			fprintf(stderr, "%d %s\n", w3stats[i], w3opts[i]);
	}
}

log_stats()
{
#ifdef	LOGFILE
	register int	i;
	register FILE	*fp;

	if ((fp = fopen(LOGFILE, "a")))
	{
		for (i = 0; i < w1size - 1; i++)
			fprintf(fp, "%4d", w1stats[i]);
		putc(';', fp);
		for (i = 0; i < w2size - 1; i++)
			fprintf(fp, "%4d", w2stats[i]);
		putc(';', fp);
		for (i = 0; i < w3size - 1; i++)
			fprintf(fp, "%4d", w3stats[i]);
		putc('\n', fp);
		fclose(fp);
	}
#endif
}

boolean	isstatic(s)	/* is an operand most likely static? */
register char	*s;
{
	if (*s == '$')
		++s;
	if (*s == '_' ||			/* C externals */
		(*s == '.' && s[1] == 'L') ||	/* C internals */
		(*s == '.' && s[1] == 'I') ||	/* Improver added */
		(*s == 'v' && s[1] == '.'))	/* F77 variable */
		return (true);
	return (false);
}

int	constval;	/* definition of storage */
double	fconstval;

boolean	isconst(s)	/* is an operand most likely an integral constant? */
register char	*s;
{
	extern	int	constval;
	register int	sign = 1;

	constval = 0;
	if (*s++ != '$')
		return (false);
	if (*s == '-')
		sign = -1, ++s;
	while (*s)
	{
		if (!isdigit(*s))
			return (false);
		constval *= 10;
		constval += *s++ - '0';
	}
	constval *= sign;
	return (true);
}

boolean	isfconst(s)	/* is an operand most likely a floating constant? */
register char	*s;
{
	extern	double	fconstval;
	register int	sign = 1;

	fconstval = 0.0;
	if (*s++ != '$')
		return (false);
	if (*s == '-')
		sign = -1, ++s;
	if (*s != '0' || (*++s != 'd' && *s != 'f'))
		return (false);
	if (sscanf(++s, "%e", &fconstval) != 1)
		return (false);
	fconstval *= sign;
	return (true);
}

boolean	isnshort(s)	/* is "s" the negative of a VAX short literal? */
register char	*s;
{
	if (isconst(s) && constval >= -63 && constval < 0)
		return (true);
	return (false);
}

error(s, f)
char *	s;
char *	f;
{
	fprintf(stderr, s, f);
	fflush(stderr);
	abort();
	/* NOTREACHED */
}

isindexd(s,t)
register char *	s;
register char *	t;
{
        char *strchr();
	register char *	p;

	if (s && t && (p = strchr(s, '[')))
	{
		while (*++p == *t++)
			;
		if (*p == ']' && *--t == '\0')
			return (true);
	}
	return (false);
}

isdisp(s)
	char **s;
{
	char *t = *s;

	if (*t == '-')
		t++;
	for (;; t++) {
		while (isalnum(*t) || *t == '_' || *t == '.')
			t++;
		if (*t != '+' && *t != '-')
			break;
		}
	if ( t > *s ) {
		*s = t;
		return 1;
		}
	return 0;
}
oneset(val,size)
	register val;
	int size;
{
	register i;

	if ( size == TB )
		val &= 0xff;
	else if (size == TW )
		val &= 0xffff;
	for (i = 1; i; i <<= 1)
		if ( !( val ^ i ))
			return 1;
	return 0;
}

char *logbit(val)
	register unsigned val;
{
	register i;
	char s[4];

	for (i = 0; ; i++, val >>= 1)
		if ( val & 1 )
			break;
	sprintf(s, "$%d", i);
	return COPY(s, 4);
}
