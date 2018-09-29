/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.oh:src/oh/if_menu.c	1.15"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "wish.h"
#include "vtdefs.h"
#include "ctl.h"
#include "menudefs.h"
#include "token.h"
#include "slk.h"
#include "actrec.h"
#include "if_menu.h"
#include "typetab.h"
#include "fm_mn_par.h"
#include "objmenu.h"
#include "var_arrays.h"
#include "terror.h"
#include	"moremacros.h"


#define MN_MENU		0
#define MN_HELP		1
#define MN_LIFE		2
#define MN_DONE		3
#define MN_INIT		4
#define MN_BEGROW	5
#define MN_BEGCOL	6
#define MN_CLOSE	7
#define MN_REREAD	8
#define MN_MULTI	9
#define MN_MSELECT	10	
#define MN_ALTSLKS	11	

#define MN_NAME		0
#define MN_DESC		1
#define MN_ACTI		2
#define MN_BUTT		3
#define MN_LININFO	4
#define MN_SHOW		5
#define MN_ARG		6
#define MN_SELECTED	7
#define MN_ITEMMSG	8

#define MN_KEYS 12
static struct attribute Mn_tab[MN_KEYS] = {
	{ "menu",	RET_STR|EVAL_ONCE,	"Menu", NULL, 0 },
	{ "help",	RET_ARGS|EVAL_ONCE,	"", NULL, 0 },
	{ "lifetime",	RET_STR|EVAL_ONCE,	"longterm", NULL, 0 },
	{ "done",	RET_ARGS|EVAL_ALWAYS,	"", NULL, 0 },
	{ "init",	RET_BOOL|EVAL_ALWAYS,	"", NULL, 0 },
	{ "begrow",	RET_STR|EVAL_ONCE,	"any", NULL, 0 },
	{ "begcol",	RET_STR|EVAL_ONCE,	"any", NULL, 0 },
	{ "close",	RET_BOOL|EVAL_ONCE,	NULL, NULL, 0 },
	{ "reread",	RET_BOOL|EVAL_ALWAYS,	NULL, NULL, 0 },
	{ "multicolumn",RET_BOOL|EVAL_ONCE,	"", NULL, 0 },
	{ "multiselect",RET_BOOL|EVAL_ONCE,	NULL, NULL, 0 },
	{ "altslks",	RET_BOOL|EVAL_ONCE,	NULL, NULL, 0 }
};

#define MN_FLD_KEYS 9
static struct attribute Mn_fld_tab[MN_FLD_KEYS] = {
	{ "name",	RET_STR|EVAL_ONCE,	NULL, NULL, 0 },
	{ "description",RET_STR|EVAL_ONCE,	NULL, NULL, 0 },
	{ "action",	RET_ARGS|EVAL_ALWAYS,	"", NULL, 0 },
	{ "button",	RET_INT|EVAL_ONCE,	"0", NULL, 0 },
	{ "lininfo",	RET_STR|EVAL_SOMETIMES,	NULL, NULL, 0 },
	{ "show",	RET_BOOL|EVAL_SOMETIMES,"", NULL, 0 },
	{ "arg",	RET_STR|EVAL_SOMETIMES,	NULL, NULL, 0 },
	{ "selected",   MAKE_COPY|RET_BOOL|EVAL_SOMETIMES, NULL, NULL, 0 },
	{ "itemmsg",    RET_STR|EVAL_SOMETIMES, NULL, NULL, 0 }
};

#define MAX_FOLTITLE (45)
#define CURmenu() (&(((menuinfo *) Cur_rec->odptr)->fm_mn))
#define CURmenuinfo() ((menuinfo *) Cur_rec->odptr)
#define DEVirt(X) (((menuinfo *) Cur_rec->odptr)->visible[X])
#define ARGS() (((menuinfo *) Cur_rec->odptr)->args)
static struct menu_line objmenu_disp();
static struct actrec *Cur_rec;
struct fm_mn parse_menu();

static token if_omsh();

/*
** Calls setaction and returns the token.
*/
static token
objmenu_help(rec)
struct actrec *rec;
{
	token make_action();
	int line;
	char *hold;

	Cur_rec = rec;
	return(setaction(sing_eval(CURmenu(), MN_HELP)));
}

/*
** Calls close function and frees the structures.
*/
static int 
objmenu_close(a) 
struct actrec *a; 
{ 
	int lcv, i;
	char *p, *strchr();

	Cur_rec = a;
	sing_eval(CURmenu(), MN_CLOSE);		/* execute the close function */

	/*
	 * Free infomation IN the menuinfo structure
	 */
	freeitup(CURmenu());				/* the menu structure */
	lcv = array_len(ARGS());			
	for (i = 0; i < lcv; i++) {			/* object variables */
		char namebuf[BUFSIZ];

		if (p = strchr(ARGS()[i], '='))
			*p = '\0';
		strcpy(namebuf, p);
		if (p)
			*p = '=';
		delaltenv(&ARGS(), namebuf);
		delAltenv(namebuf);
	}
	array_destroy(ARGS());				/* variable table */
	array_destroy(((menuinfo *)a->odptr)->slks);	/* visible slks */
	array_destroy(((menuinfo *)a->odptr)->visible);	/* visible items */

	/* 
	 * Free information in the activation record
	 */
	free(a->odptr); 		/* free the menuinfo structure */
	free(a->slks);			/* free the object specific SLKS */
	free(a->path);			/* free path of the definition file */

	return(menu_close(a->id));	/* close the menu */
}

/*
 * Rereads if reread is set
 */
static int
objmenu_reinit(a)
struct actrec *a;
{
	Cur_rec = a;
	if (sing_eval(CURmenu(), MN_REREAD))
		return(objmenu_reread(a));
	return(SUCCESS);
}

/*
** Front-end to parser(), it sets up defaults.
*/
static struct fm_mn
parse_menu(flags, info_or_file)
int flags;
char *info_or_file;
{
	struct fm_mn fm_mn;
	int i, j;

	fm_mn.single.attrs = NULL;
	fm_mn.multi = NULL;
	filldef(&fm_mn.single, Mn_tab, MN_KEYS);
	parser(flags, info_or_file, Mn_tab, MN_KEYS, &fm_mn.single, Mn_fld_tab, MN_FLD_KEYS, &fm_mn.multi);
	return(fm_mn);
}

/*
** If a->id >= 0 this is a reread.  If so it frees the old info.
** Either way it calls the parser.
*/
static int
objmenu_reread(a)
register struct actrec *a;
{
	struct fm_mn *fm_mn;
	extern struct slk Defslk[MAX_SLK + 1];
	extern struct slk Menuslk[];
	int freeit;
	register int i, but;
	char buf[10];
	char *str, *label;
	menuinfo *mi;
	int	lcv;

	Cur_rec = a;
	fm_mn = CURmenu();
	if (a->id >= 0)
		freeitup(CURmenu());
	mi = CURmenuinfo();
	*fm_mn = parse_menu(mi->flags, a->path);
	if (fm_mn->single.attrs == NULL) {
		if (a->id >= 0)
			ar_close(a);
		return(FAIL);
	}
	fm_mn->seqno = 1;
	mi->visible = NULL;
	mn_vislist(mi);
	strcpy(buf, "NR=");
	strcat(buf, itoa(array_len(mi->visible), 10));
	putaltenv(&ARGS(), buf);
	putAltenv(buf);
	if (!sing_eval(CURmenu(), MN_INIT) || array_len(mi->visible) <= 0) {
		if (a->id >= 0)
			ar_close(a);
		else {
			sing_eval(CURmenu(), MN_CLOSE);
			freeitup(CURmenu());
		}
		return(FAIL);
	}
	set_top_slks(Menuslk);
	if (sing_eval(CURmenu(), MN_MSELECT))
		set_slk_mark(TRUE);
	else
		set_slk_mark(FALSE);
	memcpy((char *)a->slks, (char *)Defslk, sizeof(Defslk));
	lcv = array_len(mi->slks);
	for (i = 0; i < lcv; i++) {
		but = atoi(multi_eval(fm_mn, mi->slks[i], MN_BUTT)) - 1;
		label = multi_eval(fm_mn, mi->slks[i], MN_NAME);
		set_obj_slk(&(a->slks[but]), label, TOK_SLK1 + but);
	}
	if (a->id >= 0) {
		a->id = menu_reinit(a->id, 0, objmenu_disp, a->odptr);
	}
	return(SUCCESS);
}

/*
** Takes this object's information out of the altenv.
*/
static int 
objmenu_noncur(a) 
struct actrec *a; 
{
	register int i;
	register char *p;
	int	lcv;

	Cur_rec = a;
	lcv = array_len(ARGS());
	for (i = 0; i < lcv; i++) {
		char namebuf[BUFSIZ];

		if (p = strchr(ARGS()[i], '='))
			*p = '\0';
		strcpy(namebuf, ARGS()[i]);
		if (p)
			*p = '=';
		delAltenv(namebuf);
	}
	return(menu_noncurrent(a->id));
}

/*
** Moves information in this object's altenv to the major altenv.
*/
static int 
objmenu_current(a) 
struct actrec *a; 
{
	int *Olist, *list;
	int line;
	char *str;

	Cur_rec = a;
	copyAltenv(ARGS());
	menu_ctl(Cur_rec->id, CTGETPOS, &line);
	if ((str = multi_eval(CURmenu(), DEVirt(line), MN_ITEMMSG)) && *str)
		mess_temp(str);
	return(menu_current(a->id));
}

/*
** Calculates the show functions to decide which menu lines and SLKs
** should be shown.
*/
mn_vislist(mi)
menuinfo *mi;
{
	int i;
	struct fm_mn *ptr;
	int	lcv;
	
	ptr = &(mi->fm_mn);
	if (!mi->visible) {
		mi->slks = (int *) array_create(sizeof(int), array_len(ptr->multi));
		mi->visible = (int *) array_create(sizeof(int), array_len(ptr->multi));
	}
	else {
		array_trunc(mi->slks);
		array_trunc(mi->visible);
	}

	lcv = array_len(ptr->multi);
	for (i = 0; i < lcv; i++)
		if (multi_eval(ptr, i, MN_SHOW)) {
			if (atoi(multi_eval(ptr, i, MN_BUTT)))
				mi->slks = (int *) array_append(mi->slks, (char *) &i);
			else
				mi->visible = (int *) array_append(mi->visible, (char *) &i);
		}
}

/*
 * TOGGLE MARK will toggle the "mark" flag for a given menu
 * item indexed by i (Multiple Selection Menus).
 */
toggle_mark(ptr, i)
struct fm_mn *ptr;
int i;
{
	struct attribute *att;

	att = (ptr->multi + i)->attrs[MN_SELECTED];
	att->flags ^= MENU_MARKED;	/* toggle flag */
}

/*
 * ISMARKED will check for the "mark" flag
 */
ismarked(ptr, i)
struct fm_mn *ptr;
int i;
{
	struct attribute *att;

	att = (ptr->multi + i)->attrs[MN_SELECTED];
	if (!att)
		return(0);
	if (sing_eval(ptr, MN_MSELECT) && !(att->flags & MENU_CHECKED) &&
			multi_eval(ptr, i, MN_SELECTED)) {
		toggle_mark(ptr, i);
		att->flags |= MENU_CHECKED;
	}
	return (att->flags & MENU_MARKED);
}

/*
** Calculates NAME, FLAGS, and DESCRIPTION
*/
static struct menu_line
objmenu_disp(n, mi)
int n;
menuinfo *mi;
{
	register int i;
	struct ott_entry *entry;
	struct fm_mn *ptr;
	struct menu_line m;

	if (n >= array_len(mi->visible))
		m.highlight = m.description = NULL;
	else {
		i = mi->visible[n];
		ptr = &(mi->fm_mn);
		m.highlight = multi_eval(ptr, i, MN_NAME);
		m.lininfo = multi_eval(ptr, i, MN_LININFO);
		if (ismarked(ptr, i))
			m.flags = MENU_MRK;
		else
			m.flags = 0;
		m.description = multi_eval(ptr, i, MN_DESC);
	}
	return(m);
}

/*
** Shrinks the title.
*/
static char *
objmenu_title(s)
char *s;
{
	static char title[MAX_FOLTITLE];

	strncpy(title, s, MAX_FOLTITLE-1);
	title[MAX_FOLTITLE-1] = '\0';
	return(title);
}

/*
** If getarg and arg is set return the arg.  Otherwise, if getarg
** FAIL.  Otherwise, pass it on.
*/
int
objmenu_ctl(rec, cmd, arg1, arg2, arg3, arg4, arg5, arg6)
struct actrec *rec;
int cmd;
int arg1, arg2, arg3, arg4, arg5, arg6;
{
	int pos;

	if (cmd == CTGETARG) {
		char *str;

		Cur_rec = rec;
		if (menu_ctl(rec->id, CTGETPOS, &pos) == FAIL)
			return(FAIL);
		pos = DEVirt(pos);
		if (str = multi_eval(CURmenu(), pos, MN_ARG)) {
			**((char ***)(&arg1)) = strsave(str);
			return(SUCCESS);
		}
		return(FAIL);
	}
	return(menu_ctl(rec->id, cmd, arg1, arg2, arg3, arg4, arg5, arg6));
}

/*
** Uses path_to_ar() and nextpath_to_ar() to see if it is a reopen
** and if so, makes it current.  Otherwise, it builds the actrec and
** calls ar_create().
*/
int
IF_omopen(args)
register char **args;
{
	register int i;
	register int but;
	int inline;
	struct actrec a, *first_rec, *ar_create(), *path_to_ar(), *nextpath_to_ar();
	struct fm_mn *fm_mn;
	extern struct slk Defslk[MAX_SLK + 1];
	menuinfo *mi;
	char *life;
	char buf[BUFSIZ], envbuf[6];
	char *begcol, *begrow;
	int startrow, startcol, type;
	char *ptr;

	if (strCcmp(args[0], "-i") == 0)
		inline = TRUE;
	else
		inline = FALSE;
	for (first_rec = Cur_rec = path_to_ar(args[inline ? 1 : 0]); Cur_rec; ) {
		char *env, *getaltenv();

		strcpy(envbuf, "ARG1");
		for (i = inline ? 2 : 1; (env = getaltenv(ARGS(), envbuf)) && args[i]; envbuf[3]++, i++)
			if (strcmp(args[i], env))
				break;
		if (!args[i] && !env) {
			ar_current(Cur_rec);
			return(SUCCESS);
		}
		Cur_rec = nextpath_to_ar(Cur_rec);
		if (Cur_rec == first_rec)	/* circular list */
			break;
	}
	mi = new(menuinfo);
	mi->flags = inline ? INLINE : 0;
	mi->args = NULL;
	a.odptr = (char *) mi;
	fm_mn = &(mi->fm_mn);
	fm_mn->single.attrs = NULL;
	a.path = strsave(args[inline ? 1 : 0]);
	if ((a.slks = (struct slk *) malloc(sizeof(Defslk))) == NULL)
		fatal(NOMEM, nil);
	a.id = -1;
	Cur_rec = &a;
	setupenv(mi->flags, args, &ARGS());
	if (objmenu_reread(&a) == FAIL)
		return(FAIL);
	begrow = sing_eval(fm_mn, MN_BEGROW);
	begcol = sing_eval(fm_mn, MN_BEGCOL);
	life = sing_eval(CURmenu(), MN_LIFE);
	life_and_pos(&a, life, begrow, begcol, &startrow, &startcol, &type);
	a.fcntbl[AR_CLOSE] = objmenu_close;
	a.fcntbl[AR_HELP] = objmenu_help;
	a.fcntbl[AR_REREAD] = objmenu_reread;
	a.fcntbl[AR_REINIT] = objmenu_reinit;
	a.fcntbl[AR_CURRENT] = objmenu_current;
	a.fcntbl[AR_NONCUR] = objmenu_noncur;
	a.fcntbl[AR_ODSH] = if_omsh;
	a.fcntbl[AR_CTL] = objmenu_ctl;

	a.id = menu_default(objmenu_title(sing_eval(fm_mn, MN_MENU)), type, startrow, startcol, objmenu_disp, (char *)mi);
	if (a.id == FAIL)
		return(FAIL);
	if (sing_eval(fm_mn, MN_ALTSLKS))
		a.flags = AR_ALTSLKS;
	else
		a.flags = 0;
	return(ar_current(Cur_rec = ar_create(&a)));
}

/*
** Takes a line number and calls setaction on its ACTION.
*/
token
linaction(n)
int n;
{
	return(setaction(multi_eval(CURmenu(), n, MN_ACTI)));
}

/*
** Catches SLKs and processes them.
*/
token
objmenu_stream(tok)
register token tok;
{
	int line; 
	char *tmp, *str;
	char *path, *ott_to_path();
	int *slks;
	extern int	Arg_count;
	int	lcv;

	if (tok == TOK_NEXT) {
		/* kludge for per-item message (menu_stream passes TOK_NEXT) */
		menu_ctl(Cur_rec->id, CTGETPOS, &line);
		if ((str = multi_eval(CURmenu(), DEVirt(line), MN_ITEMMSG)) && *str)
			mess_temp(str);
		return(TOK_NOP);
	}
	menu_ctl(Cur_rec->id, CTGETPOS, &line);
	tmp = multi_eval(CURmenu(), DEVirt(line), MN_LININFO);
	if (strlen(tmp)) {
		char buf[BUFSIZ];
		
		sprintf(buf, "LININFO=%s", tmp);
		putAltenv(buf);
	}
	else 
		delAltenv("LININFO");
	
	if (tok >= TOK_SLK1 && tok <= TOK_SLK16) {
		int num;
		int i;

		slks = CURmenuinfo()->slks;
		num = tok - TOK_SLK1 + 1;
		lcv = array_len(slks);
		for (i = 0; i < lcv; i++) {
			if (atoi(multi_eval(CURmenu(), slks[i],MN_BUTT)) == num)
				tok = linaction(slks[i]);
		}
	}
	if (tok == TOK_MARK && sing_eval(CURmenu(), MN_MSELECT)) {
		/*
		 * If this is a multiple selection menu then 
		 * mark the item (and update the menu)
		 */
		toggle_mark(CURmenu(), DEVirt(line));
		menu_ctl(Cur_rec->id, CTSETPOS, DEVirt(line));
		
		/*
		 * SELECTED is updated in the alternate
		 * environment in case the "action" descriptor
		 * contains a reference to $SELECTED 
		 */
		if (ismarked(CURmenu(), DEVirt(line)))
			putAltenv("SELECTED=true");
		else
			putAltenv("SELECTED=false");
		multi_eval(CURmenu(), DEVirt(line), MN_ACTI);
		tok = TOK_NOP;
	}
	else if (tok == TOK_OPEN && Arg_count < 2) {
		if (sing_eval(CURmenu(), MN_MSELECT))
			tok = setaction(sing_eval(CURmenu(), MN_DONE));
		else {
			menu_ctl(Cur_rec->id, CTGETPOS, &line);
			tok = linaction(DEVirt(line));
		}
	}
	return(tok);
}

/*
** Calls menu_stream and objmenu_stream()
*/
static token
if_omsh(a, t)
struct actrec *a;
register token t;
{
	token (*func[3])();
	extern token menu_stream();
	register int olifetime;

	Cur_rec = a;
	olifetime = Cur_rec->lifetime;
	Cur_rec->lifetime = AR_PERMANENT;
	func[0] = menu_stream;
	func[1] = objmenu_stream;
	func[2] = NULL;
	t = stream(t, func);
	Cur_rec->lifetime = olifetime;
	return(t);
}
