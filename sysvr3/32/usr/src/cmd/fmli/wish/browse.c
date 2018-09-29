/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.wish:src/wish/browse.c	1.13"

#include <stdio.h>
#include "wish.h"
#include "token.h"
#include "slk.h"
#include "actrec.h"
#include "terror.h"
#include "ctl.h"
#include	"moremacros.h"

#define BROWSE	1
#define PROMPT	2

/*
 * Caution: MAX_ARGS is defined in other files and should ultimately reside 
 * in wish.h
 */
#define MAX_ARGS	25	

extern int Arg_count;
extern char *Args[];
char *Pending_op, *Pending_objtype, *Pending_argv[MAX_ARGS+2];
struct slk *Pending_slks;
int Pending_type;

static char	name_string[] = "Enter the new object name: ";
static char	desc_string[] = "Enter the new description: ";

int Browse_mode = 0;

enter_browse(op, objtype, argv)
char *op, *objtype, *argv[];
{
	register int i;
	extern struct slk Browslk[], Defslk[];

	Browse_mode++;
	save_browse(op, objtype, argv);
	setslks(NULL, 0);
	mess_temp("Open or navigate to the destination folder and press SELECT");
	Pending_type = BROWSE;
}

enter_getname(op, objtype, argv)
char *op, *objtype, *argv[];
{
	register int i;
	token namevalid();

	save_browse(op, objtype, argv);
	Pending_type = PROMPT;
	get_string(namevalid, strCcmp(Pending_op, "redescribe") ? name_string : desc_string, "", 0, FALSE, Pending_op, Pending_op);
}

static token
namevalid(s, t)
char *s;
token t;
{
	register int i;
	char *errstr;

	if (t == TOK_CANCEL) {
		Pending_op = NULL;
		return TOK_NOP;
	}

	if (strCcmp(Pending_op, "create") == 0) {
		if (namecheck(Pending_argv[0], s, NULL, &errstr, TRUE) == FALSE) {
			get_string(namevalid, name_string, "", 0, FALSE, Pending_op, Pending_op);
			mess_temp(errstr);
			return TOK_NOP;
		}
	} else if (strCcmp(Pending_op, "redescribe") != 0) {
		if (namecheck(Pending_argv[1], s, Pending_objtype, &errstr, TRUE)==FALSE) {
			get_string(namevalid, desc_string, "", 0, FALSE, Pending_op, Pending_op);
			mess_temp(errstr);
			return TOK_NOP;
		}
	}

	for (i = 0; Pending_argv[i]; i++)
		;
	Pending_argv[i] = strsave(s);
	Pending_argv[i+1] = NULL;
	glob_select();
	return(TOK_NOP);
}

glob_select()
{
	register int i, prevtype = Pending_type;
	bool canselect;

	if (Pending_type == BROWSE) {
		if (ar_ctl(ar_get_current(), CTISDEST, &canselect) == FAIL || !canselect) {
			mess_temp("This frame can not be used as a destination");
			return;
		}
		for (i = 0; Pending_argv[i]; i++)
			;

		Pending_argv[i] = strsave(ar_get_current()->path);
		Pending_argv[i+1] = NULL;
	}

	if (strcmp(Pending_op, "redescribe") == 0) {
		working(TRUE);
		redescribe(&Pending_argv[0]);
	} else if (strcmp(Pending_op, "create") == 0) {
		working(TRUE);
		Create_create(&Pending_argv[0]);
	} else {
		mess_perm(NULL);
		working(TRUE);
		(void) objopv(Pending_op, Pending_objtype, Pending_argv);
	}

	if (Pending_type == prevtype)
		glob_browse_cancel();
	ar_checkworld(TRUE);
}

int
glob_browse_cancel()
{
	Browse_mode = 0;
	ar_setslks(ar_get_current()->slks);
	if (Pending_op) {
		register int i;

		free(Pending_op);
		Pending_op = NULL;
		if (Pending_objtype) {
			free(Pending_objtype);
			Pending_objtype = NULL;
		}
		for (i = 0; Pending_argv[i]; i++) {
			free(Pending_argv[i]);
			Pending_argv[i] = NULL;
		}
	}
}

token
glob_mess_nosrc(t)
token t;
{
	/* WARNING: tok_to_cmd had better NEVER return NULL here! */
	mess_temp(nstrcat("Can't ", tok_to_cmd(t), " from this frame", NULL));
	return(t);
}

static
save_browse(op, objtype, argv)
char *op, *objtype, *argv[];
{
	register int i;

	Pending_op = strsave(op);
	Pending_objtype = strsave(objtype);
	Pending_slks = ar_get_current()->slks;
	for (i = 0; argv[i]; i++)
		Pending_argv[i] = strsave(argv[i]);
	Pending_argv[i] = NULL;
}
