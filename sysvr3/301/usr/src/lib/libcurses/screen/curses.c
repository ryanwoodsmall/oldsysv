/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/curses.c	1.4"
/*
 * Define global variables
 *
 */
# include	"curses.ext"

char	*Def_term	= "unknown";	/* default terminal type	*/
WINDOW *stdscr, *curscr;
int	LINES, COLS;
struct screen *SP;
chtype *acs_map;
int _use_slk = 0;
struct _ripdef _ripstruct[5];
int _ripcounter = 0;

char *curses_version = "SVR3";

# ifdef DEBUG
FILE	*outf;			/* debug output file			*/
# endif
