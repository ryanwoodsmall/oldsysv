/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libgen:copylist.c	1.1"
/*
	copylist copies a file into a block of memory, replacing newlines
	with null characters, and returns a pointer to the copy.
*/

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<stdio.h>

static long	linecount;

char *
copylist(filenm, szptr)
char	*filenm;
long	*szptr;
{
	FILE		*strm;
	struct	stat	stbuf;
	register int	c;
	char		*malloc();
	register char	*ptr, *p;

	/* get size of file */
	if (stat(filenm, &stbuf) == -1) {
		fprintf(stderr, "copylist: can't find %s\n", filenm);
		return(NULL);
	}
	*szptr = stbuf.st_size;

	/* get block of memory */
	if((ptr = malloc((unsigned) *szptr)) == NULL) {
		fprintf(stderr, "copylist: malloc call unsuccessful\n");
		return(NULL);
	}

	/* copy contents of file into memory block, replacing newlines
	with null characters */
	if ((strm = fopen(filenm, "r")) == NULL) {
		fprintf(stderr, "copylist: can't open %s\n", filenm);
		return(NULL);
	}
	linecount = 0;
	for (p = ptr; p < ptr + *szptr  &&  (c = getc(strm)) != EOF; p++) {
		if (c == '\n') {
			*p = '\0';
			linecount++;
		}
		else
			*p = c;
	}
	fclose(strm);

	return(ptr);
}


long
clistcnt()
{
	return  linecount;
}
