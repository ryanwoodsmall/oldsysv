/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.wish:src/wish/mudge.c	1.5"

#include	<stdio.h>
#include	<ctype.h>
#include	"wish.h"
#include	"token.h"
#include	"vtdefs.h"
#include	"actrec.h"
#include	"slk.h"
#include	"ctl.h"
#include	"moremacros.h"

/* modes */
#define MODE_MOVE	1
#define MODE_RESHAPE	2

static int	mode;
static int	srow;
static int	scol;
static int	rows;
static int	cols;
static char	position1[] = "Position ";
static char	position2[] = " corner and hit ENTER";
static char	*Savemsg = NULL;

char *mess_perm();

static int
wdw_close(rec)
struct actrec *rec;
{
	(void) mess_perm(Savemsg);
	make_box(0, 0, 0, 0, 0);
	return SUCCESS;
}

/*ARGSUSED*/
static int
wdw_ctl(rec, wdw, a1, a2, a3, a4, a5, a6)
struct actrec	*rec;
int	wdw;
int	a1, a2, a3, a4, a5, a6;
{
	return FAIL;
}

/*ARGSUSED*/
static token
wdw_stream(rec, t)
struct actrec	*rec;
register token	t;
{
	register int	newsrow;
	register int	newscol;
	register int	newrows;
	register int	newcols;
	register bool	moving;
	char	*nstrcat();

	moving = FALSE;
	newsrow = srow;
	newscol = scol;
	newrows = rows;
	newcols = cols;
	switch (t) {
	case TOK_UP:
		moving = TRUE;
		if (mode & MODE_MOVE)
			newsrow--;
		else
			newrows--;
		break;
	case TOK_DOWN:
		moving = TRUE;
		if (mode & MODE_MOVE)
			newsrow++;
		else
			newrows++;
		break;
	case TOK_LEFT:
		moving = TRUE;
		if (mode & MODE_MOVE)
			newscol--;
		else
			newcols--;
		break;
	case TOK_RIGHT:
		moving = TRUE;
		if (mode & MODE_MOVE)
			newscol++;
		else
			newcols++;
		break;
	case TOK_BTAB:
		moving = TRUE;
		if (mode & MODE_MOVE)
			newscol = (newscol - 1 & ~7);
		else
			newcols = (newcols - 1 & ~7);
		break;
	case TOK_TAB:
		moving = TRUE;
		if (mode & MODE_MOVE)
			newscol = (newscol + 8 & ~7);
		else
			newcols = (newcols + 8 & ~7);
		break;
	case TOK_RETURN:
	case TOK_ENTER:
#ifdef _DEBUG
		_debug(stderr, "mode=%d\n", mode);
#endif
		if (mode & MODE_RESHAPE && mode & MODE_MOVE) {
			mode = MODE_RESHAPE;
			make_box(1, srow, scol, rows, cols);
			(void) mess_perm(nstrcat(position1, "bottom-right", position2, NULL));
		} else {
			if (mode & MODE_RESHAPE)
				ar_ctl(rec->odptr, CTSETSHAPE, srow, scol, rows, cols);
			else  {
				vt_id	vid;

				vid = vt_current(ar_ctl(rec->odptr, CTGETVT));
				vt_move(srow, scol);
				vt_current(vid);
			}
			ar_backup();
		}
		t = TOK_NOP;
		break;
	case TOK_CANCEL:
		ar_backup();
		t = TOK_NOP;
		break;
	}
	if (moving) {
		if (make_box(!(mode & MODE_MOVE), newsrow, newscol, newrows, newcols)) {
			t = TOK_NOP;
			srow = newsrow;
			scol = newscol;
			rows = newrows;
			cols = newcols;
		}
		else {
			t |= TOK_ERROR;
			make_box(!(mode & MODE_MOVE), srow, scol, rows, cols);
		}
	}
	return t;
}

static int
wdw_current(rec)
register struct actrec	*rec;
{
	vt_id vt;

	vt = ar_ctl(rec->odptr, CTGETVT);

	vt_ctl(vt, CTGETSTRT, &srow, &scol);
	vt_ctl(vt, CTGETSIZ, &rows, &cols);
	/* allow extra space for borders */
	make_box(0, --srow, --scol, rows += 2, cols += 2);
	return SUCCESS;
}

void
enter_wdw_mode(rec, reshape)
struct actrec *rec;
bool	reshape;
{
	struct actrec	a;
	char	*tmpstr, *nstrcat();
	struct actrec	*ar_create();
	extern struct slk	Echslk[];

	mode = (reshape ? (MODE_RESHAPE | MODE_MOVE) : MODE_MOVE);
	a.id = 0;
	a.flags = AR_SKIP;
	a.path = NULL;
	a.odptr = (char *) (rec ? rec : ar_get_current());
	a.fcntbl[AR_CLOSE] = wdw_close;
	a.fcntbl[AR_REINIT] = AR_NOP;
	a.fcntbl[AR_HELP] = AR_NOHELP;
	a.fcntbl[AR_NONCUR] = AR_NOP;
	a.fcntbl[AR_CURRENT] = wdw_current;
	a.fcntbl[AR_CTL] = wdw_ctl;
	a.fcntbl[AR_ODSH] = wdw_stream;
	a.lifetime = AR_SHORTERM;
	a.slks = Echslk;

	ar_current(ar_create(&a));
	/*
	 * put up a permanent message, saving the old one 
	 */
	tmpstr = mess_perm(nstrcat(position1, "top-left", position2, NULL));
	if (Savemsg)		/* ehr3 */
		free(Savemsg);	/* ehr3 */

	Savemsg = strsave(tmpstr);
}
