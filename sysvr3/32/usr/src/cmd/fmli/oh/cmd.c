/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.oh:src/oh/cmd.c	1.14"

#include <stdio.h>
#include "wish.h"
#include "token.h"
#include "slk.h"
#include "actrec.h"
#include "terror.h"
#include "ctl.h"
#include "menudefs.h"
#include "vtdefs.h"
#include "fm_mn_par.h"
#include "moremacros.h"
#include "eval.h"

extern char *Args[];
extern int Arg_count;

static struct cmdspec {
	char *name;
	token tok;
	int  helpindex;
	char *tokstr;
	char *helpaction;
};

#define NOTEXT	((int) -1)

/*
 * Table from which command defaults are selected
 */
static struct cmdspec Defaults[] = {
	{"cancel",	TOK_CLOSE,	0,	NULL, NULL },
	/* {"checkprint",  TOK_CHECKPRINT, NOTEXT,	NULL, NULL },	 */
	{"cleanup",	TOK_CLEANUP,	1,	NULL, NULL },
	{"copy",	TOK_COPY,	NOTEXT,	NULL, NULL },
	{"create",	TOK_CREATE,	NOTEXT,	NULL, NULL },
	{"delete",	TOK_DELETE,	NOTEXT,	NULL, NULL },
	{"display",	TOK_DISPLAY,	NOTEXT,	NULL, NULL },
	{"exit",	TOK_LOGOUT,	3,	NULL, NULL },
	{"find",	TOK_FIND,	NOTEXT,	NULL, NULL },
	{"frm-mgmt",	TOK_WDWMGMT,	4,	NULL, NULL },
	{"goto",	TOK_GOTO,	5,	NULL, NULL },
	{"help",	TOK_HELP,	6,	NULL, NULL },
	{"move",	TOK_MOVE,	NOTEXT,	NULL, NULL },
	{"next-frm",	TOK_NEXT_WDW,	7,	NULL, NULL },
	{"organize",	TOK_ORGANIZE,	NOTEXT,	NULL, NULL },
	{"prev-frm",	TOK_PREV_WDW,	8,	NULL, NULL },
	{"print",	TOK_PRINT,	NOTEXT,	NULL, NULL }, 
	{"redescribe",	TOK_SREPLACE,	NOTEXT,	NULL, NULL },
	{"refresh",	TOK_REFRESH,	9,	NULL, NULL },
	{"rename",	TOK_REPLACE,	NOTEXT,	NULL, NULL },
	{"run",		TOK_RUN,	NOTEXT,	NULL, NULL },
	{"security",	TOK_SECURITY,	NOTEXT,	NULL, NULL },
	{"time",	TOK_TIME,	NOTEXT,	NULL, NULL },
	{"undelete", 	TOK_UNDELETE,	NOTEXT,	NULL, NULL },
	{"unix",	TOK_UNIX,	10,	NULL, NULL },
	{"update",	TOK_REREAD,	11,	NULL, NULL },
	{NULL, 		TOK_NOP,	NOTEXT,	NULL, NULL }
};

#define MAX_CMD	64

/*
 * Command table, presented to the user via the command menu.
 * This table, once initialized, is kept in alphabetical order.
 */
static struct cmdspec Commands[MAX_CMD];

/*
 * Commands that the user doesn't see in the cmd menu, but exist
 * none-the-less, (most are used for token translations from within
 * the FMLI language).
 */
static struct cmdspec Interncmd[] = {
	{"badchar",	TOK_BADCHAR,	NOTEXT,  NULL, NULL },
	{"cancel",	TOK_CANCEL,	NOTEXT,  NULL, NULL },
	{"choices",	TOK_OPTIONS,	NOTEXT,  NULL, NULL },
	{"checkworld",	TOK_CHECKWORLD,	NOTEXT,  NULL, NULL },
	{"close",	TOK_CLOSE,	NOTEXT,  NULL, NULL },
	{"done",	TOK_DONE,	NOTEXT,  NULL, NULL },
	{"mark",	TOK_MARK,	NOTEXT,  NULL, NULL },
	{"nextpage",	TOK_NPAGE,	NOTEXT,  NULL, NULL },
	{"nop",		TOK_NOP,	NOTEXT,  NULL, NULL },
	{"nunique", 	TOK_NUNIQUE,	NOTEXT,  NULL, NULL },
	{"objop",	TOK_OBJOP,	NOTEXT,  NULL, NULL },
	{"open",	TOK_OPEN,	NOTEXT,  NULL, NULL },	
	{"prevpage",	TOK_PPAGE,	NOTEXT,  NULL, NULL },
	{"release",	TOK_RELEASE,	NOTEXT,  NULL, NULL },
	{"reset",	TOK_RESET,	NOTEXT,  NULL, NULL },
	{"run",		TOK_RUN,	NOTEXT,  NULL, NULL },
	{"togslk",	TOK_TOGSLK,	NOTEXT,  NULL, NULL },

	/* Secret commands, they wouldn't let us document them... */
	/*{"?",		TOK_REDO,	NOTEXT,  NULL, NULL },*/
	/*{"%",		TOK_DEBUG,	NOTEXT,  NULL, NULL },*/
	{"=",		TOK_SET,	NOTEXT,  NULL, NULL },

	{NULL,		TOK_NOP,	NOTEXT,  NULL, NULL }
};

static int Numdefaults = sizeof(Defaults)/sizeof(struct cmdspec);
static int Numcommands = sizeof(Commands)/sizeof(struct cmdspec);
static struct actrec *Cmd_ar;
static char *Tokstr;

extern int Vflag;	/* is this the User Interface ?? */

cmd_table_init()
{
	register int i, j;

	for (i = 0, j = 0; i < Numdefaults; i++) {
		if (Vflag || Defaults[i].helpindex != NOTEXT)
			Commands[j++] = Defaults[i];
	}
	Commands[j].name = NULL;
}

static struct menu_line
cmd_disp(n, ptr)
int n;
char *ptr;
{
	struct menu_line m;

	m.description = NULL;
	m.flags = 0;
	if (n >= Numcommands)
		m.highlight = NULL;
	else
		m.highlight = Commands[n].name;
	return m;
}

static int
cmd_odsh(a, t)
struct actrec *a;
token t;
{
	extern int Arg_count;
	char **actstr, **eval_string();
	token tok;
	int flags;

	t = menu_stream(t);
	if (t == TOK_OPEN && Arg_count <= 1) {
		int line;

		(void) menu_ctl(a->id, CTGETPOS, &line);
		if (Commands[line].tok >= 0)		/* internal */
			tok = Commands[line].tok;
		else {
			flags = RET_ARGS;
			actstr = eval_string(Commands[line].tokstr, &flags);
			tok = make_action(actstr);
		}
		t = arf_odsh(a->backup, tok);
		(void) ar_close(a);	/* Command execution causes close */
	}
	else if (t == TOK_NEXT)
		t = TOK_NOP;		/* eat it up */
	else if (t == TOK_CANCEL) {
		ar_backup();
		t = TOK_NOP;
	}
	return t;
}

static int
cmd_close(a)
struct actrec *a;
{
	Cmd_ar = NULL;
	return(AR_MEN_CLOSE(a));
}

cmd_help(cmd)
char *cmd;
{
	char help[PATHSIZ];
	int flags;
	char **helpaction, **eval_string();
	token tok, make_action();
	extern char *Filesys;
	char *cur_cmd(), *tok_to_cmd();
	struct cmdspec *command, *get_cmd();
	extern int Vflag;

	if (cmd && *cmd)
		cmd = tok_to_cmd(cmd_to_tok(cmd));
	else
		cmd = cur_cmd();
	if (!cmd || ((command = get_cmd(cmd)) == NULL)) {
		mess_temp("Could not find help on that command");
		return(SUCCESS);
	}
	
	if (command->helpindex != NOTEXT) 
		return(generic_help(cmd, command->helpindex));
	else if (strcmp(cmd, "CMD-MENU") == 0)		/* why here ? */
		return(generic_help(cmd, 3));	
	else if (Vflag) {
		sprintf(help, "%s/OBJECTS/Text.help", Filesys);
		objop("OPEN", "TEXT", help, cmd, cmd, NULL);
		return(SUCCESS);
	}
	else if (command->helpaction && command->helpaction[0] != '\0') {
		flags = RET_ARGS;
		helpaction = eval_string(command->helpaction, &flags);
		tok = make_action(helpaction);
		return(tok);
	}
	else
		mess_temp("Could not find help on that command");
	return SUCCESS;
}

extern char *Help_text[];

char *Help_args[3] = {
	"OPEN",
	"TEXT",
	"-i"
};

token
generic_help(name, helpindex)
char *name;
int  helpindex;
{
	extern char	*Args[];
	extern int	Arg_count;
	extern int	Vflag;
	register IOSTRUCT *out;

	out = io_open(EV_USE_STRING, NULL);
	putastr("title=Help Facility: \"", out);
	putastr(name, out);
	putastr("\"\n", out);
	putastr("lifetime=longterm\n", out);
	putastr("rows=12\n", out);
	putastr("columns=72\n", out);
	putastr("begrow=distinct\n", out);
	putastr("begcol=distinct\n", out);
	putastr("text=\"", out);
	putastr(Help_text[helpindex], out);
	putastr("\"\n", out);
	if (Vflag) {
		putastr("name=\"CONTENTS\"\n",out);
		putastr("button=8\n",out);
		putastr("action=OPEN MENU OBJECTS/Menu.h0.toc\n",out);
	}
	for (Arg_count = 0; Arg_count < 3; Arg_count++)
	{
		if ( Args[Arg_count])
			free( Args[Arg_count]); /* les 12/4 */
		Args[Arg_count] = strsave(Help_args[Arg_count]);
	}
	if ( Args[Arg_count])
		free( Args[Arg_count]); /* les 12/4 */
	Args[Arg_count++] = (char *) io_string(out);
	if ( Args[Arg_count])
		free( Args[Arg_count]); /* les 12/4 */
	Args[Arg_count] = NULL;
	io_close(out);
	return(TOK_OPEN);
}

struct actrec *
cmd_create()
{
	struct actrec a;
	vt_id vid;
	struct actrec *ar_create(), *ar_current();

	for (Numcommands = 0; Commands[Numcommands].name; Numcommands++)
		;

	if (Numcommands == 0) {
		mess_temp("There are no commands in the command menu");
		return(NULL);
	}

	a.id = (int) menu_default("Command Menu", VT_NONUMBER | VT_CENTER, 
			VT_UNDEFINED, VT_UNDEFINED, cmd_disp, NULL);

	ar_menu_init(&a);
	a.fcntbl[AR_CLOSE] = cmd_close;
	a.fcntbl[AR_ODSH] = cmd_odsh;
	a.fcntbl[AR_HELP] = cmd_help;
	a.flags = 0;

	Cmd_ar = ar_create(&a);
	return(ar_current(Cmd_ar));
}

token
_cmd_to_tok(cmd, partial, slk)
char *cmd;
bool partial;
bool slk;
{
	register int i;
	register int size = strlen(cmd);
	register int cmdnumatch = 0, slknumatch = 0;	/* number of matches */
	register int cmdmatch= -1, slkmatch = -1;		/* index of last match */
	extern struct slk SLK_array[MAX_SLK];
	int strnCcmp(), strCcmp();

	Tokstr = NULL;
	if (slk) {
		for (i = 0; i < MAX_SLK; i++) {
			if ((partial ? strnCcmp : strCcmp)(SLK_array[i].label, cmd, size) == 0) {
				if (slknumatch == 1 && SLK_array[i].tok == SLK_array[slkmatch].tok)
					continue;
				slknumatch++;
				slkmatch = i;
			}
		}
	}

	for (i = 0; i < Numcommands; i++) {
		if ((partial ? strnCcmp : strCcmp)(Commands[i].name, cmd, size) == 0) {
			cmdnumatch++;
			cmdmatch = i;
		}
	}

	if (slknumatch + cmdnumatch == 0) {
		/*
		 * no matches, check internal command table 
		 */
		for (i = 0; Interncmd[i].name; i++)
			if (strCcmp(Interncmd[i].name, cmd) == 0)
				return(Interncmd[i].tok);
		return(TOK_NOP);
	}
	else if (slknumatch > 1 || cmdnumatch > 1)	/* input not unique */
		return(TOK_NUNIQUE);
	else if (slknumatch == 1 && cmdnumatch == 0) {	/* matched slk only */
		Tokstr = SLK_array[slkmatch].tokstr;
		return(SLK_array[slkmatch].tok);
	}
	else if (cmdnumatch == 1 && slknumatch == 0) {    /* matched cmd only */
		Tokstr = Commands[cmdmatch].tokstr;
		return(Commands[cmdmatch].tok);
	}
	else {	/*
		 * one match in both slk and command
		 */
		if (strCcmp(SLK_array[slkmatch].label, Commands[cmdmatch].name) == 0) {
			Tokstr = SLK_array[slkmatch].tokstr;
			return(SLK_array[slkmatch].tok);	/* slk takes precedence*/
		}
		else
			return(TOK_NUNIQUE);
	}
}

/* LES: replace with MACRO's

token
cmd_to_tok(cmd)
char *cmd;
{
	return(_cmd_to_tok(cmd, TRUE, TRUE));
}

		NEVER CALLED
token
fullcmd_to_tok(cmd)
char *cmd;
{
	return(_cmd_to_tok(cmd, FALSE, TRUE));
}

token
mencmd_to_tok(cmd)
char *cmd;
{
	return(_cmd_to_tok(cmd, FALSE, FALSE));
}
*/

char *
tok_to_cmd(tok)
token tok;
{
	register int i;
	extern struct slk SLK_array[];

	/*  Most frequently referenced command is open, make it QUICK !!! */
	if (tok == TOK_OPEN)
		return("open");

	for (i = 0; i < Numcommands; i++)
		if (Commands[i].tok == tok)
			return Commands[i].name;
	for (i = 0; SLK_array[i].label; i++)
		if (SLK_array[i].tok == tok)
			return SLK_array[i].label;
	for (i = 0; Interncmd[i].name; i++)
		if (Interncmd[i].tok == tok)
			return Interncmd[i].name;
	return NULL;
}

char *
cur_cmd()
{
	int	line;
	/* char *cur_hist(); */

	if (ar_get_current() != Cmd_ar)
		return(NULL);
	menu_ctl(Cmd_ar->id, CTGETPOS, &line);
	return Commands[line].name;
}

/*
 * ADD_CMD will add a command to the command list preserving
 * alphabetical ordering
 */
add_cmd(name, tokstr, help)
char *name;
char *tokstr;
char *help;
{
	register int i, j, comp;

	for (i = 0; Interncmd[i].name; i++)
		if (strcmp(Interncmd[i].name, name) == 0)
			return;		/* no go, internal command conflict */ 
	for (i = 0; Commands[i].name; i++) {
		comp = strcmp(name, Commands[i].name);
		if (comp < 0) {
			/*
			 * shift list to make room for new entry
			 */
			for (j = MAX_CMD - 1; j > i; j--)
				Commands[j] = Commands[j - 1];

			Commands[i].name = strsave(name);
			Commands[i].tok = -1;	/* no token */ 
			Commands[i].helpindex = NOTEXT; 
			Commands[i].tokstr = strsave(tokstr);
			Commands[i].helpaction = strsave(help);
			break;
		}
		else if (comp == 0) {
			/*
			 * Command already exists
			 */
			if (Commands[i].tok >= 0) {
				/*
				 * Name conflict with a generic command,
				 * only accept redefinitions for helpaction
				 */
				if (help && (*help != '\0')) {
					Commands[i].helpindex = NOTEXT;
					Commands[i].helpaction= strsave(help);
				}
			}
			else {
				/*
				 * Redefine a previous definition
				 */
				Commands[i].name = strsave(name);
				Commands[i].tok = -1;	/* no token */ 
				Commands[i].helpindex = NOTEXT;
				Commands[i].tokstr = strsave(tokstr);
				Commands[i].helpaction = strsave(help);
			}
			break;
		}
	}
}


/*
 * DEL_CMD will remove a command from the command menu
 * (shifting the command menu accordingly)
 */
del_cmd(name)
char *name;
{
	register int i, j, comp;

	for (i = 0; Commands[i].name; i++) {	/* if not end of list */
		if ((comp = strcmp(name, Commands[i].name)) == 0) {
			/*
			 * scrunch list to remove entry
			 */
			for (j = i; j < MAX_CMD - 1; j++)
				Commands[j] = Commands[j + 1];
			break;
		}
	}
}

static struct cmdspec *
get_cmd(cmdstr)
char *cmdstr;
{
	register int i;

	for (i = 0; i < Numcommands && Commands[i].name; i++)
		if (strcmp(Commands[i].name, cmdstr) == 0)
			return(&(Commands[i]));
	return(NULL);
} 
	
token
do_app_cmd()
{
	char **strlist, **eval_string();
	token t, make_action();
	int flags;


	if (Tokstr) {		/* set in _cmd_to_tok */
		flags = RET_ARGS; 
		strlist = eval_string(Tokstr, &flags);
		t = make_action(strlist);
	}
	else
		t = TOK_NOP;
}
