/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpsched/validate.h	1.1"

/*
 * The old LP Spooler would take a job destined for a class of printers
 * if the class was accepting, regardless of the acceptance status of
 * the printers. This sounds a bit silly, so we thought we'd change it.
 * Well, that's not compatible. So YOU decide. Define the following if
 * you want compatibility, don't define it if you want to require BOTH
 * the printers (at least one) and the class to be accepting.
 */
#define CLASS_ACCEPT_PRINTERS_REJECT_SOWHAT 1	/* */

/*
 * Define the following if we should stop examing a list of printers
 * on the first one that meets all the needs of the request.
 * Currently this is done because to continue wouldn't matter. However,
 * you may add additional code that considers other factors (e.g. size
 * of queue for printer, size of file under consideration.)
 */
#define FILTER_EARLY_OUT 1			/* */

typedef struct candidate {
	PSTATUS			*pps;
	char			*slow,
				*fast;
	unsigned short		flags,
				weight;
}			CANDIDATE;

#define WEIGHT_NOFILTER	 1
#define WEIGHT_FREE	 2
#define	WEIGHT_ENABLED	 4
#define	WEIGHT_MOUNTED	 8
#define WEIGHT_SELECTS	16
#define	WEIGHT_MAX	(WEIGHT_NOFILTER + WEIGHT_FREE + WEIGHT_ENABLED + 2 * WEIGHT_MOUNTED + WEIGHT_SELECTS)

extern int		pick_filter();

extern char		*o_cpi,
			*o_lpi,
			*o_width,
			*o_length;
