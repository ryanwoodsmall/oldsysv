/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.sys:src/sys/actrec.c	1.8"

#include <stdio.h>
#include <varargs.h>
#include "wish.h"
#include "token.h"
#include "slk.h"
#include "actrec.h"
#include "menudefs.h"
#include "ctl.h"
#include "terror.h"
#include "message.h"

/* Functions for manipulating activation records */

static struct actrec *AR_head;
       struct actrec *AR_cur;
static int AR_serial = 0;

void ar_dump();

/* LES: becoming a MACRO

struct actrec *
ar_get_current()
{
	return(AR_cur);
}
*/

struct actrec *
ar_create(rec)
register struct actrec *rec;
{
	register struct actrec *newrec;
	register struct actrec	*ap;

	newrec = new(struct actrec);
	*newrec = *rec;
	newrec->serial = AR_serial++;

	/* link the new record right after the current record */

	if (AR_head == NULL) {
		/* nobody has been made yet, so make a degenerate list */
		AR_head = newrec;
		newrec->nextrec = newrec;
		newrec->prevrec = newrec;
		newrec->backup = newrec;
	}
	else {
		ap = (AR_cur ? AR_cur : AR_head);
		newrec->prevrec = ap;
		newrec->nextrec = ap->nextrec;
		newrec->backup = ap;
		ap->nextrec = newrec;
		newrec->nextrec->prevrec = newrec;
	}
	return(newrec);
}

struct actrec *
ar_reinit(rec)
register struct actrec *rec;
{
	struct actrec *ret;

	ret = (struct actrec *) arf_reinit(rec, rec);
	if (rec == AR_cur)
		ar_setslks(rec->slks, rec->flags & AR_ALTSLKS);
	return ret;
}

struct actrec *
ar_reread(rec)
register struct actrec *rec;
{
	struct actrec * ret;

	ret =  (struct actrec *) arf_reread(rec, rec);
	if (rec == AR_cur)
		ar_setslks(rec->slks, rec->flags & AR_ALTSLKS);
	return ret;
}

struct actrec *
ar_close(rec)
register struct actrec *rec;
{
	register struct actrec	*ap;
	register struct actrec	*new_cur;
	struct actrec *ar_cur_prev();

	if (rec == NULL && (rec = AR_cur) == NULL) {
		error(MUNGED, "NULL actrec");
		return(rec);
	}

	/* close the internals of the record */

	if (arf_close(rec, rec) == FAIL) {
		error(MUNGED, "can't close actrec");
		return(AR_cur);
	}

	/* if closing current record, choose a new one */
	if (rec == AR_cur)
		new_cur = AR_cur->backup;
	else
		new_cur = NULL;

	/* relink the lists */
	if (rec->nextrec == rec)
		AR_head == NULL;
	else {
		if (rec == AR_head)
			AR_head = rec->prevrec;
		rec->nextrec->prevrec = rec->prevrec;
		rec->prevrec->nextrec = rec->nextrec;
		for (ap = AR_cur; ap; ) {
			if (ap->backup == rec) {
				ap->backup = rec->backup;
				break;
			}
			if ((ap = ap->backup) == AR_cur) {
				rec = NULL;
				break;
			}
		}
	}
	if (new_cur == rec)
		AR_cur = NULL;
	else if (new_cur) {
		register struct actrec	*old_AR_cur;
		void	ar_cur_end();

		(void) arf_reinit(new_cur, new_cur);
		old_AR_cur = AR_cur;
		AR_cur = new_cur;
		ar_cur_end(old_AR_cur);
	}
#ifdef _DEBUG
	_debug(stderr, "FREEING ACTREC %x\n", rec);
#endif
	if (rec)
		free(rec);
#ifdef _DEBUG5
	ar_dump("AFTER ar_close");
#endif
	return AR_cur;
}

struct actrec *
ar_cur_next()
{
	register struct actrec	*ap;
	struct actrec *ar_current();

	for (ap = AR_cur->nextrec; ap && (ap->flags & AR_SKIP); ap = ap->nextrec)
		if (ap == AR_cur)
			break;
	if (ap != AR_cur)
		ap = ar_current(ap);
	return ap;
}

struct actrec *
ar_cur_prev()
{
	register struct actrec	*ap;
	struct actrec *ar_current();

	for (ap = AR_cur->prevrec; ap && (ap->flags & AR_SKIP); ap = ap->prevrec)
		if (ap == AR_cur)
			break;
	if (ap != AR_cur)
		ap = ar_current(ap);
	return ap;
}

/*
 * front end for ar_current and ar_backup
 * cleans up previously current actrec and calls reinit on new current one
 */
static void
ar_cur_front(ap)
register struct actrec	*ap;
{
	/*
	 * if there is a current record, and that record is not the same
	 * as the one we are making current, then either close it or make
	 * it non-current, depending on its lifetime.
	 */
	if (AR_cur && AR_cur != ap) {
		if (AR_cur->lifetime == AR_SHORTERM) {
			mess_lock();		/* don't ask ... */
			(void) ar_close(AR_cur);
			mess_unlock();
		}
		(void) arf_noncur(AR_cur, AR_cur);
	}
	(void) arf_reinit(ap, ap);
}

/*
 * back end for ar_current and ar_backup
 * calls current function and sets slks
 * "ap" is actrec to make current if it fails
 */
static void
ar_cur_end(ap)
register struct actrec	*ap;
{
	ar_setslks(AR_cur->slks, AR_cur->flags & AR_ALTSLKS);
	if (arf_current(AR_cur, AR_cur) == FAIL) {
		error(MUNGED, "can't make actrec current");
		AR_cur = ap;
		ar_setslks(AR_cur->slks, AR_cur->flags & AR_ALTSLKS);
	}
#ifdef _DEBUG5
	ar_dump("at end of ar_current");
#endif
}

struct actrec *
ar_backup()
{
	register struct actrec	*ap;
	register struct actrec	*old_AR_cur;

	ap = AR_cur->backup;
	ar_cur_front(ap);
	old_AR_cur = AR_cur;
	AR_cur = ap;
	ar_cur_end(old_AR_cur);
	return AR_cur;
}

struct actrec *
ar_current(rec)
register struct actrec *rec;
{
	register struct actrec	*ap;

	ar_cur_front(rec);
	/*
	 * backup is a circularly linked list
	 */
	if (AR_cur == NULL)
		/* produce degenerate list */
		rec->backup = rec;
	else if (AR_cur != rec) {
		/* traverse entire list */
		for (ap = AR_cur; ap->backup != AR_cur; ap = ap->backup) {
			/*
			 * if rec is in list and is not where we want it
			 *	unlink it from list
			 */
			if (ap->backup == rec && rec->backup != AR_cur)
				ap->backup = ap->backup->backup;
		}
		/* if rec is not where we want it, link it in befre cur */
		if (ap != rec) {
			rec->backup = ap->backup;
			ap->backup = rec;
		}
	}
	AR_cur = rec;
	ar_cur_end(AR_cur->backup);
	return AR_cur;
}

ar_setslks(s, flags)
struct slk	*s;
int flags;
{
	setslks(s, flags);
	return SUCCESS;
}

token
actrec_stream(t)
token t;
{
	return arf_odsh(AR_cur, t);
}

/* find an activation record via its window number */

struct actrec *
wdw_to_ar(wdw)
int wdw;
{
	struct actrec *p;

	for (p = AR_head; p; p = p->nextrec)
		if (wdw == ar_ctl(p, CTGETWDW))
			return p;
		else if (p->nextrec == AR_head)
			return NULL;
	return NULL;
}

/* find an activation record via its path */

struct actrec *
path_to_ar(s)
char *s;
{
	struct actrec *p;

	for (p = AR_head; p; p = p->nextrec)
		if (strcmp(p->path, s) == 0)
			return p;
		else if (p->nextrec == AR_head)
			return NULL;
	return NULL;
}

bool
path_isopen(s, op, exact)
char *s, *op;
bool exact;	/* if TRUE, don't allow exact match */
{
	struct actrec *p;
	int len = strlen(s);

	for (p = AR_head; p; p = p->nextrec) {
		if (exact && strcmp(p->path, s) == 0) {
			mess_temp(nstrcat("Can't ", op, 
				" an open object, close it first", NULL));
			return(TRUE);
		} else if (strncmp(p->path, s, len) == 0 && p->path[len] == '/') {
			mess_temp(nstrcat("Can't ", op, 
				" a folder with open sub-folders, close them first", NULL));
			return(TRUE);
		} else if (p->nextrec == AR_head)
			break;
	}
	return FALSE;
}

/* find the activation record past the argument given with a certain path */

struct actrec *
nextpath_to_ar(ar)
register struct actrec *ar;
{
	register struct actrec *p;

	for (p = ar->nextrec; p != ar; p = p->nextrec)
		if (strcmp(p->path, ar->path) == 0)
			return(p);
	return(NULL);
}

struct actrec *
ar_cleanup(life)		/* clean up all records with lifetime <= life */
register int life;
{
	register struct actrec *p, *nextp;

	p = AR_head;
	do {
		p = p->nextrec;
		if (p->lifetime <= life && !(life < AR_IMMORTAL && p->flags & AR_SKIP))
			(void) ar_close(p);
	} while (p != AR_head);

	return(AR_cur);
}

ar_help(rec)	/* do help on current actrec */
struct actrec *rec;
{
	return(arf_help(rec, rec));
}

void
ar_checkworld(force)
bool force;	/* if TRUE, forced check */
{
	struct actrec *p;
	static long last_check;
	extern long Mail_check;
	extern long Cur_time;

	if (force == FALSE && Cur_time <= last_check + Mail_check)
		return;

	last_check = Cur_time;

	for (p = AR_head; p; p = p->nextrec) {
		(void) arf_reinit(p, p);
		if (p->nextrec == AR_head)
			break;
	}
}

struct actrec *
menline_to_ar(n)
int n;
{
	register int i;
	register struct actrec *p;

	for (p = AR_head, i = -1; p; p = p->nextrec) {
		/* records with NULL path fields are not listed */
		if (p->path) {
			if (++i == n)
				return(p);
		}
		if (p->nextrec == AR_head)
			break;
	}
	return NULL;
}

struct menu_line
ar_menudisp(n, ptr)
register int n;
register char *ptr;
{
	register struct actrec *p;
	struct menu_line m;

	m.description = NULL;
	m.flags = 0;
	if (p = menline_to_ar(n))
		(void) ar_ctl(p, CTGETITLE, &m.highlight);
	else
		m.highlight = NULL;
	return m;
}

int
ar_ctl(rec, cmd, arg1, arg2, arg3, arg4, arg5, arg6)
struct actrec *rec;
int cmd;
int arg1, arg2, arg3, arg4, arg5, arg6;
{
	if (rec == NULL)
		return(FAIL);
	if (cmd == CTSETLIFE) {
		rec->lifetime = arg1;
		return(SUCCESS);
	} else
		return((*(rec->fcntbl[AR_CTL]))(rec, cmd, arg1, arg2, arg3, arg4, arg5, arg6));
}

#ifdef _DEBUG5

/* for debugging - print out activation record info */
void
ar_dump(s)
char	*s;
{
	struct actrec *p;

	_debug5(stderr, "---------- Activation Record Dump: %s ----------\n", s);
	_debug5(stderr, " #\tprev\tnext\tbackup\tpath\n");

	for (p = AR_head; p; p = p->nextrec) {
		_debug5(stderr, "%2d\t", p->serial);
		_debug5(stderr, "%2d\t", p->prevrec?p->prevrec->serial:-1);
		_debug5(stderr, "%2d\t", p->nextrec?p->nextrec->serial:-1);
		_debug5(stderr, "%2d\t", p->backup?p->backup->serial:-1);
		_debug5(stderr, "%5.5s", (p==AR_cur)?"CUR>>":"     ");
		_debug5(stderr, "%s\n", p->path);
		if (p->nextrec == AR_head)
			break;
	}
	_debug5(stderr, "Current = %s\n", AR_cur?AR_cur->path:"NULL");
	_debug5(stderr, "Head    = %s\n", AR_head?AR_head->path:"NULL");
	_debug5(stderr, "-------------**Dump End**------------------\n");
}
#endif
