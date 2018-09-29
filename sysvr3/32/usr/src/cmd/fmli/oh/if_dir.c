/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.oh:src/oh/if_dir.c	1.16"

#include <stdio.h>
#include <varargs.h>
#include <string.h>
#include "wish.h"
#include "menudefs.h"
#include "vtdefs.h"
#include "token.h"
#include "slk.h"
#include "actrec.h"
#include "typetab.h"
#include "ctl.h"
#include "var_arrays.h"
#include "terror.h"
#include	"moremacros.h"

#define MAX_DIRS	(16)

/* The activation record for a file folder contains the ott_tab for the
 * folder in the odptr field.  File folders use the menu human-interface,
 * so the id field of the activation record is a menu_id.
 */

long Sortmodes = OTT_SALPHA, Dispmodes = OTT_DOBJ, Prefmodtime = 0L;

static int Dirs_open = 0;	/* count how many open */

/* macro to cast the odptr field of an actrec to my struct odptr */

#define MYODPTR(X)	((struct myodptr *)(X->odptr))

static struct myodptr {
	struct ott_tab *ott;
	long dir_mtime;
	long ott_mtime;
	long pref_mtime;
};

static int 
dir_close(a) 
struct actrec *a; 
{ 
#ifdef _DEBUG
	_debug(stderr, "DIR_CLOSE\n");
#endif
	ott_unlock_inc(MYODPTR(a)->ott); /* unlock the ott so it can be swapped */
	free(MYODPTR(a));
	if (a->path)
		free(a->path);
	Dirs_open--;
	return(menu_close(a->id));		/* close the menu id */
}

static struct menu_line
dir_disp(n, ptr)
int n;
struct ott_tab *ptr;
{
	register int i;
	int size = array_len(ptr->parents);
	struct ott_entry *entry;
	struct menu_line m;
	static char ldescr[50];

	m.flags = 0;

	if (n == 0 && size == 0) {	/* empty! */
		m.highlight = "Empty Folder";
		m.description = NULL;
	} else if (n >= array_len(ptr->parents)) {
		m.highlight = m.description = NULL;
	} else {
		entry = &(ptr->ott[ptr->parents[n]]);
		m.highlight = entry->dname;

		switch (ptr->modes & DISMODES) {
		case 0:			/* don't display anything */
			m.description = NULL;
			break;
		case OTT_DMARK:
			m.description = NULL;
			strcpy(ldescr, entry->dname);
			if (entry->objmask & CL_DIR)
				strcat(ldescr, "/");
			else if (strcmp(entry->objtype, "EXECUTABLE") == 0)
				strcat(ldescr, "*");
			m.highlight = ldescr;
			break;
		default:
		case OTT_DOBJ:	/* display description only */
			m.description = entry->display;
			break;
		case OTT_DMTIME:	/* display long form */
			{
				char *p, *ct, *ctime();
				int len, i;
				bool usrdisp = FALSE;
				struct ott_entry *tmp;
				char *def_display();
				int	lcv;

				lcv = array_len(ptr->parents);
				for (i = 0; !usrdisp && i < lcv; i++) {
					tmp = &(ptr->ott[ptr->parents[i]]);
					p = def_display(tmp->objtype);
					if (tmp->display != p && strcmp(tmp->display, p) != 0)
						usrdisp = TRUE;
				}

				if (entry->display == (p = def_display(entry->objtype)) 
						|| strcmp(entry->display, p) == 0)
					len = sprintf(ldescr, "%-14.14s%*s", 
							entry->display, usrdisp ? 23 : 2, "");
				else
					len = sprintf(ldescr, "%-19.19s [%-.14s]%*s ", 
						entry->display, p, 14-strlen(p), "");
				ct = ctime(&(entry->mtime));
				sprintf(ldescr+len, "%12.12s", ct+4);
			}
			m.description = ldescr;
			break;
		}
	}
	return(m);
}

static int
dir_help(a)
struct actrec *a;
{
	extern char *Wastebasket;
	char *help, *title, path[PATHSIZ];
	char *filename(), *anyenv();

	sprintf(path, "%s/.pref", a->path);
	if ((help = anyenv(path, "HELP")) == NULL || *help == '\0') {
		if (strncmp(a->path, Wastebasket, strlen(Wastebasket)) == 0) {
			help = "T.h55.waste";
			title = "Wastebasket";
		}
		else {
			help = "T.h43.fold";
			title = "File folder";
		}
	} else {
		title = filename(a->path);
	}

	return(objop("OPEN", "TEXT", "$VMSYS/OBJECTS/Text.mfhelp", help, 
					title, NULL));
}

static int
dir_current(a)
struct actrec *a;
{
	extern char Opwd[];

	make_current(a->path);
	menu_current(a->id);
	chdir(a->path);
	sprintf(&Opwd[5], "%.*s", PATHSIZ, a->path);
	return(SUCCESS);
}

static int
dir_reread(a)
struct actrec *a;
{
	return(dir_init(a, TRUE));
}

static int
dir_reinit(a)
struct actrec *a;
{
	return(dir_init(a, FALSE));
}

static int
dir_init(a, force)
struct actrec *a;
bool force;
{
	struct ott_tab *ott = MYODPTR(a)->ott;

#ifdef _DEBUG
	_debug(stderr, "DIR_REINIT ");
#endif
	MYODPTR(a)->ott = (ott = ott_get(ott->path, Sortmodes, Dispmodes,
				ott->amask, ott->nmask));
	if (ott == NULL)
		return(FAIL);

	if (force || MYODPTR(a)->dir_mtime != ott->dir_mtime || 
				MYODPTR(a)->ott_mtime != ott->ott_mtime  ||
				Prefmodtime != MYODPTR(a)->pref_mtime ) {
#ifdef _DEBUG
		_debug(stderr, "RECREATING WINDOW %s\n", ott->path);
#endif
		MYODPTR(a)->dir_mtime = ott->dir_mtime;
		MYODPTR(a)->ott_mtime = ott->ott_mtime;
		MYODPTR(a)->pref_mtime = Prefmodtime;
		a->id = menu_reinit(a->id, 0, dir_disp, ott);
	}
#ifdef _DEBUG
	else
		_debug(stderr, "No change in directory\n");
#endif

	ott_lock_inc(ott);
	return(SUCCESS);
}

/* these arguments are kludgy, but varargs wouldn't do the trick */

static int
dir_ctl(rec, cmd, arg1, arg2, arg3, arg4, arg5, arg6)
struct actrec *rec;
int cmd;
int arg1, arg2, arg3, arg4, arg5, arg6;
{
#ifdef _DEBUG
	_debug(stderr, "DIR_CTL: cmd=%d\n", cmd);
#endif
	if (cmd == CTGETARG) {
		int line;
		char *path;
		struct ott_entry *entry;
		struct ott_tab *tab = MYODPTR(rec)->ott;
		char *ott_to_path();

		if (array_len(tab->parents) == 0)
			return(FAIL);
		(void) menu_ctl(rec->id, CTGETPOS, &line);
		entry = &(tab->ott[tab->parents[line]]);
		path = ott_to_path(entry);
		/*if ( **((char ***)(&arg1)))
			free( **((char ***)(&arg1)));     /* les 12/4 */
		**((char ***)(&arg1)) = strsave(path);
		return(SUCCESS);
	} else if (cmd == CTISDEST) {
		**((bool **)(&arg1)) = TRUE;
		return(SUCCESS);
	} else
		return(menu_ctl(rec->id, cmd, arg1, arg2, arg3, arg4, arg5, arg6));
}

static token
dir_odsh(rec, t)
struct actrec *rec;
register token t;
{
	token menu_stream();

#ifdef _DEBUG
	_debug(stderr, "dir_odsh(%o) => ", t);
#endif
	t = menu_stream(t);
#ifdef _DEBUG
	_debug(stderr, "%o\n", t);
#endif
	if (t == TOK_NEXT)
		t = TOK_NOP;	/* filter out, see menu_stream */
	return(t);
}

int
IF_dir_open(argv)
char *argv[];
{
	register int i;
	char *path;
	long amask, nmask;
	struct ott_tab *ott;
	struct actrec a, *prevdir, *path_to_ar();
	vt_id vid;
	char	*path_to_title();
	char	*nstrcat();

	struct ott_tab *ott_get();


	init_modes();
	amask = 0;
	nmask = M_WB;

	for (i = 0; argv[i]; i++) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
			case 'w':	/* wastebasket mode */
				amask = M_WB;
				nmask = 0;
				break;
			case 'a':	/* set amask only */
				amask = strtol(argv[i]+2);
				break;
			case 'n':	/* set nmask only */
				nmask = strtol(argv[i]+2);
				break;
			}
		} else
			path = argv[i];
	}

	if (prevdir = path_to_ar(path)) {
		ar_current(prevdir);
		return(SUCCESS);
	}
	if (Dirs_open > MAX_DIRS) {
		mess_temp("Too many folders open. Close some, then try again.");
		return(FAIL);
	}
	if ((ott = ott_get(path, Sortmodes, Dispmodes, amask, nmask)) == NULL) {
		mess_temp(nstrcat("Could not open folder ", path_to_title(path, NULL), NULL));
		return(FAIL);
	}

	if (dir_create(ott, &a, FALSE) == FAIL || !ar_current(ar_create(&a)))
		return(FAIL);
	Dirs_open++;
	return(SUCCESS);
}

dir_create(ott, a, cover)
struct ott_tab *ott;
struct actrec *a;
bool cover;
{
	menu_id menu_default();
	char	*path_to_title();

 	/*if ( a->path )
 		free( a->path );		/* les 12/4 */
 	a->path = strsave(ott->path);
	a->fcntbl[AR_CLOSE] = dir_close;
	a->fcntbl[AR_REREAD] = dir_reread;
	a->fcntbl[AR_REINIT] = dir_reinit;
	a->fcntbl[AR_CURRENT] = dir_current;
	a->fcntbl[AR_NONCUR] = AR_MEN_NONCUR;
	a->fcntbl[AR_HELP] = dir_help;
	a->fcntbl[AR_CTL] = dir_ctl;
	a->fcntbl[AR_ODSH] = dir_odsh;

	/* we will keep track of what the mod times were on the ott at the
	 * time we first created the window, that way we will be able to 
	 * know when we should recreate the window to reflect the new data.
	 */
	a->odptr = (char *) new(struct myodptr);
	MYODPTR(a)->ott = ott;
	MYODPTR(a)->dir_mtime = ott->dir_mtime;
	MYODPTR(a)->ott_mtime = ott->ott_mtime;
#ifdef _DEBUG
	_debug(stderr, "dir_create mtimes=%d %d\n", ott->dir_mtime, ott->ott_mtime);
#endif

	a->id = menu_default(path_to_title(ott->path, NULL), cover ? VT_COVERCUR : 0,
			VT_UNDEFINED, VT_UNDEFINED, dir_disp, ott);

	if (a->id == FAIL)
		return(FAIL);

	a->lifetime = AR_LONGTERM;
	a->flags = 0;
	a->slks = NULL;

	return SUCCESS;
}

/* selection handlers are not used by wish - they are built in to the
 * ODSH function
 */

int
IF_dsh()
{
}

int
IF_dvi()
{
}

int
IF_dmv(argv)
char *argv[];
{
	return(dir_mv_cp(TRUE, argv));
}

int
IF_dcp(argv)
char *argv[];
{
	return(dir_mv_cp(FALSE, argv));
}

static bool
eq_waste_fc(path, op)
char *path, *op;
{
	char *err = NULL;
	extern char *Filecabinet, *Wastebasket;
	char *filename();

	if (strcmp(Filecabinet, path) == 0)
		err = Filecabinet;
	else if (strcmp(Wastebasket, path) == 0)
		err = Wastebasket;
	if (err) {
		mess_temp(nstrcat("Can't ", op, " your ", filename(err), NULL));
		return(FAIL);
	} else
		return(SUCCESS);
}

static int
dir_mv_cp(mv, argv)
bool mv;
char *argv[];
{
	struct ott_entry *ott;
	char command[50+PATHSIZ*2];
	char *odi, *display, path[PATHSIZ], msg[80];
	int l, ret = SUCCESS;
	extern char *Wastebasket, Undel[];
	char *nstrcat(), *filename();
	struct ott_entry *path_to_ott();
	char	*path_to_title();

	working(TRUE);
	if (strncmp(argv[0], argv[1], l = strlen(argv[0])) == 0 &&
					argv[1][0] == '/') {
		mess_temp(nstrcat("Can't ", mv?"move":"copy",
					" a folder inside itself!", NULL));
		return(FAIL);
	}
	if (eq_waste_fc(argv[0], mv ? "move" : "copy") == FAIL)
		return(FAIL);
	if (mv && ckperms(parent(argv[0], 02)) == FAIL)
		return(FAIL);
	if (ckperms(argv[1], 02) == FAIL)
		return(FAIL);
	if (mv && path_isopen(argv[0], mv ? "move" : "copy", TRUE))
		return(FAIL);
	if ((ott = path_to_ott(argv[0])) == NULL)
		return(FAIL);
	odi = strsave(ott->odi);
	display = strsave(ott->display);
	sprintf(command, "dir_%s %s %s %s", mv?"move":"copy", argv[0], argv[1], 
				argv[2]?argv[2]:"");

	if (waitspawn(sysspawn(command)) == 0) {
		sprintf(msg, "%s %sed to folder %s",
			argv[2] ? argv[2] : filename(argv[0]),
			mv ? "mov" : "copi", path_to_title(argv[1], NULL));
		sprintf(path, "%s/%s", argv[1], argv[2] ? argv[2] : filename(argv[0]));
		if ((ott = path_to_ott(path)) == NULL)
			return(FAIL);
		/*if ( ott->odi )
			free( ott->odi );   /* les 12/4 */
		ott->odi = odi;
		/*if ( ott->display )
			free( ott->display );   /* les 12/4 */
		ott->display = display;
		(void) ott_chg_odi(ott);
		if (strncmp(path, Wastebasket, strlen(Wastebasket)) == 0)
			(void) odi_putkey(ott, Undel, parent(argv[0]));
		(void) utime(path, NULL);
		ott_mtime(ott);
		ret = SUCCESS;
	} else {
		sprintf(msg, "%s %s FAILED TO %s", 
			argv[2] ? argv[2] : filename(argv[0]),
			mv ? "MOVE" : "COPY", path_to_title(argv[1], NULL));
		ret = FAIL;
	}
	mess_temp(msg);
	return(ret);
}

int
IF_drn(argv)
char *argv[];
{
	register char	*p;
	char	*nstrcat();

	p = filename(argv[0]);
	if (ckperms(parent(argv[0]), 02) == FAIL)
		return(FAIL);
	if (path_isopen(argv[0], "rename", TRUE))
		return(FAIL);
	if (eq_waste_fc(argv[0], "rename") == FAIL)
		return(FAIL);
	if (waitspawn(spawn("/bin/mv", "mv", "-f", argv[0], 
			nstrcat(parent(argv[0]), "/", argv[1], NULL), NULL)))
		mess_temp(nstrcat(p, " rename failed", NULL));
	else
		mess_temp(nstrcat(p, " renamed to ", argv[1], NULL));
	return(SUCCESS);
}
