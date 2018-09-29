/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm:xx/oeupkg.c	1.1"
#include <stdio.h>
#include <string.h>
#include "tsys.h"
#include "terror.h"
#include "retcds.h"
#include "parse.h"

main(argc, argv)
int argc;
char **argv;
{
	FILE *fp;
	struct oeh header, *oeuparse();
	extern char *optarg;
	int unpack = FALSE;
	int c;
	char *objtype, *type, *src, *dst, *enc, *appl, *form, *prid;
	char *file;
	char *getepenv();
	char *parsehead();

	wish_init(argc, argv);
	objtype = type = src = dst = enc = appl = form = prid = NULL;
	while ((c = getopt(argc, argv, "ut:s:d:o:e:a:f:p:")) != EOF)
		switch (c) {
		case 'u':
			unpack = TRUE;
			break;
		case 't':
			type = optarg;
			break;
		case 's':
			src = optarg;
			break;
		case 'd':
			dst = optarg;
			break;
		case 'o':
			objtype = optarg;
			break;
		case 'e':
			enc = optarg;
			break;
		case 'a':
			appl = optarg;
			break;
		case 'f':
			form = optarg;
			break;
		case 'p':
			prid = optarg;
			break;
		default:
			break;
		}
	if (unpack) {
		if (!src || !dst)
			fatal(BADARGS, "");
		header.file = dst;
		if (oeuparse(src, &header, UNPACK))
			exit(R_ERR);
		if (header.encrytest)
			puts(header.encrytest);
	}
	else {
		FILE *fp;

		if ((fp = fopen(dst, "w")) == NULL)
			fatal(NOPEN, "");
		if (!oeupack(fp, src, objtype, enc, type, appl, form, prid))
			exit(R_ERR);
	}
	exit(R_OK);
}
