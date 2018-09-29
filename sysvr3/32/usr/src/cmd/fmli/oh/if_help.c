/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.oh:src/oh/if_help.c	1.22"

#include	<stdio.h>
#include	<string.h>
#include	"wish.h"
#include	"vtdefs.h"
#include	"ctl.h"
#include	"token.h"
#include	"winp.h"
#include	"form.h"
#include	"token.h"
#include	"slk.h"
#include	"actrec.h"
#include	"typetab.h"
#include	"fm_mn_par.h"
#include	"objhelp.h"
#include	"var_arrays.h"
#include	"terror.h"
#include	"moremacros.h"


#define HL_TITLE	0
#define HL_TEXT		1
#define HL_WRAP		2
#define HL_EDIT		3
#define HL_DONE		4
#define HL_INIT		5
#define HL_LIFE		6
#define HL_ROWS		7
#define HL_COLUMNS	8
#define HL_BEGROW	9
#define HL_BEGCOL	10
#define HL_HELP		11
#define HL_REREAD	12
#define HL_CLOSE	13
#define HL_ALTSLKS	14

#define HL_NAME		0
#define HL_ACTI		1
#define HL_BUTT		2
#define HL_SHOW		3

#define HL_KEYS 15
static struct attribute Hl_tab[HL_KEYS] = {
	{ "title",	RET_STR|EVAL_ONCE,	"Text", NULL, 0 },
	{ "text",	RET_STR|EVAL_ONCE,	NULL, NULL, 0 },
	{ "wrap",	RET_BOOL|EVAL_ONCE,	"", NULL, 0 },
	{ "edit",	RET_BOOL|EVAL_ONCE,	NULL, NULL, 0 },
	{ "done",	RET_BOOL|EVAL_ALWAYS,	"", NULL, 0 },
	{ "init",	RET_BOOL|EVAL_ALWAYS,	"", NULL, 0 },
	{ "lifetime",	RET_STR|EVAL_ONCE,	"longterm", NULL, 0 },
	{ "rows",	RET_INT|EVAL_ONCE,	"10", NULL, 0 },
	{ "columns",	RET_INT|EVAL_ONCE,	"30", NULL, 0 },
	{ "begrow",	RET_STR|EVAL_ONCE,	"any", NULL, 0 },
	{ "begcol",	RET_STR|EVAL_ONCE,	"any", NULL, 0 },
	{ "help",	RET_ARGS|EVAL_ONCE,	NULL, NULL, 0 },
	{ "reread",	RET_BOOL|EVAL_ALWAYS,	NULL, NULL, 0 },
	{ "close",	RET_BOOL|EVAL_ONCE,	NULL, NULL, 0 },
	{ "altslks",	RET_BOOL|EVAL_ONCE,	NULL, NULL, 0 }
};

#define HL_FLD_KEYS 4
static struct attribute Hl_fld_tab[HL_FLD_KEYS] = {
	{ "name",	RET_STR|EVAL_ONCE,	NULL, NULL, 0 },
	{ "action",	RET_ARGS|EVAL_ALWAYS	,NULL, NULL, 0 },
	{ "button",	RET_INT|EVAL_ONCE,	"0", NULL, 0 },
	{ "show",	RET_BOOL|EVAL_SOMETIMES,"", NULL, 0 }
};

#define CURhelp() (&(((helpinfo *) Cur_rec->odptr)->fm_mn))
#define CURhelpinfo() ((helpinfo *) Cur_rec->odptr)
#define ARGS() (((helpinfo *) Cur_rec->odptr)->args)
#define PTR() (((helpinfo *) Cur_rec->odptr)->holdptr)
char  *strnsave();
static struct actrec *Cur_rec;
struct fm_mn parse_help();

static token bighelp_stream();

/*
** Returns a token so that the help object can be brought up.
*/
static token 
objhelp_help(a) 
struct actrec *a; 
{ 
	return(setaction(sing_eval(CURhelp(), HL_HELP)));
}

/*
** Frees up the structures and calls the close function.
*/
static int 
objhelp_close(a) 
struct actrec *a; 
{ 
	register int i, lcv;
	char *p, *strchr();

	Cur_rec = a;
	form_close(a->id);		/* free the form FIRST */
	sing_eval(CURhelp(), HL_CLOSE);

	/*
	 * free information IN the helpinfo structure
	 */
	freeitup(CURhelp());		/* the text parse table */ 
	lcv = array_len(ARGS());	/* the object specific variable */
	for (i = 0; i < lcv; i++) {	/* (e.g., $TEXT) */
		char namebuf[BUFSIZ];

		if (p = strchr(ARGS()[i], '='))
			*p = '\0';
		strncpy(namebuf, ARGS()[i], BUFSIZ);
		namebuf[BUFSIZ-1] = '\0';
		if (p)
			*p = '=';
		delaltenv(&ARGS(), namebuf);
		delAltenv(namebuf);
	}
	array_destroy(ARGS());		/* the object variable array */

	/*
	 * Free information in the activation record structure
	 */
	free(a->odptr);			/* the helpinfo structure itself */
	free(a->slks);			/* the object specific SLKS */
	free(a->path);			/* the definition file path */

	return(SUCCESS);
}

/*
** Checks to see whether to reread and if so, calls reread.
*/
static int
objhelp_reinit(a)
struct actrec *a;
{
	Cur_rec = a;
	if (sing_eval(CURhelp(), HL_REREAD))
		return(objhelp_reread(a));
	return(SUCCESS);
}

/*
** Front-end to parser(), which sets up defaults.
*/
static struct fm_mn
parse_help(flags, info_or_file)
int flags;
char *info_or_file;
{
	struct fm_mn fm_mn;
	int i, j;

	fm_mn.single.attrs = NULL;
	fm_mn.multi = NULL;
	filldef(&fm_mn.single, Hl_tab, HL_KEYS);
	parser(flags, info_or_file, Hl_tab, HL_KEYS, &fm_mn.single, Hl_fld_tab, HL_FLD_KEYS, &fm_mn.multi);
	return(fm_mn);
}

/*
** Frees contents of old help, and sets new one.  Note:  odptr
** is set either way since freeitup will not free anything if
** the single array is empty
*/
static int
objhelp_reread(a)
register struct actrec *a;
{
	extern struct slk Defslk[MAX_SLK + 1];
	extern struct slk Textslk[];
	register int i;
	register struct fm_mn *fm_mn;
	register helpinfo *hi;
	char *label;
	int lcv, but;

	Cur_rec = a;
	fm_mn = CURhelp();
	hi = CURhelpinfo();
	if (a->id >= 0)
		freeitup(fm_mn);
	*fm_mn = parse_help(hi->flags, a->path);
	if (fm_mn->single.attrs == NULL) {
		/*
		 * very strange indeed ...
		 *
		if (a->id < 0)
			sing_eval(fm_mn, HL_CLOSE);
		*/
		return(FAIL);
	}
	PTR() = NULL;
	fm_mn->seqno = 1;
	hl_vislist(hi);

	/*
	 * If "init=false" then clean-up
	 */
	if (!sing_eval(CURhelp(), HL_INIT)) {
		if (a->id >= 0)
			ar_close(a);
		else {
			sing_eval(CURhelp(),HL_CLOSE);
			freeitup(CURhelp());
		}
		return(FAIL);
	}

	/*
	 * Set up object's SLK array
	 */
	set_top_slks(Textslk);
	memcpy((char *)a->slks, (char *)Defslk, sizeof(Defslk));
	lcv = array_len(hi->slks);
	for (i = 0; i < lcv; i++) {
		but = atoi(multi_eval(fm_mn, hi->slks[i], HL_BUTT)) - 1;
		label = multi_eval(fm_mn, hi->slks[i], HL_NAME);
		set_obj_slk(&(a->slks[but]), label, TOK_SLK1 + but);
	}
	if (a->id >= 0)
		form_ctl(a->id, CTSETDIRTY);
	return(SUCCESS);
}

/*
** Takes this object's information out of the major altenv.
*/
static int 
objhelp_noncur(a) 
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
	return(form_noncurrent(a->id));
}

/*
** Puts this object's altenv() into the major altenv().
*/
static int 
objhelp_current(a) 
struct actrec *a; 
{
	int *Olist, *list;
	int ret;

	Cur_rec = a;
	copyAltenv(ARGS());
	ret = form_current(a->id);
	form_ctl(a->id, CTSETPOS, 0, 0, 0);
	return(ret);
}

/*
** Sets up SLK array, based on show functions.
*/
hl_vislist(hi)
helpinfo *hi;
{
	int i;
	struct fm_mn *ptr;
	int	lcv;
	
	ptr = &(hi->fm_mn);
	if (!hi->slks)
		hi->slks = (int *) array_create(sizeof(int), array_len(ptr->multi));
	else
		array_trunc(hi->slks);

	lcv = array_len(ptr->multi);
	for (i = 0; i < lcv; i++)
		if (multi_eval(ptr, i, HL_SHOW))
			hi->slks = (int *) array_append(hi->slks, (char *) &i);
}

/*
** Gives text as only field, a field that has no name.
*/
static formfield
objhelp_disp(n, hi)
int n;
helpinfo *hi;
{
	register int i;
	struct ott_entry *entry;
	struct fm_mn *ptr;
	char *readfile();
	formfield m;

	ptr = &(hi->fm_mn);
	if (n > 0)
		m.name = NULL;
	else {
		m.name = nil;
		m.value = sing_eval(ptr, HL_TEXT);
		if (!m.value)
			m.value = strsave("There is no text available");
		m.frow = 0;
		m.fcol = 0;
		m.nrow = VT_UNDEFINED;
		m.ncol = VT_UNDEFINED;
		m.rows = atoi(sing_eval(CURhelp(), HL_ROWS));
		m.cols = atoi(sing_eval(CURhelp(), HL_COLUMNS));
		m.flags = I_FANCY|I_SCROLL|I_FULLWIN;
		if (!sing_eval(CURhelp(), HL_EDIT))
			m.flags |= I_NOEDIT;
		if (sing_eval(CURhelp(), HL_WRAP))
			m.flags |= I_WRAP;
		m.ptr = &PTR();
	}
	return(m);
}

/*
** There are no args, so return FAIL. Otherwise, pass it on.
*/
int
objhelp_ctl(rec, cmd, arg1, arg2, arg3, arg4, arg5, arg6)
struct actrec *rec;
int cmd;
int arg1, arg2, arg3, arg4, arg5, arg6;
{
	if (cmd == CTGETARG)
		return(FAIL);
	return(form_ctl(rec->id, cmd, arg1, arg2, arg3, arg4, arg5, arg6));
}

/*
** Uses path_to_ar and nextpath_to_ar to see if it is a reopen.  If
** so, make it current.  Otherwise, set up the actrec and call 
** ar_create.
*/
int
IF_helpopen(args)
register char **args;
{
	register int i;
	int type, startrow, startcol;
	char *begrow, *begcol;
	struct actrec a, *first_rec, *ar_create(), *path_to_ar(), *nextpath_to_ar();
	int inline;
	struct fm_mn *fm_mn;
	extern struct slk Defslk[MAX_SLK + 1];
	helpinfo *hi;
	char *life;
	char buf[BUFSIZ], envbuf[6];
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
	hi = new(helpinfo);
	hi->flags = inline ? INLINE : 0;
	hi->args = NULL;
	a.id = -1;
	a.odptr = (char *) hi;
	fm_mn = &(hi->fm_mn);
	fm_mn->single.attrs = NULL;
	a.path = strsave(args[hi->flags & INLINE ? 1 : 0]);
	if ((a.slks = (struct slk *) malloc(sizeof(Defslk))) == NULL)
		fatal(NOMEM, nil);
	Cur_rec = &a;
	setupenv(hi->flags, args, &ARGS());
	if (objhelp_reread(&a) == FAIL)
		return(FAIL);
	ptr = strnsave("TEXT=", strlen(life = sing_eval(fm_mn, HL_TEXT)) + 6);
	strcat(ptr, life);
	putaltenv(&ARGS(), ptr);
	putAltenv(ptr);
	free(ptr);
	begrow = sing_eval(fm_mn, HL_BEGROW);
	begcol = sing_eval(fm_mn, HL_BEGCOL);
	life = sing_eval(fm_mn, HL_LIFE);
	life_and_pos(&a, life, begrow, begcol, &startrow, &startcol, &type);
	a.fcntbl[AR_CLOSE] = objhelp_close;
	a.fcntbl[AR_REREAD] = objhelp_reread;
	a.fcntbl[AR_REINIT] = objhelp_reinit;
	a.fcntbl[AR_CURRENT] = objhelp_current;
	a.fcntbl[AR_NONCUR] = objhelp_noncur;
	a.fcntbl[AR_ODSH] = bighelp_stream;
	a.fcntbl[AR_HELP] = objhelp_help;
	a.fcntbl[AR_CTL] = objhelp_ctl;

	strcpy(buf, filename(sing_eval(fm_mn, HL_TITLE)));
	a.id = form_default(buf, type, startrow, startcol, objhelp_disp, (char *)hi);
	if (a.id == FAIL)
		return(FAIL);

	if (sing_eval(fm_mn, HL_ALTSLKS))
		a.flags = AR_ALTSLKS;
	else
		a.flags = 0;
	return(ar_current(Cur_rec = ar_create(&a)));
}

/*
** Intercepts SLKs after the editor. Also, TOK_SAVE is an exit.
*/
token
help_stream(tok)
register token tok;
{
	int line; 
	char *lininfo;
	char *buf, *s;
	int *slks;
	int	lcv;

	s = NULL;
	if (tok >= TOK_SLK1 && tok <= TOK_SLK16) {
		int num;
		int i;

		slks = CURhelpinfo()->slks;
		num = tok - TOK_SLK1 + 1;
		lcv = array_len(slks);
		for (i = 0; i < lcv; i++)
			if (atoi(multi_eval(CURhelp(), slks[i], HL_BUTT)) == num) {
				form_ctl(Cur_rec->id, CTGETARG, &s);
				if (sing_eval(CURhelp(), HL_EDIT))
					set_sing_cur(CURhelp(), HL_TEXT, strsave(s));
				buf = strnsave("TEXT=", strlen(s) + 6);
				strcat(buf, s);
				putaltenv(&ARGS(), buf);
				putAltenv(buf);
				tok = setaction(multi_eval(CURhelp(), slks[i], HL_ACTI));
				free(buf);
				break;
			}
	}
	if (tok == TOK_SAVE)
		tok = TOK_CLOSE;
	if (tok == TOK_CLOSE) {
		if (!s) {
			form_ctl(Cur_rec->id, CTGETARG, &s);
			buf = strnsave("TEXT=", strlen(s) + 6);
			strcat(buf, s);
			putaltenv(&ARGS(), buf);
			putAltenv(buf);
			free(buf);
		}
		tok = sing_eval(CURhelp(), HL_DONE) ? TOK_CLOSE : TOK_BADCHAR;
	}
	return(tok);
}

/*
** Sets up stream and calls stream.
*/
static token
bighelp_stream(a, t)
struct actrec *a;
register token t;
{
	token (*func[3])();
	extern int field_stream();
	register int olifetime;

	Cur_rec = a;
	olifetime = Cur_rec->lifetime;
	Cur_rec->lifetime = AR_PERMANENT;
	func[0] = field_stream;
	func[1] = help_stream;
	func[2] = NULL;
	t = stream(t, func);
	Cur_rec->lifetime = olifetime;
	return(t);
}
