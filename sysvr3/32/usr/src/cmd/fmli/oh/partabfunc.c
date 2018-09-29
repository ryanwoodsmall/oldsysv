/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */

#ident	"@(#)vm.oh:src/oh/partabfunc.c	1.1"

#include <stdio.h>
#include <string.h>
#include "but.h"
#include "tsys.h"
#include "typetab.h"
#include "ifuncdefs.h"
#include "optabdefs.h"
#include "partabdefs.h"

char *
part_match(name, template)
char *name, *template;
{
	char pre[ONAMESIZ], suf[ONAMESIZ];
	char name1[DNAMESIZ];
	char *p;
	static char retstr[ONAMESIZ];
	int  matchlen = ONAMESIZ;
	int len;

	p = pre;

	while (*template && (*template != '%' || *(template+1) == '%')) {
		*p++ = *template++;
	}

	*p = '\0';

	if (*template == '%') {
		if (*(++template) == '.')
			matchlen = atoi(++template);
		if (matchlen == 0)
			matchlen = ONAMESIZ;
		while (*template && *template >= '0' && *template <= '9')
			template++;
		if (*template == 's') {
			template++;
		}
	}

	p = suf;

	while (*template && *template != '/')
		*p++ = *template++;

	*p = '\0';

	if (*template == '/') {
		if ((p = strchr(name, '/')) == NULL)
			return(NULL);
		strncpy(name1, name, name-p);
		name1[name-p] = '\0';
	} else {
		strcpy(name1, name);
	}

	if (*pre) {
		if (strlen(name1) < strlen(pre) ||
			strncmp(name1, pre, strlen(pre)) != 0) {
			return(NULL);
		}
	}

	if (*suf) {
		if (!has_suffix(name1+strlen(pre), suf)) {
			return(NULL);
		}
	}

	if (strlen(name1) > matchlen + strlen(pre) + strlen(suf)) {
		return(NULL);
	}

	strncpy(retstr, name+strlen(pre), 
			(len=strlen(name)-strlen(pre)-strlen(suf)));
	retstr[len] = '\0';

	if (*template == '/') {
		sprintf(name1, ++template, retstr);
		if (strcmp(++p, name1) != 0)
			return(NULL);
	}
		
	return(retstr);
}

char *
part_construct(name, template)
char *name, *template;
{
	static char result[ONAMESIZ];

	sprintf(result, template, name, name);
	return(result);
}

char *
objparent(base, objtype)
char *base;
char *objtype;
{
	struct opt_entry *partab, *obj_to_parts();
	struct one_part *apart, *opt_next_part();

	if ((partab = obj_to_parts(objtype)) == NULL)
		return(NULL);
	apart = opt_next_part(partab);
	return(part_construct(base, apart->part_template));
}
