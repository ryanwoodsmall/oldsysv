/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sadmin:cmain/num.c	1.2"
/*
	check for all numeric argument
*/

main( argc, argv )
int	argc;
char	*argv[];
{
	if( argc != 2 )
		exit( 2 );
	exit( !num( argv[1] ) );
}
