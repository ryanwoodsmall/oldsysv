/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
/*	@(#) _delchars.c: 1.1 10/15/83	(1.21	2/11/83)	*/

#include "curses.ext"

extern	int	_outch();

/*
 * Delete n characters.
 */
_delchars (n)
{
#ifdef DEBUG
	if(outf) fprintf(outf, "_delchars(%d).\n", n);
#endif
	if (enter_delete_mode) {
		if (strcmp(enter_delete_mode, enter_insert_mode) == 0) {
			if (SP->phys_irm == 0) {
				tputs(enter_delete_mode,1,_outch);
				SP->phys_irm = 1;
			} 
		}
		else {
			if (SP->phys_irm == 1) {
				tputs(exit_insert_mode,1,_outch);
				SP->phys_irm = 0;
			}
			tputs(enter_delete_mode,1,_outch);
		}
	}
	while (--n >= 0) {
		/* Only one line can be affected by our delete char */
		tputs(delete_character, 1, _outch);
	}
	if (exit_delete_mode) {
		if (strcmp(exit_delete_mode, exit_insert_mode) == 0)
			SP->phys_irm = 0;
		else
			tputs(exit_delete_mode, 1, _outch);
	}
}
