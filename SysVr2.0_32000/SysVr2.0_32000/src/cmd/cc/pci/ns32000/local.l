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
%{
/*	@(#)local.l	1.4	*/

#include	"optim.h"
#include	<ctype.h>

static	boolean	codeseen;		/* have we seen anything to improve? */
static	int	opn;			/* current operand index */
static	int	op;			/* temp to hold value from oplook() */
extern	boolean	wwchanged;		/* did any window change? */

extern	USERTYPE instlook();		/* returns ptr into instruction table */

static struct node *dotenter;		/* remember location of ".enter" */

/* Tune yylex() */
#undef	YYLMAX
#define	YYLMAX	8192

static	int	lexstate;
static	int	oldstate;
#define	myBEGIN(x)	(lexstate = (x), BEGIN x)
%}

ILABEL	\.L[0-9]+

%{
/* Start conditions:
**	T	Before 1st non-white-space char on a text line
**	T1
**	T2	A comma delimited operand
**	P0
**	P1
**	P2
**	D	Before 1st non-white-space char on a data line
**	B	Before 1st non-white-space char on a bss line
**	CH	After a "case head"
**	FP0	After a ".align 1" in function prolog
**	FP1	After a "[^:\n]:\n" in function prolog
*/
%}

%START T T1 T2 P0 P1 P2 D B CH FP0 FP1

%%

^\t			{
				/* ignore leading tabs */;
			}

#[^\n]*			{
				/* eliminate comments */;
			}

<D>{ILABEL}$		{
				/* catch all referenced labels in data */
				ECHO;
				addref(yytext, yyleng + 1);
			}

<D>[^:\n]+:		{
				/* fix for FORTRAN variables: "v.#" */
				ECHO;
			}

<T>_[^:\n]+:\n		{
				/* process previous function */
				endfunc();
				/* start function prolog */
				/* add function label */
				yytext[yyleng - 2] = '\0';	/*zap ':'*/
				applbl(yytext, yyleng);
			}

<T>\.[^:\n]+:(\n)?	{
				/* make note of a text label */
				if (yytext[yyleng-1] == '\n')
				   yytext[yyleng-2] = '\0';
				else
				   yytext[yyleng-1] = '\0';
				applbl(yytext, yyleng);
				codeseen = true;
			}


<CH>[^:\n]+:		{
				saveop(opn,yytext, yyleng+1, 0);
				opn++;
			}

<CH>(\t?\.double\t{ILABEL}-{ILABEL}\n)+	{
						nsc_case();
						myBEGIN(T);
					}

<T>\.enter		{
				register USERTYPE ip;
				opn = 0;	/* 0th operand */
				ip = instlook(yytext);
				op = ip->code;
				dotenter = (NODE *) saveop(opn, ip->instr, 0, op);
				lastnode->userdata = ip;
				opn++;
				codeseen = true;
				myBEGIN(T1);
			}

<T>[a-z]+		{
				register USERTYPE ip;
				opn = 0;	/* 0th operand */
				if (!(ip = instlook(yytext)))
				{
					op = OTHER;
					saveop(opn, yytext, 1+yyleng, op);
				}
				else
				{
					op = ip->code;
					saveop(opn, ip->instr, 0, op);
				}
				lastnode->userdata = ip;
				opn++;
				codeseen = true;
				myBEGIN(T1);
			}

<T1>[, \t]*		{
				/* eat white space after opcode and operands */
				myBEGIN(T2);
			}

<T1>(#[^\n]*)?[;\n]	{
				/* the end of the instruction */
				/* deal with case[bwl] displacement table*/
				/* note pseudos are handled below */
				switch (op)
				{
				case CASE:
					myBEGIN(CH);
					lastnode->back->op = HLABEL;
					break;

				case ENTER:
					if ( dotenter ) {
						chgop(dotenter,ENTER,"enter");
						dotenter->ops[1] = COPY(lastnode->ops[1],1+strlen(lastnode->ops[1]));
						dotenter->ops[2] = COPY(lastnode->ops[2],1+strlen(lastnode->ops[2]));
						DELNODE(lastnode);
						}
				default:
					myBEGIN(T);
					break;
				}
			}

<T2>\[[^\]]*]		{
				/* stash test for [ .* ] */
				myBEGIN(T1);
				saveop(opn, yytext, yyleng + 1, 0);
				opn++;
			}

<T2>[^,#;\n]*		{
				/* stash text up to next delimiter */
				myBEGIN(T1);
                                yyleng -= zapwhite(yytext,yyleng);
				saveop(opn, yytext, yyleng + 1, 0);
				opn++;
			}

<T>(#[^\n]*)?[;\n]	;

<T,D>\.[a-z]+		{
				register USERTYPE ip;
				opn = 0;	/* 0th operand */
				if (!(ip = instlook(yytext)))
					abort("Bogus Pseudo-op!");
				else
				{
					op = ip->code;
					saveop(opn, ip->instr, 0, op);
				}
				lastnode->userdata = ip;
				opn++;
				oldstate = lexstate;
				myBEGIN(P0);
			}

<P0>[, \t]*		{
				/* eat white space after pseudo-op */
				myBEGIN(P1);
			}

<P0>(#[^\n]*)?[;\n]	{
				/* the end of the pseudo-op */
				/* deal with them! */
				switch (op)
				{
					register int	i;

				case PTEXT:
					myBEGIN(T);
					for (i = 0; i <= MAXOPS; i++)
						lastnode->ops[i] = NULL;
					DELNODE(lastnode);
					break;

				case PDATA:
					myBEGIN(D);
					prinst(lastnode);
					for (i = 0; i < MAXOPS; i++)
						lastnode->ops[i] = NULL;
					DELNODE(lastnode);
					break;

				case PLONG:
				case PFLOAT:
					assert(oldstate == D);
                                case PALIGN:
				case PBYTE:
				case PWORD:	/* reg save mask is in text */
				case PDOUBL:
				case PSPACE:
					if (oldstate == T) {
						myBEGIN(T);
						break;
						}
				case PFILE:
				case PBSS:
				case PCOMM:
				case PGLOBL:
				case PLN:
				case PORG:
				case PSET:
					/* Enter here to echo the built node */
					myBEGIN(oldstate);
					prinst(lastnode);
					for (i = 0; i < MAXOPS; i++)
						lastnode->ops[i] = NULL;
					DELNODE(lastnode);
					break;

				default:
					abort("bogus pseudo\n");
				}
			}

<P1>[^#;\n]+		{
				/* Gobble rest of line */
				myBEGIN(P0);
				saveop(opn, yytext, yyleng + 1, 0);
				opn++;
			}

%%

/*a  d fgh jk     q  t v xyz*/
/*z*/	boolean	loopflag = false;	/* true: enable aob/sob */
/*y*/	boolean	bitbflag = true;	/* true: enable bit,cbr -> bb */

yyinit(usrflags)
char	*usrflags;
{
	if (usrflags)
	{
		while (*usrflags)
		{
			switch (*usrflags++)
			{
			case 'z':	/* enable sob/aob */
				loopflag = true; break;

			case 'y':	/* disable bit,cbr -> bb */
				bitbflag = false; break;

			default:
				fprintf(stderr,
					"Option `%c' unrecognized (ignored).\n",
					*usrflags);
				break;
			}
		}
	}
	codeseen = false;
	myBEGIN(T);	/* start in the text segment */
}

nsc_case()
{
	register char	*cp;
	register char	*sp;

	yytext[yyleng - 1] = '\0';		/* zap last char (== '\n') */
	saveop(opn, yytext, yyleng, 0);
	/* <CH> (\.word\t{ILABEL}-{ILABEL}\n)+ */
	for (cp = &yytext[0]; *cp; cp++)
	{
		if (*cp == '.' && cp[1] == 'L')	/* a label */
		{
			if (cp[-1] == '\t')	/* one we want */
			{
				sp = cp;
				cp += 2;
				while (isdigit(*cp++))
					;
				if (*--cp == '-')
				{
					register NODE *p;

					*cp = '\0';
					for (ALLN(p))	/* find the label */
					{
						if (!islabel(p))
							continue;
						if (!strcmp(p->ops[0], sp))
						{
							/* mark it as HARD */
							p->op = HLABEL;
							break;
						}
					}
					addref(sp, (cp - sp) + 1);
					*cp = '-';
				}
			}
		}
	}
}

endfunc()
{
	extern	boolean	w1optim();
	extern	boolean	w2optim();
	extern	boolean	w3optim();
	register int	ndisc0;

	if (codeseen == false || n0.forw == NULL)	/* no instructions? */
		return;
	addtail(lastnode);
	filter();			/* print non-text */
	printf("\t.text\n");
	do
	{
		ndisc0 = ndisc;
		wwchanged = false;
		optim();		/* machine-independent */
		window(3, w3optim);	/* machine-dependent */
		ldanal();		/* recompute live/dead status */
		window(1, w1optim);	/* machine-dependent */
		ldanal();		/* recompute live/dead status */
		window(2, w2optim);	/* machine-dependent */
	}
	while (ndisc > ndisc0 || wwchanged == true);
	prtext();			/* print new text */
	init();				/* re-initialize */
	codeseen = false;
}

zapwhite(s,n)
        register char *s;
        register n;
{
        register char *t;
        register cn = 0;

        for ( t = &s[--n]; *t == ' ' || *t == '\t'; *t-- = '\0' )
                cn++;
        return cn;
}
