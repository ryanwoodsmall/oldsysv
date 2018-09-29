/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)dbfconv:dbfconv.c	1.3"

#include <stdio.h>
#include <sys/param.h>
#include "local.h"

extern	char	*init[];
extern	char	*strtok();

char	*Command;
char	*Outfile;	/* save output file name for error() */
char	Scratch[BUFSIZ];
FILE	*Ifp;		/* input file pointer */
FILE	*Ofp;		/* output file pointer */


main(argc, argv)
int argc;
char *argv[];
{
	register int i;
	int sversion;	/* starting version # */

	Command = argv[0];

	if (argc != 3) {
		sprintf(Scratch, "Usage: %s input_file output_file", Command);
		error(Scratch);
	}

	Ifp = fopen(argv[1], "r");
	if (Ifp == NULL) {
		sprintf(Scratch, "Cannot open <%s>", argv[1]);
		error(Scratch);
	}

	sversion = find_version();
	if (sversion == VERSION)
		exit(0);	/* nothing to do */
	else if (sversion > VERSION)
		error("Can not convert to previous version");

	rewind(Ifp);
	Ofp = fopen(argv[2], "w");
	if (Ofp == NULL) {
		sprintf(Scratch, "Cannot open <%s>", argv[2]);
		error(Scratch);
	}
	Outfile = argv[2];

/*
 * put out initial blurb
 */

	for (i = 0; *init[i]; i++)
		fprintf(Ofp, "%s\n", init[i]);

	convert(sversion);
	fclose(Ifp);
	fclose(Ofp);
	exit(0);
}


/*
 * find_version:	find the version number of the current database file
 */

find_version()
{
	char *line, *p;
	int version;

	if ((line = (char *) malloc(DBFLINESZ)) == NULL)
		error("malloc failed");
	p = line;
	while (fgets(p, DBFLINESZ, Ifp)) {
		if (!strncmp(p, VERSIONSTR, strlen(VERSIONSTR))) {
			p += strlen(VERSIONSTR);
			if (*p)
				version = atoi(p);
			else
				error("database file is corrupt");
			free(line);
			return(version);
		}
		p = line;
	}
	error("database file is corrupt");
}


/*
 * convert:	convert a database file to the current version number
 *		Note: as the versions change, it is necessary to update
 *		      this routine to reflect the new format
 */

convert(start)
int start;
{
	char buf[DBFLINESZ];
	register char *p = buf;
	int lineno = 0;
	struct dbf_v1 v1;
	register struct dbf_v1 *v1p = &v1;
	struct dbf_v2 v2;
	register struct dbf_v2 *v2p = &v2;
	int lineread; /* 0 if data to come from input file,
			 1 if data present in memory */

	while (fgets(p, DBFLINESZ, Ifp)) {
		lineread = 0;
		lineno++;
		/* check for comment only line or the unfortunate blank line
		   that is in the database */
		if ((*p == DBFCOMMENT) || !strcmp(p, " \n"))
			continue;
		switch (start) {
		/* Note: version 0 and version 1 are identical */
		case	0:
		case	1:
			if (!lineread) {
				scanner(start, lineno, p, v1p);
				lineread = 1;
			}
		case	2:
			if (!lineread)
				error("Internal error in convert");
			else {
				v2p->dbf_svccode = v1p->dbf_svccode;
				v2p->dbf_flags = v1p->dbf_flags;
				v2p->dbf_modules = v1p->dbf_modules;
				v2p->dbf_command = v1p->dbf_command;
				v2p->dbf_comment = v1p->dbf_comment;
				/* default new fields */
				v2p->dbf_id = DEFAULTID;
				v2p->dbf_reserved = RESERVED;
			}

/*
 * version 2 is current so we're done, this fprintf will move as the database
 * version numbers increase in the future
 */

			fprintf(Ofp, "%s:%s:%s:%s:%s:%s#%s\n", v2p->dbf_svccode,
				v2p->dbf_flags, v2p->dbf_id, v2p->dbf_reserved,
				v2p->dbf_modules, v2p->dbf_command, v2p->dbf_comment);
			break;
		default:
			sprintf(Scratch, "unknown version number <%d>", start);
			error(Scratch);
		}
	}
	if (feof(Ifp))
		return;
	if (ferror(Ifp))
		error("error reading input file");
}


/*
 * scanner:	break up an input line into the appropriate data structure
 *		Note: as the versions change, it is nessary to update this
 *		      routine to handle the new inputs
 */

scanner(version, lineno, inp, p)
int version;
int lineno;
char *inp;
char *p;
{
	register struct dbf_v1 *v1;
	char *tp;

	switch (version) {
	case	1:
		v1 = (struct dbf_v1 *) p;
		if (*(inp + strlen(inp) - 1) != '\n') {
			sprintf(Scratch, "missing newline on line %d", lineno);
			error(Scratch);
		}

		if ((tp = strtok(inp, DBFTOKENS)) == NULL) {
			sprintf(Scratch, "illegal entry on line %d", lineno);
			error(Scratch);
		}
		v1->dbf_svccode = tp;

		if ((tp = strtok(NULL, DBFTOKENS)) == NULL) {
			sprintf(Scratch, "illegal entry on line %d", lineno);
			error(Scratch);
		}
		v1->dbf_flags = tp;

		if ((tp = strtok(NULL, DBFTOKENS)) == NULL) {
			sprintf(Scratch, "illegal entry on line %d", lineno);
			error(Scratch);
		}
		v1->dbf_modules = tp;

		if ((tp = strtok(NULL, DBFTOKENS)) == NULL) {
			sprintf(Scratch, "illegal entry on line %d", lineno);
			error(Scratch);
		}
		v1->dbf_command = tp;

		if ((tp = strtok(NULL, DBFTOKENS)) == NULL) {
			sprintf(Scratch, "illegal entry on line %d", lineno);
			error(Scratch);
		}
		v1->dbf_comment = tp;

		return;
	default:
		error("Internal error in scanner");
	}
}


error(s)
char *s;
{
	fprintf(stderr, "%s: %s\n", Command, s);
	fclose(Ifp);
	unlink(Outfile);	/* don't leave droppings around */
	fclose(Ofp);
	exit(2);
}
