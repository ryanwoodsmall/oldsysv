/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/printers/headings.c	1.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "sys/types.h"

#include "lp.h"
#include "printers.h"

struct {
	char			*v;
	short			len,
				okremote;
}			prtrheadings[PR_MAX] = {

#define	ENTRY(X)	X, sizeof(X)-1
	ENTRY("Banner:"),	   1,	/* PR_BAN */
	ENTRY("CPI:"),		   1,	/* PR_CPI */
	ENTRY("Character sets:"),  1,	/* PR_CS */
	ENTRY("Content types:"),   1,	/* PR_ITYPES */
	ENTRY("Device:"),	   0,	/* PR_DEV */
	ENTRY("Dial:"),		   0,	/* PR_DIAL */
	ENTRY("Fault:"),	   0,	/* PR_RECOV */
	ENTRY("Interface:"),	   0,	/* PR_INTFC */
	ENTRY("LPI:"),		   1,	/* PR_LPI */
	ENTRY("Length:"),	   1,	/* PR_LEN */
	ENTRY("Login:"),	   0,	/* PR_LOGIN */
	ENTRY("Printer type:"),    1,	/* PR_PTYPE */
	ENTRY("Remote:"),	   1,	/* PR_REMOTE */
	ENTRY("Speed:"),	   0,	/* PR_SPEED */
	ENTRY("Stty:"),		   0,	/* PR_STTY */
	ENTRY("Width:"),	   1,	/* PR_WIDTH */
#undef	ENTRY

};
