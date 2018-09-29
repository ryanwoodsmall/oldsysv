/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)forms:mecho.c	1.2"

/*
 * Identical to echo.c, except does not know about \n, \b,
 * etc.  Used to mimick shell interpretation of command lines.
 */
main(argc, argv)
char **argv;
{
   register char *cp, **last_pt;
   if(--argc == 0) 
   {
      putchar('\n');
      exit(0);
   }
   last_pt = (argv++) + argc;
   for (; argv<=last_pt; argv++)
   {
      for (cp = *argv; *cp; cp++) putchar(*cp);
      putchar(argv == last_pt? '\n': ' ');
   }
   exit(0);
}
