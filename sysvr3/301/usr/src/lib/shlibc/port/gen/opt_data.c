/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/opt_data.c	1.2"
/*
 * Global variables
 * used in getopt
 */
#if SHLIB
int	opterr = 1;
int	optind = 1;
int	optopt = 0;
char	*optarg = 0;
#endif
