/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/gutil.d/cvrtopt.c	1.3"
/* <: t-5 d :> */
/* cvrtopt is a program to convert an option string from
	the form allowed by a graphics program to a form
	easily processed by a Bourne shell program.
	The form is

		cvrtopt [=cntrl_string] options  .

	The control string enables options that take string,
	integer, or float arguments to be handled correctly.
	Example:

		Input:  cvrtopt =fa,ib,sc -d,a2.5e5 -b23,cstring file1
		Output:  -d -a2.5e5 -b23 -cstring file1
*/

#include <stdio.h>
#include "errpr.h"
#include "util.h"
#include "setop.h"
#define KEYLEN 4	/* option keywords may only be 1 or 2 letters long */
struct option  {
		char type;
		char opt[KEYLEN];
	};
#define ISDELIM(c)  (c == ' ' || c == ',' || c == '\0')
#define FLAG '0'
#define FLT '1'
#define STR '2'
#define INT '3'
#define TWO '4'
#define MAXOPT 100
char *nodename;

main(argc, argv)
int argc;
char *argv[];

{
	int ac, k = 0;
	char **av, *cp;
	struct option opttab[MAXOPT];

	nodename = *argv;
	for (ac = argc-1, av = argv+1; ac > 0; ac--, av++)
		if ( *(cp=av[0]) == '=')
			for (cp++; *cp != '\0';)
				switch (*cp)  {
					case 's':
						opttab[k].type = STR;
						cp++;
						SETSTR(opttab[k].opt, cp, KEYLEN);
						if (strlen(opttab[k].opt) > 2)
							ERRPR1(option name %s truncated, opttab[k].opt);
						++k;
						break;
					case 'd':
					case 'i':
						opttab[k].type = INT;
						cp++;
						SETSTR(opttab[k].opt, cp, KEYLEN);
						if (strlen(opttab[k].opt) > 2)
							ERRPR1(option name %s truncated, opttab[k].opt);
						++k;
						break;
					case 'f':
						opttab[k].type = FLT;
						cp++;
						SETSTR(opttab[k].opt, cp, KEYLEN);
						if (strlen(opttab[k].opt) > 2)
							ERRPR1(option name %s truncated, opttab[k].opt);
						++k;
						break;
					case 't':
						opttab[k].type = TWO;
						cp++;
						SETSTR(opttab[k].opt, cp, KEYLEN);
						if (strlen(opttab[k].opt) > 2)
							ERRPR1(option name %s truncated, opttab[k].opt);
						++k;
						break;
					case ' ': case ',': cp++;  break;
					default:
						ERRPR1(illegal format code:  %c, *cp);
						cp++;
						break;
				}
	optcvrt(opttab, ++argv, --argc, k);
	exit(0);
}




/*  optcvrt.c takes the information in the option table
	opttab and uses it to process the strings pointed to
	by av.
*/
#define  MAXSIZE  500

optcvrt(opttab, av, ac, opt_ct)
struct option opttab[];
char *av[];
int ac, opt_ct;
{
	int  x = 0, y = 0, j;
	char *cp, optype, file_s[MAXSIZE], opt_s[MAXSIZE];

/*  Each argument is processed here.  If it begins with 
	a '-' it is processed as an option string.  If it
	begins with anything else, it is processed as a file.
	In the file handling, arguments that begin with '='
	are considered control options to cvrtopt and so
	are ignored.
*/

	for (; ac > 0; av++, ac--)
		if ( (cp = *av)[0] == '-' && *(cp +1) != '\0')		/* this begins processing of option string */
			for (++cp; *cp != '\0';)
				if (ISDELIM(*cp)) cp++;
	
				else  {
					for (optype = FLAG, j = 0; j < opt_ct; j++)
						if (opttab[j].opt[0] == *cp)
							if (opttab[j].opt[1] == '\0' ||
								opttab[j].opt[1] == *(cp+1) )  {
								optype = opttab[j].type;
								break;		/* j contains index to opttab array */
							}
	
					opt_s[x++] = '-';
	
					if (optype != FLAG)  {
						++cp;
						opt_s[x++] = opttab[j].opt[0];
						if (opttab[j].opt[1] != '\0')  {
							++cp;
							opt_s[x++] = opttab[j].opt[1];
						}
					}
	
					switch (optype)  {
						case FLAG:
							opt_s[x++] = *cp++;
							break;
						case INT:
							for (; isdigit(*cp) || *cp == '-'; cp++)
								opt_s[x++] = *cp;
							break;
						case FLT:
							for (; ISDOUB(*cp); cp++)
								opt_s[x++] = *cp;
							break;
						case STR:
							for (; !ISDELIM(*cp); cp++)
								opt_s[x++] = *cp;
							break;
						case TWO:
							break;
					}
					opt_s[x++] = ' ';
	
				}
	
		else					/* this begins processing of file names */
			if (*cp != '=')  {		/* cvrtopt control strings are ignored */
				for (; *cp != '\0'; cp++)
					file_s[y++] = *cp;
				file_s[y++] = ' ';
			}
	
	opt_s[x] = '\0';
	file_s[y] = '\0';
	strcat(opt_s, file_s);
	printf("%s\n", opt_s);
}
