/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/filters/insfilter.c	1.6"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "assert.h"
#include "string.h"
#include "errno.h"

#include "lp.h"
#include "filters.h"

extern char		*malloc();

typedef struct PARM {
	char			*keyword;
	unsigned short		flags;
	char			**pvalue;
}			PARM;

#define	X_MUST	0x0800	/* Pipeline MUST use this parm */
#define	X_FIRST	0x1000	/* Use parm only in 1st cmd of pipeline */
#define	X_FIXED	0x2000	/* Get value from elsewhere, not parms */
#define	X_MANY	0x4000	/* Several values allowed for parm */
#define	X_USED	0x8000	/* Used already, don't use again */

static struct S {
	TYPE			input_type,
				output_type,
				printer_type;
	PARM			*parms;
}			S;

static int		searchlist_t(),
			instantiate(),
			check_pipeline(),
			match();

static char		*build_pipe();

/**
 ** Table of recognized keywords, with info. about them.
 **/

#define NFIXED 3

static PARM		parmtable[] = {

/* These must be the first NFIXED, and in this order */
PARM_INPUT,   X_FIXED,				&S.input_type.name,
PARM_OUTPUT,  X_FIXED,				&S.output_type.name,
PARM_TERM,    X_FIXED,				&S.printer_type.name,

PARM_CPI,     FPARM_CPI,			0,
PARM_LPI,     FPARM_LPI,			0,
PARM_LENGTH,  FPARM_LENGTH,			0,
PARM_WIDTH,   FPARM_WIDTH,			0,
PARM_PAGES,   FPARM_PAGES | X_FIRST | X_MUST,	0,
PARM_CHARSET, FPARM_CHARSET,			0,
PARM_FORM,    FPARM_FORM,			0,
PARM_COPIES,  FPARM_COPIES | X_FIRST,		0,
PARM_MODES,   FPARM_MODES | X_MANY | X_MUST,	0,
0,	      0,				0,
};

/**
 ** insfilter()
 **/

FILTERTYPE		insfilter (
				pipes,
				input_type,
				output_type,
				printer_type,
				printer,
				parms,
				flagsp
			)
	char			**pipes,
				*input_type,
				*output_type,
				*printer_type,
				*printer,
				**parms;
	unsigned short		*flagsp;
{
	_FILTER			*pipeline;

	FILTERTYPE		ret;

	S.input_type.name = input_type;
	S.input_type.info = isterminfo(input_type);
	S.output_type.name = output_type;
	S.output_type.info = isterminfo(output_type);
	S.printer_type.name = printer_type;
	S.printer_type.info = isterminfo(printer_type);

	/*
	 * If the filters have't been loaded yet, do so now.
	 * We'll load the standard table, but the caller can override
	 * this by first calling "loadfilters()" with the appropriate
	 * filter table name.
	 */
	if (!filters && loadfilters((char *)0) == -1)
		return (fl_none);

	/*
	 * Allocate and initialize space to hold additional
	 * information about each item in "parms".
	 * THIS SPACE MUST BE FREED BEFORE EXITING THIS ROUTINE!
	 */
	{
		register int		n;

		register PARM		*pp,
					*ppt;

		register char		**p;



		for (n = 0, p = parms; *p; n++, p++)
			;
		n /= 2;
		n += NFIXED; /* for fixed parms (in/out/printer types) */

		if (!(S.parms = (PARM *)malloc((n + 1) * sizeof(PARM)))) {
			errno = ENOMEM;
			return (fl_none);
		}

		for (ppt = parmtable; ppt->keyword; ppt++)
			ppt->flags &= ~X_USED;

		/*
		 * Load the parameter list with the fixed ``type''
		 * parameters. Mark them as used (if appropriate)
		 * so we don't pick them up from the callers list.
		 */
		pp = S.parms;
		for (ppt = parmtable; ppt < parmtable + NFIXED; ppt++) {
			pp->keyword = ppt->keyword;
			pp->flags = ppt->flags;
			if (ppt->flags & X_FIXED)
				pp->pvalue = ppt->pvalue;
			else
				pp->pvalue = parms + 1;
			if (!(ppt->flags & X_MANY))
				ppt->flags |= X_USED;
			pp++;
		}

		/*
		 * Copy each parameter from the caller supplied list
		 * to another list, adding information gathered from
		 * the keyword table. Note that some keywords should
		 * be given only once; additional occurrances in the
		 * caller's list will be ignored.
		 */
		for (p = parms; *p; p += 2)
			for (ppt = parmtable; ppt->keyword; ppt++)
				if (
					STREQU(*p, ppt->keyword)
				     && !(ppt->flags & X_USED)
				) {

					pp->keyword = ppt->keyword;
					pp->flags = ppt->flags;
					if (ppt->flags & X_FIXED)
						pp->pvalue = ppt->pvalue;
					else
						pp->pvalue = p + 1;

					if (!(ppt->flags & X_MANY))
						ppt->flags |= X_USED;

					pp++;
					break;

				}
			
		pp->keyword = 0;

	}

	/*
	 * Preview the list of filters, to rule out those that
	 * can't possibly work.
	 */
	{
		register _FILTER	*pf;

		for (pf = filters; pf->name; pf++) {

			pf->mark = FL_CLEAR;

			if (
				printer
			     && !searchlist(printer, pf->printers)
			)
				pf->mark = FL_SKIP;

			else if (
				printer_type
			     && !searchlist_t(&(S.printer_type), pf->printer_types)
			)
				pf->mark = FL_SKIP;

		}
	}

	/*
	 * Find a pipeline that will convert the input-type to the
	 * output-type and map the parameters as well.
	 */
	if (!instantiate(
		&pipeline,
		&S.input_type,
		&S.output_type,
		check_pipeline,
		S.parms
	)) {
		ret = fl_none;
		goto Return;
	}

	if (!pipes) {
		ret = fl_both;
		goto Return;

	} else {
		register _FILTER	*pf,
					*pfastf, /* first in fast pipe */
					*pslowf; /* last in slow pipe */

		/*
		 * Found a pipeline, so now build it.
		 */

		/*
		 * Split pipeline after last slow filter.
		 * "pipeline" will point to first filter in slow
		 * pipe, "pfastf" will point to first filter in
		 * fast pipe.
		 */
		for (
			pf = pfastf = pipeline, pslowf = 0;
			pf;
			pf = pf->next
		)
			if (pf->type == fl_slow) {
				pslowf = pf;
				pfastf = pf->next;
			}

		if (pslowf) {
			assert (pslowf != pfastf);
			pslowf->next = 0;
			pipes[0] = build_pipe(pipeline, S.parms, flagsp);
			ret = fl_slow;
		} else
			pipes[0] = 0;

		if (pfastf) {
			pipes[1] = build_pipe(pfastf, S.parms, flagsp);
			ret = fl_fast;
		} else
			pipes[1] = 0;

		if (pslowf && pfastf)
			ret = fl_both;

		/*
		 * Check for the oops case.
		 */
		if (pslowf && !pipes[0] || pfastf && !pipes[1])
			ret = fl_none;

	}

Return:	free ((char *)S.parms);

	return (ret);
}

/**
 ** searchlist_t() - SEARCH (TYPE *) LIST FOR ITEM
 **/

static int		typematch (type1, type2)
	TYPE			*type1, *type2;
{
	if (
		STREQU(type1->name, NAME_ANY)
	     || STREQU(type2->name, NAME_ANY)
	     || STREQU(type1->name, type2->name)
	     || (
			STREQU(type1->name, NAME_TERMINFO)
		     && type2->info
		)
	     || (
			STREQU(type2->name, NAME_TERMINFO)
		     && type1->info
		)
	)
		return (1);
	else
		return (0);
}

static int		searchlist_t (itemp, list)
	TYPE			*itemp;
	register TYPE		*list;
{
	if (!list || !list->name)
		return (0);

	/*
	 * This is a linear search--we believe that the lists
	 * will be short.
	 */
	while (list->name) {
		if (typematch(itemp, list))
			return (1);
		list++;
	}
	return (0);
}

/**
 ** instantiate() - CREATE FILTER-PIPELINE KNOWING INPUT/OUTPUT TYPES
 **/

/*
 *	The "instantiate()" routine is the meat of the "insfilter()"
 *	algorithm. It is given an input-type and output-type and finds a
 *	filter-pipline that will convert the input-type into the
 *	output-type. Since the filter-pipeline must meet other criteria,
 *	a function "ver" is also given, along with a set of criteria in
 *	"crit"; these are used by "instantiate()" to verify a
 *	filter-pipeline. 
 *
 *	The filter-pipeline is built up and returned in "pipeline".
 *	Conceptually this is just a list of filters, with the pipeline to
 *	be constructed by simply concatenating the filter simple-commmands
 *	(after filling in option templates) in the order found in the
 *	list. What is used in the routine, though, is a pair of linked
 *	lists, one list forming the ``right-half'' of the pipeline, the
 *	other forming the ``left-half''. The pipeline is then the two
 *	lists taken together.
 *
 *	The "instantiate()" routine looks for a single filter that matches
 *	the input-type and output-type and satisfies the criteria. If one
 *	is found, it is added to the end of the ``left-half'' list (it
 *	could be added to the beginning of the ``right-half'' list with no
 *	problem). The two lists are linked together to form one linked
 *	list, which is passed, along with the set of criteria, to the
 *	"ver" routine to check the filter-pipeline. If it passes the
 *	check, the work is done. 
 *
 *	If a single filter is not found, "instantiate()" examines all
 *	pairs of filters where one in the pair can accept the input-type
 *	and the other can produce the output-type. For each of these, it
 *	calls itself again to find a filter that can join the pair
 *	together--one that accepts as input the output-type of the first
 *	in the pair, and produces as output the input-type of the second
 *	in the pair.  This joining filter may be a single filter or may
 *	be a filter-pipeline. "instantiate()" checks for the trivial case
 *	where the input-type is the output-type; with trivial cases it
 *	links the two lists without adding a filter and checks it with
 *	"ver".
 */

/**
 ** instantiate()
 **/

static int		_instantiate();

/*
 * A PIPELIST is what is passed to each recursive call to "instantiate()".
 * It contains a pointer to the end of the ``left-list'', a pointer to the
 * head of the ``right-list'', and a pointer to the head of the left-list.
 * The latter is passed to "ver". The end of the right-list (and thus the
 * end of the entire list when left and right are joined) is the filter
 * with a null ``next'' pointer.
 */
typedef struct PIPELIST {
	_FILTER			*lhead,
				*ltail,
				*rhead;
}			PIPELIST;

static int		peg;

static int		instantiate (pline, input, output, ver, crit)
	_FILTER			**pline;
	TYPE			*input,
				*output;
	int			(*ver)();
#if	defined(lint)
	char			*crit;
#else
	void			*crit;
#endif
{
	PIPELIST		p;
	int			ret;

	peg = 0;
	p.lhead = p.ltail = p.rhead = 0;
	ret = _instantiate(&p, input, output, ver, crit);
	*pline = p.lhead;
	return (ret);
}

#define	ENTER()		int our_tag = ++peg;

#define	LEAVE(Y)	if (!Y) { \
				register _FILTER *f; \
				for (f = filters; f->name; f++) \
					CLEAR(f); \
				return(0); \
			} else return(1)

#define MARK(F,M)	(((F)->mark |= M), (F)->level = our_tag)

#define CLEAR(F)	if ((F)->level == our_tag) \
				(F)->level = 0, (F)->mark = FL_CLEAR

#define CHECK(F,M)	(((F)->mark & M) && (F)->level == our_tag)

#define	USED(F)		((F)->mark)

static int		_instantiate (pp, inputp, outputp, ver, crit)
	PIPELIST		*pp;
	TYPE			*inputp,
				*outputp;
	int			(*ver)();
	void			*crit;
{
	register _FILTER	*prev_lhead,
				*prev_ltail;

	/*
	 * Must be first ``statement'' after declarations.
	 */
	ENTER ();

	/*
	 * We're done when we've added filters on the left and right
	 * that let us connect the left and right directly; i.e. when
	 * the output of the left is the same type as the input of the
	 * right. HOWEVER, there must be at least one filter involved,
	 * to allow the filter feature to be used for handling modes,
	 * pages, copies, etc. not just FILTERING data.
	 */
	if (typematch(inputp, outputp) && pp->lhead) {

		/*
		 * Getting here means that we must have a left and right
		 * pipeline. Why? For "pp->lhead" to be non-zero it
		 * must have been set below. The first place below
		 * doesn't set the right pipeline, but it also doesn't
		 * get us here (at least not directly). The only
		 * place we can get to here again is the second place
		 * "pp->phead" is set, and THAT sets the right pipeline.
		 */
		pp->ltail->next = pp->rhead;
		if ((*ver)(pp->lhead, crit))
			LEAVE (1);
		else
			LEAVE (0);

	}

	/*
	 * Each time we search the list of filters, we examine
	 * them in the order given and stop searching when a filter
	 * that meets the needs is found. If the list is ordered with
	 * fast filters before slow filters, then fast filters will
	 * be chosen over otherwise-equal filters.
	 */

	/*
	 * See if there's a single filter that will work.
	 * Just in case we can't find one, mark those that
	 * will work as left- or right-filters,
	 * to save time later.
	 */
	{
		register _FILTER		*pf;

		for (pf = filters; pf->name; pf++) {

			if (USED(pf))
				continue;

			if (searchlist_t(inputp, pf->input_types))
				MARK (pf, FL_LEFT);
			if (searchlist_t(outputp, pf->output_types))
				MARK (pf, FL_RIGHT);

			if (CHECK(pf, FL_LEFT) && CHECK(pf, FL_RIGHT)) {
				prev_lhead = pp->lhead;
				prev_ltail = pp->ltail;

				if (!pp->lhead)
					pp->lhead = pf;
				else
					pp->ltail->next = pf;
				(pp->ltail = pf)->next = pp->rhead;

				if ((*ver)(pp->lhead, crit))
					LEAVE (1);

				if ((pp->ltail = prev_ltail))
					pp->ltail->next = 0;
				pp->lhead = prev_lhead;

			}

		}
	}

	/*
	 * Try all DISJOINT pairs of left- and right-filters; recursively
	 * call this function to find a filter that will connect
	 * them (it might be a ``null'' filter).
	 */
	{
		register _FILTER	*pfl,
					*pfr;

		register TYPE		*llist,
					*rlist;

		for (pfl = filters; pfl->name; pfl++) {

			if (!CHECK(pfl, FL_LEFT))
				continue;

			for (pfr = filters; pfr->name; pfr++) {

				if (pfr == pfl || !CHECK(pfr, FL_RIGHT))
					continue;

				prev_lhead = pp->lhead;
				prev_ltail = pp->ltail;

				if (!pp->lhead)
					pp->lhead = pfl;
				else
					pp->ltail->next = pfl;
				(pp->ltail = pfl)->next = 0;

				pfr->next = pp->rhead;
				pp->rhead = pfr;

				for (
					llist = pfl->output_types;
					llist->name;
					llist++
				)
					for (
						rlist = pfr->input_types;
						rlist->name;
						rlist++
					)
						if (_instantiate(
							pp,
/*
							inputp,
							outputp,
*/
							llist,
							rlist,
							ver,
							crit
						))
							LEAVE (1);

				pp->rhead = pfr->next;
				if ((pp->ltail = prev_ltail))
					pp->ltail->next = 0;
				pp->lhead = prev_lhead;

			}

		}
	}

	LEAVE (0);
}

/**
 ** check_pipeline() - CHECK THAT PIPELINE HANDLES MODES, PAGE-LIST
 **/

static int		check_pipeline (pipeline, parms)
	_FILTER			*pipeline;
	PARM			*parms;
{
	register PARM		*pm;

	register _FILTER	*pf;

	register TEMPLATE	*pt;

	register int		fail;

	for (fail = 0, pm = parms; !fail && pm->keyword; pm++) {

		if ( !(pm->flags & X_MUST) )
			continue;

		for (pf = pipeline; pf; pf = pf->next) {

			if (!(pt = pf->templates))
				continue;

			for (; pt->keyword; pt++)
				if (
					STREQU(pt->keyword, pm->keyword)
				     && pt->result
#if	defined(PATT_RE)
				     && match(pt, pm, (char **)0)
#else
				     && match(pt, pm)
#endif
				)
					goto Okay;

		}
		fail = 1;
		continue;

Okay:		;

	}

	return (fail? 0 : 1);
}
	
/**
 ** build_filter() - CONSTRUCT PIPELINE FROM LINKED LIST OF FILTERS
 **/

static unsigned		build_simple_cmd();

static char		*build_pipe (pipeline, parms, fp)
	_FILTER			*pipeline;
	PARM			*parms;
	unsigned short		*fp;
{
	register _FILTER	*pf;

	register unsigned	nchars,
				n;

	char			*p,	/* NOT register */
				*ret;

	/*
	 * This is a two-pass routine. In the first pass we add
	 * up how much space is needed for the pipeline, in the second
	 * pass we allocate the space and construct the pipeline.
	 */

	for (nchars = 0, pf = pipeline; pf; pf = pf->next)
		if ((n = build_simple_cmd((char **)0, pf, parms, fp)) > 0)
			nchars += n + 1;   /* +1 for '|' or ending null */

	if (!(ret = p = malloc(nchars))) {
		errno = ENOMEM;
		return (0);
	}

	for (pf = pipeline; pf; pf = pf->next, *p++ = (pf? '|' : 0))
		(void)build_simple_cmd (&p, pf, parms, fp);

	return (ret);
}
/**
 ** build_simple_cmd()
 **/

static unsigned		build_simple_cmd (pp, pf, parms, flagsp)
	char			**pp;
	_FILTER			*pf;
	PARM			*parms;
	unsigned short		*flagsp;
{
	register char		*p,
				*q;

	register unsigned	ncount	= 0;

	register TEMPLATE	*pt;

	register PARM		*pm;

	if (pp)
		p = *pp;

#define	CCPY(SRC)	if ((ncount++, pp)) \
				*p++ = SRC

#define	SCPY(SRC)	if (pp) { \
				register char	*r; \
				for (r = (SRC); *r; ncount++) \
					*p++ = *r++; \
			} else \
				ncount += strlen(SRC)

	if (pf->command)
		SCPY (pf->command);

	if (!pf->templates)
		goto Done;

	for (pm = parms; pm->keyword; pm++) {

#if	defined(PATT_RE)
		register unsigned	len	= strlen(*(pm->pvalue))+1;

		int			mn;

		char			*m[10],
					last_resort[256];
#endif

		if ((pm->flags & X_USED) || !*(pm->pvalue))
			continue;

#if	defined(PATT_RE)
		for (mn = 0; mn < 10; mn++)
			if (!(m[mn] = malloc(len)))
				m[mn] = last_resort;
#endif

		for (pt = pf->templates; pt->keyword; pt++) {

			if (
				STREQU(pt->keyword, pm->keyword)
			     && pt->result
#if	defined(PATT_RE)
			     && match(pt, pm, m)
#else
			     && match(pt, pm)
#endif
			) {
				CCPY (' ');
				for (q = pt->result; *q; q++)
					switch (*q) {
					case '*':
						SCPY (*(pm->pvalue));
						break;
					case '\\':
						switch (*++q) {
#if	defined(PATT_RE)
						case '0':
						case '1':
						case '2':
						case '3':
						case '4':
						case '5':
						case '6':
						case '7':
						case '8':
						case '9':
							SCPY (m[*q-'0']);
#endif
						default:
							CCPY (*q);
							break;
						}
						break;
					default:
						CCPY (*q);
						break;
					}

				/*
				 * Difficulty here due to the two pass
				 * nature of this code. The first pass
				 * just counts the number of bytes; if
				 * we mark the once-only parms as being
				 * used, then we don't pick them up the
				 * second time through. We could get
				 * difficult and mark them temporarily,
				 * but that's hard. So on the first pass
				 * we don't mark the flags. The only
				 * problem is an estimate too high.
				 */
				if (pp && pm->flags & X_FIRST)
					pm->flags |= X_USED;

				*flagsp |= pm->flags;

			}
		}

#if	defined(PATT_RE)
		for (mn = 0; mn < 10; mn++)
			if (m[mn] != last_resort)
				free (m[mn]);
#endif

	}

Done:	if (pp)
		*pp = p;

	return (ncount);
}

/**
 ** match() - TEST MATCH OF TEMPLATE/PATTERN WITH PARAMETER
 **/

#if	defined(PATT_RE)
static int		match (pt, pm, m)
	register TEMPLATE	*pt;
	register PARM		*pm;
	register char		**m;
#else
static int		match (pt, pm)
	register TEMPLATE	*pt;
	register PARM		*pm;
#endif
{
#if	defined(PATT_RE)
	if (m)

		return (
			STREQU(pt->pattern, PATT_STAR)
		     || regex(pt->re, *(pm->pvalue), m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8], m[9])
		);

	else {

		register char		*bb;	/* bit-bucket */

		char			last_resort[256];

		int			ret;

		if (!(bb = malloc(strlen(*(pm->pvalue)) + 1)))
			bb = last_resort;

		ret = (
			STREQU(pt->pattern, PATT_STAR)
		     || regex(pt->re, *(pm->pvalue), bb, bb, bb, bb, bb, bb, bb, bb, bb, bb)
		);
		if (bb != last_resort)
			free (bb);
		return (ret);
	}
#else
	return (
		STREQU(pt->pattern, PATT_STAR)
	     || STREQU(pt->pattern, *(pm->pvalue))
	);
#endif
}
