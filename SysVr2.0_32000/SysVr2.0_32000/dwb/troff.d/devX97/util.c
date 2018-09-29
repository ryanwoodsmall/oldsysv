char utilxver[] = "@(#)util.c	1.1";		/* SCCS */

# include <stdio.h>
# include "structs.h"

error(msg,v1,v2,v3)
char	*msg ;
int	v1 ;
int	v2 ;
int	v3 ;
{
	if ( input_line_num > 0 )
		fprintf(stderr,"%s, line %d: ",prog_name,input_line_num) ;
	else
		fprintf(stderr,"%s: ",prog_name) ;
	fprintf(stderr,msg,v1,v2,v3) ;
	exit(1) ;
}

f_ree(p)
char	*p ;
{
	free(p) ;
}

int
get_num()
{
	int	n ;

	switch ( scanf("%d",&n) )
	{
		case EOF :
			error("eof in get_num\n") ;

		case 0 :
			error("can't find number in get_num\n") ;

		case 1 :
			break ;

		default :
			error("internal error in get_num\n") ;
	}
	return(n) ;
}

char	*
get_str()
{
	static	char	buf[BUFSIZE] ;
	int		i ;
	int		c ;

	while ( (c=getchar()) != EOF )
		if ( c != ' ' )
			break ;
	buf[0] = c ;
	for ( i=1 ; i<BUFSIZE ; i++ )
		switch(c=getchar())
		{
			case EOF :
				error("EOF in get_str\n") ;

			case ' ' :
			case '\n' :
				buf[i] = '\0' ;
				ungetc(c,stdin) ;
				return(buf) ;

			default :
				buf[i] = c ;
				break ;
		}
	error("string too large for buffer in get_str\n") ;
}

char	*
m_alloc(size)
int	size ;
{
	char	*p ;
	char	*malloc() ;

	p = malloc(size) ;
	if ( p == NULL )
		error("out of memory\n") ;
	return(p) ;
}

page_init()
{
	int	i ;

	for ( i=0 ; i<dev.paperlength ; i++ )
		lines[i] = NULL ;
	line_avail = 0 ;
}

char	*
re_alloc(p,size)
char	*p ;
int	size ;
{
	char	*q ;
	char	*realloc() ;

	q = realloc(p,size) ;
	if ( q == NULL )
		error("bad return from realloc\n") ;
	return(q) ;
}
r_ead(fp,buf,nbytes,msg)
FILE	*fp ;
char	*buf ;
int	nbytes ;
char	*msg ;
{
	if ( nbytes == 0 )
		return ;
	if ( fread(buf,nbytes,1,fp) != 1 )
		error("cannot read %s (%d)\n",msg,errno) ;
}
warning(msg,v1,v2,v3)
char	*msg ;
int	v1 ;
int	v2 ;
int	v3 ;
{
	if ( input_line_num > 0 )
		fprintf(stderr,"%s, line %d: ",prog_name,input_line_num) ;
	else
		fprintf(stderr,"%s: ",prog_name) ;
	fprintf(stderr,msg,v1,v2,v3) ;
}
