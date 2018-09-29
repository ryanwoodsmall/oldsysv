/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.oh:src/oh/if_form.c	1.37"

#include	<ctype.h>
#include	<stdio.h>
#include	<string.h>
#include	"wish.h"
#include	"menudefs.h"
#include	"message.h"
#include	"token.h"
#include	"vtdefs.h"
#include	"ctl.h"
#include	"slk.h"
#include	"actrec.h"
#include	"typetab.h"
#include	"token.h"
#include	"winp.h"
#include	"form.h"
#include	"fm_mn_par.h"
#include	"objform.h"
#include	"eval.h"
#include	"terror.h"
#include	"var_arrays.h"
#include	"moremacros.h"

#define FM_TITLE	0
#define FM_LIFE		1
#define FM_INIT		2
#define FM_DONE		3
#define FM_BEGROW	4
#define FM_BEGCOL	5
#define FM_HELP		6
#define FM_PROMPT	7
#define FM_REREAD	8
#define FM_CLOSE	9
#define FM_ALTSLKS	10

#define FM_NAME		0
#define FM_FROW		1
#define FM_FCOL		2
#define FM_NROW		3
#define FM_NCOL		4
#define FM_ROWS		5
#define FM_COLS		6
#define FM_FLEN		7
#define FM_VALUE	8
#define FM_RMENU	9
#define FM_CHOICEMSG	10
#define FM_VALID	11
#define FM_NOECHO	12
#define FM_MENUO	13
#define FM_SHOW		14
#define FM_SCROLL	15
#define FM_WRAP		16
#define FM_PAGE		17	
#define FM_ACTI		18
#define FM_BUTT		19
#define FM_VALMSG	20
#define FM_INACTIVE	21
#define FM_FIELDMSG	22

#define FM_KEYS 11
static struct attribute Fm_tab[FM_KEYS] = {
	{ "form",	RET_STR|EVAL_ONCE,	"Form", NULL, 0 },
	{ "lifetime",	RET_STR|EVAL_ONCE,	"longterm", NULL, 0 },
	{ "init",	RET_BOOL|EVAL_ALWAYS,	"", NULL, 0 },
	{ "done",	RET_ARGS|EVAL_ALWAYS,	"", NULL, 0 },
	{ "begrow",	RET_INT|EVAL_ONCE,	"any", NULL, 0 },
	{ "begcol",	RET_INT|EVAL_ONCE,	"any", NULL, 0 },
	{ "help",	RET_ARGS|EVAL_ONCE,	"", NULL, 0 },
	{ "prompt",	RET_STR|EVAL_ONCE,	"", NULL, 0 },
	{ "reread",	RET_BOOL|EVAL_ALWAYS,	NULL, NULL, 0 },
	{ "close",	RET_BOOL|EVAL_ONCE,	NULL, NULL, 0 },
	{ "altslks",	RET_BOOL|EVAL_ONCE,	NULL, NULL, 0 }
};

#define FM_FLD_KEYS 23
static struct attribute Fm_fld_tab[FM_FLD_KEYS] = {
	{ "name",	RET_STR|EVAL_ONCE,	NULL, NULL, 0 },
	{ "frow",	RET_INT|EVAL_ONCE,	"-1", NULL, 0 },
	{ "fcol",	RET_INT|EVAL_ONCE,	"-1", NULL, 0 },
	{ "nrow",	RET_INT|EVAL_ONCE,	"-1", NULL, 0 },
	{ "ncol",	RET_INT|EVAL_ONCE,	"-1", NULL, 0 },
	{ "rows",	RET_INT|EVAL_ONCE,	"-1", NULL, 0 },
	{ "columns",	RET_INT|EVAL_ONCE,	"-1", NULL, 0 },
	{ "flen",	RET_INT|EVAL_ONCE,	NULL, NULL, 0 },
	{ "value",	MAKE_COPY|RET_STR|EVAL_SOMETIMES,"", NULL, 0 },
	{ "rmenu",	RET_LIST|EVAL_ONCE,	"", NULL, 0 },
	{ "choicemsg",	RET_STR|EVAL_ONCE,	NULL, NULL, 0 },
	{ "valid",	RET_BOOL|EVAL_SOMETIMES,"", NULL, 0 },
	{ "noecho",	RET_BOOL|EVAL_ONCE,	NULL, NULL, 0 },
	{ "menuonly",	RET_BOOL|EVAL_ONCE,	NULL, NULL, 0 },
	{ "show",	RET_BOOL|EVAL_SOMETIMES,"", NULL, 0 },
	{ "scroll",	RET_BOOL|EVAL_ONCE,	NULL, NULL, 0 },
	{ "wrap",	RET_BOOL|EVAL_ONCE,	NULL, NULL, 0 },
	{ "page", 	RET_STR|EVAL_ONCE,	"1", NULL, 0 },
	{ "action",	RET_ARGS|EVAL_ALWAYS,	"", NULL, 0 },
	{ "button",	RET_INT|EVAL_ONCE,	"0", NULL, 0 },
	{ "invalidmsg",	RET_STR|EVAL_SOMETIMES,	NULL, NULL, 0 },
	{ "inactive",	RET_BOOL|EVAL_SOMETIMES,	NULL, NULL, 0 },
	{ "fieldmsg",	RET_STR|EVAL_SOMETIMES, NULL, NULL, 0}
};

#define DEVirt(X) (((forminfo *) Cur_rec->odptr)->visible[X])
#define CURforminfo() ((forminfo *) Cur_rec->odptr)
#define CURform() (&(((forminfo *) Cur_rec->odptr)->fm_mn))
#define NUMvis() (array_len(((forminfo *) Cur_rec->odptr)->visible))
#define VALS() (((forminfo *) Cur_rec->odptr)->mulvals)
#define PTRS() (((forminfo *) Cur_rec->odptr)->holdptrs)
#define NUMflds() (array_len(((forminfo *) Cur_rec->odptr)->fm_mn.multi))
#define CURfield() (((forminfo *) Cur_rec->odptr)->curfield)
#define CURffield() (objform_disp(DEVirt(((forminfo *) Cur_rec->odptr)->curfield), CURforminfo()))
#define SET_curfield(X) (((forminfo *) Cur_rec->odptr)->curfield) = X
#define CURpage() ((((forminfo *) Cur_rec->odptr)->curpage))
#define LASTpage() ((((forminfo *) Cur_rec->odptr)->lastpage))
#define NUMSTR(X) (strcpy(Field_str + 1, itoa(X, 10)), Field_str)

#define NX_ANY		0	/* pseudo flag */
#define NX_NOCUR	1
#define NX_ADVANCE	2

char *expand();
char *getaltenv();
struct actrec *ar_create(), *ar_current();

/* globals */
int Noreshape = 0;

static void chg_curfield();
static int objform_close();
static int objform_reinit();
static int objform_reread();
static int objform_current();
static int objform_noncur();
static int objform_ctl();
static int objform_stream();
struct fm_mn parse_form();

static struct actrec *Cur_rec;
static char *Equal = "=";
static char *Field_str = "F000";

#define QUERY -1
#define MAX_TITLE 45
#define TOOBIG 1000
#define TOOSMALL -1000

/*
 * Returns the virtual index of the given actual field number ........
 *
 * The ACTUAL number is the order that the field appears in the form's
 * definition file.
 *
 * The VIRTUAL number is the index into the VISIBLE array of fields
 * (i.e., the subset of ACTUAL fields that contains only those fields
 * that appear on the "current page" or whose "show" descriptor = "true").
 */
virt(i)
register int i;
{
	register int j;
	int	lcv;

	lcv = NUMvis();
	for (j = 0; j < lcv; j++)
		if (DEVirt(j) == i)
			return(j);
}

/*
** Starting with start, recalculate the values until they are
** all set.  This works by faking out each value into being
** a control sequence and then substituting the real value for
** the control sequence
*/
redo_vals(start)
int start;
{
	int changed;
	register char *envbuf;
	register int i;
	char *hold1, *hold2;
	char buf[BUFSIZ];
	int	lcv;

	upseqno(CURform());

	envbuf = buf;
	lcv = NUMflds();
	for (i = start; i < lcv; i++) {
		strcpy(envbuf, NUMSTR(i + 1));
		strcat(envbuf, "=\001");
		strcat(envbuf, NUMSTR(i + 1));
		putAltenv(envbuf);
	}

	changed = FALSE;
	lcv = NUMflds();
	for (i = start; i < lcv; i++) {
		int dofree, maxamt, amt;

		dofree = 0;
		hold1 = multi_eval(CURform(), i, FM_VALUE);
		maxamt = BUFSIZ - strlen(NUMSTR(i + 1)) - 2;
		if ((amt = strlen(hold1)) > maxamt) { 
			/*
			 * Value is greater than 1K so malloc 
			 * enough space to hold it. 
			 */
			maxamt = amt + strlen(NUMSTR(i + 1)) + 2;
			if ((envbuf = (char *) malloc(maxamt)) == NULL)
				fatal(NOMEM, nil); 
			dofree++;
		}
		else {
			/*
			 * ... otherwise, use static 1K buffer
			 */
			envbuf = buf;
			dofree = 0;
		}
		strcpy(envbuf, NUMSTR(i + 1));
		changed |= strcmp(hold1, (hold2 = getaltenv(VALS(), envbuf)) ? hold2 : nil) == 0;
		strcat(envbuf, Equal);
		strncat(envbuf, hold1, maxamt); 
		putAltenv(envbuf);
		putaltenv(&VALS(), envbuf);
		if (dofree)	/* if buffer was malloc'd, free it */
			free(envbuf);
	}
	while (changed) {
		changed = FALSE;
		lcv = NUMflds();
		for (i = start; i < lcv; i++) {
			register char *p;
			int dofree, amt, maxamt;

			dofree = 0;
			envbuf = buf;
			strcpy(envbuf, NUMSTR(i + 1));
			hold1 = getaltenv(VALS(), envbuf);
			for (p = NULL; p = strchr(hold1, '\001'); )
				*p = '$';
			if (!p)
				continue;
			hold2 = expand(hold1);
			maxamt = BUFSIZ - strlen(NUMSTR(i + 1)) - 2;
			if ((amt = strlen(hold2)) > maxamt) { 
				/*
				 * Value is greater than 1K so malloc 
				 * enough space to hold it. 
				 */
				maxamt = amt + strlen(NUMSTR(i + 1)) + 2;
				if ((envbuf = (char *) malloc(maxamt)) == NULL)
					fatal(NOMEM, nil); 
				strcpy(envbuf, NUMSTR(i + 1));
				dofree++;
			}
			else {
				/*
				 * ... otherwise, use static 1K buffer
				 */
				dofree = 0;
			}
			strcat(envbuf, Equal);
			strncat(envbuf, hold2, maxamt); 
			changed = TRUE;
			free(hold2);
			putaltenv(&VALS(), envbuf);
			putAltenv(envbuf);
			if (dofree)
				free(envbuf);
		}
	}
}

/*
** Returns the current value of the field, fieldno.
*/
char *
curval(fieldno)
int fieldno;
{
	return(getaltenv(VALS(), NUMSTR(fieldno + 1)));
}

/*
** Figure out which fields are on the screen as decided by the show
** function values.
*/
fm_vislist(ptr)
forminfo *ptr;
{
	int i, num;
	struct fm_mn *fm_mn;
	char *page;
	int	lcv;

	fm_mn = &(ptr->fm_mn);
	if (!ptr->visible)  {
		ptr->slks = (int *) array_create(sizeof(int),array_len(fm_mn->multi));
		ptr->visible = (int *) array_create(sizeof(int), NUMflds());
	}
	else  {
		array_trunc(ptr->visible);
		array_trunc(ptr->slks);
	}

	lcv = NUMflds();
	for (i = 0; i < lcv; i++)
		if (multi_eval(fm_mn, i, FM_SHOW)) {
			if (atoi(multi_eval(fm_mn, i, FM_BUTT)))
				ptr->slks = (int *) array_append(ptr->slks, (char *) &i);
			else {
				/*
				 * Only make visible fields on the CURRENT 
				 * page ...
				 */ 
				page = multi_eval(fm_mn, i, FM_PAGE);
				if (num = atoi(page)) {
					if (num == CURpage())
						ptr->visible = (int *) array_append(ptr->visible, &i);
					if (num > LASTpage())
						LASTpage() = num;
				}
				else if (page && (strcmp(page, "all") == 0) ||
						(*page == '*'))
					ptr->visible = (int *) array_append(ptr->visible, &i);
			}
		}
}

/*
** Does nothing.  Form objects cannot be reread.
*/
static int
objform_reinit(a)
struct actrec *a;
{
	Cur_rec = a;
	if (sing_eval(CURform(), FM_REREAD))
		return(objform_reread(a));
	return(SUCCESS);
}

/*
** A front end to parser() which will set up most of the defaults for
** a form.
*/
static struct fm_mn
parse_form(flags, info_or_file)
int flags;
char *info_or_file;
{
	struct fm_mn fm_mn;
	int i, j;

	fm_mn.single.attrs = NULL;
	fm_mn.multi = NULL;
	filldef(&fm_mn.single, Fm_tab, FM_KEYS);
	parser(flags, info_or_file, Fm_tab, FM_KEYS, &fm_mn.single, Fm_fld_tab, FM_FLD_KEYS, &fm_mn.multi);
	return(fm_mn);
}

/*
** Read the form object indicated by this actrec, if a->id > 0
** then the object is being reread.
*/
static int
objform_reread(a)
struct actrec *a;
{
	extern struct slk Defslk[MAX_SLK + 1];
	extern struct slk Formslk[];
	forminfo *fi;
	register int i, but;
	char *label;
	int	lcv;

	Cur_rec = a;
	if (a->id >= 0)
		freeitup(CURform());	/* if posted then free it old one */
	fi = CURforminfo();
	*(CURform()) = parse_form(fi->flags, a->path);
	if ((CURform())->single.attrs == NULL) {
#ifdef _DEBUG4
		_debug4(stderr, "Couldn't parse it\n");
#endif
		return(FAIL);
	}
	(CURform())->seqno = 1;
	if (PTRS())
		free(PTRS());
	if (NUMflds() && (PTRS() = (char **) calloc(NUMflds(), sizeof(char *))) == NULL)
		fatal(NOMEM, nil);
	lcv = NUMflds();
	for (i = 0; i < lcv; i++)
		PTRS()[i] = (char *) NULL;

	fi->visible = NULL;		/* initialize array of visible fields */
	fi->slks = NULL;		/* initialize array of object's SLKS */
	redo_vals(0);			/* initialize field values */
	fm_vislist(CURforminfo());	/* set up visible field list */
	if (a->id < 0)
		SET_curfield(-1);

	/*
 	 * If "init=false" or Form is empty then cleanup
	 */
	if (!sing_eval(CURform(), FM_INIT) || (NUMvis() <= 0)) {
		if (a->id >= 0)
			ar_close(a);	/* form is already posted */
		else {
			sing_eval(CURform(), FM_CLOSE);
			freeitup(CURform());
		}
		return(FAIL);
	}

	/*
    	 * Set up object specific SLK array
 	 */
	set_top_slks(Formslk);
	memcpy((char *)a->slks, (char *)Defslk, sizeof(Defslk));
	lcv = array_len(fi->slks);
	for (i = 0; i < lcv; i++) {
		but = atoi(multi_eval(CURform(), fi->slks[i], FM_BUTT)) - 1;
		label = multi_eval(CURform(), fi->slks[i], FM_NAME);
		set_obj_slk(&(a->slks[but]), label, TOK_SLK1 + but);
	}

	if (a->id >= 0)
		form_ctl(a->id, CTSETDIRTY);
	return(SUCCESS);
}

/*
** Frees up the structures and evaluates the "close" function.
*/
static int 
objform_close(a) 
struct actrec *a; 
{ 
	register int i, lcv;
	char *p, *strchr();

	Cur_rec = a;
	form_close(a->id);			/* remove the form FIRST */ 
	sing_eval(CURform(), FM_CLOSE);		/* evaluate close function */

	/*
	 * Free information IN the forminfo structure
	 */
	freeitup(CURform());			/* the form parse table */
	if (PTRS())				/* holdptrs array */
		free(PTRS());
	lcv = array_len(VALS());
	for (i = 0; i < lcv; i++) {		/* form specific variables */
		char namebuf[BUFSIZ];		/* (e.g., F1, F2, .... ) */

		if (p = strchr(VALS()[i], '='))
			*p = '\0';
		strcpy(namebuf, VALS()[i]);
		if (p)
			*p = '=';
		delaltenv(&VALS(), namebuf);
		delAltenv(namebuf);
	}
	array_destroy(VALS());				/* variables array */
	array_destroy(((forminfo *)a->odptr)->slks);	/* visible SLKS */
	array_destroy(((forminfo *)a->odptr)->visible); /* visible fields */

	/*
	 * Free information in the activation record
	 */
	free(a->odptr);				/* the forminfo structure */
	free(a->slks);				/* object specific SLKS */
	free(a->path);				/* form definition file */

	return(SUCCESS);
}

/*
** Takes this objects's information out of the major altenv.
*/
static int 
objform_noncur(a) 
struct actrec *a; 
{
	register int i;
	register char *p;
	int	lcv;

	Cur_rec = a;
	lcv = array_len(VALS());
	for (i = 0; i < lcv; i++) {
		char namebuf[BUFSIZ];

		if (p = strchr(VALS()[i], '='))
			*p = '\0';
		strcpy(namebuf, VALS()[i]);
		if (p)
			*p = '=';
		delAltenv(namebuf);
	}
	return(form_noncurrent());
}

/*
** Sets up the major alternate environment based on the values
** for the altenv that pertains to this object.
*/
static int 
objform_current(a) 
struct actrec *a; 
{
	int ret;
	char *choice, *s;
	static char *Form_Choice = "Form_Choice";

	Cur_rec = a;
	form_current(a->id);
	if (CURfield() == -1)
		nextfield(0, 0, 1, NX_ANY);
	if (choice = getAltenv(Form_Choice)) {
		copyAltenv(VALS());
		if (set_curval(strsave(choice)) == SUCCESS) {
			fm_vislist(CURforminfo());
			redo_vals(CURfield() + 1);
			form_ctl(Cur_rec->id, CTSETDIRTY);
		}
		delAltenv(Form_Choice);
	}
	else
		copyAltenv(VALS());
	set_form_field(a->id, CURfield());
	return(SUCCESS);
}

/*
** Evaluates many of the fields to return a form structure that includes
** name, value, their positions, editing capabilities and a structure
** that is held here and is used by the low-level form code to pertain
** to each field.
*/
static formfield
objform_disp(n, fi)
int n;
forminfo *fi;
{
	register int i;
	struct ott_entry *entry;
	struct fm_mn *ptr;
	formfield m;

	ptr = &(fi->fm_mn);
	if (n >= NUMvis())
		m.name = NULL;
	else {
		i = DEVirt(n);
		m.name = multi_eval(ptr, i, FM_NAME);
		m.value = (char *) curval(i);
		m.frow = atoi(multi_eval(ptr, i, FM_FROW));
		m.fcol = atoi(multi_eval(ptr, i, FM_FCOL));
		m.nrow = atoi(multi_eval(ptr, i, FM_NROW));
		m.ncol = atoi(multi_eval(ptr, i, FM_NCOL));
		m.rows = atoi(multi_eval(ptr, i, FM_ROWS));
		m.cols = atoi(multi_eval(ptr, i, FM_COLS));
		if (multi_eval(ptr, i, FM_INACTIVE))
			m.flags = I_FANCY;
		else
			m.flags = I_BLANK|I_FANCY|I_FILL;
		m.ptr = PTRS() + i;
		if (multi_eval(ptr, i, FM_WRAP))
			m.flags |= I_WRAP;
		if (multi_eval(ptr, i, FM_SCROLL))
			m.flags |= I_SCROLL;
		if (multi_eval(ptr, i, FM_NOECHO)) {
			m.flags |= I_INVISIBLE;
			m.flags &= ~(I_BLANK | I_FILL);
		}
	}
	return(m);
}

/*
** Evaluates the help field and returns a token for it.
*/
static token
objform_help(rec)
struct actrec *rec;
{
	token make_action();
	char *hold;

	Cur_rec = rec;
	return(make_action(sing_eval(CURform(), FM_HELP)));
}

/*
** Forms have no arguments to give, so that must fail.  All else is
** passed on.
*/
static int
objform_ctl(rec, cmd, arg1, arg2, arg3, arg4, arg5, arg6)
struct actrec *rec;
int cmd;
int arg1, arg2, arg3, arg4, arg5, arg6;
{
	Cur_rec = rec;
	if (cmd == CTGETARG)
		return(FAIL);
	return(form_ctl(rec->id, cmd, arg1, arg2, arg3, arg4, arg5, arg6));
}

/*
** Calls path_to_ar() and nextpath_to_ar() to decide whether this is a
** reopen or a first open.  If it is the latter, it sets up the actrec
** and calls ar_create().
*/
int
IF_ofopen(args)
register char **args;
{
	register int i, but;
	struct actrec a, *first_rec, *path_to_ar(), *nextpath_to_ar();
	extern struct slk Defslk[MAX_SLK + 1];
	int startrow, startcol;
	int inline;
	int type;
	char *life;
	char *begcol, *begrow;
	register struct fm_mn *fm_mn;
	forminfo *fi;
	char *label;
	char buf[BUFSIZ], envbuf[6];
	int	lcv;

	if (strCcmp(args[0], "-i") == 0)
		inline = TRUE;
	else
		inline = FALSE;
	for (first_rec = Cur_rec = path_to_ar(args[inline ? 1 : 0]); Cur_rec; ) {
		char *env;

		strcpy(envbuf, "ARG1");
		for (i = inline ? 2 : 1; (env = getaltenv(VALS(), envbuf)) && args[i]; envbuf[3]++, i++)
			if (strcmp(args[i], env))
				break;
		if (!args[i] && !env) {
			ar_current(Cur_rec);
			return(SUCCESS);
		}
		Cur_rec = nextpath_to_ar(Cur_rec);
		if (Cur_rec == first_rec)
			break;
	}
	fi = new(forminfo);
	fi->flags = inline ? INLINE : 0;
	fi->mulvals = NULL;
	fm_mn = &(fi->fm_mn);
	a.odptr = (char *) fi;
	a.id = -1;
	fm_mn->single.attrs = NULL;
	a.path = strsave(args[inline ? 1 : 0]);
	if ((a.slks = (struct slk *) malloc(sizeof(Defslk))) == NULL)
		fatal(NOMEM, nil);
	Cur_rec = &a;
	CURpage() = 1;
	setupenv(fi->flags, args, &VALS());
	if (objform_reread(&a) == FAIL)
		return(NULL);
	type = 0;
	life = sing_eval(CURform(), FM_LIFE);
	begrow = sing_eval(fm_mn, FM_BEGROW);
	begcol = sing_eval(fm_mn, FM_BEGCOL);
	life_and_pos(&a, life, begrow, begcol, &startrow, &startcol, &type);
	a.fcntbl[AR_CLOSE] = objform_close;
	a.fcntbl[AR_HELP] = objform_help;
	a.fcntbl[AR_REREAD] = objform_reread;
	a.fcntbl[AR_REINIT] = objform_reinit;
	a.fcntbl[AR_CURRENT] = objform_current;
	a.fcntbl[AR_NONCUR] = objform_noncur;
	a.fcntbl[AR_CTL] = objform_ctl;
	a.fcntbl[AR_ODSH] = objform_stream;
	a.id = form_default(sing_eval(fm_mn, FM_TITLE), type, startrow, startcol, objform_disp, (char *)fi);
	if (sing_eval(fm_mn, FM_ALTSLKS))
		a.flags = AR_ALTSLKS;
	else
		a.flags = 0;
	if (a.id == FAIL)
		return(FAIL);
	return((int) ar_current(Cur_rec = (struct actrec *) ar_create(&a)));
}

/*
** Set the value of the current field.
*/
set_curval(str)
char *str;
{
	char *s;
	char *tmp=NULL;
	char buf[BUFSIZ];
	char *envbuf;

	strcpy(buf, NUMSTR(CURfield() + 1));
	form_ctl(Cur_rec->id, CTGETARG, &tmp);
	if (strcmp(str, s = getaltenv(VALS(), buf)) || strcmp(str,tmp)) {
		/*
		 * If the current value and the passed value are
		 * different then add the passed value (str) to
		 * the environment
		 */
		int dofree, maxamt, amt;

		dofree = 0;
		maxamt = BUFSIZ - strlen(NUMSTR(CURfield() + 1)) - 2;
		if ((amt = strlen(str)) > maxamt) { 
			/*
			 * Value is greater than 1K so malloc 
			 * enough space to hold it. 
			 */
			maxamt = amt + strlen(NUMSTR(CURfield() + 1)) + 2;
			if ((envbuf = (char *) malloc(maxamt)) == NULL)
				fatal(NOMEM, nil); 
			dofree++;
		}
		else {
			/*
			 * ... otherwise, use static 1K buffer
			 */
			envbuf = buf;
			dofree = 0;
		}
		set_cur(CURform(), CURfield(), FM_VALUE, strsave(str));
		strcpy(envbuf, NUMSTR(CURfield() + 1));
		strcat(envbuf, Equal);
		strncat(envbuf, str, maxamt); 
		putaltenv(&VALS(), envbuf);
		putAltenv(envbuf);
		if (dofree)
			free(envbuf);
		return(SUCCESS);
	}
	return(FAIL);
}

/*
** Given a form_field structure, it will get the value of the current
** field and see if it is A) different and B) valid.  If the value is
** different all sorts of recalculation must go on (the show function,
** all the new values).  If the value is valid, it returns SUCCESS,
** so that the function calling it can navigate or close.
*/
fld_get_ck(form_field)
formfield *form_field;
{
	char *s;
	char *str;
	register int ret;
	char buf[BUFSIZ];

	if ( form_field->flags & I_SCROLL )
		s = NULL;
	else
		s = (form_field->rows * form_field->cols >= BUFSIZ) ? NULL : buf;
	form_ctl(Cur_rec->id, CTGETARG, &s);
	if (set_curval(s) == SUCCESS) {
		redo_vals(CURfield() + 1);
		fm_vislist(CURforminfo());
		form_ctl(Cur_rec->id, CTSETDIRTY);
	}
	ret = SUCCESS;
	if (!multi_eval(CURform(), CURfield(), FM_VALID)) {
		if ((str=multi_eval(CURform(), CURfield(), FM_VALMSG)) && *str) 
			mess_temp(str);
		else
			warn(VALID, s);
		mess_lock();
		ret = FAIL;
	}
	else if (multi_eval(CURform(), CURfield(), FM_MENUO)) {
		char **list;

		if ((list = (char **) multi_eval(CURform(), CURfield(), FM_RMENU)) && *list != '\0') {
			int i, lcv;

			lcv = array_len(list) - 1;
			for (i = 1; i < lcv; i++)
				if (strcmp(s, list[i]) == 0)
					break;
			if (i == lcv) {
				if ((str = multi_eval(CURform(), CURfield(), FM_VALMSG)) && *str) 
					mess_temp(str);
				else
					warn(VALID, s);
				mess_lock();
				ret = FAIL;
			}
		}
		else {
			if ((str = multi_eval(CURform(), CURfield(), FM_VALMSG)) && *str) 
				mess_temp(str);
			else
				warn(VALID, s);
			mess_lock();
			ret = FAIL;
		}
	}
	return(ret);
}

/*
** Move to another field.
*/
static void
chg_curfield(virtnum)
int virtnum;
{
	int num;

	num = DEVirt(virtnum);
	if (CURfield() == num)
		return;
	SET_curfield(num);
	set_form_field(Cur_rec->id, num);
}

/*
** Calculates the next field to go to.  Mode is either positive or
** negative 1 for forward and backward navigation.  Canbecur
** decides whether the current field should be eliminated from the
** choices for nextfield().
*/
nextfield(currow, curcol, mode, flags)
int currow;
int curcol;
register int mode;
int flags;
{
	register int i, j, frow, fcol;
	int curi, rows, cols, num;
	int newcurrow, newcurcol;
	int leastcol, leastrow, leasti;
	struct fm_mn *curf;
	char *page, *inactive;
	int no_current, page_advance;
	int	lcv;

	no_current = page_advance = 0;
	if (flags & NX_NOCUR)
		no_current++;
	if (flags & NX_ADVANCE)
		page_advance++;

	curf = CURform();
	leastrow = mode * TOOBIG;
	leastcol = mode * TOOBIG;
	newcurrow = mode * TOOBIG;
	newcurcol = mode * TOOBIG;
	curi = -1;
	leasti = -1;
	lcv = NUMvis();
	for (i = 0; i < lcv; i++) {
		/*
		 * First eliminate fields that can be eliminated 
 		 *
		 * IF ...
		 *	1. field is inactive   OR
	         *	2. field is current and isn't eligible  OR
		 * 	3. either rows or cols <= 0	OR
		 * 	4. frow or fcol < 0
	         *
		 * THEN skip the field 
		 *
		 * NOTE: The reason that fields that do not satisfy
		 *	 (3) and (4) are visible is that one can
		 * 	 have a field name with no field associated
		 *	 with it .... (ETI does not have such a
		 *	 field/field-name association)
		 *
		 */ 
		j = DEVirt(i);
		if (multi_eval(curf, j, FM_INACTIVE) ||
                   ((j == CURfield()) && no_current))
			continue;

		rows = atoi(multi_eval(curf, j, FM_ROWS));
		cols = atoi(multi_eval(curf, j, FM_COLS));
		frow = atoi(multi_eval(curf, j, FM_FROW));
		fcol = atoi(multi_eval(curf, j, FM_FCOL));
		if (rows <= 0 || cols <= 0 || frow < 0 || fcol < 0)
			continue;

		/*
		 * Determine whether the "ith" visible field is next
		 * A few comments here would help !!!
		 */
		if ((mode * frow >= mode * currow) && (mode * frow <= mode * newcurrow)) {
			if (((mode * frow > mode * currow) || (mode * fcol >= mode * curcol)) && ((mode * frow < mode * newcurrow) || (mode * fcol < mode * newcurcol))) {
				newcurcol = fcol;
				newcurrow = frow;
				curi = i;
				continue;
			}
		}
		if ((mode * frow <= mode * leastrow)) {
			if ((mode * frow < mode * leastrow) || (mode * fcol <= mode * leastcol)) {
				leastcol = fcol;
				leastrow = frow;
				leasti = i;
			}
		}
	}
	if ((newcurrow == mode * TOOBIG) && (newcurcol == mode * TOOBIG)) {
		/*
		 * User has reached a page boundary (i.e., there is no 
		 * next/previous field on the current page)
		 */ 
		if (LASTpage() != 1 && page_advance) {
			/*
			 * If this is a multi-page form AND the page should be
			 * automatically advanced on page boundaries then ...
			 */
			if (mode < 0) {		/* prev field */
				if (CURpage() != 1)
					CURpage()--;
				else
					CURpage() = LASTpage();
				fm_vislist(CURforminfo());
				form_ctl(Cur_rec->id, CTSETPAGE, CURpage()); 
				nextfield(1000, 1000, -1, NX_ANY);
				return;
			}
			else { 			/* next field */
				if (CURpage() != LASTpage())
					CURpage()++;
				else
					CURpage() = 1;
				fm_vislist(CURforminfo());
				form_ctl(Cur_rec->id, CTSETPAGE, CURpage()); 
				nextfield(0, 0, 1, NX_ANY);
				return;
			}
		}
		else {
			/*
			 * simply wrap around to the top/bottom of the page 
			 */
			curi = leasti;
		}
	}
	if (curi < 0)
		curi = virt(CURfield());	/* only one active field */
	chg_curfield(curi);
}

#define TOGGLE 1
#define LONGLIST 2
#define ACTION 3

/*
** Checks an "rmenu" to see if it is a small list (less than three
** members), a large list or a command.
*/
testlist(list)
char **list;
{
	if (list[0][0] == '{') {
		if (array_len(list) <= 5)
			return(TOGGLE);
		return(LONGLIST);
	}
	return(ACTION);
}

char *Choice_list[3] = {
	"OPEN",
	"MENU",
	"-i"
};

/*
** Turns an rmenu field into a command.
*/
token
rmenuaction(list)
register char **list;
{
	extern char	*Args[];
	extern int	Arg_count;
	int	lcv;

	if (testlist(list) == LONGLIST) {
		char *s;
		register int i;
		register IOSTRUCT *out;
		char **help;

		out = io_open(EV_USE_STRING, NULL);
		putastr("menu=Choices\n", out);
		putastr("lifetime=shortterm\n", out);
		putastr("Help=", out);
		help = (char **) sing_eval(CURform(), FM_HELP);
		lcv = array_len(help);
		for (i = 0; i < lcv; i++) {
			putastr(help[i], out);
			putac(' ', out);
		}
		putac('\n', out);
		lcv = array_len(list) - 1;
		for (i = 1; i < lcv; i++) {
			putac('\n', out);
			putastr("name=\"", out);
			putastr(list[i], out);
			putastr("\"\n", out);
			putastr("lininfo=\"", out);
			putastr(list[i], out);
			putastr("\"\n", out);
			putastr("action=`set -l Form_Choice=\"", out);
			putastr(list[i], out);
			putastr("\"`close", out);
			putac('\n', out);
		}
		putastr("name=CANCEL\nbutton=1\naction=close\n", out);
		putastr("name=\nbutton=2\naction=badchar\n", out);
		putastr("name=\nbutton=3\naction=badchar\n", out);
		putastr("name=\nbutton=4\naction=badchar\n", out);
		putastr("name=\nbutton=5\naction=badchar\n", out);
		putastr("name=\nbutton=7\naction=badchar\n", out);
		putastr("name=\nbutton=8\naction=badchar\n", out);
		for (Arg_count = 0; Arg_count < 3; Arg_count++) {
			if (Args[Arg_count])
				free(Args[Arg_count]); /* les */

			Args[Arg_count] = strsave(Choice_list[Arg_count]);
		}

		if (Args[Arg_count])
			free(Args[Arg_count]);  /* les */

		Args[Arg_count++] = io_string(out);
		io_close(out);

		if (Args[Arg_count])
			free(Args[Arg_count]);  /* les */

		Args[Arg_count] = NULL;
		return(TOK_OPEN);
	}
	return(setaction(list));
}

/*
** Processes characters after the editor.
*/
token
post_stream(t)
register token t;
{
	formfield form_field;
	struct menu_line m;
	char *str;
	char **list;
	int *slks;
	int i;
	int num;
	int nextflags, flag;
	char *s;
	int	lcv;

	nextflags = flag = 0;
	form_field = CURffield();

	s = NULL;
	if (t >= TOK_SLK1 && t <= TOK_SLK16) {
		slks = CURforminfo()->slks;
		num = t - TOK_SLK1 + 1;
		lcv = array_len(slks);
		for(i = 0; i < lcv; i++)
			if (atoi(multi_eval(CURform(), slks[i], FM_BUTT)) == num) {
				form_ctl(Cur_rec->id, CTGETARG, &s);
				t = setaction(multi_eval(CURform(), slks[i], FM_ACTI));
				break;
			}
	}
	switch(t) {
	case TOK_OPTIONS:
		t = TOK_NOP;
		if (list = (char **) multi_eval(CURform(), CURfield(), FM_RMENU)) {
			int i;
			char *str;

			if ((str = multi_eval(CURform(), CURfield(), FM_CHOICEMSG)) && *str) {
				mess_temp(str);
				mess_lock();	/* don't overwrite it !!! */
			}
			if (array_len(list) == 0) {
				if (!(str && *str))
					mess_temp("There are no choices available");
			}
			else if (testlist(list) == TOGGLE) {
				char *s;
				int len;

				s = getaltenv(VALS(), NUMSTR(CURfield() + 1));
				len = array_len(list) - 2;
				list = list + 1;

				for (i = 0; i < len - 1; i++)
					if (strcmp(s, list[i]) == 0)
						break;
				if (set_curval(strsave(list[(i + 1) % len])) == SUCCESS) {
					fm_vislist(CURforminfo());
					redo_vals(CURfield() + 1);
					form_ctl(Cur_rec->id, CTSETDIRTY);
				}
			}
			else
				t = rmenuaction(list);
		}
		break;
	case TOK_RESET:
		{
			char *s = NULL;

			form_ctl(Cur_rec->id, CTGETARG, &s);
			de_const(CURform(), CURfield(), FM_VALUE);
			redo_vals(CURfield());
			fm_vislist(CURforminfo());
			form_ctl(Cur_rec->id, CTSETDIRTY);
			t = TOK_NOP;
			break;
		}
	case TOK_DONE:
		t = TOK_BADCHAR;
		if (fld_get_ck(&form_field) != SUCCESS)
			t = TOK_NOP;
		else if (str = sing_eval(CURform(), FM_DONE))
			t = make_action(str);
		else {
			warn(VALID, "");
			mess_lock();
		}
		break;
	case TOK_UP:
		nextflags |= NX_ADVANCE;
		/* fall through */
	case TOK_PREVIOUS:
		nextflags |= NX_NOCUR;
		if (fld_get_ck(&form_field) == SUCCESS)
			nextfield(atoi(multi_eval(CURform(), CURfield(), FM_FROW)), atoi(multi_eval(CURform(), CURfield(), FM_FCOL)), -1, nextflags);
		else
			set_form_field(Cur_rec->id, CURfield());
		t = TOK_NOP;
		break;
	case TOK_DOWN:
		nextflags |= NX_ADVANCE;
		/* fall through */
	case TOK_SAVE:
	case TOK_NEXT:
		nextflags |= NX_NOCUR;
		if (fld_get_ck(&form_field) == SUCCESS)
			nextfield(atoi(multi_eval(CURform(), CURfield(), FM_FROW)), atoi(multi_eval(CURform(), CURfield(), FM_FCOL)), 1, nextflags);
		else
			set_form_field(Cur_rec->id, CURfield());
		t = TOK_NOP;
		break;
	case TOK_PPAGE:
		if (fld_get_ck(&form_field) == SUCCESS) {
			if (CURpage() > 1) {
				CURpage()--;
				fm_vislist(CURforminfo());
				form_ctl(Cur_rec->id, CTSETPAGE, CURpage()); 
				nextfield(0, 0, 1, NX_ANY);
			}
			else {
				set_form_field(Cur_rec->id, CURfield());
				t = TOK_BADCHAR;
				break;
			}
		}
		else 
			set_form_field(Cur_rec->id, CURfield());
		t = TOK_NOP;
		break;
	case TOK_NPAGE:
		if (fld_get_ck(&form_field) == SUCCESS) {
			if (LASTpage() != CURpage()) {
				CURpage()++;
				fm_vislist(CURforminfo());
				form_ctl(Cur_rec->id, CTSETPAGE, CURpage()); 
				nextfield(0, 0, 1, NX_ANY);
			}
			else {
				set_form_field(Cur_rec->id, CURfield());
				t = TOK_BADCHAR;
				break;
			}
		}
		else 
			set_form_field(Cur_rec->id, CURfield());
		t = TOK_NOP;
		break;
	}
	return(t);
}

/*
** Processes characters before the editor.
*/
int
pre_stream(t)
register token t;
{
	formfield form_field;
	int cur;

/* les */
	if ( t > 037 && t < 0177 )
		return t;
/*******/

	form_field = CURffield();
	switch(t) {
	case TOK_END:
		nextfield(1000, 1000, -1, NX_ANY);
		t = TOK_NOP;
		break;
	case TOK_BEG:
		nextfield(0, 0, 1, NX_ANY);
		t = TOK_NOP;
		break;
	case TOK_BTAB:
		if (fld_get_ck(&form_field) == SUCCESS)
			nextfield(atoi(multi_eval(CURform(), CURfield(), FM_FROW)), atoi(multi_eval(CURform(), CURfield(), FM_FCOL)), -1, NX_NOCUR | NX_ADVANCE);
		else
			set_form_field(Cur_rec->id, CURfield());
		t = TOK_NOP;
		break;
	case TOK_TAB:
		t = TOK_SAVE;
	case TOK_WDWMGMT:
		Noreshape++;
		break;
	}
	return(t);
}

/*
** Sets up the stream for forms.
*/
static int
objform_stream(a, t)
struct actrec *a;
token t;
{
	int (*func[5])();
	register int olifetime;
	extern int field_stream();

	Cur_rec = a;
	olifetime = Cur_rec->lifetime;
	Cur_rec->lifetime = AR_PERMANENT;
	func[0] = pre_stream;
	func[1] = field_stream;
	func[2] = post_stream;
	func[3] = NULL;
	t = stream(t, func);
	Cur_rec->lifetime = olifetime;
	return(t);
}

set_form_field(id, field_num)
int id, field_num;
{
	char *str;
	
	if ((str = multi_eval(CURform(), field_num, FM_FIELDMSG)) && *str)
		mess_temp(str);
	form_ctl(id, CTSETPOS, virt(field_num), 0, 0);
}
