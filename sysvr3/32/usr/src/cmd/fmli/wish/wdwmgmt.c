/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.wish:src/wish/wdwmgmt.c	1.6"

#include <stdio.h>
#include "wish.h"
#include "token.h"
#include "slk.h"
#include "actrec.h"
#include "terror.h"
#include "ctl.h"
#include "menudefs.h"
#include "vtdefs.h"
#include "moremacros.h"

#define NMUDGES	4

static struct menu_line	Mgmt_lines[NMUDGES] = {
	{ "list",	"list all frames",	0 },
	{ "move",	"move a frame",	0 },
	{ "reshape",	"reshape a frame",	0 },
	{ NULL,		NULL,			0 }
};

extern int Noreshape;		/* (yuk) see if_form.c */

static struct menu_line
mgmt_disp(n, ptr)
int n;
char *ptr;
{ 
	if (n >= NMUDGES)
		n = NMUDGES - 1;
	return Mgmt_lines[n];
}

static int
mgmt_odsh(a, t)
struct actrec *a;
token t;
{
	int 	line;
	token	menu_stream();

	t = menu_stream(t);
	if (t == TOK_ENTER || t == TOK_OPEN) {
		(void) menu_ctl(a->id, CTGETPOS, &line);
		switch (line) {
		case 0: /* list */
			list_create();
			break;
		case 1:	/* move */
			enter_wdw_mode((struct actrec *)(a->odptr), FALSE);
			break;
		case 2: /* reshape */
			if (Noreshape) {
				Noreshape = 0;
				mess_temp("Forms cannot be reshaped");
				t = TOK_NOP;
			}
			else 
				enter_wdw_mode((struct actrec *)(a->odptr), TRUE);
			break;
		}
		t = TOK_NOP;
	} else if (t == TOK_CANCEL) {
		ar_backup();
		t = TOK_NOP;
	} else if (t == TOK_NEXT)
		t = TOK_NOP;		/* filter out, see menu_stream */
	return t;
}

static int
mgmt_help(a)
struct actrec *a;
{
	return(objop("OPEN", "TEXT", "$VMSYS/OBJECTS/Text.mfhelp", "frm-mgmt", "Frame Management", NULL));
}

int
mgmt_create()
{
	char	*cmd;
	int		len;
	vt_id vid;
	struct actrec a, *ar, *ar_create(), *window_arg();
	extern int Arg_count;
	extern char *Args[];

	switch (Arg_count) {
	case 0:
	case 1:
		cmd = NULL;
		ar = ar_get_current();
		break;
	case 2:
		/* This case is ambiguous, since the argument could be either one
		 * of the three commands "move" "reshape" or "list" or it could be
		 * a window path or number.  So, let's just say its a window if it
		 * isn't move reshape or list.  Hope nobody tries this on a window
		 * named "list"!
		 */
		len = strlen(Args[1]);
		if (strnCcmp(Args[1], "move", len) == 0 ||
			strnCcmp(Args[1], "reshape", len) == 0 ||
			strnCcmp(Args[1], "list", len) == 0) {
			cmd = Args[1];
			ar = ar_get_current();
		} else {
			cmd = NULL;
			if ((ar = window_arg(1, Args + 1)) == NULL)
				mess_temp(nstrcat("Unknown command: ", Args[1], " ignored", NULL));
			return(FAIL);
		}
		break;
	default:	
		cmd = Args[1];
		ar = window_arg(1, Args + 2);
	}

	if (cmd == NULL) {
		a.id = (int) menu_default("Frame Management", VT_NONUMBER | VT_CENTER, 
				VT_UNDEFINED, VT_UNDEFINED, mgmt_disp, NULL);
		if (a.id == FAIL)
			return(FAIL);

		ar_menu_init(&a);
		a.fcntbl[AR_ODSH] = mgmt_odsh;
		a.fcntbl[AR_HELP] = mgmt_help;
		a.odptr = (char *) ar;
		a.flags = 0;

		return(ar_current(ar_create(&a)) == NULL?FAIL:SUCCESS);
	} else if (strncmp(cmd, "list", strlen(cmd)) == 0)
		list_create();
	else if (strncmp(cmd, "move", strlen(cmd)) == 0)
		enter_wdw_mode(ar, FALSE);
	else if (strncmp(cmd, "reshape", strlen(cmd)) == 0) {
		if (Noreshape) {
			Noreshape = 0;
			mess_temp("Forms cannot be reshaped");
		}
		else
			enter_wdw_mode(ar, TRUE);
	}
	else {
		mess_temp("Bad argument to frmmgmt: try list, move or reshape");
		return(FAIL);
	}

	return(SUCCESS);
}
