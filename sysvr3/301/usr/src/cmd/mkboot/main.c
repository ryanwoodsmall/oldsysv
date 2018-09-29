/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mkboot-3b2:main.c	1.4"

# include	<stdio.h>
# include	"mkboot.h"
# include	<ctype.h>
# include	<a.out.h>
# include	<fcntl.h>
# include	<errno.h>

extern	FILE		*yyin;		/* LEX's input stream */

jmp_buf			*jmpbuf;
char			any_error = FALSE;

struct master		master, *Opthdr;
struct depend		depend[MAXDEP], *ndepend;
struct param		param[MAXPARAM+1], *nparam;
struct routine		routine[MAXRTN], *nroutine;
struct variable		variable[MAXVAR], *nvariable;
struct format		format[MAXINIT], *nformat;
char			element[MAXEXPR+sizeof(union element)], *nelement;
char			string[MAXSTRING], *nstring;

struct	tnode		tree[MAXNODE], *ntree;

char *mfile	= "/etc/master.d";	/* master database directory */
char *boot	= "/boot";		/* boot directory */
char *usage	= "%s [-m master_directory] [-d boot_directory] [-k kernel] [objfile] ...";
char master_file[BUFSIZ];		/* current master file entity */

char	**Argv;
char	Debug;

main(argc,argv)	/* input command line */
	int argc;
	char *argv[];
	{
	extern	char	*optarg;
	extern	int	optind;

	int	c;
	char	*Usage = "Usage", *kernel = NULL, name[DIRSIZ+1], *p;

	Argv = &Usage;

	Debug = FALSE;
	while ((c = getopt(argc, argv, "xm:d:k:")) != EOF)
		switch (c) {

		case 'x':
			Debug = TRUE;
			break;

		case 'm':
			mfile = optarg;
			break;

		case 'd':
			boot = optarg;
			break;

		case 'k':
			kernel = optarg;
			break;

		case '?':
			fatal( usage, argv[0] );
		}

	if ( kernel == NULL && optind >= argc )
		fatal( usage, argv[0] );

	Argv = argv;

	/* allocate memory to build optional header for driver */
	Opthdr = (struct master *) malloc(	sizeof(master) +
						sizeof(depend) +
						sizeof(param) +
						sizeof(routine) +
						sizeof(variable) +
						sizeof(format) +
						sizeof(element) +
						sizeof(string) );

	if ( Opthdr == NULL )
		fatal( "out of memory for optional header" );

	/*
	 * do the UNIX kernel object file if necessary
	 */
	if ( kernel != NULL )
		{
		master.flag = KERNEL;

		object( kernel, "kernel", 0777 );
		}

	/*
	 * do each module object file
	 */
	for( ; optind<argc; optind++ )
		{
		strncat( strcpy(name,""), basename(argv[optind]), sizeof(name)-1 );

		if ( (p=strchr(name,'.')) != NULL )
			*p = '\0';

		master.flag = 0;

		object( argv[optind], lcase(name), 0666 );
		}

	/*
	 * return proper exit status
	 */
	if ( any_error )
		return( 1 );
	else
		return( 0 );
	}

/*
 * Process an object file.
 */
object( object_file, module, mode )
	char *object_file;
	char *module;
	int mode;
	{
	extern int yylineno;
	char bootname[DIRSIZ+1], *p;
	int size;
	jmp_buf	env[1];

	ndepend = depend;
	nparam = param;
	nroutine = routine;
	nvariable = variable;
	nformat = format;
	nelement = &element[1];	/* so OFFSET can never be NULL */
	nstring = &string[1];	/* so OFFSET can never be NULL */

	/*
	 * build /boot name
	 */
	strncat( strcpy(bootname,""), basename(object_file), sizeof(bootname)-1 );

	if ( (p=strchr(bootname,'.')) != NULL )
		*p = '\0';

	/*
	 * scan /etc/master.d to find the parameter declarations
	 */
	strcpy( master_file, mfile );
	strcat( master_file, "/" );
	strcat( master_file, module );

	if ((yyin = fopen(master_file,"r")) == NULL)
		{
		warn( "%s: not processed; cannot open %s", object_file, master_file );
		return;
		}

	if ( setjmp(jmpbuf=env) )
		{
		jmpbuf = NULL;
		fclose( yyin );
		warn( "%s: not processed", object_file );
		return;
		}
	
	getparam( yyin );

	if ( ferror(yyin) )
		fatal( "%s: I/O error", master_file );

	/*
	 *	Start parsing master file from the beginning
	 */
	rewind( yyin );

	lexinit();

	yylineno = 1;

	if ( yyparse() == 0 && check_master(module,&master) == TRUE )
		{
		/*
		 * build the new optional header
		 */
		size = build_header();

		if ( Debug )
			print_master( module, Opthdr );

		copy_driver( object_file, bootname, mode, Opthdr, size );
		}
	else
		warn( "%s: not processed", object_file );

	jmpbuf = NULL;

	fclose(yyin);
	}

/*
 * getparam()
 *
 * Get the parameters from part 2 of each master file; store in the param[]
 * array
 */
 void
getparam( master )
	FILE *master;
	{
	extern int yylineno;

	char line[BUFSIZ];
	char part2;		/* flag */
	char *name, *value, *p;


	yylineno = 0;
	part2 = FALSE;

	while( fgets(line,sizeof(line),master) != NULL )
		{
		++yylineno;

		if ( line[0] == '*' )
			continue;

		if ( line[0] == '$' )
			{
			part2 = TRUE;
			continue;
			}

		if ( ! part2 )
			continue;

		if ( strlen(line) == strspn(line," \t\n") )
			/* blank line */
			continue;

		if ( (p=strchr(line,'=')) == NULL )
			{
			yyerror( "syntax error" );
			continue;
			}

		*p++ = '\0';

		if ( (name=strtok(line," \t")) == NULL ||
			!isascii(name[0]) ||
			!isalpha(name[0]) ||
			strlen(name) != strspn(name,"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_") )
			{
			yyerror( "syntax error" );
			continue;
			}

		if ( strlen(name) > PARAMNMSZ )
			name[PARAMNMSZ] = '\0';

		if ( findparam(name) != NULL )
			{
			yyerror( "parameter %s already defined", name );
			continue;
			}

		if ( nparam-param > MAXPARAM )
			yyfatal( "parameter table overflow" );

		strncpy( nparam->name, name, PARAMNMSZ );

		if ( strlen(p) == strspn(p," \t\n") )
			{
			yyerror( "syntax error" );
			continue;
			}

		value = p + strspn(p," \t");

		if ( *value == '"' )
			/*
			 * character string
			 */
			{
			for( p=value+1; *p; ++p )
				if ( p[0] == '"' && p[-1] != '\\' )
					break;

			if ( *p != '"' )
				{
				yyerror( "missing '\"' for end of string" );
				continue;
				}
			else
				*p = '\0';

			nparam->type = '"';
			nparam->value.string = OFFSET(copystring(value+1),string);
			}
		else
			{
			nparam->type = 'N';
			nparam->value.number = strtol( strtok(value," \t\n"), &p, 0 );

			if ( *p )
				{
				yyerror( "value not numeric or string" );
				continue;
				}
			}

		++nparam;
		}
	}

/*
 * getsize( format, number )
 *
 * Compute the size of a variable given the size format items
 */

#define	ALIGN(locctr,type)	((locctr+sizeof(type)-1)&~(sizeof(type)-1))

getsize( format, n )
	register struct format *format;
	int n;
	{
	int locctr;
	char round;

	if ( n > 1 )
		round = TRUE;
	else
		round = FALSE;

	for( locctr=0; --n >= 0; ++format )
		{
		if ( format->type & FSKIP )
			{
			round = TRUE;
			locctr = ALIGN( locctr, int );
			locctr += format->value;
			continue;
			}

		switch( format->type & FTYPE )
			{
		case FSTRING:
			locctr += format->strlen * sizeof(char);
			break;
		case FCHAR:
			locctr += sizeof(char);
			break;
		case FSHORT:
			locctr = ALIGN( locctr, short );
			locctr += sizeof(short);
			break;
		case FINT:
			locctr = ALIGN( locctr, int );
			locctr += sizeof(int);
			break;
		case FLONG:
			locctr = ALIGN( locctr, long );
			locctr += sizeof(long);
			break;
			}
		}

	if ( round )
		locctr = ALIGN( locctr, int );

	return( locctr );
	}

/*
 * copy_driver( file, name, mode, opthdr, size )
 *
 * Copy the driver object file incorporating the new optional header;
 * the file will be written to /boot/NAME with access mode `mode'
 */
 char
copy_driver( file, name, mode, opthdr, size )
	char	*file;
	char	*name;
	int	mode;
	struct master *opthdr;
	int	size;
	{
	int	fdin = -1, fdout = -1;
	char	bootname[256];
	char	tempname[256];
	int	delta;
	int	nbytes, rc;
	char	buffer[BUFSIZ];
	FILHDR	fhdr;
	SCNHDR	shdr;

	/*
	 * create the new names
	 */
	strcat( strcpy( bootname, boot ), "/" );
	ucase( strcat( bootname+strlen(bootname), name ));

	sprintf( tempname, "%s/<temp%d>", boot, getpid() );

	/*
	 * open the files
	 */
	if ( (fdin = open(file,O_RDONLY)) == -1 )
		{
		myperror( "%s: cannot open", file );
		goto bad;
		}

	if ( (fdout = open(tempname,O_WRONLY|O_CREAT|O_TRUNC,mode)) == -1 )
		{
		myperror( "%s: cannot open", tempname );
		goto bad;
		}

	/*
	 * read the file header
	 */
	if ( myread(fdin,(char*)&fhdr,FILHSZ,file) )
		goto bad;

	if ( fhdr.f_magic != FBOMAGIC )
		{
		warn( "%s: bad magic", file );
		goto bad;
		}

	if ( mylseek(fdin,(long)fhdr.f_opthdr,1,file) )
		goto bad;

	delta = size - fhdr.f_opthdr;

	/*
	 * update the file header for the new file, and write the headers
	 */
	if ( fhdr.f_symptr )
		fhdr.f_symptr += delta;
	fhdr.f_opthdr = size;
	fhdr.f_timdat = time( (long*)NULL );

	if ( mywrite(fdout,(char*)&fhdr,FILHSZ,tempname) )
		goto bad;

	if ( mywrite(fdout,(char*)opthdr,(unsigned)size,tempname) )
		goto bad;

	/*
	 * now copy each section header to the new object file
	 */

	while( fhdr.f_nscns-- != 0 )
		{
		if ( myread(fdin,(char*)&shdr,SCNHSZ,file) )
			goto bad;

		/*
		 * update section header pointers
		 */
		if ( shdr.s_scnptr )
			shdr.s_scnptr += delta;
		if ( shdr.s_relptr )
			shdr.s_relptr += delta;
		if ( shdr.s_lnnoptr )
			shdr.s_lnnoptr += delta;

		if ( mywrite(fdout,(char*)&shdr,SCNHSZ,tempname) )
			goto bad;
		}

	/*
	 * now copy the rest of the object file
	 */
	while( (nbytes = read(fdin,buffer,sizeof(buffer))) > 0)
		if ( (rc=write(fdout,buffer,(unsigned)nbytes)) != nbytes)
			{
			if ( rc == -1 )
				myperror( tempname );
			else
				warn( "%s: truncated write", tempname );
			goto bad;
			}

	if ( nbytes == -1 )
		{
		myperror( file );
		goto bad;
		}

	close( fdin );
	close( fdout );
	fdin = fdout = -1;

	/*
	 * now that the file has been successfully created, rename it
	 */
	if ( unlink(bootname) == -1 )
		{
		extern errno;

		if ( errno != ENOENT )
			{
			myperror( "%s: cannot unlink existing file", bootname );
			goto bad;
			}
		}

	if ( link(tempname,bootname) == -1 )
		{
		myperror( "%s: cannot link", bootname );
		goto bad;
		}

	if ( unlink(tempname) == -1 )
		myperror( "%s: cannot unlink", tempname );

	return;


	/*
	 * Failure!
	 */
bad:	if ( fdin != -1 )
		close( fdin );
	if ( fdout != -1 )
		close( fdout );

	unlink( tempname );

	warn( "%s: not processed", file );
	}

/*
 * build_header()
 *
 * Build the new optional header; return the total size
 */

struct	sort
	{
	char	*string;	/* -> the string or expression */
	offset	*referenced;	/* -> the offset that references the string or expression */
	};

 static
 int
cmpstrlen( s1, s2 )
	struct sort *s1, *s2;
	{
	return( strlen(s2->string) - strlen(s1->string) );
	}

 static
 int
cmpexprlen( e1, e2 )
	struct sort *e1, *e2;
	{
	return( exprlen(e2->string) - exprlen(e1->string) );
	}


 int
build_header()
	{
	struct	depend	*dp;
	struct	param *pp;
	struct	routine	*rp;
	struct	variable *vp;
	struct	format	*fp, *tfp;
	char		*xp, *p, *q, *s;
	int		i, j, k, size;
	unsigned int	total_strings = 0; /* count total strings to be sorted */
	unsigned int	total_expressions = 0; /* count total expressions to be sorted */
	struct sort	*sorted;

	/*
	 * copy master structure information
	 */
	*Opthdr = master;

	/*
	 * copy dependencies
	 *
	 * dp points to the word boundary one past the end
	 * of the master struct pointed to by Opthdr
	 */
	dp = (struct depend *) ROUNDUP(Opthdr+1);

	if (Opthdr->ndep > 0)
		{
		Opthdr->o_depend = OFFSET(dp,Opthdr);

		for (i=0; i<Opthdr->ndep; i++)
			{
			dp[i] = depend[i];
			++total_strings;
			}
		}
	else
		Opthdr->o_depend = NULL;

	/*
	 * copy parameters
	 *
	 * pp points to the word boundary one past the end
	 * of the last dependency structure
	 */
	pp = (struct param *) ROUNDUP(dp+(Opthdr->ndep));

	if ( (Opthdr->nparam = nparam - param) > 0 )
		{
		Opthdr->o_param = OFFSET(pp,Opthdr);

		for (i=0; i<Opthdr->nparam; i++)
			{
			pp[i] = param[i];

			if ( pp[i].type == '"' )
				++total_strings;
			}
		}
	else
		Opthdr->o_param = NULL;
	
	/*
	 * copy routine information
	 *
	 * rp points to the word boundary one past the end
	 * of the last parameter structure
	 */
	rp = (struct routine *) ROUNDUP(pp+(Opthdr->nparam));

	if (Opthdr->nrtn > 0)
		{
		Opthdr->o_routine = OFFSET(rp,Opthdr);

		/*                             routine name */
		for ( i=0; i<Opthdr->nrtn; i++,total_strings++ )
			rp[i] = routine[i];
		}
	else
		Opthdr->o_routine = NULL;

	/*
	 * copy variable information
	 *
	 * vp points to the word boundary one past the end
	 * of the last routine structure
	 */
	vp =(struct variable *) ROUNDUP(rp+(Opthdr->nrtn));
	/*
	 * fp points to memory beyond the last variable structure
	 */
	fp = (struct format *) ROUNDUP(vp+(Opthdr->nvar));
	
	if (Opthdr->nvar > 0)
		{
		Opthdr->o_variable = OFFSET(vp,Opthdr);

		/*
		 * for each variable ...
		 */
		for (i=0; i < Opthdr->nvar; i++)
			{
			vp[i] = variable[i];

			/* variable name */
			++total_strings;
		
			/* array dimension expression */
			if (vp[i].dimension != NULL)
				++total_expressions;

			/*
			 * copy format information for the structure
			 */
			if ( vp[i].ninit > 0 )
				{
				tfp = (struct format *)POINTER(vp[i].initializer,format);

				vp[i].initializer = OFFSET(fp,Opthdr);

				for( j=0; j<vp[i].ninit; ++j )
					{
					fp[j] = tfp[j];

					if ( fp[j].type & FEXPR )
						++total_expressions;
					}

				fp += vp[i].ninit;
				}
			else
				vp[i].initializer = NULL;
			}
		}
	else
		Opthdr->o_variable = NULL;

	/*
	 * copy the expressions; this is done in a way to minimize the
	 * size of the expression area in the optional header
	 *
	 *	1. locate all expressions
	 *	2. sort into decreasing order by length
	 *	3. for each expression, search the expression area for an exact
	 *		match, and if found use that one, otherwise, copy the
	 *		unique expression into the expression area
	 *
	 * xp points to memory beyond the last format structure
	 */
	xp = (char *) ROUNDUP(fp);

	if ( (sorted=(struct sort*)malloc(total_expressions*sizeof(*sorted))) == NULL )
		fatal( "no memory for building expressions" );

	i = 0;

	for( j=0; j<Opthdr->nvar; ++j )
		{
		if ( vp[j].dimension )
			{
			sorted[i].string = (char*)POINTER(vp[j].dimension,element);
			sorted[i++].referenced = &vp[j].dimension;
			}

		fp = (struct format *) POINTER(vp[j].initializer,Opthdr);

		for( k=0; k<vp[j].ninit; ++k )
			{
			if ( fp[k].type & FEXPR )
				{
				sorted[i].string = (char*)POINTER(fp[k].value,element);
				sorted[i++].referenced = &fp[k].value;
				}
			}
		}

	qsort( sorted, total_expressions, sizeof(*sorted), cmpexprlen );

	for( i=0,size=0; i<total_expressions; ++i )
		{
		k = exprlen( s=sorted[i].string );

		/* search the new expression area for a duplicate */
		for( p=xp, q=xp+size-k; p <= q; ++p )
			{
			if ( 0 == memcmp(p,s,k) )
				break;
			}

		if ( p <= q )
			/* its already in the new expression area */
			*sorted[i].referenced = OFFSET(p,Opthdr);
		else
			/* copy the unique expression into the new expression area */
			{
			*sorted[i].referenced = OFFSET(memcpy(xp+size,s,k),Opthdr);
			size += k;
			}
		}

	free( sorted );

	/*
	 * copy the strings and build a string table; this is done in a way
	 * to minimize the size of the string table
	 *
	 *	1. extract all strings
	 *	2. sort into decreasing order by length
	 *	3. for each string, search the expression area and the new
	 *		string table for an exact match, and if found use
	 *		that one, otherwise, copy the unique string into the
	 *		new string table. 
	 */

	if ( (sorted=(struct sort*)malloc(total_strings*sizeof(*sorted))) == NULL )
		fatal( "no memory for building string table" );

	i = 0;

	for( j=0; j<Opthdr->ndep; ++j )
		{
		sorted[i].string = (char*)POINTER(dp[j].name,string);
		sorted[i++].referenced = &dp[j].name;
		}

	for( j=0; j<Opthdr->nparam; ++j )
		{
		if ( pp[j].type == '"' )
			{
			sorted[i].string = (char*)POINTER(pp[j].value.string,string);
			sorted[i++].referenced = &pp[j].value.string;
			}
		}

	for( j=0; j<Opthdr->nrtn; ++j )
		{
		sorted[i].string = (char*)POINTER(rp[j].name,string);
		sorted[i++].referenced = &rp[j].name;
		}

	for( j=0; j<Opthdr->nvar; ++j )
		{
		sorted[i].string = (char*)POINTER(vp[j].name,string);
		sorted[i++].referenced = &vp[j].name;
		}

	qsort( sorted, total_strings, sizeof(*sorted), cmpstrlen );

	for( i=0; i<total_strings; ++i )
		{
		k = strlen( s=sorted[i].string );

		/* search the expression area and the new string table for a duplicate */
		for( p=xp, q=xp+size-k; p < q; ++p )
			{
			if ( 0 == strcmp(p,s) )
				break;
			}

		if ( p < q )
			/* its already in the new string table */
			*sorted[i].referenced = OFFSET(p,Opthdr);
		else
			/* copy the unique string into the new string table */
			{
			*sorted[i].referenced = OFFSET(strcpy(xp+size,s),Opthdr);
			size += k+1;
			}
		}

	free( sorted );

	return( ROUNDUP(OFFSET(xp+size, Opthdr)) );
	}

/*
 * polish( root )
 *
 * Traverse an expression tree and create a prefix polish expression.
 * The result is stored beginning at "nelement".
 */
 void
polish( root )
	register struct tnode *root;
	{
	register int	i;
	register union element *ep;

	if ( nelement-element >= MAXEXPR )
		yyfatal( "too many expressions" );

	ep = (union element *)nelement;

	switch (root->type)
		{

		case 'N': /* number */
			ep->literal[0] = root->type;
			for (i=1;i<5;++i)
				ep->literal[i] = root->value>>((4-i)*8) & 0xff;
			nelement = (char*)(XBUMP(ep,literal));
			break;
		case '&':
 		case '#':
 		case '"':
			ep->size_of[0] = root->type;
			strcpy( &ep->size_of[1], root->name );
			nelement = (char*)(XBUMP(ep,size_of));
			break;
 		case 'I':
			ep->identifier[0] = root->type;
			strncpy( &ep->identifier[1], root->name, PARAMNMSZ );
			nelement = (char*)(XBUMP(ep,identifier));
			break;
		case 'C':
		case 'D':
		case 'M':
			ep->nC[0] = root->type;
			if ( root->name )
				strcpy( &ep->nC[1], root->name );
			else
				ep->nC[1] = '\0';
			nelement = (char*)(XBUMP(ep,nC));
			break;
		case '+':
		case '-':
		case '*':
		case '/':
		case '>':
		case '<':
			ep->operator = root->type;
			nelement = (char*)(XBUMP(ep,operator));
			polish( root->left );
			polish( root->right );
			break;
		default:
			yyfatal( "illegal node type \"%c\" in expression", root->type );
		}
	}

/*
 * exprlen( expression )
 *
 * Determine the total length (in bytes) of a prefix-polish expression
 */
 int
exprlen( expression )
	union element * expression;
	{
	static union element *xp;

	if ( expression == NULL )
		expression = xp;

	switch( expression->operator )
		{
	case '+':
	case '-':
	case '*':
	case '/':
	case '<':
	case '>':
		xp = XBUMP( expression, operator );
		exprlen( (union element *)NULL );
		exprlen( (union element *)NULL );
		break;
	case 'D':
	case 'C':
	case 'M':
	case '#':
	case '&':
	case '"':
		xp = XBUMP( expression, nD );
		break;
	case 'I':
		xp = XBUMP( expression, identifier );
		break;
	case 'N':
		xp = XBUMP( expression, literal );
		break;
		}

	return( (int)xp - (int)expression );
	}
