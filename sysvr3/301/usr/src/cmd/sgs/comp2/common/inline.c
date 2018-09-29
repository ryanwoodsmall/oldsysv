/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcc2:common/inline.c	10.1"
#include <ctype.h>
#include "mfile2.h"

/* An implementation of inline asm expansion with arguments.
/* 'asm' pseudo-function definitions, whose bodies contain code 
/* to be expanded into the assembly code output depending on the
/* storage locations of their arguments, are recognized by the
/* compiler front end and stored away.
/* When asm function calls are expanded in-line,
/* arguments are evaluated left-to-right if they're
/* not LTYPEs (leaf types, i.e. easy addressing modes).
/* Finally,
/* we grovel around in the temp file stashed by squirrelmacro, which
/* is called from the parser, find the definition, expand it, replacing
/* the "arguments" by the adrput of the nodes in question.
/* If the arguments do not match any of the storage classes specified
/* in the expansion directive, a regular function call is generated.
/* 
/* This version, implemented for PCC2, does *NOT* have the alternate option
/* of inserting a macro line into the assembly code output, as does the
/* original version implemented in PCC1.
/*
/* The rest of the PCC2 compiler has to be compiled with the IN_LINE
/* symbol defined in order to turn on the asm psuedo-function expansion
/* feature.
 */


struct Sym_ent { char *name; int class; NODE *pnode; };

#define N_MAC_ARGS 10

static struct Sym_ent macarg_tab[N_MAC_ARGS];

extern int asmdebug;

extern char inlargs[];
extern int ninlargs;

static int nargs;

static int expanding = 0;		/* non-zero if currently expanding */

genicall(p, goal)
register NODE *p;
int goal;
{
	if (expanding) {
	    uerror("Nested asm calls not now supported");
	    return( 1 );
	}
	expanding = 1;
	nargs = 0;
	if (p->in.op == INCALL && genfargs(p->in.right)) return(1);
	if (p->in.left->in.op != ICON)
		cerror("asm pseudo-function must be called directly");
	inlexpand(p, goal);
	clmacst();
	expanding = 0;
	return (0);
}

static
genfargs(p)
register NODE *p;
{
    OPTAB *q;
    NODE *aop, *l, *r;
    int i, flag;

    if (p->in.op == CM)
	return( genfargs(p->in.left) || genfargs(p->in.right) );

    nargs = nargs + 1; 		/* count arguments as we go */

    /* want the left subtree of the ARG node 
    */
    p = p->in.left;

    /* had better be an evaluable expression */
    if (optype(p->in.op) != LTYPE)	/* don't bother if it's a leaf */
    {
	again :
	    rewcom( p, NRGS );

#ifndef	STINCC
	    if( costs(p) ) goto again;

	    nins = 0;
	    insout(p,NRGS);
#else
	    nins = 0;
	    switch (INSOUT( p, NRGS ))
	    {
	    case REWROTE:
		goto again;		/* tree was rewritten */

	    case OUTOFREG:
		e2print( p );
		cerror("In-line runs out of registers\n");
	    /* default:  fall thru */
	    }
#endif

	    flag = 0;
	    for( i=0; i<nins; ++i)
	    {
		if( inst[i].goal == CTEMP )
		    if( rewsto( inst[i].p ) ) 
			flag = 1;
	    }

	    if( flag ) goto again;

	    insprt();

	    /* have result in a register.  create a tree storing it to
	    /* a temp, and copy the temp into the argument tree.
	    /* musical nodes:
	    /* create a temp for the lhs of the store, copy the register 
	    /* result in p into the rhs, copy the temp into the argument
	    /* tree, then fill in the assignment node and produce
	    /* code for the store.
	    */
	    aop = talloc();
	    l = talloc();
	    r = talloc();

	    l->tn.op = TEMP;
	    l->tn.lval = freetemp( argsize(p)/SZINT );
	    l->tn.lval = BITOOR( l->tn.lval );
	    l->tn.name = (char *) 0;
	    l->tn.type = p->tn.type;

	    *r = *p;
	    *p = *l;
	    aop->in.op = ASSIGN;
	    aop->in.left = l;
	    aop->in.right = r;
	    aop->in.type = p->tn.type;

#ifndef	STINCC
	    costs(aop);
	    nins = 0;
	    insout(aop,CEFF);
#else
	    nins = 0;
	    if (INSOUT(aop, CEFF))	/* expect zero return */
		cerror("In-line fails to assign to TEMP\n");
#endif
	    insprt();

    }
    return(0);
}


extern FILE *inlfp;
static char sbuf[BUFSIZ];
static char *inlname;


/* define some tokens */
#define IT_undet	(-1)
#define	IT_done		0
#define IT_class	1
#define IT_name		2
#define	IT_comma	3
#define	IT_semi		4
#define IT_sym		5

#define ISCL_undef	0
#define ISCL_ureg	1
#define ISCL_treg	2
#define ISCL_reg	3
#define	ISCL_const	4
#define	ISCL_mem	8
#define ISCL_lab	16


static
inlexpand(p, goal)
register NODE *p;
int goal;
{
	/* find the expansion */
	register long seekaddr;
	char *s;
	register char *t;

	fseek(inlfp, 0L, 0);		/* rewind macro stash file */
	inlname = p->in.left->tn.name;
	while (t = fgets(sbuf, BUFSIZ, inlfp))
	{
		if (*t != '$')		/* look for macro name */
		    continue;		/* try next line */

		/* assume length of name < BUFSIZ, so \n ends line */
		*(t+strlen(t)-1) = '\0'; /* overwrite \n with NUL */

		/* check name match */
		if (strcmp(t+1, inlname) == 0)  {

	/* Read in number of arguments and list of argument names 
	/* Leave initial '#'.
	/* Change name delimiting '#'s to '\0's and final '\0' to '\n'
	*/

			s = fgets( inlargs, BUFSIZ, inlfp);
			if ( *s != '#') 
		    	    cerror(" error in asm formal argument storage");
			s++;
			while( *s != '\0' )
			    { if ( *s == '#' )  *s = '\0' ;
			      s++;
			    }
			*s = '\n';
			/* get number of arguments */
			ninlargs = atoi( inlargs + 1 );

	/* now we're ready for the works--first, try to find the %
	   line that applies to the arguments we're stuck with */

			while (s = fgets(sbuf, BUFSIZ, inlfp))
				if (*s++ == '%')
				{
					if (*s == '\n' || *s == '\0')
					{
					/* no match found change to a call */
					    if ( ninlargs || nargs )
						if ( p->in.op == INCALL )
						    p->in.op = CALL;
						else
						    p->in.op = UNARY CALL;
					    else
						{
						/* no arguments always match 
						/* have  to rewind to start of
						/* def.
						*/
						fseek(inlfp, 0L, 0);
						while (t = fgets(sbuf, BUFSIZ, inlfp))
						{
							if (*t != '$')		/* look for macro name */
							    continue;		/* try next line */

							/* assume length of name < BUFSIZ, so \n ends line */
							*(t+strlen(t)-1) = '\0'; /* overwrite \n with NUL */

							/* check name match */
							if (strcmp(t+1, inlname) == 0) break;

						}
						/* advance to # line after %name line */
						t = fgets( sbuf, BUFSIZ, inlfp );

						expmac();
						}
					}
					else if (iargmatch(s, p))
						expmac(); /* got it -- expand */
					else
					{
						while (isspace(*s))
							++s;
						t = s;
						if ((gettok(&s))==IT_sym &&
						    !strcmp(t, "error"))
							uerror("%%error specification reached in asm expansion -- check arguments");
						else
							continue;     
							  /* strange loop */
					}
				fseek(inlfp, 0L, 2);
				      /* in case we see another def */
				return;
				}
			}
	}
	cerror("inlexpand: asm expansion runs off end of file");
	/*NOTREACHED*/
}


static int isclass = ISCL_undef;

static
iargmatch(s, p)
char *s;
NODE *p;
{
	/* line looks like "[<class> <name> [, <name>]* ; ]*\n" */
	/* tree looks like   (AWD is addressable word)

		  INCALL
		 /      \
	ICON (name)      ,
		       [/ \
		      ARG  ARG
	 	      /    / 
		    AWD   AWD ]*
	*/
	/* nice to have coroutines here--one to prewalk the arg tree, */
	/* another to parse the %args string */

	char *curtok;
	int  curarg = 0;

	clmacst();

#ifndef NODBG
	if (asmdebug)
	{
		printf("iargmatch: nargs = %d, p = \n", ninlargs);
		e2print(p);
	}
#endif

	while (isspace(*s))
		++s;
	while (isdigit(*s))
		++s;
	if ( ninlargs && p->in.op == INCALL)
		return (pcllist(&s, p->in.right, &curarg));
	else
	{
		pcllist(&s, p, &curarg);	/* to expand any labels */
		return (!ninlargs);
	}
}


static
pcllist(s, p, pcurarg)
char **s;
NODE *p;
int *pcurarg;
{
	int retval;
	char *t = *s, errbuf[BUFSIZ];

	switch(gettok(s))
	{
		case IT_done:
			if (*pcurarg != nargs)
			    return (0);
			else
			    return (1);

		case IT_semi:
			return(pcllist(s, p, pcurarg));

		case IT_class:
			return (pfargs(s, p, pcurarg) &&
			    pcllist(s, p, pcurarg));

		default:
			sprintf(errbuf,
			    "syntax error in asm argument list\nsee %s, expecting a storage class\nasm function %s",
			    t, inlname);
			uerror(errbuf);
			return (0);
	}
}


static
pfargs(s, p, pcurarg )
char **s;
NODE *p;
int *pcurarg;
{
	/* arg [, arg]* ; */
	/* install in table with current class */
	char *t;
	NODE *mklabnode(), *getnth(), *q;
	
	while (*s)
	{
		while (isspace(**s))
			++*s;
		t = *s;
		switch (gettok(s))
		{
			case IT_sym:
				if (isclass == ISCL_lab)
					q = mklabnode();
				else
				    {
					++*pcurarg;
					q = getnth(p, whatn(t));
				    }
				if (!arg_install(t, q, isclass))
				{
					clmacst();
					return (0);
				}
				break;

			case IT_semi:
			case IT_done:
				return (1);

			default:
				uerror ("asm expansion: expecting argument declaration");
				return (0);
		}
	}
	cerror("asm expansion error: out of switch in pfargs");
	/*NOTREACHED*/
}

static NODE *
mklabnode()
{
	static int labno;
	NODE *talloc(), *p;

	p = talloc();
	p->tn.op = ICON;
	p->tn.name = "";
	p->tn.lval = ++labno;
	return (p);
}

static NODE *
getnth(p, n)
register NODE *p;
int n;
	/* Returns pointer to nth argument on list (left subtree of
	/* nth ARG node) or NULL if there isn't an nth argument.
	/* First argument the deepest, figure how deep to go
	/* from total number of parameters.
	*/
{
	int howdeep;

	if ( n > nargs ) return( (NODE*) 0 );

	for ( howdeep = nargs-n; howdeep > 0; --howdeep )
	    p = p->in.left;
	if ( p->in.op == CM )  return( p->in.right->in.left );
	else  return( p->in.left );
}


static struct Sym_ent *
mac_lookup(name)
char *name;
{
	register int n = N_MAC_ARGS;
	struct Sym_ent *p = macarg_tab;

	while (n--)
	{
		if (!p->name || !strcmp(p->name, name))
{
			return (p);
}
		else
			++p;
	}
	cerror("asm expansion error: off end of macro arg table");
	/*NOTREACHED*/
}


static
arg_install(name, p, class)
char *name;
NODE *p;
int class;
{
	char *malloc(), *strcpy();
	struct Sym_ent *psym;


	switch (class)		/* make sure it matches */
	{
		case ISCL_ureg:		/* user register */
			if (p->tn.op != REG || istreg(p->tn.rval))
				return (0);	/* so sorry */
			break;

		case ISCL_treg:		/* temp register */
			if (p->tn.op != REG || ! istreg(p->tn.rval))
				return (0);
			break;

		case ISCL_reg:
			if (p->tn.op != REG)
				return (0);
			break;

		case ISCL_lab:
		case ISCL_const:
			if (p->tn.op != ICON)
				return (0);
			break;

		case ISCL_mem:			/* applies to any?? */
			break;

		default:
			cerror("asm expansion error:  goofy class in arg_install");
			/*NOTREACHED*/
	}
	psym = mac_lookup(name);
	if (psym->name)
	{
		uerror("error in asm expansion: \n\tduplicate argument name in storage class specification");
		return (0);
	}
	if ((psym->name = malloc(strlen(name) + 1)) == NULL)
	{
		cerror("asm expansion error: out of space in arg_install");
		/*NOTREACHED*/
	}
	strcpy(psym->name, name);
	psym->class = class;
	psym->pnode = p;
	return (1);
}

static
clmacst()
{
	register int n = N_MAC_ARGS;
	register struct Sym_ent *p = macarg_tab;

	while (n--)
	{
		if (p->class == ISCL_lab)
			tfree(p->pnode);
		p->class = ISCL_undef;
		p->name = (char *) (p->pnode = (NODE *) NULL);
		p++;
	}
}

static
gettok(ps)
char **ps;
{
	register char *s;
	static int wassemi = 0;
	int retval;

	if (wassemi)
	{
		wassemi = 0;
		return( IT_semi );
	}

	while (isspace(**ps))
		++*ps;
	s = *ps;
	while (**ps != '\0' && !isspace(**ps) && **ps != ',' && **ps != ';' &&
		**ps != '#' && **ps != ':' && **ps != '\n' && **ps != '('
		&& **ps != ')' )
		++*ps;

	retval = IT_undet;

	/* check for lone delimeter */
	if ( *s == '\0' || *s == '\n' )
		retval = IT_done;
	else if ( *s == ';' ) 
		retval = IT_semi;
	else if ( s == *ps )
		retval = IT_sym;
	if ( retval != IT_undet ) 
	{
		*(*ps)++ = '\0';
		return( retval );
	}

	/* check longer strings */
	wassemi = (**ps == ';') ? 1 : 0;
	*(*ps)++ = '\0';
	if (!strcmp(s, "reg"))
	{
		isclass = ISCL_reg;
		retval = IT_class;
	}
	else if (!strcmp(s, "ureg"))
	{
		isclass = ISCL_ureg;
		retval = IT_class;
	}
	else if (!strcmp(s, "treg"))
	{
		isclass = ISCL_treg;
		retval = IT_class;
	}
	else if (!strcmp(s, "con"))
	{
		isclass = ISCL_const;
		retval = IT_class;
	}
	else if (!strcmp(s, "lab"))
	{
		isclass = ISCL_lab;
		retval = IT_class;
	}
	else if (!strcmp(s, "mem"))
	{
		isclass = ISCL_mem;
		retval = IT_class;
	}
	else
		retval = IT_sym;
	return (retval);
}

static
expmac()
{
	char *p, *m, mybuf[BUFSIZ], *strcpy();
	register char *s;
	register struct Sym_ent *q;
	int ngunk;

#ifdef ASM_COMMENT
	/* put out comment to warn optimizer */
	printf( "%s\n", ASM_COMMENT );
#endif


	while (p = fgets(sbuf, BUFSIZ, inlfp))
	{
		/* gettok is somewhat destructive of it's input, */
		/* determine what gets output quite carefully */

		if (*p == '%')
		{
#ifdef ASM_END
			/* another comment to mark end of expansions */
			printf( "%s\n", ASM_END  );
#endif
			return;
		}
		else
		{
			m = strcpy(mybuf, sbuf);
			while (1)
			{
				while (isspace(*p))
					++p;
				s = p;
				/* put out non-significant gunk */
				if ((ngunk = (s-sbuf) - (m-mybuf)) != 0)
					fwrite(m, ngunk, 1, stdout);
				switch(gettok(&p))
				{
				case IT_sym:		/* the tricky case */
					/* is it substitutible? */
					if ((q = mac_lookup(s))->name)
						if (q->class == ISCL_lab)
						{
							if (q->pnode->in.op != ICON)
							{
								cerror("asm expansion:  bad label argument type");
								/*NOTREACHED*/
							}
							else
								printf(".ASM%d", q->pnode->tn.lval);
						}
						else
							adrput(q->pnode);
					else
						fputs(s, stdout);
					m += ngunk + strlen(s);
					continue;
				
				case IT_done:
					break;	/* only way out */

				default:
					fputs(s, stdout);
					m += ngunk + strlen(s);
					continue;
				}
				break;	/* out of while */
			}
		}
	}
	cerror("asm expansion:  off end of file in expmac");
	/*NOTREACHED*/
}



squirrelmacro(name) 	/* called from grammar to put it away */
char *name;
{
	int c;

	/* use external form of name, because that's what we'll see in
	** pass2 when we expand the macro
	*/
	fprintf(inlfp, "$%s\n", exname(name));
#ifndef NODBG
	if (asmdebug)
	{
		printf( "\n++++++ asm squirrel file written to:\n");
		printf("$%s\n", name);
	}
#endif

	/* write out number of formal arguments and
	/* formal argument names, if any, collected in inlargs[] by 
	/* ftnarg().  line has formal param names delimited 
        /* by '#'.  
        */

	fprintf(inlfp,"#%d#%s\n",ninlargs,inlargs);
#ifndef NODBG
	if (asmdebug)
		printf("#%d#%s\n",ninlargs,inlargs);
#endif

	while ((c = getchar()) != '}')
	{
		if (c == EOF)
		{
			cerror("EOF in asm pseudo-function definition");
			/*NOTREACHED*/
		}
		putc(c, inlfp);
#ifndef NODBG
	if (asmdebug)
		putchar(c);
#endif
		if (c == '\n')
		{
			++lineno;
			if ((c = getchar()) == '%')
			{
				fprintf(inlfp, "%% " );
#ifndef NODBG
				if (asmdebug)
					printf("%% " );
#endif
			}
			else
				ungetc(c, stdin);
		}
	}
	/* throw closing '%' into file to as endmarker */
	fputs("%\n", inlfp);
#ifndef NODBG
	if (asmdebug)
	{
		puts("%\n");
		printf("++++++\n\n");
	}
#endif
	/* make parser see "{}" to belive it's a function */
	ungetc('}', stdin);	
	/* clear argument list and count */
	inlargs[0] = '\0';
	ninlargs = 0;
}

whatn( argname )
char *argname;
{
    /* Count down list of arg names, looking for a match.
    /* Return ordinal of argument, if on list, or 0 if not found.
    /* The string searched through starts with '#' number-of-argumemts '#',
    /* followed by the each argument name (if any), each terminated with '\0'.
    /* The list is terminated by a '\n'
    */
    char *arg;
    int n, len;

    n = 1;
    arg = inlargs + 1;

    while (isdigit(*arg)) arg++;
    arg++;		/* past '/0' following number of arguments */

    while ( *arg != '\n' )
    {
	if ( !strcmp(argname,arg) )  return(n);
	while ( *arg != '\0' ) arg++;
	arg++;
	n++;
    }

    return(0);
}

