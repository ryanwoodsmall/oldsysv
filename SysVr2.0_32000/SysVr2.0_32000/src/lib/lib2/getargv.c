/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
/*	@(#)getargv.c	1.1	*/

# include <stdio.h>

# define SCAN 0
# define FILL 1

getargv (cmd, argvp, ff)
char *cmd;
char *(*argvp[]); {
	register unsigned count = 1;
	register char *quote, *ptr2;
	static char buf[256];
	extern char *malloc ();
	extern char *strncpy ();
	extern char *strchr ();
	int pass = SCAN;
	int shift = 0;
	char *ptr1;

	fputs ("$$ ", stdout);
	fputs (cmd, stdout);
	putc (' ', stdout);

	if (ff)
		free ((char *) *argvp);

	ptr1 = ptr2 = &strchr (strncpy (buf, cmd, sizeof (buf)), '\0')[1];

	for (;;) {
		if (pass == SCAN && (ptr2-buf) < sizeof (buf) - 1) {
			fgets (ptr2, sizeof (buf) - (ptr2-buf) - 1, stdin);
			buf[sizeof (buf) - 1] = '\n';
			*strchr (ptr2, '\n') = '\0';
		}
		while ((ptr2-buf) < sizeof (buf) - 1) {
			while (*ptr2) {
				if (*ptr2 != ' ' && *ptr2 != '\t')
					break;
				++ptr2;
			}
			if (*ptr2 == '\0')
				break;
			if (pass == FILL)
				(*argvp)[count] = ptr2;
			++count;
			while (*ptr2) {
				if (*ptr2 == ' ' || *ptr2 == '\t')
					break;
				if (*ptr2 == '"' || *ptr2 == '\'') {
					quote = strchr (&ptr2[1], *ptr2);
					if (pass == FILL) {
						strncpy (ptr2, &ptr2[1], quote - ptr2 - 1);
						shift += 2;
					}
					ptr2 = quote;
				}
				else
					ptr2[-shift] = *ptr2;
				++ptr2;
			}
			if (pass == FILL) {
				ptr2[-shift] = '\0';
				shift = 0;
				++ptr2;
			}
		}
		if (pass == FILL)
			return (count);
		*argvp = (char **) malloc (++count * sizeof (char *));
		(*argvp)[0] = buf;
		ptr2 = ptr1;
		count = 1;
		pass = FILL;
	}
}
