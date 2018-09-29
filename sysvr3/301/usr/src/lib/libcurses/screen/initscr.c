/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/initscr.c	1.3.1.2"
# include	"curses.ext"

extern	char	*getenv();

/*
 * This routine initializes the current and standard screen,
 * and sets up the terminal.  In case of error, initscr aborts.
 * If you want an error status returned, call
 *	scp = newterm(getenv("TERM"), stdout, stdin);
 *
 */

WINDOW *
initscr()
{
	register char *sp;
	struct screen *scp;
	extern char *_c_why_not;
	static int i_called_before = 0;

	if (i_called_before && SP)
		freetabs();
	if ((sp = getenv("TERM")) == NULL)
		sp = Def_term;
	scp = newterm(sp, stdout, stdin);
# ifdef DEBUG
	if(outf) fprintf(outf, "INITSCR: term = %s\n", sp);
# endif
	if (scp == NULL)
		_ec_quit(_c_why_not, sp);
	i_called_before = 1;
	return stdscr;
}

/*
 * Free the data structures we are about to throw away so we
 * can reuse the memory.
 */
static
freetabs()
{
	extern void free();
	(void) del_curterm(SP->tcap);
	delwin(curscr);
	delwin(stdscr);
	free((char *) SP->cur_body);
	free((char *) SP->std_body);
	free((char *) SP->input_queue);
	free((char *) SP);
	SP = NULL;
}
