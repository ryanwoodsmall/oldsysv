/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.oh:src/oh/if_init.c	1.17"

#include	<stdio.h>
#include	<string.h>
#include	"wish.h"
#include	"fm_mn_par.h"
#include	"terror.h"
#include	"var_arrays.h"
#include	"vtdefs.h"
#include	"ctl.h"
#include	"attrs.h"
#include	"color_pair.h"
#include	"moremacros.h"

/* single instance desriptors in commands file */
#define INIT_SING_KEYS		20

#define INIT_TITLE		0 
#define INIT_TEXT		1 
#define INIT_ROWS		2 
#define INIT_COLUMNS		3 
#define INIT_BANNER		4
#define INIT_BANCOL		5
#define INIT_WORKING		6
#define COL_SCREEN		7
#define COL_BANNER_TEXT		8
#define COL_WINDOW_TEXT	 	9	
#define COL_ACTIVE_BORD		10
#define COL_INACTIVE_BORD	11
#define COL_ACTIVE_TITLE	12
#define COL_ACTIVE_TITLE_BAR	13
#define COL_INACTIVE_TITLE	14
#define COL_INACTIVE_TITLE_BAR	15
#define COL_BAR			16
#define COL_BAR_TEXT		17
#define COL_SLK_BAR		18
#define COL_SLK_TEXT		19	

/* Multi-instance descriptors for initialization file */
#define INIT_MULT_KEYS 4

#define SLK_NAME		0
#define SLK_ACTION		1
#define SLK_BUTTON		2
#define SLK_SHOW		3

/* Single-instance descriptors for commands file */
/*
 * three is none
 */
#define CMD_SING_KEYS		0

/* Multi-instance descriptors for commands file */
#define CMD_MULT_KEYS		4

#define CMD_NAME		0
#define CMD_ACTION		1
#define CMD_HELP		2
#define CMD_SHOW		3

static struct attribute Init_single_tab[INIT_SING_KEYS] = {
	{ "title",	RET_STR|EVAL_ONCE,	"", NULL, 0 },
	{ "text",	RET_STR|EVAL_ONCE,	"", NULL, 0 },
	{ "rows",	RET_STR|EVAL_ONCE,	"10", NULL, 0 },
	{ "columns",	RET_STR|EVAL_ONCE,	"50", NULL, 0 },
	{ "banner",	RET_STR|EVAL_ONCE,	NULL, NULL, 0 },
	{ "bancol",	RET_INT|EVAL_ONCE,	"", NULL, 0 },
	{ "working",	RET_STR|EVAL_ONCE,	"Working", NULL, 0 },
	{ "screen",		RET_STR|EVAL_ONCE,	"black", NULL, 0 },	
	{ "banner_text",	RET_STR|EVAL_ONCE,	"", NULL, 0 },	
	{ "window_text",	RET_STR|EVAL_ONCE,	"", NULL, 0 },	
	{ "active_border",	RET_STR|EVAL_ONCE,	"", NULL, 0 },
	{ "inactive_border",	RET_STR|EVAL_ONCE,	"", NULL, 0 },
	{ "active_title_text",	RET_STR|EVAL_ONCE,	"", NULL, 0 },
	{ "active_title_bar",	RET_STR|EVAL_ONCE,	"", NULL, 0 },
	{ "inactive_title_text",RET_STR|EVAL_ONCE,	"", NULL, 0 },
	{ "inactive_title_bar",	RET_STR|EVAL_ONCE,	"", NULL, 0 },
	{ "highlight_bar",	RET_STR|EVAL_ONCE,	"", NULL, 0 },	
	{ "highlight_bar_text",	RET_STR|EVAL_ONCE,	"", NULL, 0 },
	{ "slk_bar",		RET_STR|EVAL_ONCE,	"", NULL, 0 },	
	{ "slk_text",		RET_STR|EVAL_ONCE,	"", NULL, 0 }
};

/*
 * Table for SLKS and CMDS
 */
static struct attribute Init_multi_tab[INIT_MULT_KEYS] = {
	{ "name",	RET_STR|EVAL_ALWAYS,	NULL, NULL, 0 },
	{ "action",	RET_ARGS|EVAL_ALWAYS,	NULL, NULL, 0 },
	{ "button",	RET_INT|EVAL_ALWAYS,	"0", NULL, 0 },
	{ "show",	RET_BOOL|EVAL_ALWAYS, "", NULL, 0 }
};

#define CMD_MULT_KEYS 4
static struct attribute Cmd_multi_tab[CMD_MULT_KEYS] = {
	{ "name",	RET_STR|EVAL_ALWAYS,	NULL, NULL, 0 },
	{ "action",	RET_ARGS|EVAL_ALWAYS,	NULL, NULL, 0 },
	{ "help",	RET_ARGS|EVAL_ALWAYS,	NULL, NULL, 0 },
	{ "show",	RET_BOOL|EVAL_ALWAYS,	"", NULL, 0 }
};

extern int Vflag;
extern int Color_terminal;

#define DEF_WORKING_MSG	"Working"
#define FOURFOUR	"4-4"

/*
 * Introductory object attributes
 */
static	char *Intro_title;
static	char *Intro_text;
static  int  Intro_rows;
static 	int  Intro_cols;
static  char *Banner = NULL;
static  char *Bancolstr;
static  char *Col_screen;
static  char *Col_banner_text;
static  char *Col_window_text;
static  char *Col_active_bord;
static  char *Col_inactive_bord;
static  char *Col_active_title;
static  char *Col_active_title_bar;
static  char *Col_inactive_title;
static  char *Col_inactive_title_bar;
static  char *Col_bar;
static  char *Col_bar_text;
static  char *Col_slk_bar;
static  char *Col_slk_text;

/*
 * extern globals
 */
char	*Work_msg = NULL;
int	Work_col;
int	Mail_col;

char *strnsave();
static struct fm_mn Inits;

/*
** Front-end to parser(), which sets up defaults.
*/
read_inits(initfile)
char *initfile;
{
	register int i, j, numslks;
	char  *get_def();
	static char *set_default();
	static int center_it();
	char *bancolstr;
	static int free_inits = FALSE;

	if (access(initfile, 04) < 0)
		return;

	if (free_inits == TRUE)
		freeitup(&Inits);
	else
		free_inits = TRUE;

	/*
	 * Parse initialization file
	 */
	Inits.single.attrs = NULL;
	Inits.multi = NULL;
	filldef(&Inits.single, Init_single_tab, INIT_SING_KEYS);
	parser(0, initfile, Init_single_tab, INIT_SING_KEYS, &Inits.single,
			Init_multi_tab, INIT_MULT_KEYS, &Inits.multi);

	numslks = array_len(Inits.multi);
	for (i = 0; i < numslks; i++) {
		if (multi_eval(&Inits, i, SLK_SHOW)) {
			set_def_slk(atoi(multi_eval(&Inits, i, SLK_BUTTON)) - 1,
				multi_eval(&Inits, i, SLK_NAME),
				get_def(&Inits, i, SLK_ACTION));
		}
	}

	/*
	 * Introductory object info 
	 */
	Intro_title = set_default(INIT_TITLE);
	Intro_text = set_default(INIT_TEXT);
	Intro_rows = atoi(set_default(INIT_ROWS));
	Intro_cols = atoi(set_default(INIT_COLUMNS));

	/*
	 * BANNER line info
	 */
	Banner = set_default(INIT_BANNER);
	Bancolstr = set_default(INIT_BANCOL);
	Work_msg = set_default(INIT_WORKING);

	/*
	 * COLOR specifications
	 */
	Col_screen = set_default(COL_SCREEN);
	Col_banner_text = set_default(COL_BANNER_TEXT);
	Col_window_text = set_default(COL_WINDOW_TEXT);
	Col_active_bord = set_default(COL_ACTIVE_BORD);
	Col_inactive_bord = set_default(COL_INACTIVE_BORD);
	Col_active_title = set_default(COL_ACTIVE_TITLE);
	Col_active_title_bar = set_default(COL_ACTIVE_TITLE_BAR);
	Col_inactive_title = set_default(COL_INACTIVE_TITLE);
	Col_inactive_title_bar = set_default(COL_INACTIVE_TITLE_BAR);
	Col_bar = set_default(COL_BAR);
	Col_bar_text = set_default(COL_BAR_TEXT);
	Col_slk_bar = set_default(COL_SLK_BAR);
	Col_slk_text = set_default(COL_SLK_TEXT);
}

/*
 * SET_DEFAULT determines the value of the appropriate descriptor
 * and makes it the new default in the Initialization table.
 */
static char* 
set_default(index)
int index;
{
	char *tmp;
	
	tmp = sing_eval(&Inits, index);
	if (strcmp(tmp, Init_single_tab[index].def) != 0)
		Init_single_tab[index].def = strsave(tmp); 
	return(Init_single_tab[index].def);
}

read_cmds(cmdfile)
char *cmdfile;
{
	struct fm_mn cmds;
	char *command, *action;
	register int i, j, numcmds;
	FILE* fp, *fopen();

	if (access(cmdfile, 04) < 0)
		return;

	/*
	 * Parse commands file
	 */
	cmds.single.attrs = NULL;
	cmds.multi = NULL;
	parser(0, cmdfile, NULL, 0, NULL, Cmd_multi_tab, CMD_MULT_KEYS,
			&cmds.multi);

	numcmds = array_len(cmds.multi);
	for (i = 0; i < numcmds; i++) {
		if ((command = multi_eval(&cmds, i, CMD_NAME)) == NULL)
			continue;
		action = get_def(&cmds, i, CMD_ACTION);
		if (action && strCcmp(action, "nop") == 0)
			del_cmd(command);	/* delete from command table */
		else
			add_cmd(command, action, get_def(&cmds, i, CMD_HELP));
	}
}

#define MAKEpair(x, y, z)	setpair(x, getcolor_id(y), getcolor_id(z))
#define COL_STRSIZE	40

/*
 * SET_DEF_COLORS initializes the color attributes 
 */
set_def_colors()
{
	static int refresh_scr = TRUE;

	if (!Color_terminal)
		return;
	MAKEpair(WINDOW_PAIR, Col_window_text, Col_screen);
	MAKEpair(ACTIVE_TITLE_PAIR, Col_active_title_bar, Col_active_title);
	MAKEpair(INACTIVE_TITLE_PAIR, Col_inactive_title_bar, Col_inactive_title);
	MAKEpair(ACTIVE_BORD_PAIR, Col_active_bord, Col_screen);
	MAKEpair(INACTIVE_BORD_PAIR, Col_inactive_bord, Col_screen);
	MAKEpair(BAR_PAIR, Col_bar, Col_bar_text);	/* reverse video */
	MAKEpair(BANNER_PAIR, Col_banner_text, Col_screen);
	MAKEpair(SLK_PAIR, Col_slk_bar, Col_slk_text);
	set_slk_color(SLK_PAIR);
	set_scr_color(WINDOW_PAIR, refresh_scr);
	refresh_scr = FALSE;	/* only refresh first time around */
}

/*
 * SET_DEF_STATUS initializes the status (banner) line
 */
set_def_status()
{
	int r, c, bancol;
	vt_id oldvid, vt_current();

	vt_ctl(STATUS_WIN, CTGETSIZ, &r, &c);
	if (Vflag) {
		Mail_col = 0;
		showmail(TRUE);
	}
	if (!Work_msg)
		Work_msg = DEF_WORKING_MSG; 
	Work_col = c - strlen(Work_msg);
	if (Banner) {
		if ((Bancolstr && *Bancolstr == '\0') ||
		    (strCcmp(Bancolstr, "center") == 0))
			bancol = center_it(Banner);
		else
			bancol = atoi(Bancolstr);
		if (bancol < 0 || bancol > c)
			bancol = c;
		oldvid = vt_current(STATUS_WIN);
		vt_ctl(STATUS_WIN, CTSETATTR, ATTR_NORMAL, BANNER_PAIR);
		wclrwin();
		wgo(0, bancol);
		wprintf(Banner);
		vt_current(oldvid);
	}
}

/*
 * GET_DESC_VAL scans the filename for the "target" descriptor ...
 * Though the function is generic, its primary use is the "slk_layout"
 * descriptor in the Initialization file....
 * Unfortunately this descriptor has to be evaluated BEFORE curses
 * is ever initialized, therefore, it must be accounted for before the
 * Initialization file is parsed completely.
 */ 
char *
get_desc_val(filename, descname)
char *filename;
char *descname;
{
	FILE *fp;
	int evalflags;
	char strbuf[BUFSIZ];
	static char *retstr = nil;
	char *ptr, *eval_string();

	if ((fp = fopen(filename, "r")) == NULL) {
		warn(NOPEN, filename);
		return(retstr);
	}
	while (fgets(strbuf, BUFSIZ, fp) != NULL) {
		if ((ptr = strchr(strbuf, '=')) != NULL) {
			*ptr = '\0';
			if (strCcmp(strbuf, descname) == 0) {
				evalflags = RET_STR;
				retstr = eval_string(++ptr, &evalflags);
				break;
			}
		}
	}
	return(retstr);
}

/*
 * COPYRIGHT puts up the initial text object which can be customized by
 * the application (e.g., FACE puts up a copyright notice)
 */ 
vt_id 
copyright()
{
	vt_id	vid, vt_create();
	char	*text;

	if ((Intro_text && (*Intro_text != '\0')) ||
	    (Intro_title && (*Intro_title != '\0'))) {
		if ((vid = vt_create(Intro_title, VT_CENTER | VT_NONUMBER,
			 -1, -1, Intro_rows, Intro_cols)) == FAIL)
				return((vt_id)NULL);
		text = Intro_text;
	}
	else
		return((vt_id)NULL);
	vt_current(vid);
	wputs(text);
	vt_flush();
	sleep(2);
	return(vid);
}

static int
center_it(str)
char *str;
{
	int	r, c;
	int	datecol;
	int	s;

	s = strlen(str);
	vt_ctl(STATUS_WIN, CTGETSIZ, &r, &c);
	if (s < c)
		datecol = (c - s) / 2;
	else
		datecol = 0;
	return(datecol);
}
