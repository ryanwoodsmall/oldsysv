/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/vidputs.c	1.5"
#include "curses.ext"

char *tparm();

/* nooff: modes that don't have an explicit "turn me off" capability */
#define nooff	(A_PROTECT|A_INVIS|A_BOLD|A_DIM|A_BLINK|A_REVERSE)
/* hilite: modes that could be faked with standout in a pinch. */
#define hilite	(A_UNDERLINE|A_BOLD|A_BLINK|A_REVERSE)

vidputs(newmode, outc)
register int newmode;
register int (*outc)();
{
	register chtype curmode, oldmode, faked, nfaked;

	oldmode = curmode = cur_term->sgr_mode;
	nfaked = faked = cur_term->sgr_faked;

#ifdef DEBUG
	if (outf) fprintf(outf, "vidputs oldmode=%o, newmode=%o\n", oldmode, newmode);
#endif
	if (newmode || !exit_attribute_mode) {
		/*
			Either we will leave an attribute on, or there
			is no exit_attribute_mode available
		*/
			
		if ((newmode&A_ALTCHARSET) && ena_acs) {
			tputs(ena_acs, 1, outc);
			ena_acs = NULL;	/* only once */
		}
		if (set_attributes) {
			tputs(tparm(set_attributes,
					newmode & A_STANDOUT,
					newmode & A_UNDERLINE,
					newmode & A_REVERSE,
					newmode & A_BLINK,
					newmode & A_DIM,
					newmode & A_BOLD,
					newmode & A_INVIS,
					newmode & A_PROTECT,
					newmode & A_ALTCHARSET),
				1, outc);
			curmode = newmode;
		} else {
			/* Check for things to turn off. */
			/* First check for modes with no OFF cap. */
			if ((oldmode&nooff) > (newmode&nooff)) {
				if (exit_attribute_mode) {
					tputs(exit_attribute_mode, 1, outc);
				} else if (oldmode == A_UNDERLINE &&
					   exit_underline_mode) {
					tputs(exit_underline_mode, 1, outc);
				} else if (exit_standout_mode) {
					tputs(exit_standout_mode, 1, outc);
				}
				oldmode = curmode = faked = 0;
			}

			/* Now check modes with OFF caps. */
			if (!(newmode&A_ALTCHARSET) && (oldmode&A_ALTCHARSET)) {
				tputs(exit_alt_charset_mode, 1, outc);
				if (!cur_term->rmacs_rmso_neq && (oldmode&A_STANDOUT))
					curmode &= ~A_STANDOUT;
				if ((!cur_term->rmacs_sgr0_neq) || (faked && !cur_term->rmacs_rmso_neq)) {
					/*
						If equal, we blasted all
						attributes
					*/
					curmode = oldmode = faked = 0;
				} else curmode &= ~A_ALTCHARSET;
			}
			if (!(newmode&A_UNDERLINE) && (oldmode&A_UNDERLINE)) {
				if (nfaked&A_UNDERLINE) {
					/*
						stop faked underline
					*/
					nfaked &= ~A_UNDERLINE;
				} else {
					/*
						stop real underline
					*/
					if ((!cur_term->rmul_sgr0_neq) || (faked && !cur_term->rmul_rmso_neq)) {
						curmode = oldmode = faked = 0;
					}
					tputs(exit_underline_mode, 1, outc);
					if (!cur_term->rmul_rmso_neq) {
						if (oldmode&A_STANDOUT) 
							curmode &= ~A_STANDOUT;
						faked = 0;
					}
				}
				curmode &= ~A_UNDERLINE;
			}
			if (!(newmode&A_STANDOUT) && (oldmode&A_STANDOUT)) {
				/*
					Since smso is used for fakes we
					leave it on if something is
					faked
				*/
				if (!nfaked) {
					/*
						No fakes, issue rmso,
						check for reset
					*/
					tputs(exit_standout_mode, 1, outc);
					if (!cur_term->rmso_sgr0_neq) {
						/*
							If equal, we 
							blasted all
							attributes
						*/
						curmode = oldmode = faked = 0;
					}
				}
				curmode &= ~A_STANDOUT;
			}

			/* Check for modes to turn on. */
			/*
			 * First check for modes that cannot be faked
			 * with standout mode.
			 */
			if ((newmode&A_ALTCHARSET) && !(oldmode&A_ALTCHARSET)) {
				tputs(enter_alt_charset_mode, 1, outc);
				curmode |= A_ALTCHARSET;
			}
			if ((newmode&A_PROTECT) && !(oldmode&A_PROTECT)) {
				tputs(enter_protected_mode, 1, outc);
				curmode |= A_PROTECT;
			}
			if ((newmode&A_INVIS) && !(oldmode&A_INVIS)) {
				tputs(enter_secure_mode, 1, outc);
				curmode |= A_INVIS;
			}

			/*
			 * Now check for modes that can be faked
			 * with standout mode.
			 */
			if ((newmode&A_BOLD) && !(oldmode&A_BOLD)) {
				if (enter_bold_mode) {
					curmode |= A_BOLD;
					tputs(enter_bold_mode, 1, outc);
				} else nfaked |= A_BOLD;
			}
			if ((newmode&A_DIM) && !(oldmode&A_DIM)) {
				if (enter_dim_mode) {
					curmode |= A_DIM;
					tputs(enter_dim_mode, 1, outc);
				} else nfaked |= A_DIM;
			}
			if ((newmode&A_BLINK) && !(oldmode&A_BLINK)) {
				if (enter_blink_mode) {
					curmode |= A_BLINK;
					tputs(enter_blink_mode, 1, outc);
				} else nfaked |= A_BLINK;
			}
			if ((newmode&A_REVERSE) && !(oldmode&A_REVERSE)) {
				if (enter_reverse_mode) {
					curmode |= A_REVERSE;
					tputs(enter_reverse_mode, 1, outc);
				} else nfaked |= A_REVERSE;
			}
			if ((newmode&A_UNDERLINE) && !(oldmode&A_UNDERLINE)) {
				if (enter_underline_mode) {
					curmode |= A_UNDERLINE;
					tputs(enter_underline_mode,1,outc);
				} else nfaked |= A_UNDERLINE;
			}
			if ((newmode&A_STANDOUT) && !(oldmode&A_STANDOUT))
				if (enter_standout_mode) {
					curmode |= A_STANDOUT;
					if (!faked) tputs(enter_standout_mode,1,outc);
				}
		}
	} else {
		/*
			!newmode, turn everything off please.
		*/
		if (exit_attribute_mode)
			tputs(exit_attribute_mode, 1, outc);
		else if (oldmode == A_UNDERLINE && exit_underline_mode)
			tputs(exit_underline_mode, 1, outc);
		else if (exit_standout_mode)
			tputs(exit_standout_mode, 1, outc);
		curmode = 0;
		nfaked = 0;
	}

	/*
	 * If we asked for a hilighting on a terminal without that
	 * specific capability, we fake it with A_STANDOUT
	 */
	if ((newmode&hilite) && (curmode!=newmode)) {
		/*
			If not in A_STANDOUT turn it on please
		*/
		if (!(curmode&A_STANDOUT) && !faked)
			tputs(enter_standout_mode, 1, outc);

		/*
			We set curmode=newmode so that faked things
			look on. We do not turn on A_STANDOUT cause
			then we wouldn't know who turned it on!
		*/
		curmode = newmode;
	}

	cur_term->sgr_mode = curmode;
	cur_term->sgr_faked = nfaked;
}
