/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mkboot-3b2:util.c	1.10"

# include	<stdio.h>
# include	"mkboot.h"
# include	<ctype.h>

/*
 *	check_master( master ) - do some basic sanity checking for the
 *				 master file entry; the following table
 *				 shows the valid flag combinations
 *
 *                 R                                               N 
 *                 E                             F     F           O 
 *                 Q                 B           U     U           T 
 *                 A     T           L     C     N     N     S     A 
 *                 D     T     R     O     H     D     M     O     D 
 *                 D     Y     E     C     A     R     O     F     R 
 *                 R     S     Q     K     R     V     D     T     V 
 *                (a)   (t)   (r)   (b)   (c)   (f)   (m)   (s)   (x)
 *              +-----+-----+-----+-----+-----+-----+-----+-----+-----+
 *              |     |     |     |     |     |     |     |     |     |
 *  ONCE    (o) |  #  |  #  |  #  |  #  |  #  |  #  |  O  |  O  |  O  |
 *              |     |     |     |     |     |     |     |     |     |
 *              +-----+-----+-----+-----+-----+-----+-----+-----+-----+
 *                    |     |     |     |     |     |     |     |     |
 *  REQADDR (a)       |  #  |  #  |  Y  |  Y  |  Y  |  Y  |  N  |  Y  |
 *                    |     |     |     |     |     |     |     |     |
 *              ------+-----+-----+-----+-----+-----+-----+-----+-----+
 *                          |     |     |     |     |     |     |     |
 *  TTYS    (t)             |  #  |  #  |  O  |  O  |  #  |  #  |  #  |
 *                          |     |     |     |     |     |     |     |
 *              ------------+-----+-----+-----+-----+-----+-----+-----+
 *                                |     |     |     |     |     |     |
 *  REQ     (r)                   |  #  |  #  |  #  |  O  |  O  |  O  |
 *                                |     |     |     |     |     |     |
 *              ------------------+-----+-----+-----+-----+-----+-----+
 *                                      |     |     |     |     |     |
 *  BLOCK   (b)                         |  Y  |  Y  |  Y  |  Y  |  N  |
 *                                      |     |     |     |     |     |
 *              ------------------------+-----+-----+-----+-----+-----+
 *                                            |     |     |     |     |
 *  CHAR    (c)                               |  N  |  Y  |  Y  |  N  |
 *                                            |     |     |     |     |
 *              ------------------------------+-----+-----+-----+-----+
 *                                                  |     |     |     |
 *  FUNDRV  (f)                                     |  Y  |  Y  |  N  |
 *                                                  |     |     |     |
 *              ------------------------------------+-----+-----+-----+
 *                                                        |     |     |
 *  FUNMOD  (m)                                           |  Y  |  N  |
 *                                                        |     |     |
 *              ------------------------------------------+-----+-----+
 *                                                              |     |
 *  SOFT    (s)                                                 |  N  |
 *                                                              |     |
 *              ------------------------------------------------+-----+
 *                (a)   (t)   (r)   (b)   (c)   (f)   (m)   (s)   (x)
 *  
 *  
 *              O: the flag is only valid in this combination
 *              Y: the flag combination is valid
 *              N: the flag combination is not valid
 *              #: not applicable
 *
 * For example, BLOCK is valid with CHAR, FUNDRV, FUNMOD and SOFT, but not valid
 * with NOTADRV.  As another example, ONCE may only be used with FUNMOD,
 * SOFT or NOTADRV; all other flags are immaterial with respect to ONCE.
 *
 * If everything is OK, then return TRUE; otherwise return FALSE if the object
 * file should be skipped.
 */
 boolean
check_master( name, mp )
	char *name;
	register struct master *mp;
	{
	boolean skip = FALSE;

	if ( mp->flag & KERNEL )
		/*
		 * Kernel is handled a little differently
		 */
		{
		if ( mp->vec || mp->flag & ~KERNEL )
			{
			warn( "%s: illegal flags -- no flags allowed for kernel object files", name );
			mp->flag = KERNEL;
			mp->vec = 0;
			}

		if ( mp->nvec || mp->prefix[0] || mp->soft || mp->ndev || mp->ipl )
			{
			warn( "%s: vectors, prefix, major, devices and interrupt level ignored", name );
			mp->nvec = 0;
			strncpy( mp->prefix, "", sizeof(mp->prefix) );
			mp->soft = 0;
			mp->ndev = 0;
			mp->ipl = 0;
			}

		if ( mp->ndep > 0 )
			{
			warn( "%s: dependencies not applicable for kernel", name );
			mp->ndep = 0;
			}

		if ( mp->nrtn > 0 )
			{
			warn( "%s: routine definitions not applicable for kernel", name );
			mp->nrtn = 0;
			}
		}

	if ( mp->flag & TTYS && ! (mp->flag & (CHAR|FUNDRV)) )
		{
		warn( "%s: illegal flag combination -- TTYS only valid with CHAR|FUNDRV", name );
		skip = TRUE;
		}

	if ( (mp->flag & (BLOCK|CHAR|FUNDRV|FUNMOD|SOFT)) && (mp->flag & NOTADRV) )
		{
		warn( "%s: illegal flag combination -- BLOCK|CHAR|FUNDRV|FUNMOD|SOFT mutually exclusive with NOTADRV", name );
		skip = TRUE;
		}

	if ( (mp->flag & CHAR) && (mp->flag & FUNDRV) )
		{
		warn( "%s: illegal flag combination -- CHAR mutually exclusive with FUNDRV", name );
		skip = TRUE;
		}

	if ( (mp->flag & REQADDR) && (mp->flag & SOFT) )
		{
		warn( "%s: illegal flag combination -- REQADDR not valid with SOFT", name );
		skip = TRUE;
		}

	if ( (mp->flag & ONCE) && ! (mp->flag & (SOFT|NOTADRV|FUNMOD)) )
		{
		warn( "%s: illegal flag combination -- ONCE only valid for SOFT|NOTADRV", name );
		skip = TRUE;
		}

	if ( (mp->flag & REQ) && ! (mp->flag & (SOFT|NOTADRV|FUNMOD)) )
		{
		warn( "%s: illegal flag combination -- REQ only valid for SOFT|NOTADRV|FUNMOD", name );
		skip = TRUE;
		}

	if ( (unsigned)mp->soft > 127 )
		{
		warn( "%s: major number cannot exceed 127", name );
		skip = TRUE;
		}

	if ( mp->flag & SOFT && mp->nvec > 0 && mp->vec == 0 )
		{
		warn( "%s: vector number required when interrupt vectors exist and SOFT specified", name );
		skip = TRUE;
		}

	if ( mp->flag & (NOTADRV|FUNMOD) && (mp->nvec || mp->ndev || mp->ipl) )
		{
		warn( "%s: fields not applicable for NOTADRV|FUNMOD", name );
		skip = TRUE;
		}

	return( skip? FALSE : TRUE );
	}

/*
 *	lcase(str) - convert all upper case characters in 'str' to lower
 *			case and return pointer to start of string
 */

char *
lcase(str)
char	*str;
	{
	register char	*ptr;

	for (ptr = str; *ptr != '\0'; ++ptr)
		if (isascii(*ptr) && isupper(*ptr))
			*ptr = tolower(*ptr);
	return( str );
	}

/*
 *	ucase(str) - convert all lower case characters in 'str' to upper
 *			case and return pointer to start of string
 */

char *
ucase(str)
char	*str;
	{
	register char	*ptr;

	for (ptr = str; *ptr != '\0'; ++ptr)
		if (isascii(*ptr) && islower(*ptr))
			*ptr = toupper(*ptr);
	return( str );
	}

/*
 *	basename(path) - obtain the file name associated with the (possibly)
 *			 full pathname argument 'path'
 */

char *
basename(path)
char	*path;
	{
	char	*retval;

	/* return the entire name if no '/'s are found */
	retval = path;

	while (*path != NULL)
		if (*path++ == '/')
			retval = path;	/* now path points past the '/' */
		
	return( retval );
	}

/*
 * Copy a string to the string table; handle all of the C language escape
 * sequences with the exception of \0
 */
 char *
copystring( text )
	register char *text;
	{
	register char c;
	char *start;
	int count, byte;

	if ( nstring-string > MAXSTRING-strlen(text)-1 )
		yyfatal( "string table overflow" );

	start = nstring--;

	while( *text )
		{
		if ( (*++nstring = *text++) == '\\' )
			{
			switch( c = *text++ )
				{
			default:
				*nstring = c;
				break;
			case 'n':
				*nstring = '\n';
				break;
			case 'v':
				*nstring = '\v';
				break;
			case 't':
				*nstring = '\t';
				break;
			case 'b':
				*nstring = '\b';
				break;
			case 'r':
				*nstring = '\r';
				break;
			case 'f':
				*nstring = '\f';
				break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
				for( count=0,byte=c-'0'; ++count<3 && ((c=(*text))>='0'&&c<='7'); ++text )
					byte = 8 * byte + c - '0';
				if ( byte == 0 )
					yyfatal( "\\0 illegal within string" );
				if ( byte > 255 )
					yyfatal( "illegal character escape: \\%o", byte );
				*nstring = byte;
				break;
				}
			}
		}

	*++nstring = 0;
	++nstring;

	return( start );
	}

/*
 * Search the parameter table for a given parameter name; return the
 * pointer to the appropriate structure if found, or return NULL if the
 * parameter is not found
 */
 struct param *
findparam( name )
	register char *name;
	{
	register struct param *pp;

	for( pp=param; pp<nparam; ++pp )
		if ( strncmp(name,pp->name,PARAMNMSZ) == 0 )
			return( pp );

	return( NULL );
	}

/*
 * Read with error checking
 */
 int
myread( fildes, buf, nbyte, fname )
	int fildes;
	char *buf;
	register unsigned nbyte;
	register char *fname;
	{
	register int rbyte;

	if ( (rbyte = read(fildes,buf,nbyte)) == -1 )
		{
		myperror( fname );
		return( 1 );
		}
	else
		if ( rbyte != nbyte )
			{
			warn( "%s: truncated read", fname );
			return( 1 );
			}
		else
			return( 0 );
	}

/*
 * Write with error checking
 */
 int
mywrite( fildes, buf, nbyte, fname )
	int fildes;
	char *buf;
	register unsigned nbyte;
	register char *fname;
	{
	register int wbyte;

	if ( (wbyte = write(fildes,buf,nbyte)) == -1 )
		{
		myperror( fname );
		return( 1 );
		}
	else
		if ( wbyte != nbyte )
			{
			warn( "%s: truncated write", fname );
			return( 1 );
			}
		else
			return( 0 );
	}

/*
 * Lseek with error checking
 */
 int
mylseek( fildes, seekoff, whence, fname )
	int fildes;
	long seekoff;
	int whence;
	char *fname;
	{

	if ( lseek(fildes,seekoff,whence) == -1L )
		{
		myperror( fname );
		return( 1 );
		}
	else
		return( 0 );
	}

/*
 * myperror( message, a1, a2, a3, a4, a5 )
 *
 * Printf for perror error messages; error message is:
 *
 *	mkboot: <message>: <syserr message>
 */
 /*VARARGS1*/
 void
myperror( message, a1, a2, a3, a4, a5 )
	char	*message;
	int	a1, a2, a3, a4, a5;
	{
	extern	char	**Argv;
	char buffer[256];

	sprintf( buffer, "%s: ", *Argv );
	sprintf( buffer+strlen(buffer), message, a1, a2, a3, a4, a5 );

	perror( buffer );
	}

/*
 * Print_master( name, master )
 *
 * Print the master structure
 */
 void
print_master( name, master )
	char *name;
	struct master *master;
	{
	register struct depend *dp;
	register struct param *pp;
	register struct routine *rp;
	register struct variable *vp;
	register struct format *fp;
	register int i, j;


	printf( "%s:\n", name );

	printf( "     flag=" );
	if ( master->vec )
		printf( "%d %s\n", master->vec, print_flag(master->flag) );
	else
		printf( "%s\n", print_flag(master->flag) );
	printf( "     nvec=%-2d  prefix=%-4s  software=%-3d  ndev=%-2d  ipl=%-2d\n",
			master->nvec, master->prefix, master->soft,
			master->ndev, master->ipl );

	if ( master->ndep )
		{
		dp = (struct depend *) POINTER( master->o_depend, master );
		for( i=0; i<master->ndep; ++i, ++dp )
			printf( "     dependency=%s\n", POINTER(dp->name,master) );
		}

	if ( master->nparam )
		{
		pp = (struct param *) POINTER( master->o_param, master );
		for( i=0; i<master->nparam; ++i, ++pp )
			{
			printf( "     parameter: %-*.*s = ", PARAMNMSZ, PARAMNMSZ, pp->name );
			if ( pp->type == '"' )
				printf( "\"%s\"\n", POINTER(pp->value.string,master) );
			else
				printf( "0x%X\n", pp->value.number );
			}
		}

	if ( master->nrtn )
		{
		static char *id[] = { "", "nosys", "nodev", "true", "false", "fsnull", "fsstray", "nopkg", "noreach" };
		rp = (struct routine *) POINTER( master->o_routine, master );
		for( i=0; i<master->nrtn; ++i, ++rp )
			printf( "     routine %s(){%s}\n", POINTER(rp->name,master), id[rp->id] );
		}

	if ( master->nvar )
		{
		vp = (struct variable *) POINTER( master->o_variable, master );
		for( i=0; i<master->nvar; ++i, ++vp )
			{
			printf( "     variable %s", POINTER(vp->name,master) );
			if ( vp->dimension )
				{
				printf( "[" );
				print_expression((union element *) POINTER( vp->dimension, master ));
				printf( "]" );
				}
			printf( "(0x%X)\n", vp->size );
			if ( vp->ninit )
				{
				static char *type[] = { "%%0x%X", "%%c", "%%s", "%%i", "%%l", "%%%dc" };
				fp = (struct format *) POINTER( vp->initializer, master );
				for( j=0; j<vp->ninit; ++j,++fp )
					{
					printf( "           ={ " );
					printf( type[fp->type&FTYPE], ((fp->type&FTYPE)!=FSTRING)? fp->value : fp->strlen );
					if ( fp->type & FEXPR )
						{
						printf( " expression=" );
						print_expression((union element *) POINTER( fp->value, master ));
						printf( " }\n" );
						continue;
						}
					if ( fp->type & FSKIP )
						{
						printf( " }\n" );
						continue;
						}
					printf( " 0x%X }\n", fp->value );
					}
				}
			}
		}
	}



/*
 * Print_expression( expression )
 *
 * Print an expression
 */
 void
print_expression( expression )
	register union element *expression;
	{

	static union element *xp;
	int temp;
	char c;

	if ( expression == NULL )
		expression = xp;

	switch( c=expression->operator )
		{
	case '+':
	case '-':
	case '*':
	case '/':
			{
			int need_parens;

			xp = XBUMP(expression,operator);

			if ( (c == '*' || c == '/') && (xp->operator == '+' || xp->operator == '-') )
				{
				need_parens = TRUE;
				printf( "(" );
				}
			else
				need_parens = FALSE;

			print_expression( (union element *)NULL );

			if ( need_parens )
				printf( ")" );

			printf( "%c", c );

			if ( ((c == '*' || c == '/') && (xp->operator == '+' || xp->operator == '-')) ||
				(c == '-' && (xp->operator == '-' || xp->operator == '+' || xp->operator == '*' || xp->operator == '/')) )
				{
				need_parens = TRUE;
				printf( "(" );
				}
			else
				need_parens = FALSE;

			print_expression( (union element *)NULL );

			if ( need_parens )
				printf( ")" );
			}
		break;
	case '>':
	case '<':
		printf( (c=='<')? "min(" : "max(" );
		xp = XBUMP(expression,function);
		print_expression( (union element *)NULL );
		printf( "," );
		print_expression( (union element *)NULL );
		printf( ")" );
		break;
	case 'C':
	case 'D':
	case 'M':
		printf( "#%c", c );
		if ( expression->nC[1] )
			printf( "(%s)", expression->nC+1 );
		xp = XBUMP(expression,nC);
		break;
	case '#':
	case '&':
		printf( "%c%s", c, expression->size_of+1 );
		xp = XBUMP(expression,size_of);
		break;
	case 'I':
		printf( "%.*s", PARAMNMSZ, expression->identifier+1 );
		xp = XBUMP(expression,identifier);
		break;
	case '"':
		printf( "\"%s\"", expression->string+1 );
		xp = XBUMP(expression,string);
		break;
	case 'N':
		temp = ((expression->literal[1]<<8 | expression->literal[2] & 0xFF)<<8 | expression->literal[3] & 0xFF)<<8 | expression->literal[4] & 0xFF;
		printf( "%d", temp );
		xp = XBUMP(expression,literal);
		break;
		}
	}


/*
 * print_flag( flag )
 *
 * Print the flags symbolically
 */
 char *
print_flag( flag )
	register unsigned short flag;
	{
	static char buffer[256];

	strcpy( buffer, "" );

	if ( flag == 0 )
		return( "none" );

	if ( flag & KERNEL )
		strcat( buffer, ",KERNEL" );

	if ( flag & ONCE )
		strcat( buffer, ",ONCE" );

	if ( flag & REQ )
		strcat( buffer, ",REQ" );

	if ( flag & BLOCK )
		strcat( buffer, ",BLOCK" );

	if ( flag & CHAR )
		strcat( buffer, ",CHAR" );

	if ( flag & REQADDR )
		strcat( buffer, ",REQADDR" );

	if ( flag & TTYS )
		strcat( buffer, ",TTYS" );

	if ( flag & SOFT )
		strcat( buffer, ",SOFT" );

	if ( flag & NOTADRV )
		strcat( buffer, ",NOTADRV" );

	if ( flag & FUNDRV )
		strcat( buffer, ",FUNDRV" );

	if ( flag & FUNMOD )
		strcat( buffer, ",FUNMOD" );

	if ( flag & FSTYP ) 
		strcat(buffer, ",FSTYP");

	return( buffer+1 );
	}
