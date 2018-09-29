/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:3b/make_tables.c	1.3"

/*	tool for processing ops.in file.  makes:				*/
/*										*/
/*		ind.out - switch index defines for the instruction classes	*/
/*		typ.out - type arrays for mnemonics				*/
/*		mnm.out - mnemonic table initialization				*/
/*		sem.out - semantics for instructions				*/

#include	<stdio.h>
#include	<ctype.h>

int	curtypeno = 0, curindno = 0;
FILE	*fd_index,*fd_types,*fd_ops,*fd_sem;

main()
{
	fd_index = fopen("ind.out","w");
	fd_types = fopen("typ.out","w");
	fd_ops = fopen("mnm.out","w");
	fd_sem = fopen("sem.out","w");
	fprintf(fd_types,"static unsigned long t0[] = {0};\n");
	while (getblock()) ;
}


getblock()
{
	char 		line[BUFSIZ], indexname[BUFSIZ];
	register char	*lptr, *ptr;
	int		typeno = 0;

	if (gets(line) == NULL) return(0);
	lptr = line;
	ptr = indexname;
	while (isalnum(*lptr))
		*ptr++ = *lptr++;
	*ptr = 0;
	fprintf(fd_index,"#define %s %d\n",indexname,++curindno);
	while (isspace(*lptr)) lptr++;
	if (*lptr)
		fprintf(fd_types,"static unsigned long t%d[] = {%s, 0};\n",
						typeno= ++curtypeno,lptr);
	fprintf(fd_sem, "case %s:\n", indexname);
	do {
		if (gets(line) == NULL) error(line);
		fprintf(fd_sem,"%s\n",line);
	} while (line[0] != '}') ;
	fprintf(fd_sem, "break;\n");
	if (gets(line) == NULL) error(line);
	do {
		if (line[0] == '#') {	/* send cpp directives on through */
			fprintf(fd_ops,"%s\n",line);
			goto getanother;
		}
		fprintf(fd_ops,"{%s", line);
		if (typeno)
			fprintf(fd_ops,", t%d", typeno);
		else
			fprintf(fd_ops,", t0");
		fprintf(fd_ops,", %s", indexname);
		fprintf(fd_ops,"},\n");
getanother:
		if (gets(line) == NULL) error(line);
	} while (line[0] != '.');
	return(1);
}


error(msg)
{
	fprintf(stderr,"syntax error: %s\n",msg);
	exit(1);
}
