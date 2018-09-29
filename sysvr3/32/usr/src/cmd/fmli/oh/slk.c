/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.oh:src/oh/slk.c	1.22"

#include	<stdio.h>
#include	"wish.h"
#include	"token.h"
#include	"fm_mn_par.h"
#include	"slk.h"
#include	"moremacros.h"

struct slk Moreslk = {
	"CHG-KEYS",	TOK_TOGSLK,	NULL
};

struct slk Markslk = {
	"MARK",		TOK_MARK,	NULL
};

struct slk Blankslk = {
	"",		TOK_NOP,	NULL
};

struct slk Formslk[] = {
	{"CANCEL",	TOK_CLOSE,	NULL},
	{"CHOICES",	TOK_OPTIONS,	NULL},
	{"SAVE",	TOK_DONE,	NULL},
	{"PREV-FRM",	TOK_PREV_WDW,	NULL},
	{"NEXT-FRM",	TOK_NEXT_WDW,	NULL},
	{"HELP",	TOK_HELP,	NULL},
	{"CMD-MENU",	TOK_CMD,	NULL},
	{"",		TOK_NOP,	NULL},
	{NULL, TOK_NOP, NULL}
};

#define MARK 1

struct slk Menuslk[] = {
	{"CANCEL",	TOK_CLOSE,	NULL},
	{"",		TOK_NOP,	NULL},		/* MARK for mult. selections  */
	{"",	TOK_NOP,	NULL},
	{"PREV-FRM",	TOK_PREV_WDW,	NULL},
	{"NEXT-FRM",	TOK_NEXT_WDW,	NULL},
	{"HELP",	TOK_HELP,	NULL},
	{"CMD-MENU",	TOK_CMD,	NULL},
	{"",		TOK_NOP,	NULL},
	{NULL,		TOK_NOP,	NULL}
};

struct slk Textslk[] = {
	{"CANCEL",	TOK_CLOSE,	NULL},
	{"PREVPAGE",	TOK_PPAGE,	NULL},
	{"NEXTPAGE",	TOK_NPAGE,	NULL},
	{"PREV-FRM",	TOK_PREV_WDW,	NULL},
	{"NEXT-FRM",	TOK_NEXT_WDW,	NULL},
	{"HELP",	TOK_HELP,	NULL},
	{"CMD-MENU",	TOK_CMD,	NULL},
	{"",		TOK_NOP,	NULL},
	{NULL,		TOK_NOP,	NULL}
};

/*
 * FACE user interface specific SLKS
 */
struct slk Vmslk[] = {
	{"CREATE",	TOK_CREATE,	NULL},
	{"COPY",	TOK_COPY,	NULL},
	{"MOVE",	TOK_MOVE,	NULL},
	{"DELETE",	TOK_DELETE,	NULL},
	{"RENAME",	TOK_REPLACE,	NULL},
	{"HELP",	TOK_HELP,	NULL},
	{"SECURITY",	TOK_SECURITY,	NULL},
	{"",		TOK_NOP,	NULL},
	{NULL,		TOK_NOP,	NULL}
};

struct slk Echslk[] = {		/* Enter/Cancel/Help slks */
	{"CANCEL",	TOK_CANCEL,	NULL},
	{"",		TOK_BADCHAR,	NULL},
	{"",		TOK_BADCHAR,	NULL},
	{"",		TOK_BADCHAR,	NULL},
	{"",		TOK_BADCHAR,	NULL},
	{"HELP",	TOK_HELP,	NULL},
	{"",		TOK_BADCHAR,	NULL},
	{"",		TOK_BADCHAR,	NULL},
	{NULL,		TOK_NOP,	NULL}
};

extern int Browse_mode;

struct slk Browslk[] = {	/* slks when browsing */
	{"CANCEL",	TOK_CANCEL,	NULL},
	{"",		TOK_NOP,	NULL},
	{"",		TOK_NOP,	NULL},
	{"PREV-FRM",	TOK_PREV_WDW,	NULL},
	{"NEXT-FRM",	TOK_NEXT_WDW,	NULL},
	{"HELP",	TOK_HELP,	NULL},
	{"CMD-MENU",	TOK_CMD,	NULL},
	{"SELECT",	TOK_SELECT,	NULL},
	{NULL,		TOK_NOP,	NULL}
};

#define CHG_KEYS	7	/* SLK number of first level CHG_KEYS */
#define SECOND_LEVEL	8	/* number of slks displayable */
#define ALT_CHG_KEYS	15	/* SLK number of second level CHG_KEYS */
#define MAX_SLKS	16	/* total number of slks */	

struct slk No_slks[1];
struct slk SLK_array[MAX_SLKS];
struct slk Defslk[MAX_SLKS];
struct slk *Curslk = &Defslk[0];
static int SLK_level = 0;

static void showslks();
extern int Vflag;

/*
 * SETUP_SLK_ARRAY will initialize defaults for the SLKS
 */
setup_slk_array()
{
	register int i, j;

	for (i = 0; i < SECOND_LEVEL; i++)
		Defslk[i] = Menuslk[i];

	if (Vflag) { 
		for (i = SECOND_LEVEL, j = 0; i < MAX_SLKS; i++, j++)
			Defslk[i] = Vmslk[j];
	}
	else {
		for (i = SECOND_LEVEL; i < MAX_SLKS; i++) {
			Defslk[i].label = nil; 
			Defslk[i].tok = TOK_NOP;
			Defslk[i].tokstr = NULL;
		}
	}
}

#define REDEFINED(x)	 ((x).label && (*((x).label) != '\0' || (x).tok < 0))

/*
 * SETSLKS will make "slks" the currently displayed SLKS
 */
void
setslks(slks, flag)
struct slk	slks[];
int flag;
{
	register int	i, more_slks;
	void	showslks();

#ifdef _DEBUG
	_debug(stderr, "in setslk!\n");
#endif
	if (slks == NULL) {	/* e.g., directory object */
		if (Vflag && Browse_mode)
			set_top_slks(Browslk);
		else
			set_top_slks(Menuslk);
		setslks(Defslk, 0);
		return;
	}
	else if (slks == No_slks) {
#ifdef _DEBUG
		_debug(stderr, "slks are history\n");
#endif
		SLK_level = -1;
		slk_clear();
		return;
	}
	more_slks = 0;
	for (i = 0; slks[i].label && i < MAX_SLKS; i++) {
		if (i >= SECOND_LEVEL && *(slks[i].label) != '\0')
			more_slks++;
#ifdef _DEBUG
		_debug(stderr, "SLK_array[%d] = '%s'\n", i, slks[i].label);
#endif
		SLK_array[i] = slks[i];
	}
#ifdef _DEBUG
	if (slks[i].label)
		_debug(stderr, "setslks was passed an array without a NULL terminator\n");
#endif
	while (i < MAX_SLKS)
		SLK_array[i++].label = nil;
	if (more_slks && !(REDEFINED(SLK_array[CHG_KEYS]) ||
			 REDEFINED(SLK_array[ALT_CHG_KEYS]))) 
		SLK_array[CHG_KEYS] = SLK_array[ALT_CHG_KEYS] = Moreslk;
	showslks(flag);
}

int Refresh_slks = 0;

/*
 * SHOWSLKS will do the actial displaying
 */
static void
showslks(flag)
int flag;
{
	register int	i;
	register int	j;

	if (SLK_level < 0) {
		Refresh_slks++;
		/*slk_restore();    defer to vt_flush() */
	}
	if (flag)
		SLK_level = j = SECOND_LEVEL;
	else
		SLK_level = j = 0;
	for (i = 1; i <= SECOND_LEVEL; i++)
		slk_set(i, SLK_array[j++].label, 1);
	slk_noutrefresh();
}

/*
 * SLK_TOKEN will determine the token action for a given SLK
 */
token
slk_token(t)
token	t;
{
	register int	n;
	int flags;
	char **arglist, **eval_string();

	n = t - TOK_SLK1 + SLK_level;
#ifdef _DEBUG
	_debug(stderr, "slk %d is labeled '%s' and returns token %d\n", t, n, SLK_array[n].label,SLK_array[n].tok);
#endif
	if (SLK_array[n].label && SLK_array[n].label[0]) {
		if (SLK_array[n].tok >= 0)	/* internally-defined */
			return(SLK_array[n].tok);
		else {
			flags = RET_ARGS;
			arglist = eval_string(SLK_array[n].tokstr, &flags);
			return(make_action(arglist));
		}
	}
	return TOK_BADCHAR;
}

set_top_slks(slks)
struct slk slks[];
{
	register int i;

	for (i = 0; i < SECOND_LEVEL; i++)
		Defslk[i] = slks[i];
}

/*
 * SET_OBJ_SLK is called by objects that wish to redefine the
 * GLOBAL default SLKS ... 
 * The object will pass a "slktok" in the range SLK1 - SLK16.
 * If a token in this range is "caught" by the object,
 * the object itself will determine the appropriate action.
 */ 
set_obj_slk(slk, label, slktok)
struct slk *slk;
char *label;
int slktok;
{
	int slknum;

	if (label && (*label == '\0')) {     /* disable any SLK */
		slk->label = "";
		slk->tok = -1;
		slk->tokstr = NULL;
	}
	else {
		slknum = slktok - TOK_SLK1;  /* (adjust for array offset) */
		if (slknum >= CHG_KEYS) {    /* redefine certain SLKS */ 
			slk->label = strsave(label);
			/*
			 * unfortunately this test needs to be made ...
			 * (if odsh gets prevpage or nextpage its too late)
			 * We must search for a better way !!!
			 */
			if (strCcmp(label, "prevpage") == 0)
				slk->tok = TOK_PPAGE;
			else if (strCcmp(label, "nextpage") == 0)
				slk->tok = TOK_NPAGE;
			else if (strCcmp(label, "prev-frm") == 0)
				slk->tok = TOK_PREV_WDW;
			else if (strCcmp(label, "next-frm") == 0)
				slk->tok = TOK_NEXT_WDW;
			else
				slk->tok = slktok;
			slk->tokstr = NULL;
		}
		else {			     /* must use the defaults! */ 
			slk->label = Defslk[slknum].label;
			slk->tok = Defslk[slknum].tok;
			slk->tokstr = Defslk[slknum].tokstr;
		}
	}
}


/*
 * SET_DEF_SLK will over-write the GLOBAL default SLKS 
 */
set_def_slk(slknum, label, action)
int slknum;
char *label;
char *action;
{
	/*
	 * if less then CHG_KEYS do nothing (can't redefine first level)
	 */
	if (label && (*label == '\0')) {	/* disable any SLK */
		Defslk[slknum].label = "";
		Defslk[slknum].tok = -1;
		Defslk[slknum].tokstr = NULL;
	}
	else if (slknum >= CHG_KEYS) {		/* redefine certain SLKS */
		Defslk[slknum].label = strsave(label);
		Defslk[slknum].tok = -1;
		Defslk[slknum].tokstr = strsave(action);
	}
	/*
	 * if SLK is at the first level, then change the object
	 * SLKS to reflect the redefined or disabled SLK 
	 */
	if (slknum <= CHG_KEYS) {	
		Formslk[slknum] = Defslk[slknum];
		Textslk[slknum] = Defslk[slknum];
		Menuslk[slknum] = Defslk[slknum];
	}
}

/*
 * SLK_TOGGLE will toggle the currently displayed SLKS
 */
slk_toggle()
{
	showslks(!SLK_level);
}

set_slk_mark(flag)
int flag;
{
	if (flag == TRUE)
		Menuslk[MARK] = Markslk;
	else
		Menuslk[MARK] = Blankslk;
	Defslk[MARK] = Menuslk[MARK];
}
