/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)led:led.c	1.2"
/*
 *		Copyright 1984 AT&T
 */
#include <stdio.h>
#include <sys/sys3b.h>

main (argc, argv)
int argc;
char **argv;
{
	int c;
	char y_n;
	extern int optind;
	extern char *optarg;
	int retval = 0;
	int errflg = 0;

/*
 * The caller must be super-user.  The -o option invokes sys3b to set
 * the green light to a solid on state.  The -f option invokes sys3b
 * to set the green light flashing.  
 */
	while ((c = getopt(argc, argv, "of")) != EOF) 
		switch(c)	{
		case 'o':
			retval = sys3b(GRNON);
			if(retval < 0)
				errflg++;
			break;
			
		case 'f':
			retval = sys3b(GRNFLASH);
			if(retval < 0)
				errflg++;
			break;
			
		case '?':
			printf("Usage: led [-o -f]\n");
			return(-1);
		}
		if(errflg != 0) {
			printf("led: User must be super user\n");
			return(-1);
		}
}
