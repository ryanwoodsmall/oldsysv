/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/lp/set_size.c	1.5"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "string.h"

#include "lp.set.h"

extern char		*malloc(),
			*tparm();

extern void		free();

extern double		strtod();

extern short		output_res_char,
			output_res_line,
			output_res_horz_inch,
			output_res_vert_inch;

#define SET_FORM_LENGTH	user9
#define SFL		"u9"

/**
 ** set_size()
 **/

int			set_size (str, which, putout)
	char			*str;
	int			which,
				putout;
{
	static int		cleared_margins_already	= 0;

	double			size;

	int			i,
				isize,
				ret;

	short			curval,
				output_res,
				output_res_inch;

	char			*rest,
				*set_margin1,
				*set_margin2,
				*set_margin1_parm,
				*set_margin2_parm,
				*set_both_margins,
				*move1,
				*move2,
				*step2,
				*p1,
				*p2,
				*sp1,
				*sp2,
				*carriage_return,
				*parm_right_cursor,
				*column_address,
				*repeat_char,
				*cursor_right,
				*parm_down_cursor,
				*row_address,
				*cursor_down,
				*clear_margins,
				*finale;

#if	defined(SET_FORM_LENGTH)
	char			*SET_FORM_LENGTH;
#endif


	if (which == 'W') {

		tidbit ((char *)0, "cols", &curval);

		if (output_res_char == -1)
			tidbit ((char *)0, "orc", &output_res_char);
		output_res = output_res_char;

		if (output_res_horz_inch == -1)
			tidbit ((char *)0, "orhi", &output_res_horz_inch);
		output_res_inch = output_res_horz_inch;

		tidbit ((char *)0, "smgl", &set_margin1);
		tidbit ((char *)0, "smgr", &set_margin2);
		tidbit ((char *)0, "smglp", &set_margin1_parm);
		tidbit ((char *)0, "smgrp", &set_margin2_parm);

	} else {

		tidbit ((char *)0, "lines", &curval);

		if (output_res_line == -1)
			tidbit ((char *)0, "orl", &output_res_line);
		output_res = output_res_line;

		if (output_res_vert_inch == -1)
			tidbit ((char *)0, "orvi", &output_res_vert_inch);
		output_res_inch = output_res_vert_inch;

		tidbit ((char *)0, "smgt", &set_margin1);
		tidbit ((char *)0, "smgb", &set_margin2);
		tidbit ((char *)0, "smgtp", &set_margin1_parm);
		tidbit ((char *)0, "smgbp", &set_margin2_parm);

#if	defined(SET_FORM_LENGTH)
		tidbit ((char *)0, SFL, &SET_FORM_LENGTH);
#endif

	}

	size = strtod(str, &rest);
	if (size <= 0)
		return (E_BAD_ARGS);

	switch (*rest) {
	case ' ':
	case 0:
		break;
	case 'c':
		/*
		 * Convert to inches.
		 */
		size /= 2.54;
		/* fall through */
	case 'i':
		/*
		 * Convert to lines/columns.
		 */
		if (output_res == -1 || output_res_inch == -1)
			return (E_FAILURE);
		size *= output_res_inch / output_res;
		break;
	default:
		return (E_BAD_ARGS);
	}

	
	if ((isize = R(size)) == curval)
		return (E_SUCCESS);

	/*
	 * We number things 0 through N (e.g. an 80 column
	 * page is numbered 0 to 79). Thus if we are asked
	 * to set a width of 132, we set the left margin at
	 * 0 and the right at 131.
#if	defined(SET_FORM_LENGTH)
	 * Of course, if we're using the SET_FORM_LENGTH string,
	 * we give the length as N+1.
#endif
	 */
	isize--;

	if (which == 'W') {

		tidbit ((char *)0, "cr", &carriage_return);
		tidbit ((char *)0, "cuf", &parm_right_cursor);
		tidbit ((char *)0, "hpa", &column_address);
		tidbit ((char *)0, "rep", &repeat_char);
		tidbit ((char *)0, "cuf1", &cursor_right);

		if (OKAY(carriage_return))
			move1 = carriage_return;
		else
			move1 = "\r";

		if (
			OKAY(parm_right_cursor)
		     && (move2 = tparm(parm_right_cursor, isize))
		)
			;

		else if (
			OKAY(column_address)
		     && (move2 = tparm(column_address, isize))
		)
			;

		else if (
			OKAY(repeat_char)
		     && (move2 = tparm(repeat_char, ' ', isize))
		)
			;

		else if (OKAY(cursor_right)) {
			move2 = 0;
			step2 = cursor_right;

		} else {
			move2 = 0;
			step2 = " ";
		}

		finale = move1;		/* i.e. carriage return */

	} else {

		tidbit ((char *)0, "cud", &parm_down_cursor);
		tidbit ((char *)0, "vpa", &row_address);
		tidbit ((char *)0, "cud1", &cursor_down);

		move1 = "";	/* Assume we're already at top-of-page */

		if (
			OKAY(parm_down_cursor)
		     && (move2 = tparm(parm_down_cursor, isize))
		)
			;

		else if (
			OKAY(row_address)
		     && (move2 = tparm(row_address, isize))
		)
			;

		else if (OKAY(cursor_down)) {
			move2 = 0;
			step2 = cursor_down;

		} else {
			move2 = 0;
			step2 = "\n";
		}

		/*
		 * This has to be smarter, but we don't have the
		 * smarts ourselves, yet.
		 */
		tidbit ((char *)0, "ff", &finale);

	}

	if (OKAY(set_margin1_parm) && !OKAY(set_margin2_parm))
		set_both_margins = set_margin1_parm;

	else if (OKAY(set_margin2_parm) && !OKAY(set_margin1_parm))
		set_both_margins = set_margin2_parm;

	else
		set_both_margins = 0;

	sp1 = sp2 = 0;

#if	defined(SET_FORM_LENGTH)
	if (
		which == 'L'
	     && OKAY(SET_FORM_LENGTH)
	     && (p1 = tparm(SET_FORM_LENGTH, isize + 1))
	) {
		if (putout)
			putp (p1);
		finale = 0;
		ret = E_SUCCESS;
		
	} else
#endif
	if (
		OKAY(set_both_margins)
	     && (p1 = tparm(set_both_margins, 0, isize))
	     && *p1
	     && (sp1 = strdup(p1))
	) {

		if (putout) {

			if (!cleared_margins_already) {
				tidbit ((char *)0, "mgc", &clear_margins);
				if (OKAY(clear_margins)) {
					cleared_margins_already = 1;
					putp (clear_margins);
				}
			}

			putp (sp1);

		}
		ret = E_SUCCESS;

	/*
	 * The "smgbp" string takes two parameters; each defines the
	 * position of the margin, the first counting lines from the top
	 * of the page, the second counting lines from the bottom of the
	 * page. This shows the flaw in using the set-margin commands
	 * for setting the page length, because BY DEFINITION the second
	 * parameter must be 0 for us. But giving 0 won't cause a change
	 * in the page length, will it!
	 * 
	 * Anyway, the "smgrp" expects just one parameter (thus will
	 * ignore a second parameter) so we can safely give the second
	 * parameter without caring which of width or length we're
	 * setting.
	 */
	} else if (
		OKAY(set_margin1_parm)
	     && (p1 = tparm(set_margin1_parm, 0))
	     && *p1
	     && (sp1 = strdup(p1))
	     && OKAY(set_margin2_parm)
	     && (p2 = tparm(set_margin2_parm, isize, 0))
	     && *p2
	     && (sp2 = strdup(p2))
	) {

		if (putout) {

			if (!cleared_margins_already) {
				tidbit ((char *)0, "mgc", &clear_margins);
				if (OKAY(clear_margins)) {
					cleared_margins_already = 1;
					putp (clear_margins);
				}
			}

			putp (sp1);
			putp (sp2);

		}
		ret = E_SUCCESS;

	} else if (
		OKAY(set_margin1)
	     && OKAY(set_margin2)
	     && (OKAY(move2) || OKAY(step2))
	) {

		register char		*p,
					*q;

		register int		free_it = 0;

		if (putout) {

			if (!cleared_margins_already) {
				tidbit ((char *)0, "mgc", &clear_margins);
				if (OKAY(clear_margins)) {
					cleared_margins_already = 1;
					putp (clear_margins);
				}
			}

			putp (move1);
			putp (set_margin1);

			if (!move2) {
				move2 = malloc(isize * strlen(step2) + 1);
				if (!move2)
					return (E_MALLOC);
				for (p = move2, i = 0; i < isize; i++)
					for (q = step2; *q; )
						*p++ = *q++;
				*p = 0;
				free_it = 1;
			}

			putp (move2);
			putp (set_margin2);

			if (free_it)
				free (move2);
		}
		ret = E_SUCCESS;

	} else
		ret = E_FAILURE;

	if (putout && OKAY(finale))
		putp (finale);

	if (sp1)
		free (sp1);
	if (sp2)
		free (sp2);
	return (ret);
}
