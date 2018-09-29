/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.qued:src/qued/fstream.c	1.1"

#include	<stdio.h>
#include	<ctype.h>
#include	"wish.h"
#include	"terror.h"
#include	"token.h"
#include	"winp.h"
#include	"fmacs.h"
#include	"vtdefs.h"
#include	"ctl.h"	
#include	"attrs.h"

#define MAXSUBS		5

/* ODSH functions */
extern token singleline();
extern token multiline();
extern token editsingle();
extern token editmulti();

token (*Substream[][MAXSUBS])() = {
	{
		editsingle,
		singleline,
		NULL
	},
	{
		editsingle,
		editmulti,
		multiline,
		NULL
	},
};

token
field_stream(tok)
token tok;
{
	token stream();

	return(stream(tok, Substream[Currtype]));
}
