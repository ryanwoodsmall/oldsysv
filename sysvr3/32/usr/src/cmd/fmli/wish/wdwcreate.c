/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.wish:src/wish/wdwcreate.c	1.2"

#include <stdio.h>
#include "wish.h"
#include "token.h"
#include "slk.h"
#include "actrec.h"
#include "terror.h"
#include "ctl.h"
#include "menudefs.h"
#include "vtdefs.h"

extern char *Args[];
extern int Arg_count;

int
glob_create()
{
	char path[PATHSIZ], *errstr;
	static char *argv[3];
	extern char *Filecabinet;
	char *path_to_full(), *path_to_title(), *cur_path();

	argv[0] = argv[1] = argv[2] = NULL;
	if (parse_n_in_fold(&argv[1], &argv[0]) == FAIL)
		return(TOK_CREATE);
	if (eqwaste(argv[0]))
		return(FAIL);
	if (isfolder(argv[0]) == FALSE) {
		mess_temp("You can only create new objects inside File folders");
		return(FAIL);
	}
	if (access(argv[0], 02) < 0) {
		mess_temp(nstrcat("You don't have permission to create objects in ", path_to_title(argv[0]), NULL));
		return(FAIL);
	}
	if (argv[1] == NULL) {
		enter_getname("create", "", argv);
		return(TOK_NOP);
	}
	if (namecheck(argv[0], argv[1], NULL, &errstr, TRUE) == FALSE) {
		mess_temp(errstr);
		argv[1] = NULL;
		enter_getname("create", "", argv);
		return(TOK_NOP);
	}
	Create_create(argv);
	return(TOK_NOP);
}

int
Create_create(argv)
char *argv[];
{
	char *path_to_title();

	working(TRUE);
	return(objop("OPEN", "MENU", "$VMSYS/OBJECTS/Menu.create", argv[0], argv[1], path_to_title(argv[0], NULL), NULL));
}

static int
eqwaste(str)
char *str;
{
	extern char *Wastebasket;

	if (strncmp(str, Wastebasket, strlen(Wastebasket)) == 0) {
		mess_temp("You cannot create objects in your WASTEBASKET");
		return(1);
	}
	return(0);
}
