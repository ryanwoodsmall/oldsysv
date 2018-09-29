%{
/*	@(#)local.l	1.4	*/

#include	"optim.h"
#include	<ctype.h>

static	boolean	codeseen;		/* have we seen anything to improve? */
static	int	opn;			/* current operand index */
static	int	op;			/* temp to hold value from oplook() */
extern	boolean	wwchanged;		/* did any window change? */

extern	USERTYPE instlook();		/* returns ptr into instruction table */

/* Tune yylex() */
#undef	YYLMAX
#define	YYLMAX	4096

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

<D>{ILABEL}$		{
				/* catch all referenced labels in data */
				ECHO;
				addref(yytext, yyleng + 1);
			}

<T>[^:\n]+:\n		{
				/* make note of a text label */
				yytext[yyleng - 2] = '\0';	/*zap ':'*/
				applbl(yytext, yyleng);
				codeseen = true;
			}


<FP0>[^:\n]+:\n		{
				/* add function label */
				myBEGIN(FP1);
				yytext[yyleng - 2] = '\0';	/*zap ':'*/
				applbl(yytext, yyleng);
			}

<FP1>\.word\t.*\n	{
				/* and save register mask */
				myBEGIN(T);
				yytext[yyleng - 1] = '\0';	/*zap '\n'*/
				appmisc(yytext, yyleng);
			}

<CH>[^:\n]+:		{
				/* Should be operand 4 */
				saveop(opn, yytext, yyleng + 1, 0);
				opn++;
			}

<CH>(\t?\.word\t{ILABEL}-{ILABEL}\n)+	{
						vax_case();
						myBEGIN(T);
					}

<T>[a-z0-9#]+		{
				register USERTYPE ip;
				opn = 0;	/* 0th operand */
				if (!(ip = instlook(yytext)))
				{
					op = OTHER;
					saveop(opn, yytext, yyleng - 1, op);
					fprintf(stderr,
						"Warning: unrecognized instruction [%*s]\n",
						yyleng - 1, yytext);
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

<T1>\n			{
				/* the end of the instruction */
				/* deal with case[bwl] displacement table*/
				/* note pseudos are handled below */
				switch (op)
				{
				case CASE:
					myBEGIN(CH);
					break;

				default:
					myBEGIN(T);
					break;
				}
			}

<T2>[^,\n]*		{
				/* stash text up to next delimiter */
				myBEGIN(T1);
				saveop(opn, yytext, yyleng + 1, 0);
				opn++;
			}

<T>\n			;

<T,D>\.[a-z0-9#]+	{
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

<P0>\n			{
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

				case PBYTE:
				case PDOUBL:
				case PFLOAT:
				case PLONG:
				case PSPACE:
					assert(oldstate == D);
				case PALIGN:
					if (oldstate == T)
					{
						/* process previous function */
						endfunc();
						/* start function prolog */
						myBEGIN(FP0);
						break;
					}
				case PWORD:	/* reg save mask is in text */
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

<P1>[^\n]+		{
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

vax_case()
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
	extern	boolean	fixpeep();
	extern	boolean	w1optim();
	extern	boolean	w2optim();
	extern	boolean	w3optim();
	register int	ndisc0;

	if (codeseen == false || n0.forw == NULL)	/* no instructions? */
		return;
	addtail(lastnode);
	filter();			/* print non-text */
	printf("\t.text\n\t.align\t4\n");
	window(1, fixpeep);		/* fixup pushl/movl $Lx */
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
