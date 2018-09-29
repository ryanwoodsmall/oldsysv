/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */

#ident	"@(#)vm:oh/partab.c	1.1"

/* Note: this file created with tabstops set to 4.
 *
 * Definition of the Object Parts Table (OPT).
 *
 */

#include <stdio.h>
#include "tsys.h"
#include "but.h"
#include "typetab.h"
#include "ifuncdefs.h"
#include "optabdefs.h"
#include "partabdefs.h"


/*** NOTE: the ordering of the objects in this table must be the same
 *** as the order in the object operations table (In optab.c), as this table is
 *** used as an index into that table.
 ***/

struct opt_entry Partab[MAX_TYPES] = {
	{ "DIRECTORY",	"File folder",	CL_DIR, "?", "?", "?", "?", "?", 0, 2},
	{ "ASCII",		"Standard file",	CL_DOC,	"?", "?", "?", "?", "?", 2, 1},
	{ "MENU",		"Menu",	CL_DYN | CL_FMLI,	"?", "?", "?", "?", "?", 3, 1},
	{ "FORM",		"Form",	CL_FMLI,	"?", "?", "?", "?", "?", 4, 1},
	{ "TEXT",		"Text",	CL_FMLI,	"?", "?", "?", "?", "?", 5, 1},
	{ "EXECUTABLE",	"Executable",	CL_FMLI, "?", "?", "?", "?", "?", 7, 1},
	{ "TRANSFER",	"Foreign file",	CL_OEU, "?", "?", "?", "?", "?", 6, 1},
	{ "UNKNOWN",	"Data file",	NOCLASS, "?", "?", "?", "?", "?", 7, 1},
	{ "", "", NOCLASS, NULL, NULL, NULL, NULL, NULL, 0, 0}
};

struct one_part Parts[MAXPARTS] = {
	{"1",	"%.14s", PRT_DIR},						/* 0  DIRECTORY */
	{"2",	"%.14s/.pref",	PRT_FILE|PRT_OPT},		/* 1            */
	{"1",	"%.14s", PRT_FILE},						/* 2  ASCII     */
	{"1",   "Menu.%.9s", PRT_FILE},					/* 3  MENU      */
	{"1",   "Form.%.9s", PRT_FILE},					/* 4  FORM      */
	{"1",   "Text.%.9s", PRT_FILE},					/* 5  TEXT      */
	{"1",	"%.14s", PRT_FILE|PRT_BIN},				/* 6  TRANSFER  */
	{"1",	"%.14s", PRT_FILE|PRT_BIN},				/* 7  UNKNOWN/EXEC*/
	{"","",0}
};
