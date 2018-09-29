/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */

#ident	"@(#)vm.vt:src/vt/message.c	1.4"

#include	<stdio.h>
#include	<curses.h>
#include	"wish.h"
#include	"vt.h"
#include	"message.h"
#include	"vtdefs.h"

#define MESSIZ	(80)

static char	MESS_perm[MESSIZ] = "";
static char	SAVE_perm[MESSIZ];
static char	MESS_temp[MESSIZ];
static bool	MESS_isblank = TRUE;

/*
 * Mess_lock is used to avoid simultaneous writes to the message line
 * (see message.h)
 */
int Mess_lock = 0;

void
mess_flash(s)
register char	*s;
{
/* find nil !!! */

	if (!s)
		s = nil;
	if (s[0] || !MESS_isblank) {
		register vt_id	oldvid;
/* new */
		WINDOW		*win;
	
		win = VT_array[ MESS_WIN ].win;
		mvwaddstr( win, 0, 0, s ); 
		wclrtobot( win );
		MESS_isblank = !s[0];
		wnoutrefresh( win );
/*		doupdate();
*/
/***/
/*
		oldvid = vt_current(MESS_WIN);
		wgo(0, 0);
		wclrwin();
		wputs(s);
		MESS_isblank = !s[0];
		vt_current(oldvid);
*/
	}
/*
	flush_output();
*/
}

void
mess_temp(s)
register char	*s;
{
	if (!s)
		s = nil;
	if (!Mess_lock)
		strncpy(MESS_temp, s, sizeof(MESS_temp) - 1);
}

char *
mess_perm(s)
register char	*s;
{
	char *sptr, *strcpy();

	if (!s)
		s = nil;
	sptr = strcpy(SAVE_perm, MESS_perm);
	strncpy(MESS_perm, s, sizeof(MESS_perm) - 1);
	return(sptr);		/* return previous perm message */
}

void
mess_flush(permanent)
bool	permanent;
{
	if (permanent || MESS_temp[0] == '\0')
		mess_flash(MESS_perm);
	else if (MESS_temp[0]) {
		mess_flash(MESS_temp);
		MESS_temp[0] = '\0';
	}
/*
	else
		flush_output();
*/
}
