/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */

#ident	"@(#)vm.oh:src/oh/evstr.c	1.6"

#include	<stdio.h>
#include	<string.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<ctype.h>
#include	"wish.h"
#include	"token.h"
#include	"fm_mn_par.h"
#include	"var_arrays.h"
#include	"terror.h"
#include	"eval.h"
#include	"moremacros.h"

char *pline();
char **parselist();

extern int EV_retcode;
extern int EV_backquotes;

/*
 * EVAL_STRING sets up calls to the evaluator (eval) given the
 * expected descriptor type
 */ 
char *
eval_string(defstring, flags)
char *defstring;
int *flags;
{
	char *retval;

	switch(*flags & RETS) {
	case RET_BOOL:
		retval = pline(defstring, flags);
		if (retval && (strCcmp(retval, "false") == 0)) {
			free(retval);
			retval = NULL;
		}
		break;
	case RET_INT:
	case RET_STR:
		if (!(retval = pline(defstring, flags)))
			retval = strsave(nil);
		break;
	case RET_ARGS:
	case RET_LIST:
		retval = (char *) parselist(defstring, flags, '\0', '\0');
		break;
	}
	return(retval);
}

/*
** Calls eval() repeatedly to generate one string.
*/
char *
pline(line, pflags)
char *line;
int *pflags;
{
	register char *str;
	char *ret;
	IOSTRUCT *in, *out;
	int firstime;
	int more;

	if (!line)
		return(NULL);
/* could be useful in future */
	/* if (strcspn(line, "'\"`$\\") == strlen(line)) */
		/* return(strsave(line)); */
	ret = nil;
	firstime = TRUE;
	in = io_open(EV_USE_STRING|EV_READONLY, line);
	out = io_open(EV_USE_STRING, NULL);
	if (strcspn(line, "`$") == strlen(line))
		io_size(out, strlen(line) + 1);
	while (eval(in, out, EV_TOKEN)) {
		if (EV_retcode) {
			ret = NULL;
			break;
		}
		if (firstime) {
			char *str;

			str = io_string(out);
			if (strncmp(str, "vary", 4) == 0) {
				if (*pflags & EVAL_ONCE)
					*pflags = (*pflags & (RETS | FREEIT)) | EVAL_SOMETIMES;
				io_seek(out, 0);
			}
			else if (strncmp(str, "const", 5) == 0) {
				if (*pflags & EVAL_SOMETIMES)
					*pflags = (*pflags & (RETS | FREEIT)) | EVAL_ONCE;
				io_seek(out, 0);
			}
			else
				firstime = FALSE;
			free(str);
		}
		if (!firstime)
			putac(' ', out);
	}
	/* remove trailing space */
	if (!firstime)
		unputac(out);
	if (ret)
		ret = io_string(out);
	io_close(out);
	io_close(in);
	return(ret);
}

/*
** Repeatedly calls "eval" to get a list of tokens.
*/
char **
parselist(list, pflags, beg, end)
char *list;
register int *pflags;
char beg;
char end;
{
	IOSTRUCT *in, *out;
	register char **hold;
	char *str;
	int more;

	hold = (char **) array_create(sizeof(char *), 5);
	if (!(list && *list))
		return(hold);
	in = io_open(EV_USE_STRING|EV_READONLY, list);
	out = io_open(EV_USE_STRING, NULL);
	if (eval(in, out, EV_TOKEN)) {
		str = io_ret_string(out);
		if (str && *str == '\0') {
			io_seek(out, 0);
			if (eval(in, out, EV_TOKEN))
				str = io_ret_string(out);
		}
		if (strncmp(str, "vary", 4) == 0) {
			if (*pflags & EVAL_ONCE)
				*pflags = (*pflags & (RETS | FREEIT)) | EVAL_SOMETIMES;
			io_seek(out, 0);
			if (eval(in, out, EV_TOKEN))
				str = io_ret_string(out);
		}
		else if (strncmp(str, "const", 5) == 0) {
			if (*pflags & EVAL_SOMETIMES)
				*pflags = (*pflags & (RETS | FREEIT)) | EVAL_ONCE;
			io_seek(out, 0);
			if (eval(in, out, EV_TOKEN))
				str = io_ret_string(out);
		}
		if (beg && (str[0] != beg)) {
			FILE *fp;
			char buf[BUFSIZ];

			if ((fp = fopen(str, "r")) == NULL) {
				warn(NOPEN, nil);
				io_close(in);
				io_close(out);
				return(NULL);
			}
			while (fgets(buf, BUFSIZ, fp)) {
				char *p;

				buf[strlen(buf) -1] = '\0';
				p = strsave(buf);
				hold = (char **) array_append(hold, &p);
			}
			io_close(in);
			io_close(out);
			return(hold);
		}
		if (!beg) {
			char *p, *q;

			str = strsave(str);
			if (EV_backquotes && (p = strtok(str, " \t\n"))) {
				for (; p; p = strtok(NULL, " \t\n")) {
					q = strsave(p);
					hold = (char **) array_append(hold, &q);
				}
				free(str);
			}
			else
				hold = (char **) array_append(hold, &str);
		}
		io_seek(out, 0);
		while (eval(in, out, EV_TOKEN)) {
			if (EV_retcode != 0) {
				array_trunc(hold);
				break;
			}
			str = io_string(out);
			io_seek(out, 0);
			if (!end || (*str != end))
				hold = (char **) array_append(hold, &str);
			else if (str)
				free(str);
		}
	}
	io_close(out);
	io_close(in);
	return(hold);
}
