/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */

#ident	"@(#)vm.oeu:src/oeu/oeupack.c	1.3"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include "tsys.h"
#include "io.h"
#include "typetab.h"
#include "obj.h"
#include "retcds.h"
#include "parse.h"
#include "smdef.h"
#include "mail.h"
#include "partabdefs.h"
#include "terror.h"

#define SAVE 1
#define RESTORE 2
#undef BACKUP
#define BACKUP		LBUT-1
#define TEMPLATE "oXXXXXX"
#define LOCLEN		2

char 	*getepenv();
char 	*getenv();
char *mktemp();

int
oeupack(fp, fullpath, objtype, encryption, type, appl, form, prid)
FILE *fp;
char *fullpath;
char *objtype;
char *encryption;
char *type, *appl, *form, *prid;
{
	struct opt_entry *part_ent, *obj_to_parts();
	struct one_part *apart, *opt_next_part();
	char temp[PATHSIZ];
	char *dirpath;
	char *partname;
	char *name, *filename(), *parent();
	long partspot, retspot;
	int mode;
	int i, num;
	char *part_match(), *part_construct();

	part_ent = obj_to_parts(objtype);
	if (!part_ent)
		return(FALSE);
	apart = opt_next_part(part_ent);
	num = part_ent->numparts;
	dirpath = parent(fullpath);
	name = part_match(filename(fullpath), apart->part_template);
	if (!name)
		return(FALSE);
	fputs("Content-Version: 2\n", fp);
	if (strcmp(objtype, "TRANSFER")) {
		fprintf(fp, "Type: %s\n", part_ent->objclass);
		fprintf(fp, "Application: %s\n", part_ent->objapp);
		fprintf(fp, "Format: %s\n", part_ent->objformat);
		fprintf(fp, "Product-Id: %s\n", part_ent->objprod);
	}
	else {
		fprintf(fp, "Type: %s\n", type);
		fprintf(fp, "Application: %s\n", appl);
		fprintf(fp, "Format: %s\n", form);
		fprintf(fp, "Product-Id: %s\n", prid);
	}
	fprintf(fp, "Object-Number: %s\n", part_ent->objtype);
	fprintf(fp, "Count: %d\n", num);
	fprintf(fp, "Name: %s\n", name);
	for (i = 0; (i < num); i++, apart = opt_next_part(NULL)) {
		partname = part_construct(name, apart->part_template);
		strcpy(temp, dirpath);
		strcat(temp, "/");
		strcat(temp, partname);
		if (access(temp, 04) == -1) {
			if (!(apart->part_flags & PRT_OPT)) {
				fclose(fp);
				return(FALSE);
			}
			continue;
		}
		if (apart->part_flags & PRT_DIR)
			continue;
		if (num > 1)
			fprintf(fp, "Part-Type: %s\n", apart->part_name);
		mode = (PRT_BIN & apart->part_flags) || encryption;
		if (mode) {
			if (encryption) {
				fprintf(fp, "Encrypted: crypt\n");
				fprintf(fp, "Encryption-test: %s\n", encryption);
			}
			fputs("Part-encoding: binary\n", fp);
			fputs("Encoding-algorithm: btoa\n", fp);
			fputs("Part-Length: ", fp);
			partspot = ftell(fp);
			fprintf(fp, "%-*.*s\n", LENLEN, LENLEN, " ");
			if (!btoa(fp, temp))
				return(FALSE);
		}
		else {
			fputs("Part-encoding: ascii\n", fp);
			fputs("Part-Length: ", fp);
			partspot = ftell(fp);
			fprintf(fp, "%-*.*s\n", LENLEN, LENLEN, " ");
			if (!catfile(fp, temp))
				return(FALSE);
		}
		retspot = ftell(fp);
		fseek(fp, partspot, 0);
		fprintf(fp, "%ld", retspot - partspot - LENLEN - 1);
		fseek(fp, retspot, 0);
	}
	return(TRUE);
}

catfile(fp, file)
FILE *fp;
char *file;
{
	FILE *fp2;
	int c;

	if ((fp2 = fopen(file, "r")) == NULL)
		return(FALSE);
	while ((c = getc(fp2)) != EOF)
		putc(c, fp);
	fclose(fp2);
	fflush(fp);
	return(TRUE);
}
btoa(fp, file)
FILE *fp;
char *file;
{
	char comm[STR_SIZE];
	FILE *fp2;
	int c;

	sprintf(comm, "btoa <%s", file);
	if ((fp2 = popen(comm, "r")) == NULL)
		return(FALSE);
	while ((c = getc(fp2)) != EOF)
		putc(c, fp);
	pclose(fp2);
	fflush(fp);
	return(TRUE);
}
