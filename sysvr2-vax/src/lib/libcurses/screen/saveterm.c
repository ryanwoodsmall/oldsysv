/*	@(#) saveterm.c: 1.1 10/15/83	(1.36	3/18/83)	*/

#include "curses.ext"
#include "../local/uparm.h"

extern	struct term *cur_term;

saveterm()
{
	def_prog_mode();
}