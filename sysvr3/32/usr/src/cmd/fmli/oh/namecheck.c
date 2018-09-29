/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 *  Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */

#ident	"@(#)vm.oh:src/oh/namecheck.c	1.5"

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "mio.h"
#include "tsys.h"
#include "typetab.h"
#include "partabdefs.h"

extern struct ott_tab *Cur_ott;

bool
namecheck(path, name, objtype, errstr, new)
char *path, *name, *objtype, **errstr;
bool new;
{
	int len;
	static char error[80];
	char pathbuf[PATHSIZ], oldpath[PATHSIZ];
	char *p;
	char *template = NULL;
	int span;
	int maxchar;
	int maxpath;
	int retval;
	struct opt_entry *opt;
	struct ott_entry *ott;
	extern struct one_part Parts[];
	struct opt_entry *obj_to_parts();
	struct ott_entry *dname_to_ott();

#ifdef _DEBUG
	_debug(stderr, "in namecheck(%s)\n", name);
#endif

	if (name == NULL || name[0] == '\0') {
		*errstr = "Object name must have at least 1 character";
		return(FALSE);
	}
	for (p = name; *p; p++)	/* convert spaces and tabs to underlines */
		if (*p == ' ' || *p == '\t')
			*p = '_';

	if (objtype)
		opt = obj_to_parts(objtype);
	else
		opt = NULL;
	if (opt) {
		template = Parts[opt->part_offset].part_template;
		maxchar = find_max(template);
	} else
		maxchar = 12;	/* reasonably safe default */

	if ((len=strlen(name)) > maxchar) {
		sprintf(error, "Object name cannot have more than %d characters\n", maxchar);
		*errstr = error;
		return(FALSE);
	}
	/* check if total pathsize to big */
	if (opt)
		maxpath = len + 4;	/* 4: "/" + prefix(2) + NULL */
	else
		maxpath = 22;	/* 12 for name + /.pref + 3 is biggest defined */
	if ((path?strlen(path):0) + maxpath > PATHSIZ) {
		sprintf(error, "Can't create object: folder path length exceeds %d characters\n", PATHSIZ - maxpath);
		*errstr = error;
		return(FALSE);
	}

	if ((span=strcspn(name, "!@#$^&*(){}[]|\\`~;\"'<>/?")) < len) {
		sprintf(error, "Object name cannot contain the special character '%c'", name[span]);
		*errstr = error;
		return(FALSE);
	}
	if ((span=strspn(name, "-+.")) != 0) {
		sprintf(error, "Object name cannot start with the character '%c'", *name);
		*errstr = error;
		return(FALSE);
	}

	if (!new) {
#ifdef _DEBUG
		_debug(stderr, "namecheck returning true\n");
#endif
		return(TRUE);
	}

	if (path) {
		strcat(strcpy(pathbuf, path), "/");
	} else
		pathbuf[0] = '\0';

	if (template)
		strcat(pathbuf, part_construct(name, template));
	else
		strcat(pathbuf, name);
#ifdef _DEBUG
	_debug(stderr, "namecheck: checking existance of %s\n",pathbuf);
#endif

#ifndef WISH
	/* check if the object is in the wastebasket */

	if (Cur_ott && path && strcmp(Cur_ott->path, path) != 0) {
		strcpy(oldpath, Cur_ott->path);
		ott_lock_inc(NULL);
		ott_get(path?path:".", 0L, 0L, 0L, 0L);
	} else
		oldpath[0] = '\0';

	if ((ott = dname_to_ott(name)) != NULL) {
		if (ott->objmask & M_WB)
			sprintf(error, "object exists in wastebasket");
		else
			sprintf(error, "An object with that name already exists");
		*errstr = error;
		retval = FALSE;
	} else if (access(pathbuf, 0) != -1) {
		*errstr = "An object with that name already exists";
		retval = FALSE;
	} else
		retval = TRUE;

	if (oldpath[0]) {		/* restore old ott */
		make_current(oldpath);
		ott_unlock_inc(NULL);
	}
#else
	if (access(pathbuf, 0) != -1) {
		static char dbuf[80];

		sprintf(dbuf, "An object with that name already exists in %s", path_to_title(path, NULL));
		*errstr = dbuf;
		retval = FALSE;
	} else
		retval = TRUE;
#endif

#ifdef _DEBUG
	_debug(stderr, "namecheck returning %d\n", retval);
#endif
	return(retval);
}

static int
find_max(template)
char *template;
{
	int max;
	register char *p = template;

	while (*p && *p != '%')
		p++;

	if (*p == '\0')
		return(12);

	p++;

	if (*p != '.')
		return(12);

	p++;

	if ((max = atoi(p)) == 0)
		return(12);

	return(max);
}
