/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

%{
#ident	"@(#)mkboot-3b2:mkboot.y	1.8"

# include	<stdio.h>
# include	"mkboot.h"

struct	flag
	{
	short	bits;
	short	vec;
	};

static struct flag *cvtflag();
%}


%union	{
	char			word[DIRSIZ+1];
	char			*string;
	struct tnode		*tptr;
	struct format		*fptr;
	int			number;
	struct flag		flag;
	}

%token	<word>		HWORD
%token	<number>	NUMBER
%token	<string>	STRING DWORD ROUTINE

%type	<word>		header prefix
%type	<string>	routine variable
%type	<flag>		flag
%type	<number>	master nvector
%type	<number>	software ndevice ipl dependencies data type
%type	<number>	dimension size initialization initializer
%type	<fptr>		element
%type	<tptr>		expression initialvalue

%left	'+' '-'
%left	'*' '/'

%%

%{
static struct format *nextformat;	/* -> next format[] during initializer list processing */
%}

master		:		header data
				{
				master.magic = MMAGIC;
				}
		;

header		:	flag nvector prefix software ndevice ipl dependencies
				{
				master.flag |= $1.bits;
				master.vec = $1.vec;
				master.nvec = $2;
				strncat( strcpy(master.prefix,""), $3, sizeof(master.prefix)-1 );
				master.soft = $4;
				master.ndev = $5;
				master.ipl = $6;
				master.ndep = $7;
				}
		|	flag nvector prefix software ndevice ipl
				{
				master.flag |= $1.bits;
				master.vec = $1.vec;
				master.nvec = $2;
				strncat( strcpy(master.prefix,""), $3, sizeof(master.prefix)-1 );
				master.soft = $4;
				master.ndev = $5;
				master.ipl = $6;
				master.ndep = 0;
				}
		|	flag nvector prefix software ndevice
				{
				master.flag |= $1.bits;
				master.vec = $1.vec;
				master.nvec = $2;
				strncat( strcpy(master.prefix,""), $3, sizeof(master.prefix)-1 );
				master.soft = $4;
				master.ndev = $5;
				master.ipl = 0;
				master.ndep = 0;
				}
		|	flag nvector prefix software
				{
				master.flag |= $1.bits;
				master.vec = $1.vec;
				master.nvec = $2;
				strncat( strcpy(master.prefix,""), $3, sizeof(master.prefix)-1 );
				master.soft = $4;
				master.ndev = 0;
				master.ipl = 0;
				master.ndep = 0;
				}
		|	flag nvector prefix
				{
				master.flag |= $1.bits;
				master.vec = $1.vec;
				master.nvec = $2;
				strncat( strcpy(master.prefix,""), $3, sizeof(master.prefix)-1 );
				master.soft = 0;
				master.ndev = 0;
				master.ipl = 0;
				master.ndep = 0;
				}
		|	flag nvector
				{
				master.flag |= $1.bits;
				master.vec = $1.vec;
				master.nvec = $2;
				master.prefix[0] = '\0';
				master.soft = 0;
				master.ndev = 0;
				master.ipl = 0;
				master.ndep = 0;
				}
		|	flag
				{
				master.flag |= $1.bits;
				master.vec = $1.vec;
				master.nvec = 0;
				master.prefix[0] = '\0';
				master.soft = 0;
				master.ndev = 0;
				master.ipl = 0;
				master.ndep = 0;
				}
		;

flag		:	HWORD
				{
				struct flag *f;

				if ( (f=cvtflag($1)) == NULL )
					YYERROR;

				$$ = *f;
				}
		|	'-'
				{
				$$ = *cvtflag("none");
				}
		;

nvector		:	'-'
				{
				$$ = 0;
				}
		|	NUMBER
		;

prefix		:	'-'
				{
				strcpy( $$, "" );
				}
		|	HWORD
				{
				if ( strlen($1) >= sizeof(master.prefix) )
					yyerror( "prefix \"%s\" truncated to %d characters", $1, sizeof(master.prefix)-1 );
				}
		;

software	:	'-'
				{
				$$ = 0;
				}
		|	NUMBER
		;

ndevice		:	'-'
				{
				$$ = 0;
				}
		|	NUMBER
		;

ipl		:	'-'
				{
				$$ = 0;
				}
		|	NUMBER
		;

/*
 * <dependencies> will maintain the total number of struct depend elements
 */
dependencies	:	dependencies ',' HWORD
				{
				if ( ndepend-depend >= MAXDEP )
					{
					yyerror( "too many dependencies" );
					YYERROR;
					}

				if ( nstring-string >= MAXSTRING-strlen($3)-1 )
					{
					yyerror( "string table overflow" );
					YYERROR;
					}

				ndepend->name = OFFSET( strcpy(nstring,ucase($3)), string );
				nstring += strlen(nstring) + 1;
				++ndepend;
				$$ = $1 + 1;
				}
		|	HWORD
				{
				if ( nstring-string >= MAXSTRING-strlen($1)-1 )
					{
					yyerror( "string table overflow" );
					YYERROR;
					}

				ndepend->name = OFFSET( strcpy(nstring,ucase($1)), string );
				nstring += strlen(nstring) + 1;

				++ndepend;
				$$ = 1;
				}
		;

data		:	/* empty */
				{
				ntree = tree;
				master.nrtn = 0;
				master.nvar = 0;
				}
		|	data routine
				{
				++master.nrtn;
				++nroutine;
				}
		|	data variable
				{
				ntree = tree;
				++master.nvar;
				++nvariable;
				}
		;

routine		:	ROUTINE '{' type '}'
				{
				if ( nroutine-routine >= MAXRTN )
					{
					yyerror( "too many routine definitions" );
					YYERROR;
					}

				nroutine->id = $3;
				nroutine->name = OFFSET( $1, string );
				}
		;

/*
 * <type> is the routine type: RNULL, RNOSYS, RNODEV, etc.
 */
type		:	/* empty */
				{
				$$ = RNULL;
				}
		|	DWORD
				{
				static struct
					{
					char	*name;
					int	flag;
					}
					table[] ={
						{ "nosys", RNOSYS },
						{ "nodev", RNODEV },
						{ "true", RTRUE },
						{ "false", RFALSE },
						{ "nopkg", NOPKG},
						{ "noreach", NOREACH},
						0
						};
				int i;

				lcase( $1 );

				for( i=0; table[i].name; ++i )
					if ( 0 == strcmp(table[i].name,$1) )
						break;

				if ( table[i].name )
					$$ = table[i].flag;
				else
					{
					yyerror( "unknown routine type \"%s\"", $1 );
					YYERROR;
					}
				}
		;

variable	:	DWORD dimension '(' size ')' initialization
				{
				if ( nvariable-variable >= MAXVAR )
					{
					yyerror( "too many variable definitions" );
					YYERROR;
					}

				nvariable->name = OFFSET( $1, string );
				nvariable->dimension = $2;
				nvariable->size = getsize( (struct format*)POINTER(nvariable->initializer,format), $4 );

				/*
				 * reclaim format[] elements
				 */
				if ( (nvariable->ninit = $6) == 0 )
					nformat = (struct format*)POINTER(nvariable->initializer,format);
				else
					nformat = (struct format*)POINTER(nvariable->initializer,format) + $6;
				}
		;

/*
 * <dimension> will either be NULL or OFFSET to expression
 */
dimension	:	/* empty */
				{
				$$ = NULL;
				}
		|	'[' expression ']'  
				{
				$$ = OFFSET( nelement, element );
				polish( $2 );
				ntree = tree;
				}
		;

/*
 * <size> will maintain total number of struct format elements;
 * nvariable->initializer = OFFSET( &format[first], format )
 */
size		:	size element
				{
				$$ = $1 + 1;
				}
		|	element
				{
				if ( nvariable-variable >= MAXVAR )
					{
					yyerror( "too many variable definitions" );
					YYERROR;
					}

				/*
				 * remember first struct format element
				 */
				nvariable->initializer = OFFSET( $1, format );
				$$ = 1;
				}
		;

/*
 * <element> = &format[]
 */
element		:	'%' NUMBER
				{
				if ( nformat-format >= MAXINIT )
					{
					yyerror( "too many size specifiers" );
					YYERROR;
					}

				nformat->type = FSKIP;
				nformat->strlen = 0;
				nformat->value = $2;
				$$ = nformat++;
				}
		|	'%' NUMBER DWORD
				{
				char *p;

				if ( nformat-format >= MAXINIT )
					{
					yyerror( "too many size specifiers" );
					YYERROR;
					}

				if ( strlen($3) != 1 || (p=strchr("cC",*$3)) == NULL || $2 == 0 )
					{
					yyerror( "\"%%%d%s\" is an illegal length specifier", $2, $3 );
					YYERROR;
					}

				if ( $2 > 255 )
					{
					yyerror( "character strings must not be larger than 255 characters" );
					YYERROR;
					}

				nformat->type = FSTRING;
				nformat->strlen = $2;
				nformat->value = 0;

				$$ = nformat++;
				}
		|	'%' DWORD
				{
				char *p;

				if ( nformat-format >= MAXINIT )
					{
					yyerror( "too many size specifiers" );
					YYERROR;
					}

				if ( strlen($2) != 1 || (p=strchr("cCsSiIlL",*$2)) == NULL )
					{
					yyerror( "\"%%%s\" is an illegal length specifier", $2 );
					YYERROR;
					}
				else
					switch( *p )
						{
					case 'c':
					case 'C':
						nformat->type = FCHAR;
						break;
					case 's':
					case 'S':
						nformat->type = FSHORT;
						break;
					case 'i':
					case 'I':
						nformat->type = FINT;
						break;
					case 'l':
					case 'L':
						nformat->type = FLONG;
						break;
						}

				nformat->strlen = 0;
				nformat->value = 0;
				$$ = nformat++;
				}
		;

/*
 * <initialization> will maintain total number of initializer expressions;
 * that is, the total number of struct format items used beginning with
 * nvariable->initializer
 */
initialization	:	/* empty */
				{
				$$ = 0;
				}
		|	'=' '{' initializer '}'
				{
				$$ = $3;
				}
		;

initializer	:	initializer ',' initialvalue
				{
				while( (++nextformat)->type & FSKIP && nextformat <= nformat )
					$1 = $1 + 1;

				if ( nextformat >= nformat )
					{
					yyerror( "more initializers than size specifiers" );
					YYERROR;
					}

				if ( (nextformat->type & FTYPE) == FSTRING )
					{
					char type, *svalue;
					int nvalue;
					struct param *pp;

					switch( type = $3->type )
						{
					case 'N':
						nvalue = $3->value;
						break;
					case '"':
						svalue = $3->name;
						break;
					case 'I':
						if ( (pp=findparam($3->name)) != NULL )
							switch( type = pp->type )
								{
							case 'N':
								nvalue = pp->value.number;
								break;
							case '"':
								svalue = (char*) POINTER(pp->value.string,string);
								break;
								}

						if ( type == 'N' || type == '"' || type == 'I' )
							break;
					default:
						yyerror( "illegal initialization for %%%dc", nextformat->strlen );
						YYERROR;
						}

					if ( type == 'N' && nvalue != 0 )
						{
						yyerror( "numeric initialization must be zero for %%%dc", nextformat->strlen );
						YYERROR;
						}

					if ( type == '"' && strlen(svalue) > nextformat->strlen )
						{
						yyerror( "string initialization too long for %%%dc", nextformat->strlen );
						YYERROR;
						}
					}

				if ( $3->type == 'N' && $3->value <= MAXLITERAL && $3->value >= -MAXLITERAL-1 )
					nextformat->value = $3->value;
				else
					{
					nextformat->type |= FEXPR;
					nextformat->value = OFFSET( nelement, element );
					polish( $3 );
					}

				ntree = tree;

				$$ = $1 + 1;
				}
		|	initialvalue
				{
				$$ = 0;

				/*
				 * start with first struct initializer for this variable
				 */
				nextformat = (struct format *) POINTER(nvariable->initializer,format);

				while( nextformat->type & FSKIP )
					{
					$$ = $$ + 1;
					if ( ++nextformat >= nformat )
						{
						yyerror( "more initializers than size specifiers" );
						YYERROR;
						}
					}

				if ( (nextformat->type & FTYPE) == FSTRING )
					{
					char type, *svalue;
					int nvalue;
					struct param *pp;

					switch( type = $1->type )
						{
					case 'N':
						nvalue = $1->value;
						break;
					case '"':
						svalue = $1->name;
						break;
					case 'I':
						if ( (pp=findparam($1->name)) != NULL )
							switch( type = pp->type )
								{
							case 'N':
								nvalue = pp->value.number;
								break;
							case '"':
								svalue = (char*) POINTER(pp->value.string,string);
								break;
								}

						if ( type == 'N' || type == '"' || type == 'I' )
							break;
					default:
						yyerror( "illegal initialization for %%%dc", nextformat->strlen );
						YYERROR;
						}

					if ( type == 'N' && nvalue != 0 )
						{
						yyerror( "numeric initialization must be zero for %%%dc", nextformat->strlen );
						YYERROR;
						}

					if ( type == '"' && strlen(svalue) > nextformat->strlen )
						{
						yyerror( "string initialization too long for %%%dc", nextformat->strlen );
						YYERROR;
						}
					}

				if ( $1->type == 'N' && $1->value <= MAXLITERAL && $1->value >= -MAXLITERAL-1 )
					nextformat->value = $1->value;
				else
					{
					nextformat->type |= FEXPR;
					nextformat->value = OFFSET( nelement, element );
					polish( $1 );
					}

				ntree = tree;

				$$ = $$ + 1;
				}
		;

initialvalue	:	/* empty */
				{
				$$ = node('N',(struct tnode*)NULL,(struct tnode*)NULL,0);
				}
		|	expression
		;

/*
 * <expression> is the struct tnode pointer to the root of the expression tree
 */
expression	:	expression '+' expression
				{
				$$ = node('+',$1,$3,NULL);
				}
		|	expression '-' expression
				{
				$$ = node('-',$1,$3,NULL);
				}
		|	expression '*' expression
				{
				$$ = node('*',$1,$3,NULL);
				}
		|	expression '/' expression
				{
				$$ = node('/',$1,$3,NULL);
				}
		|	DWORD '(' expression ',' expression ')'
				{
				static struct
					{
					char	*name;
					char	type;
					}
					table[] ={
						{ "min", '<' },
						{ "max", '>' },
						0
						};
				int i;

				lcase( $1 );

				for( i=0; table[i].name; ++i )
					if ( 0 == strcmp(table[i].name,$1) )
						break;

				if ( table[i].name )
					$$ = node(table[i].type,$3,$5,NULL);
				else
					{
					yyerror( "unknown function \"%s\"", $1 );
					YYERROR;
					}
				}
		|	'(' expression	')'
				{
				$$ = $2;
				}
		|	'#' DWORD
				{	
				static char *table[] ={ "C", "D", "M", "c", "d", "m", NULL };
				char *p, **q = table;

				while( p = *q++ )
					if ( 0 == strcmp(p,$2) )
						break;

				if ( p )
					$$ = node(*ucase($2),(struct tnode*)NULL,(struct tnode*)NULL,NULL);
				else
					$$ = node('#',(struct tnode*)NULL,(struct tnode*)NULL,$2);
				}
		|	'#' DWORD '(' DWORD ')'
				{	
				static char *table[] ={ "C", "D", "M", "c", "d", "m", NULL };
				char *p, **q = table;

				while( p = *q++ )
					if ( 0 == strcmp(p,$2) )
						break;

				if ( p )
					$$ = node(*ucase($2),(struct tnode*)NULL,(struct tnode*)NULL,$4);
				else
					{
					yyerror( "unknown variable reference \"#%s(%s)\"", $2, $4 );
					YYERROR;
					}
				}
		|	'&' DWORD
				{
				$$ = node('&',(struct tnode*)NULL,(struct tnode*)NULL,$2);
				}
		|	STRING
				{
				$$ = node('"',(struct tnode*)NULL,(struct tnode*)NULL,$1);
				}
		|	NUMBER
				{
				$$ = node('N',(struct tnode*)NULL,(struct tnode*)NULL,$1);
				}
		|	DWORD
				{
				if ( strlen($1) > PARAMNMSZ )
					$1[PARAMNMSZ] = '\0';

				$$ = node('I',(struct tnode*)NULL,(struct tnode*)NULL,$1);
				}
		;

%% /*start of programs */

/*
 * yyfatal( message, a1, a2, a3, a4, a5 )
 *
 * Printf for fatal error messages; error message is:
 *
 *	mkboot: /etc/master: line 0, <message>
 */
 /*VARARGS1*/
 void
yyfatal( message, a1, a2, a3, a4, a5 )
	char *message;
	int	a1, a2, a3, a4, a5;
	{
	yyerror( message, a1, a2, a3, a4, a5 );

	if ( jmpbuf != NULL )
		longjmp( jmpbuf );
	else
	exit( 1 );
	}

/*
 * yyerror( message, a1, a2, a3, a4, a5 )
 *
 * Printf for warning error messages; error message is:
 *
 *	mkboot: /etc/master: line 0, <message>
 */
 /*VARARGS1*/
 void
yyerror( message, a1, a2, a3, a4, a5 )
	char *message;
	int	a1, a2, a3, a4, a5;
	{
	extern char master_file[];
	extern char **Argv;
	extern int yylineno;
	char buffer[256];

	any_error = TRUE;

	sprintf( buffer, message, a1, a2, a3, a4, a5 );

	fprintf( stderr, "%s: %s: line %d, %s\n",
			*Argv, master_file, yylineno, buffer );
	}

/*
 * fatal( message, a1, a2, a3, a4, a5 )
 *
 * Printf for fatal error messages; error message is:
 *
 *	mkboot: <message>
 */
 /*VARARGS1*/
 void
fatal( message, a1, a2, a3, a4, a5 )
	char	*message;
	int	a1, a2, a3, a4, a5;
	{
	warn( message, a1, a2, a3, a4, a5 );

	if ( jmpbuf != NULL )
		longjmp( jmpbuf );
	else
		exit( 1 );
	}

/*
 * warn( message, a1, a2, a3, a4, a5 )
 *
 * Printf for warning error messages; error message is:
 *
 *	mkboot: <message>
 */
 /*VARARGS1*/
 void
warn( message, a1, a2, a3, a4, a5 )
	char	*message;
	int	a1, a2, a3, a4, a5;
	{
	extern	char	**Argv;
	char buffer[256];

	any_error = TRUE;

	sprintf( buffer, message, a1, a2, a3, a4, a5 );

	fprintf( stderr, "%s: %s\n", *Argv, buffer );
	}

/*
 * This routine converts the alphanumeric string of flags
 * (from /etc/master) into the correct bit flags and vector number.
 */
 static
 struct flag*
cvtflag(string)
register  char  *string;
{
	register  char  	c;
	char			*f = string;
	char			once = FALSE;
	static struct flag	flag;

	flag.bits = 0;
	flag.vec = 0;

	if (strcmp(f,"none") == 0)
		return( &flag );

	while(c = *f++)
		switch (c) {
		case 'o':
			flag.bits |= ONCE;
			break;
		case 'r':
			flag.bits |= REQ;
			break;
		case 'b':
			flag.bits |= BLOCK;
			break;
		case 'c':
			flag.bits |= CHAR;
			break;
		case 'a':
			flag.bits |= REQADDR;
			break;
		case 's':
			flag.bits |= SOFT;
			break;
		case 't':
			flag.bits |= TTYS;
			break;
		case 'x':
			flag.bits |= NOTADRV;
			break;
		case 'f':
			flag.bits |= FUNDRV;
			break;
		case 'm':
			flag.bits |= FUNMOD;
			break;
		case 'j':
			flag.bits |= FSTYP;
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			if ( once )
				{
				yyerror( "multiple vector numbers in \"%s\"", string );
				return( NULL );
				}
			once = TRUE;
			flag.vec = strtol( f-1, &f, 0 );
			break;
		default:
			yyerror( "illegal device flags \"%s\"", string );
			return( NULL );
		}
	return( &flag );
}

/*
 * node( type, left, right, value )
 *
 * Create a node in an expression tree.  Fold constant expressions if
 * possible
 */
 struct tnode *
node(type,left,right,value)
	char		type;
	struct tnode	*left, *right;
	union	{
		int	number;
		char	*string;
		}
		value;
	{

	if ( ntree-tree >= MAXNODE )
		yyfatal( "too many expression nodes" );

	ntree->left = left;
	ntree->right = right;

	switch( ntree->type = type )
		{
	case '+':
	case '-':
	case '*':
	case '/':
	case '<':
	case '>':
		if ( left->type == 'N' && right->type == 'N' )
			/*
			 * fold constant expressions
			 */
			{
			switch( type )
				{
			case '+':
				left->value = left->value + right->value;
				break;
			case '-':
				left->value = left->value - right->value;
				break;
			case '*':
				left->value = left->value * right->value;
				break;
			case '/':
				if ( right->value == 0 )
					left->value = 0;
				else
					left->value = left->value / right->value;
				break;
			case '<':
				left->value = min( left->value, right->value );
				break;
			case '>':
				left->value = max( left->value, right->value );
				break;
				}

			return( left );
			}
		break;
	case 'C':
	case 'D':
	case 'M':
	case '#':
	case '&':
	case '"':
	case 'I':
		ntree->name = value.string;
		break;
	case 'N':
		ntree->value = value.number;
		break;
	default:
		yyfatal( "illegal node type \"%c\" for expression", type );
		}

	return( ntree++ );
	}

# include "lex.yy.c"


/*
 * reset lex to its initial state prior to each file
 */
lexinit()
	{
	BEGIN INITIAL;
	NLSTATE;
	yymorfg = 0;
	yysptr = yysbuf;
	}
