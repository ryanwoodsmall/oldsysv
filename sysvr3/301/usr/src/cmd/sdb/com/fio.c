/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:com/fio.c	1.10"

#include "head.h"
char	fbuf[256];	/* current line from file */

/*
 * These procedures manage the source files examined by sdb.
 */

/* Change the current source file to `name'. */
finit( name )
char *name;
{
	register char *p, *q;
	
#if DEBUG
	if (debugflag ==1)
	{
		enter1("finit");
	}
	else if (debugflag == 2)
	{
		enter2("finit");
		arg("name");
		printf("\"%s\"",name);
		closeparen();
	}
#endif
	if (fiobuf.fd) close(fiobuf.fd);
	strcpy(filework,name);
	if( ( p = findfl( name ) ) == NULL ||
	    ( fiobuf.fd = open( p, 0 ) ) == -1 )
	{
		nolines = 1;
		perror( p ? p : name );
#if DEBUG
		if (debugflag == 1)
		{
			exit1();
		}
		else if (debugflag == 2)
		{
			exit2("finit");
			endofline();
		}
#endif
		return;
	}
	strcpy( filework, p );
	binit( &fiobuf );
	cpstr( curfile, p );
	for ( p = fbuf; ; p++ )
	{
		if ( p >= fbuf + sizeof fbuf )
		{
			p = fbuf + sizeof fbuf - 1;
		}
		if (bread(&fiobuf,p,1) <= 0)
		{
			nolines = 1;
			fprintf(FPRT1, "%s: No lines in file\n", name);
#if DEBUG
			if (debugflag == 1)
			{
				exit1();
			}
			else if (debugflag == 2)
			{
				exit2("finit");
				endofline();
			}
#endif
			return;
		}
		if (*p == '\n') break;
	}
	curstmt.lnno = 1;
	curstmt.stno = 1;
	maxfline = 0;
	nolines = 0;
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("finit");
		endofline();
	}
#endif
}

/*
 *	findfl( name )	( like execvp, but does path search and finds files )
 *			Taken from Make(I) source.
 */

static char fname[ 128 ];

char *
findfl( name )
register char	*name;
{
	char	*execat();
	register char	*p;

#if DEBUG
	if (debugflag ==1)
	{
		enter1("findfl");
	}
	else if (debugflag == 2)
	{
		enter2("findfl");
		arg("name");
		printf("\"%s\"",name);
		closeparen();
	}
#endif
	if( strchr( name,'/' ) || !sdbpath[ 0 ] )
	{
		p = ":";
	}
	else
	{
		p = sdbpath;
	}

	do
	{
		p = execat( p, name, fname );
		if( access( fname, 4 ) == 0 )
		{
#if DEBUG
			if (debugflag == 1)
			{
				exit1();
			}
			else if (debugflag == 2)
			{
				exit2("findfl");
				printf("0x%x",fname);
				endofline();
			}
#endif
			return( fname );
		}
	} while ( p );
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("findfl");
		printf("0x%x",NULL);
		endofline();
	}
#endif
	return( NULL );
}

/*
 *	Identical to version in the system(3) subroutine,
 *	but version in system is declared local!
 *	Taken from make( I ) source. 
 */

char *
execat( s1, s2, si )
register char	*s1, *s2;
char	*si;
{
	register char	*s,*s3;

#if DEBUG
	if (debugflag ==1)
	{
		enter1("execat");
	}
	else if (debugflag == 2)
	{
		enter2("execat");
		arg("s1");
		printf("\"%s\"",s1);
		comma();
		arg("s2");
		printf("\"%s\"",s2);
		comma();
		arg("si");
		printf("\"%s\"",si);
		closeparen();
	}
#endif
	s = si;
	while ( *s1 && *s1 != ':' && *s1 != '-' )
	{
		*s++ = *s1++;
	}
	if ( si != s )
	{
		*s++ = '/';
	}
	while ( *s2 )
	{
		*s++ = *s2++;
	}
	*s = NULL;
	s3 =( *s1? ++s1 : 0 );
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("execat");
		printf("0x%x",s3);
		endofline();
	}
#endif
	return s3;
}

/* Make the next line current. */
fnext()
{
	register char *p;
	char xxx;
	int i;
	
#if DEBUG
	if (debugflag ==1)
	{
		enter1("fnext");
	}
	else if (debugflag == 2)
	{
		enter2("fnext");
		closeparen();
	}
#endif
	if ( nolines )
	{
#if DEBUG
		if (debugflag == 1)
		{
			exit1();
		}
		else if (debugflag == 2)
		{
			exit2("fnext");
			endofline();
		}
#endif
		return;
	}
	i = 2;
	p = fbuf;
	do {
		if ( bread( &fiobuf, &xxx, 1 ) <= 0 )
		{
			xxx = ' ';
			blseek( &fiobuf, 0L, 0 );
			curstmt.lnno = 0;
		}
		else if (i < sizeof fbuf)
		{
			*p = xxx;
			p ++ ;
		}
		else if (i == sizeof fbuf)
		{
			*p = '\n';
			p ++ ;
		}
		i ++ ;
	} while (xxx != '\n');
	curstmt.lnno++;
	curstmt.stno = 1;
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("fnext");
		endofline();
	}
#endif
}


/* Make the previous line current. */
fprev()
{
	char c;
	register int	i;
	
#if DEBUG
	if (debugflag ==1)
	{
		enter1("fprev");
	}
	else if (debugflag == 2)
	{
		enter2("fprev");
		closeparen();
	}
#endif
	if ( nolines )
	{
#if DEBUG
		if (debugflag == 1)
		{
			exit1();
		}
		else if (debugflag == 2)
		{
			exit2("fprev");
			endofline();
		}
#endif
		return;
	}
	for( i = 0; i < 3; i++ )
	{
		for ( ;; )
		{
			if ( bread( &fiobuf, &c + 1, -1 ) <= 0 )
			{
				if ( maxfline ) blseek( &fiobuf, 0L, 2 )
					;
				else
				{
					blseek( &fiobuf, 0L, 0 );
					for( ;; )
					{
						if( bread(&fiobuf, &c, 1) <= 0 )
						{
							break;
						}
						if ( c == '\n' )
						{
							maxfline++;
						}
					}
				}
			}
			if ( c == '\n' )
			{
				break;
			}
		}
	}
	bread( &fiobuf, &c, 1 );  /* eat the '\n' */
	
	curstmt.lnno -= 2;
	if ( curstmt.lnno < 0 ) curstmt.lnno = maxfline - 1;
	curstmt.stno = 1;
	
	fnext();
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("fprev");
		endofline();
	}
#endif
}


/* Print the current line. */
fprint()
{
	register char	*p;
	int i;
	
#if DEBUG
	if (debugflag ==1)
	{
		enter1("fprint");
	}
	else if (debugflag == 2)
	{
		enter2("fprint");
		closeparen();
	}
#endif
	if ( nolines )
	{
		error( "No lines in file" );
#if DEBUG
		if (debugflag == 1)
		{
			exit1();
		}
		else if (debugflag == 2)
		{
			exit2("fprint");
			endofline();
		}
#endif
		return;
	}
	printstmt( curstmt );
	printf( ": " );
	i = 0;
	p = fbuf;
	while( i++ < 255 && putchar( *p++ ) != '\n' )
		;
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("fprint");
		endofline();
	}
#endif
}

/* Make line `num' current. */
ffind( num )
register int	num;
{
	register int	i;
	register int	ofline;
	
#if DEBUG
	if (debugflag ==1)
	{
		enter1("ffind");
	}
	else if (debugflag == 2)
	{
		enter2("ffind");
		arg("num");
		printf("0x%x",num);
		closeparen();
	}
#endif
	if ( nolines )
	{
#if DEBUG
		if (debugflag == 1)
		{
			exit1();
		}
		else if (debugflag == 2)
		{
			exit2("ffind");
			endofline();
		}
#endif
		return;
	}
	ofline = curstmt.lnno;
	if ( num > curstmt.lnno )
	{
		for ( i = curstmt.lnno; i < num; i++ )
		{
			fnext();
			if ( curstmt.lnno == 1 )
			{
				goto bad;
			}
		}
	}
	if ( num < curstmt.lnno )
	{
		for ( i = num; i < ofline; i++ )
		{
			fprev();
		}
	}
		
	if ( maxfline & num > maxfline )
	{
		goto bad;
	}
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("ffind");
		endofline();
	}
#endif
	return;

bad:	error( "Not that many lines in file" );
	ffind( ofline );

#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("ffind");
		endofline();
	}
#endif
}

/* Go back n lines. */
fback( n )
{
	int	i;
	
#if DEBUG
	if (debugflag ==1)
	{
		enter1("fback");
	}
	else if (debugflag == 2)
	{
		enter2("fback");
		arg("n");
		printf("0x%x",n);
		closeparen();
	}
#endif
	if ( nolines )
	{
#if DEBUG
		if (debugflag == 1)
		{
			exit1();
		}
		else if (debugflag == 2)
		{
			exit2("fback");
			printf("0x%x",0);
			endofline();
		}
#endif
		return( 0 );
	}
	for ( i = 0; i < n; i++ )
	{
		if ( curstmt.lnno == 1 )
		{
#if DEBUG
			if (debugflag == 1)
			{
				exit1();
			}
			else if (debugflag == 2)
			{
				exit2("fback");
				printf("0x%x",i);
				endofline();
			}
#endif
			return( i );
		}
		fprev();
	}
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("fback");
		printf("0x%x",i);
		endofline();
	}
#endif
	return( i );
}

/* Go forwards n lines. */
fforward( n )
{
	int	i;
	
#if DEBUG
	if (debugflag ==1)
	{
		enter1("fforward");
	}
	else if (debugflag == 2)
	{
		enter2("fforward");
		arg("n");
		printf("0x%x",n);
		closeparen();
	}
#endif
	if ( nolines )
	{
#if DEBUG
		if (debugflag == 1)
		{
			exit1();
		}
		else if (debugflag == 2)
		{
			exit2("fforward");
			printf("0x%x",0);
			endofline();
		}
#endif
		return( 0 );
	}
	for ( i = 0; i < n; i++ )
	{
		fnext();
		if ( curstmt.lnno == 1 )
		{
			fprev();
			return( i );
		}
	}
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("fforward");
		printf("0x%x",i);
		endofline();
	}
#endif
	return( i );
}

/* Print n lines. */
fprintn(n) {
	int i;
	
#if DEBUG
	if (debugflag ==1)
	{
		enter1("fprintn");
	}
	else if (debugflag == 2)
	{
		enter2("fprintn");
		arg("n");
		printf("0x%x",n);
		closeparen();
	}
#endif
	if (nolines){
		error("No lines in file");
#if DEBUG
		if (debugflag == 1)
		{
			exit1();
		}
		else if (debugflag == 2)
		{
			exit2("fprintn");
			printf("0x%x",0);
			endofline();
		}
#endif
		return(0);
	}
	for (i=0; i<n; i++) {
		fprint();
		fnext();
		if (curstmt.lnno == 1) break;
	}
	fprev();
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("fprintn");
		printf("0x%x",1);
		endofline();
	}
#endif
	return(i);
}
